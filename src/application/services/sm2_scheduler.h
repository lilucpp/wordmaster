#ifndef WORDMASTER_APPLICATION_SM2_SCHEDULER_H
#define WORDMASTER_APPLICATION_SM2_SCHEDULER_H

#include "domain/repositories.h"
#include "domain/entities.h"

namespace WordMaster {
namespace Application {

/**
 * @brief SM-2 复习调度器
 * 
 * 实现 SuperMemo SM-2 算法
 * 
 * 算法原理：
 * - 根据回答质量调整复习间隔
 * - 间隔呈指数增长
 * - 难度系数动态调整
 * 
 * 质量评分（0-5）：
 * - 0 - 完全不记得（Again）
 * - 3 - 困难（Hard）
 * - 4 - 良好（Good）
 * - 5 - 简单（Easy）
 */
class SM2Scheduler {
public:
    /**
     * @brief SM-2 计算结果
     */
    struct SM2Result {
        int interval;              // 下次复习间隔（天）
        double easinessFactor;     // 新的难度系数
        int repetitionCount;       // 新的复习次数
        
        SM2Result() : interval(1), easinessFactor(2.5), repetitionCount(0) {}
    };
    
    explicit SM2Scheduler(Domain::IReviewScheduleRepository& repo);
    
    /**
     * @brief 初始化新单词的复习计划
     * @param wordId 单词ID
     * @param bookId 词库ID
     */
    void initializeSchedule(int wordId, const QString& bookId);
    
    /**
     * @brief 更新复习计划
     * @param wordId 单词ID
     * @param quality 复习质量
     */
    void updateSchedule(int wordId, Domain::ReviewQuality quality);
    
    /**
     * @brief 获取今日待复习单词
     * @param bookId 词库ID
     * @return 单词ID列表
     */
    QList<int> getTodayReviewWords(const QString& bookId);
    
    /**
     * @brief 获取未学习单词
     * @param bookId 词库ID
     * @param limit 数量限制
     * @return 单词ID列表
     */
    QList<int> getUnlearnedWords(const QString& bookId, int limit = -1);
    
    /**
     * @brief SM-2 算法核心计算
     * 
     * 算法公式：
     * 1. EF' = EF + (0.1 - (5-q) * (0.08 + (5-q) * 0.02))
     * 2. I(1) = 1, I(2) = 6, I(n) = I(n-1) * EF
     * 
     * @param currentInterval 当前间隔
     * @param currentEF 当前难度系数
     * @param repetitionCount 当前复习次数
     * @param quality 复习质量
     * @return SM-2 计算结果
     */
    static SM2Result calculateSM2(int currentInterval,
                                   double currentEF,
                                   int repetitionCount,
                                   Domain::ReviewQuality quality);

private:
    Domain::IReviewScheduleRepository& repo_;
    
    // 更新掌握度
    void updateMasteryLevel(Domain::ReviewPlan& plan);
};

} // namespace Application
} // namespace WordMaster

#endif // WORDMASTER_APPLICATION_SM2_SCHEDULER_H