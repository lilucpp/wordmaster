#include "sqlite_adapter.h"
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <QSqlRecord>
#include <QRegularExpression>


namespace WordMaster {
namespace Infrastructure {

SQLiteAdapter::SQLiteAdapter(const QString& dbPath)
    : dbPath_(dbPath)
    , connectionName_(QUuid::createUuid().toString())
{
}

SQLiteAdapter::~SQLiteAdapter() {
    close();
}

bool SQLiteAdapter::open() {
    if (isOpen()) {
        return true;
    }
    
    db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName_);
    db_.setDatabaseName(dbPath_);
    
    if (!db_.open()) {
        qWarning() << "Failed to open database:" << db_.lastError().text();
        return false;
    }
    
    // 启用外键约束
    execute("PRAGMA foreign_keys = ON");
    
    // 优化性能
    execute("PRAGMA synchronous = NORMAL");
    execute("PRAGMA journal_mode = WAL");
    
    qDebug() << "Database opened successfully:" << dbPath_;
    return true;
}

void SQLiteAdapter::close() {
    if (isOpen()) {
        db_.close();
        QSqlDatabase::removeDatabase(connectionName_);
        qDebug() << "Database closed:" << dbPath_;
    }
}

bool SQLiteAdapter::isOpen() const {
    return db_.isOpen();
}

bool SQLiteAdapter::execute(const QString& sql) {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    lastQuery_ = db_.exec(sql);
    
    if (lastQuery_.lastError().isValid()) {
        qWarning() << "SQL execution failed:" 
                   << lastQuery_.lastError().text()
                   << "\nSQL:" << sql;
        return false;
    }
    
    return true;
}

QSqlQuery SQLiteAdapter::query(const QString& sql) {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return QSqlQuery();
    }
    
    lastQuery_ = db_.exec(sql);
    
    if (lastQuery_.lastError().isValid()) {
        qWarning() << "SQL query failed:" 
                   << lastQuery_.lastError().text()
                   << "\nSQL:" << sql;
    }
    
    return lastQuery_;
}

QSqlQuery SQLiteAdapter::prepare(const QString& sql) {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return QSqlQuery();
    }
    
    QSqlQuery q(db_);
    
    if (!q.prepare(sql)) {
        qWarning() << "SQL prepare failed:" 
                   << q.lastError().text()
                   << "\nSQL:" << sql;
    }
    
    return q;
}

bool SQLiteAdapter::beginTransaction() {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    bool success = db_.transaction();
    if (!success) {
        qWarning() << "Failed to begin transaction:" << db_.lastError().text();
    }
    return success;
}

bool SQLiteAdapter::commit() {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    bool success = db_.commit();
    if (!success) {
        qWarning() << "Failed to commit transaction:" << db_.lastError().text();
    }
    return success;
}

bool SQLiteAdapter::rollback() {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    bool success = db_.rollback();
    if (!success) {
        qWarning() << "Failed to rollback transaction:" << db_.lastError().text();
    }
    return success;
}

int SQLiteAdapter::lastInsertId() const {
    if (lastQuery_.isActive()) {
        return lastQuery_.lastInsertId().toInt();
    }
    return -1;
}

int SQLiteAdapter::affectedRows() const {
    if (lastQuery_.isActive()) {
        return lastQuery_.numRowsAffected();
    }
    return -1;
}

QString SQLiteAdapter::lastError() const {
    if (lastQuery_.lastError().isValid()) {
        return lastQuery_.lastError().text();
    }
    return db_.lastError().text();
}

QStringList splitSqlStatements(const QString &sqlContent)
{
    QString cleaned = sqlContent;

    // 1. 去除 /* ... */ 块注释
    QRegularExpression blockComment("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption);
    cleaned.remove(blockComment);

    // 2. 去除 -- 注释
    QStringList lines = cleaned.split('\n');
    QStringList noCommentLines;

    for (QString line : lines) {
        int pos = line.indexOf("--");
        if (pos >= 0) {
            line = line.left(pos);
        }
        if (!line.trimmed().isEmpty())
            noCommentLines << line;
    }

    // 3. 重新组合，并按 ; 分割 SQL 语句
    QString joined = noCommentLines.join('\n');

    QStringList rawStatements = joined.split(';', Qt::SkipEmptyParts);

    // 4. 去空白
    QStringList statements;
    for (QString stmt : rawStatements) {
        QString trimmed = stmt.trimmed();
        if (!trimmed.isEmpty()) {
            statements << trimmed + ';';   // 补回 ;
        }
    }
    return statements;
}

bool SQLiteAdapter::initializeDatabase(const QString& migrationFile) {
    if (!isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    QFile file(migrationFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open migration file:" << migrationFile;
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString sql = in.readAll();
    file.close();
    
    // // 分割SQL语句（简单实现，假设以分号分隔）
    // QStringList statements = sql.split(';', Qt::SkipEmptyParts);
    
    QStringList statements = splitSqlStatements(sql);

    beginTransaction();
    
    for (const QString& statement : statements) {
        QString trimmed = statement.trimmed();

        if (trimmed.isEmpty()) {
            continue;
        }
        
        if (!execute(trimmed)) {
            qWarning() << "Migration failed at statement:" << trimmed;
            rollback();
            return false;
        }
    }
    
    if (!commit()) {
        return false;
    }
    
    qDebug() << "Database initialized successfully from:" << migrationFile;
    return true;
}

QSqlDatabase& SQLiteAdapter::getConnection() {
    return db_;
}

} // namespace Infrastructure
} // namespace WordMaster