#include "book_repository.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariant>
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

BookRepository::BookRepository(SQLiteAdapter& adapter)
    : adapter_(adapter)
{
}

bool BookRepository::save(const Domain::Book& book) {
    if (!book.isValid()) {
        qWarning() << "Invalid book object";
        return false;
    }
    
    QString sql = R"(
        INSERT OR REPLACE INTO books 
        (id, name, description, category, tags, url, word_count, 
         language, translate_language, is_active)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(book.id);
    query.addBindValue(book.name);
    query.addBindValue(book.description);
    query.addBindValue(book.category);
    query.addBindValue(serializeTags(book.tags));
    query.addBindValue(book.url);
    query.addBindValue(book.wordCount);
    query.addBindValue(book.language);
    query.addBindValue(book.translateLanguage);
    query.addBindValue(book.isActive ? 1 : 0);
    
    if (!query.exec()) {
        qWarning() << "Failed to save book:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Domain::Book BookRepository::getById(const QString& id) {
    QString sql = "SELECT * FROM books WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to query book:" << query.lastError().text();
        return Domain::Book();
    }
    
    if (query.next()) {
        return buildBookFromQuery(query);
    }
    
    return Domain::Book();
}

QList<Domain::Book> BookRepository::getAll() {
    QList<Domain::Book> books;
    
    QString sql = "SELECT * FROM books ORDER BY imported_at DESC";
    auto query = adapter_.query(sql);
    
    while (query.next()) {
        books.append(buildBookFromQuery(query));
    }
    
    return books;
}

bool BookRepository::remove(const QString& id) {
    QString sql = "DELETE FROM books WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete book:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool BookRepository::exists(const QString& id) {
    QString sql = "SELECT COUNT(*) as cnt FROM books WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt() > 0;
    }
    
    return false;
}

QList<Domain::Book> BookRepository::getByCategory(const QString& category) {
    QList<Domain::Book> books;
    
    QString sql = "SELECT * FROM books WHERE category = ? ORDER BY name";
    auto query = adapter_.prepare(sql);
    query.addBindValue(category);
    
    if (!query.exec()) {
        qWarning() << "Failed to query books by category:" << query.lastError().text();
        return books;
    }
    
    while (query.next()) {
        books.append(buildBookFromQuery(query));
    }
    
    return books;
}

Domain::Book BookRepository::getActiveBook() {
    QString sql = "SELECT * FROM books WHERE is_active = 1 LIMIT 1";
    auto query = adapter_.query(sql);
    
    if (query.next()) {
        return buildBookFromQuery(query);
    }
    
    return Domain::Book();
}

bool BookRepository::setActive(const QString& id, bool active) {
    // 如果设置为激活，先取消其他词库的激活状态
    if (active) {
        adapter_.execute("UPDATE books SET is_active = 0");
    }
    
    QString sql = "UPDATE books SET is_active = ? WHERE id = ?";
    auto query = adapter_.prepare(sql);
    query.addBindValue(active ? 1 : 0);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to set active status:" << query.lastError().text();
        return false;
    }
    
    return true;
}

int BookRepository::getTotalWordCount(const QString& bookId) {
    QString sql = "SELECT word_count FROM books WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("word_count").toInt();
    }
    
    return 0;
}

int BookRepository::getLearnedWordCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(DISTINCT word_id) as cnt
        FROM review_schedule
        WHERE book_id = ?
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

int BookRepository::getMasteredWordCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(*) as cnt
        FROM review_schedule
        WHERE book_id = ? AND mastery_level = 2
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

Domain::Book BookRepository::buildBookFromQuery(QSqlQuery& query) {
    Domain::Book book;
    
    book.id = query.value("id").toString();
    book.name = query.value("name").toString();
    book.description = query.value("description").toString();
    book.category = query.value("category").toString();
    book.tags = deserializeTags(query.value("tags").toString());
    book.url = query.value("url").toString();
    book.wordCount = query.value("word_count").toInt();
    book.language = query.value("language").toString();
    book.translateLanguage = query.value("translate_language").toString();
    book.importedAt = query.value("imported_at").toDateTime();
    book.isActive = query.value("is_active").toBool();
    
    return book;
}

QString BookRepository::serializeTags(const QStringList& tags) {
    QJsonArray jsonArray;
    for (const QString& tag : tags) {
        jsonArray.append(tag);
    }
    return QJsonDocument(jsonArray).toJson(QJsonDocument::Compact);
}

QStringList BookRepository::deserializeTags(const QString& json) {
    QStringList tags;
    
    if (json.isEmpty()) {
        return tags;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isArray()) {
        QJsonArray array = doc.array();
        for (const QJsonValue& value : array) {
            tags.append(value.toString());
        }
    }
    
    return tags;
}

} // namespace Infrastructure
} // namespace WordMaster