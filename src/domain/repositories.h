#ifndef WORDMASTER_DOMAIN_REPOSITORIES_H
#define WORDMASTER_DOMAIN_REPOSITORIES_H

#include "entities.h"
#include <QList>
#include <QDate>
#include <memory>

namespace WordMaster {
namespace Domain {

// ============================================
// IBookRepository - 词库仓储接口
// ============================================
class IBookRepository {
public:
    virtual ~IBookRepository() = default;
    
    // 基本CRUD
    virtual bool save(const Book& book) = 0;
    virtual Book getById(const QString& id) = 0;
    virtual QList<Book> getAll() = 0;
    virtual bool remove(const QString& id) = 0;
    virtual bool exists(const QString& id) = 0;
    
    // 查询
    virtual QList<Book> getByCategory(const QString& category) = 0;
    virtual Book getActiveBook() = 0;
    virtual bool setActive(const QString& id, bool active) = 0;
    
    // 统计
    virtual int getTotalWordCount(const QString& bookId) = 0;
    virtual int getLearnedWordCount(const QString& bookId) = 0;
    virtual int getMasteredWordCount(const QString& bookId) = 0;
};

// ============================================
// IWordRepository - 单词仓储接口
// ============================================
class IWordRepository {
public:
    virtual ~IWordRepository() = default;
    
    // 基本CRUD
    virtual bool save(const Word& word) = 0;
    virtual Word getById(int id) = 0;
    virtual QList<Word> getByIds(const QList<int>& ids) = 0;
    virtual bool remove(int id) = 0;
    virtual bool exists(int id) = 0;
    
    // 查询
    virtual QList<Word> getByBookId(const QString& bookId, int limit = -1, int offset = 0) = 0;
    virtual QList<Word> searchByWord(const QString& word) = 0;
    virtual Word getByBookAndWord(const QString& bookId, const QString& word) = 0;
    
    // 批量操作
    virtual bool saveBatch(const QList<Word>& words) = 0;
    virtual bool removeByBookId(const QString& bookId) = 0;
    
    // 事务支持
    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

// ============================================
// IStudyRecordRepository - 学习记录仓储接口
// ============================================
class IStudyRecordRepository {
public:
    virtual ~IStudyRecordRepository() = default;
    
    // 基本CRUD
    virtual bool save(const StudyRecord& record) = 0;
    virtual StudyRecord getById(int id) = 0;
    virtual QList<StudyRecord> getByWordId(int wordId) = 0;
    
    // 查询
    virtual QList<StudyRecord> getByDateRange(const QDate& start, const QDate& end) = 0;
    virtual QList<StudyRecord> getTodayRecords() = 0;
    virtual QList<StudyRecord> getByBookId(const QString& bookId) = 0;
    
    // 统计
    virtual int getTodayLearnCount(const QString& bookId) = 0;
    virtual int getTodayReviewCount(const QString& bookId) = 0;
    virtual int getTotalStudyDuration(const QDate& date) = 0;
};

// ============================================
// IReviewScheduleRepository - 复习计划仓储接口
// ============================================
class IReviewScheduleRepository {
public:
    virtual ~IReviewScheduleRepository() = default;
    
    // 基本CRUD
    virtual bool save(const ReviewPlan& plan) = 0;
    virtual ReviewPlan get(int wordId) = 0;
    virtual bool remove(int wordId) = 0;
    virtual bool exists(int wordId) = 0;
    
    // 查询
    virtual QList<int> getTodayReviewWords(const QString& bookId) = 0;
    virtual QList<int> getOverdueWords(const QString& bookId) = 0;
    virtual QList<int> getUnlearnedWords(const QString& bookId, int limit = -1) = 0;
    
    // 统计
    virtual int getLearnedCount(const QString& bookId) = 0;
    virtual int getMasteredCount(const QString& bookId) = 0;
    virtual int getTodayReviewCount(const QString& bookId) = 0;
};

// ============================================
// IWordTagRepository - 单词标签仓储接口
// ============================================
class IWordTagRepository {
public:
    virtual ~IWordTagRepository() = default;
    
    // 基本CRUD
    virtual bool add(int wordId, const QString& tagType) = 0;
    virtual bool remove(int wordId, const QString& tagType) = 0;
    virtual bool exists(int wordId, const QString& tagType) = 0;
    
    // 查询
    virtual QList<int> getWordsByTag(const QString& tagType) = 0;
    virtual QStringList getWordTags(int wordId) = 0;
    
    // 批量操作
    virtual bool addBatch(const QList<int>& wordIds, const QString& tagType) = 0;
    virtual bool removeBatch(const QList<int>& wordIds, const QString& tagType) = 0;
    
    // 统计
    virtual int getTagCount(const QString& tagType) = 0;
};

// ============================================
// IUserPreferenceRepository - 用户设置仓储接口
// ============================================
class IUserPreferenceRepository {
public:
    virtual ~IUserPreferenceRepository() = default;
    
    // 基本操作
    virtual bool save(const UserPreference& pref) = 0;
    virtual QString get(const QString& key, const QString& defaultValue = QString()) = 0;
    virtual bool exists(const QString& key) = 0;
    virtual bool remove(const QString& key) = 0;
    
    // 批量操作
    virtual QMap<QString, QString> getAll() = 0;
};

} // namespace Domain
} // namespace WordMaster

#endif // WORDMASTER_DOMAIN_REPOSITORIES_H