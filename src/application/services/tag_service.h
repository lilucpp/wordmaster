#ifndef WORDMASTER_APPLICATION_TAG_SERVICE_H
#define WORDMASTER_APPLICATION_TAG_SERVICE_H

#include "domain/repositories.h"
#include "domain/entities.h"

namespace WordMaster {
namespace Application {

class TagService {
public:
    explicit TagService(Domain::IWordTagRepository& tagRepo);
    
    bool addTag(int wordId, const QString& tagType);
    bool removeTag(int wordId, const QString& tagType);
    bool toggleTag(int wordId, const QString& tagType);
    bool hasTag(int wordId, const QString& tagType);
    
    QList<int> getWordsByTag(const QString& tagType);
    QStringList getWordTags(int wordId);
    
    int getDifficultCount();
    int getWrongCount();
    int getFavoriteCount();

private:
    Domain::IWordTagRepository& tagRepo_;
};

} // namespace Application
} // namespace WordMaster

#endif