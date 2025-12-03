#include "tag_service.h"

namespace WordMaster {
namespace Application {

TagService::TagService(Domain::IWordTagRepository& tagRepo)
    : tagRepo_(tagRepo)
{
}

bool TagService::addTag(int wordId, const QString& tagType) {
    return tagRepo_.add(wordId, tagType);
}

bool TagService::removeTag(int wordId, const QString& tagType) {
    return tagRepo_.remove(wordId, tagType);
}

bool TagService::toggleTag(int wordId, const QString& tagType) {
    if (tagRepo_.exists(wordId, tagType)) {
        return tagRepo_.remove(wordId, tagType);
    } else {
        return tagRepo_.add(wordId, tagType);
    }
}

bool TagService::hasTag(int wordId, const QString& tagType) {
    return tagRepo_.exists(wordId, tagType);
}

QList<int> TagService::getWordsByTag(const QString& tagType) {
    return tagRepo_.getWordsByTag(tagType);
}

QStringList TagService::getWordTags(int wordId) {
    return tagRepo_.getWordTags(wordId);
}

int TagService::getDifficultCount() {
    return tagRepo_.getTagCount(Domain::WordTag::TAG_DIFFICULT);
}

int TagService::getWrongCount() {
    return tagRepo_.getTagCount(Domain::WordTag::TAG_WRONG);
}

int TagService::getFavoriteCount() {
    return tagRepo_.getTagCount(Domain::WordTag::TAG_FAVORITE);
}

} // namespace Application
} // namespace WordMaster