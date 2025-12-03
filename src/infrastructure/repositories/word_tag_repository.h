#ifndef WORDMASTER_INFRASTRUCTURE_WORD_TAG_REPOSITORY_H
#define WORDMASTER_INFRASTRUCTURE_WORD_TAG_REPOSITORY_H

#include "domain/repositories.h"
#include "infrastructure/sqlite_adapter.h"

namespace WordMaster {
namespace Infrastructure {

class WordTagRepository : public Domain::IWordTagRepository {
public:
    explicit WordTagRepository(SQLiteAdapter& adapter);
    ~WordTagRepository() override = default;
    
    bool add(int wordId, const QString& tagType) override;
    bool remove(int wordId, const QString& tagType) override;
    bool exists(int wordId, const QString& tagType) override;
    
    QList<int> getWordsByTag(const QString& tagType) override;
    QStringList getWordTags(int wordId) override;
    
    bool addBatch(const QList<int>& wordIds, const QString& tagType) override;
    bool removeBatch(const QList<int>& wordIds, const QString& tagType) override;
    
    int getTagCount(const QString& tagType) override;

private:
    SQLiteAdapter& adapter_;
};

} // namespace Infrastructure
} // namespace WordMaster

#endif