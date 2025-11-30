#ifndef WORDMASTER_INFRASTRUCTURE_REVIEW_SCHEDULE_REPOSITORY_H
#define WORDMASTER_INFRASTRUCTURE_REVIEW_SCHEDULE_REPOSITORY_H

#include "domain/repositories.h"
#include "infrastructure/sqlite_adapter.h"

namespace WordMaster {
namespace Infrastructure {

/**
 * @brief 复习计划仓储实现
 * 
 * 职责：
 * - 复习计划的持久化
 * - 复习单词查询
 * - 掌握度统计
 */
class ReviewScheduleRepository : public Domain::IReviewScheduleRepository {
public:
    explicit ReviewScheduleRepository(SQLiteAdapter& adapter);
    ~ReviewScheduleRepository() override = default;
    
    // 基本CRUD
    bool save(const Domain::ReviewPlan& plan) override;
    Domain::ReviewPlan get(int wordId) override;
    bool remove(int wordId) override;
    bool exists(int wordId) override;
    
    // 查询
    QList<int> getTodayReviewWords(const QString& bookId) override;
    QList<int> getOverdueWords(const QString& bookId) override;
    QList<int> getUnlearnedWords(const QString& bookId, int limit = -1) override;
    
    // 统计
    int getLearnedCount(const QString& bookId) override;
    int getMasteredCount(const QString& bookId) override;
    int getTodayReviewCount(const QString& bookId) override;

private:
    SQLiteAdapter& adapter_;
    
    // 辅助方法：从 QSqlQuery 构建 ReviewPlan 对象
    Domain::ReviewPlan buildPlanFromQuery(QSqlQuery& query);
};

} // namespace Infrastructure
} // namespace WordMaster

#endif // WORDMASTER_INFRASTRUCTURE_REVIEW_SCHEDULE_REPOSITORY_H