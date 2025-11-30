#ifndef WORDMASTER_INFRASTRUCTURE_STUDY_RECORD_REPOSITORY_H
#define WORDMASTER_INFRASTRUCTURE_STUDY_RECORD_REPOSITORY_H

#include "domain/repositories.h"
#include "infrastructure/sqlite_adapter.h"

namespace WordMaster {
namespace Infrastructure {

/**
 * @brief 学习记录仓储实现
 * 
 * 职责：
 * - 学习记录的持久化
 * - 学习记录查询和统计
 */
class StudyRecordRepository : public Domain::IStudyRecordRepository {
public:
    explicit StudyRecordRepository(SQLiteAdapter& adapter);
    ~StudyRecordRepository() override = default;
    
    // 基本CRUD
    bool save(const Domain::StudyRecord& record) override;
    Domain::StudyRecord getById(int id) override;
    QList<Domain::StudyRecord> getByWordId(int wordId) override;
    
    // 查询
    QList<Domain::StudyRecord> getByDateRange(const QDate& start, 
                                               const QDate& end) override;
    QList<Domain::StudyRecord> getTodayRecords() override;
    QList<Domain::StudyRecord> getByBookId(const QString& bookId) override;
    
    // 统计
    int getTodayLearnCount(const QString& bookId) override;
    int getTodayReviewCount(const QString& bookId) override;
    int getTotalStudyDuration(const QDate& date) override;

private:
    SQLiteAdapter& adapter_;
    
    // 辅助方法：从 QSqlQuery 构建 StudyRecord 对象
    Domain::StudyRecord buildRecordFromQuery(QSqlQuery& query);
};

} // namespace Infrastructure
} // namespace WordMaster

#endif // WORDMASTER_INFRASTRUCTURE_STUDY_RECORD_REPOSITORY_H