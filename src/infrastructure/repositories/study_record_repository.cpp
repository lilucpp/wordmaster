#include "study_record_repository.h"
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

StudyRecordRepository::StudyRecordRepository(SQLiteAdapter& adapter)
    : adapter_(adapter)
{
}

bool StudyRecordRepository::save(const Domain::StudyRecord& record) {
    QString sql = R"(
        INSERT INTO study_records 
        (word_id, book_id, study_type, result, study_duration)
        VALUES (?, ?, ?, ?, ?)
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(record.wordId);
    query.addBindValue(record.bookId);
    query.addBindValue(Domain::StudyRecord::typeToString(record.studyType));
    query.addBindValue(Domain::StudyRecord::resultToString(record.result));
    query.addBindValue(record.studyDuration);
    
    if (!query.exec()) {
        qWarning() << "Failed to save study record:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Domain::StudyRecord StudyRecordRepository::getById(int id) {
    QString sql = "SELECT * FROM study_records WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to query study record:" << query.lastError().text();
        return Domain::StudyRecord();
    }
    
    if (query.next()) {
        return buildRecordFromQuery(query);
    }
    
    return Domain::StudyRecord();
}

QList<Domain::StudyRecord> StudyRecordRepository::getByWordId(int wordId) {
    QList<Domain::StudyRecord> records;
    
    QString sql = R"(
        SELECT * FROM study_records 
        WHERE word_id = ? 
        ORDER BY studied_at DESC
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query records by word:" << query.lastError().text();
        return records;
    }
    
    while (query.next()) {
        records.append(buildRecordFromQuery(query));
    }
    
    return records;
}

QList<Domain::StudyRecord> StudyRecordRepository::getByDateRange(
    const QDate& start, 
    const QDate& end) 
{
    QList<Domain::StudyRecord> records;
    
    QString sql = R"(
        SELECT * FROM study_records 
        WHERE DATE(studied_at) BETWEEN ? AND ?
        ORDER BY studied_at DESC
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(start.toString(Qt::ISODate));
    query.addBindValue(end.toString(Qt::ISODate));
    
    if (!query.exec()) {
        qWarning() << "Failed to query records by date range:" << query.lastError().text();
        return records;
    }
    
    while (query.next()) {
        records.append(buildRecordFromQuery(query));
    }
    
    return records;
}

QList<Domain::StudyRecord> StudyRecordRepository::getTodayRecords() {
    QList<Domain::StudyRecord> records;
    
    QString sql = R"(
        SELECT * FROM study_records 
        WHERE DATE(studied_at) = DATE('now')
        ORDER BY studied_at DESC
    )";
    
    auto query = adapter_.query(sql);
    
    while (query.next()) {
        records.append(buildRecordFromQuery(query));
    }
    
    return records;
}

QList<Domain::StudyRecord> StudyRecordRepository::getByBookId(
    const QString& bookId) 
{
    QList<Domain::StudyRecord> records;
    
    QString sql = R"(
        SELECT * FROM study_records 
        WHERE book_id = ?
        ORDER BY studied_at DESC
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query records by book:" << query.lastError().text();
        return records;
    }
    
    while (query.next()) {
        records.append(buildRecordFromQuery(query));
    }
    
    return records;
}

int StudyRecordRepository::getTodayLearnCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(*) as cnt FROM study_records
        WHERE book_id = ?
          AND study_type = 'learn'
          AND DATE(studied_at) = DATE('now')
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

int StudyRecordRepository::getTodayReviewCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(*) as cnt FROM study_records
        WHERE book_id = ?
          AND study_type = 'review'
          AND DATE(studied_at) = DATE('now')
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

int StudyRecordRepository::getTotalStudyDuration(const QDate& date) {
    QString sql = R"(
        SELECT SUM(study_duration) as total FROM study_records
        WHERE DATE(studied_at) = ?
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(date.toString(Qt::ISODate));
    
    if (query.exec() && query.next()) {
        return query.value("total").toInt();
    }
    
    return 0;
}

Domain::StudyRecord StudyRecordRepository::buildRecordFromQuery(QSqlQuery& query) {
    Domain::StudyRecord record;
    
    record.id = query.value("id").toInt();
    record.wordId = query.value("word_id").toInt();
    record.bookId = query.value("book_id").toString();
    record.studyType = Domain::StudyRecord::stringToType(
        query.value("study_type").toString()
    );
    record.result = Domain::StudyRecord::stringToResult(
        query.value("result").toString()
    );
    record.studyDuration = query.value("study_duration").toInt();
    record.studiedAt = query.value("studied_at").toDateTime();
    record.studiedAt.setTimeSpec(Qt::UTC); // 明确告诉 Qt 这是 UTC
    record.studiedAt = record.studiedAt.toLocalTime();
    
    return record;
}

} // namespace Infrastructure
} // namespace WordMaster