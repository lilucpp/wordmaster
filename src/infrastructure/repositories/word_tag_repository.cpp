#include "word_tag_repository.h"
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

WordTagRepository::WordTagRepository(SQLiteAdapter& adapter)
    : adapter_(adapter)
{
}

bool WordTagRepository::add(int wordId, const QString& tagType) {
    QString sql = "INSERT OR IGNORE INTO word_tags (word_id, tag_type) VALUES (?, ?)";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    query.addBindValue(tagType);
    
    return query.exec();
}

bool WordTagRepository::remove(int wordId, const QString& tagType) {
    QString sql = "DELETE FROM word_tags WHERE word_id = ? AND tag_type = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    query.addBindValue(tagType);
    
    return query.exec();
}

bool WordTagRepository::exists(int wordId, const QString& tagType) {
    QString sql = "SELECT COUNT(*) as cnt FROM word_tags WHERE word_id = ? AND tag_type = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    query.addBindValue(tagType);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt() > 0;
    }
    return false;
}

QList<int> WordTagRepository::getWordsByTag(const QString& tagType) {
    QList<int> wordIds;
    
    QString sql = "SELECT word_id FROM word_tags WHERE tag_type = ? ORDER BY tagged_at DESC";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(tagType);
    
    if (query.exec()) {
        while (query.next()) {
            wordIds.append(query.value("word_id").toInt());
        }
    }
    
    return wordIds;
}

QStringList WordTagRepository::getWordTags(int wordId) {
    QStringList tags;
    
    QString sql = "SELECT tag_type FROM word_tags WHERE word_id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    
    if (query.exec()) {
        while (query.next()) {
            tags.append(query.value("tag_type").toString());
        }
    }
    
    return tags;
}

bool WordTagRepository::addBatch(const QList<int>& wordIds, const QString& tagType) {
    if (wordIds.isEmpty()) return true;
    
    adapter_.beginTransaction();
    
    for (int wordId : wordIds) {
        if (!add(wordId, tagType)) {
            adapter_.rollback();
            return false;
        }
    }
    
    return adapter_.commit();
}

bool WordTagRepository::removeBatch(const QList<int>& wordIds, const QString& tagType) {
    if (wordIds.isEmpty()) return true;
    
    adapter_.beginTransaction();
    
    for (int wordId : wordIds) {
        remove(wordId, tagType);
    }
    
    return adapter_.commit();
}

int WordTagRepository::getTagCount(const QString& tagType) {
    QString sql = "SELECT COUNT(*) as cnt FROM word_tags WHERE tag_type = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(tagType);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

} // namespace Infrastructure
} // namespace WordMaster