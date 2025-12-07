#include "sm2_scheduler.h"
#include <QDate>
#include <QDebug>
#include <cmath>

namespace WordMaster {
namespace Application {

SM2Scheduler::SM2Scheduler(Domain::IReviewScheduleRepository& repo)
    : repo_(repo)
{
}

void SM2Scheduler::initializeSchedule(int wordId, const QString& bookId) {
    // 如果已存在，不重复初始化
    if (repo_.exists(wordId)) {
        return;
    }
    
    Domain::ReviewPlan plan;
    plan.wordId = wordId;
    plan.bookId = bookId;
    // 新学习的单词，当天就可以复习（立即复习模式）
    plan.nextReviewDate = QDate::currentDate();
    plan.reviewInterval = 1;
    plan.repetitionCount = 0;
    plan.easinessFactor = 2.5;  // 默认难度系数
    plan.masteryLevel = Domain::ReviewPlan::MasteryLevel::Learning;
    
    repo_.save(plan);
    
    qDebug() << "Initialized schedule for word" << wordId 
             << "next review:" << plan.nextReviewDate.toString();
}

void SM2Scheduler::updateSchedule(int wordId, Domain::ReviewQuality quality) {
    Domain::ReviewPlan plan = repo_.get(wordId);
    
    // 如果不存在，先初始化（不应该发生）
    if (plan.wordId == 0) {
        qWarning() << "Review plan not found for word:" << wordId;
        return;
    }
    
    // 应用 SM-2 算法
    SM2Result result = calculateSM2(
        plan.reviewInterval,
        plan.easinessFactor,
        plan.repetitionCount,
        quality
    );
    
    // 更新计划
    plan.lastReviewDate = QDate::currentDate();
    plan.reviewInterval = result.interval;
    plan.easinessFactor = result.easinessFactor;
    plan.repetitionCount = result.repetitionCount;
    plan.nextReviewDate = QDate::currentDate().addDays(result.interval);
    
    // 更新掌握度
    updateMasteryLevel(plan);
    
    // 保存
    repo_.save(plan);
    
    qDebug() << "Updated schedule for word" << wordId 
             << ": interval=" << plan.reviewInterval
             << ", EF=" << plan.easinessFactor
             << ", reps=" << plan.repetitionCount
             << ", next=" << plan.nextReviewDate.toString();
}

QList<int> SM2Scheduler::getTodayReviewWords(const QString& bookId) {
    return repo_.getTodayReviewWords(bookId);
}

QList<int> SM2Scheduler::getUnlearnedWords(const QString& bookId, int limit) {
    return repo_.getUnlearnedWords(bookId, limit);
}

SM2Scheduler::SM2Result SM2Scheduler::calculateSM2(
    int currentInterval,
    double currentEF,
    int repetitionCount,
    Domain::ReviewQuality quality)
{
    SM2Result result;
    
    int q = static_cast<int>(quality);
    
    // 1. 计算新的 EF 值
    // EF' = EF + (0.1 - (5-q) * (0.08 + (5-q) * 0.02))
    double newEF = currentEF + (0.1 - (5 - q) * (0.08 + (5 - q) * 0.02));
    result.easinessFactor = qMax(1.3, newEF);  // EF 最小值为 1.3
    
    // 2. 计算新的间隔
    if (q < 3) {
        // 回答质量差（Again），重新开始
        result.interval = 1;
        result.repetitionCount = 0;
    } else {
        // 回答质量好（Hard/Good/Easy >= 3），增加间隔
        result.repetitionCount = repetitionCount + 1;
        
        if (result.repetitionCount == 1) {
            // 第一次复习：1天后
            result.interval = 1;
        } else if (result.repetitionCount == 2) {
            // 第二次复习：6天后
            result.interval = 6;
        } else {
            // 后续复习：I(n) = I(n-1) * EF
            result.interval = qRound(currentInterval * result.easinessFactor);
        }
    }
    
    return result;
}

void SM2Scheduler::updateMasteryLevel(Domain::ReviewPlan& plan) {
    // 掌握度判断标准：
    // - 已掌握：复习次数 >= 5 且间隔 >= 30天
    // - 学习中：复习次数 > 0
    // - 未学习：复习次数 = 0
    
    if (plan.repetitionCount >= 5 && plan.reviewInterval >= 30) {
        plan.masteryLevel = Domain::ReviewPlan::MasteryLevel::Mastered;
    } else if (plan.repetitionCount > 0) {
        plan.masteryLevel = Domain::ReviewPlan::MasteryLevel::Learning;
    } else {
        plan.masteryLevel = Domain::ReviewPlan::MasteryLevel::NotLearned;
    }
}

} // namespace Application
} // namespace WordMaster