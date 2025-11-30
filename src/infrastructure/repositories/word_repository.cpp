#include "word_repository.h"
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

WordRepository::WordRepository(SQLiteAdapter& adapter)
    : adapter_(adapter)
{
}

bool WordRepository::save(const Domain::Word& word) {
    if (!word.isValid()) {
        qWarning() << "Invalid word object";
        return false;
    }
    
    QString sql = R"(
        INSERT OR REPLACE INTO words 
        (book_id, word_id, word, phonetic_uk, phonetic_us, 
         translations, sentences, phrases, synonyms, 
         related_words, etymology)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(word.bookId);
    query.addBindValue(word.wordId);
    query.addBindValue(word.word);
    query.addBindValue(word.phoneticUk);
    query.addBindValue(word.phoneticUs);
    query.addBindValue(word.translations);
    query.addBindValue(word.sentences);
    query.addBindValue(word.phrases);
    query.addBindValue(word.synonyms);
    query.addBindValue(word.relatedWords);
    query.addBindValue(word.etymology);
    
    if (!query.exec()) {
        qWarning() << "Failed to save word:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Domain::Word WordRepository::getById(int id) {
    QString sql = "SELECT * FROM words WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to query word:" << query.lastError().text();
        return Domain::Word();
    }
    
    if (query.next()) {
        return buildWordFromQuery(query);
    }
    
    return Domain::Word();
}

QList<Domain::Word> WordRepository::getByIds(const QList<int>& ids) {
    QList<Domain::Word> words;
    
    if (ids.isEmpty()) {
        return words;
    }
    
    // 构建 IN 子句
    QStringList placeholders;
    for (int i = 0; i < ids.size(); ++i) {
        placeholders.append("?");
    }
    
    QString sql = QString("SELECT * FROM words WHERE id IN (%1)")
                      .arg(placeholders.join(","));
    
    auto query = adapter_.prepare(sql);
    for (int id : ids) {
        query.addBindValue(id);
    }
    
    if (!query.exec()) {
        qWarning() << "Failed to query words by ids:" << query.lastError().text();
        return words;
    }
    
    while (query.next()) {
        words.append(buildWordFromQuery(query));
    }
    
    return words;
}

bool WordRepository::remove(int id) {
    QString sql = "DELETE FROM words WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete word:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool WordRepository::exists(int id) {
    QString sql = "SELECT COUNT(*) as cnt FROM words WHERE id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt() > 0;
    }
    
    return false;
}

QList<Domain::Word> WordRepository::getByBookId(const QString& bookId, 
                                                 int limit, 
                                                 int offset) {
    QList<Domain::Word> words;
    
    QString sql = "SELECT * FROM words WHERE book_id = ? ORDER BY word_id";
    
    if (limit > 0) {
        sql += QString(" LIMIT %1 OFFSET %2").arg(limit).arg(offset);
    }
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query words by book:" << query.lastError().text();
        return words;
    }
    
    while (query.next()) {
        words.append(buildWordFromQuery(query));
    }
    
    return words;
}

QList<Domain::Word> WordRepository::searchByWord(const QString& word) {
    QList<Domain::Word> words;
    
    QString sql = "SELECT * FROM words WHERE word LIKE ? ORDER BY word LIMIT 50";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(QString("%%1%").arg(word));
    
    if (!query.exec()) {
        qWarning() << "Failed to search words:" << query.lastError().text();
        return words;
    }
    
    while (query.next()) {
        words.append(buildWordFromQuery(query));
    }
    
    return words;
}

Domain::Word WordRepository::getByBookAndWord(const QString& bookId, 
                                               const QString& word) {
    QString sql = "SELECT * FROM words WHERE book_id = ? AND word = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    query.addBindValue(word);
    
    if (!query.exec()) {
        qWarning() << "Failed to query word:" << query.lastError().text();
        return Domain::Word();
    }
    
    if (query.next()) {
        return buildWordFromQuery(query);
    }
    
    return Domain::Word();
}

bool WordRepository::saveBatch(const QList<Domain::Word>& words) {
    if (words.isEmpty()) {
        return true;
    }
    
    if (!beginTransaction()) {
        return false;
    }
    
    for (const Domain::Word& word : words) {
        if (!save(word)) {
            rollback();
            return false;
        }
    }
    
    return commit();
}

bool WordRepository::removeByBookId(const QString& bookId) {
    QString sql = "DELETE FROM words WHERE book_id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete words by book:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Deleted" << query.numRowsAffected() << "words from book:" << bookId;
    return true;
}

bool WordRepository::beginTransaction() {
    return adapter_.beginTransaction();
}

bool WordRepository::commit() {
    return adapter_.commit();
}

bool WordRepository::rollback() {
    return adapter_.rollback();
}

Domain::Word WordRepository::buildWordFromQuery(QSqlQuery& query) {
    Domain::Word word;
    
    word.id = query.value("id").toInt();
    word.bookId = query.value("book_id").toString();
    word.wordId = query.value("word_id").toInt();
    word.word = query.value("word").toString();
    word.phoneticUk = query.value("phonetic_uk").toString();
    word.phoneticUs = query.value("phonetic_us").toString();
    word.translations = query.value("translations").toString();
    word.sentences = query.value("sentences").toString();
    word.phrases = query.value("phrases").toString();
    word.synonyms = query.value("synonyms").toString();
    word.relatedWords = query.value("related_words").toString();
    word.etymology = query.value("etymology").toString();
    word.createdAt = query.value("created_at").toDateTime();
    
    return word;
}

} // namespace Infrastructure
} // namespace WordMaster