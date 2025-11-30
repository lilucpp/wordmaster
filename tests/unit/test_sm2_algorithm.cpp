#include <gtest/gtest.h>
#include <QDebug>
#include "application/services/sm2_scheduler.h"

using namespace WordMaster::Application;
using namespace WordMaster::Domain;

/**
 * @brief SM-2 算法单元测试
 * 
 * 测试 SuperMemo SM-2 算法的正确性
 */
class SM2AlgorithmTest : public ::testing::Test {
protected:
    // 测试 SM-2 计算
    SM2Scheduler::SM2Result calculate(int interval, double ef, int reps, 
                                      ReviewQuality quality) {
        return SM2Scheduler::calculateSM2(interval, ef, reps, quality);
    }
};

// ============================================
// 测试：第一次复习 - 质量 Good (3)
// ============================================
TEST_F(SM2AlgorithmTest, FirstReview_QualityGood_ReturnsInterval1) {
    // 初始状态：interval=0, EF=2.5, reps=0
    auto result = calculate(0, 2.5, 0, ReviewQuality::Good);
    
    // 第一次复习，间隔应该是 1 天
    EXPECT_EQ(result.interval, 1);
    EXPECT_EQ(result.repetitionCount, 1);
    EXPECT_NEAR(result.easinessFactor, 2.5, 0.2);
}

// ============================================
// 测试：第二次复习 - 质量 Good (3)
// ============================================
TEST_F(SM2AlgorithmTest, SecondReview_QualityGood_ReturnsInterval6) {
    // 第一次复习后状态：interval=1, EF=2.5, reps=1
    auto result = calculate(1, 2.5, 1, ReviewQuality::Good);
    
    // 第二次复习，间隔应该是 6 天
    EXPECT_EQ(result.interval, 6);
    EXPECT_EQ(result.repetitionCount, 2);
}

// ============================================
// 测试：第三次及以后 - 质量 Good (3)
// ============================================
TEST_F(SM2AlgorithmTest, ThirdReview_QualityGood_UsesEFMultiplier) {
    // 第二次复习后状态：interval=6, EF=2.5, reps=2
    auto result = calculate(6, 2.5, 2, ReviewQuality::Good);

    // I(3) = I(2) * EF = 6 * 2.5 = 15
    EXPECT_EQ(result.interval, 15);
    EXPECT_EQ(result.repetitionCount, 3);
}

// ============================================
// 测试：质量 Easy (5) - EF 增加
// ============================================
TEST_F(SM2AlgorithmTest, QualityEasy_IncreasesEF) {
    // interval=6, EF=2.5, reps=2, quality=Easy(5)
    auto result = calculate(6, 2.5, 2, ReviewQuality::Easy);
    
    // EF 应该增加
    EXPECT_GT(result.easinessFactor, 2.5);
    EXPECT_EQ(result.repetitionCount, 3);
}

// ============================================
// 测试：质量 Hard (2) - EF 降低
// ============================================
TEST_F(SM2AlgorithmTest, QualityHard_DecreasesEF) {
    // interval=6, EF=2.5, reps=2, quality=Hard(2)
    auto result = calculate(6, 2.5, 2, ReviewQuality::Hard);
    
    // EF 应该降低
    EXPECT_LT(result.easinessFactor, 2.5);
}

// ============================================
// 测试：质量 Again (0) - 重置间隔
// ============================================
TEST_F(SM2AlgorithmTest, QualityAgain_ResetsInterval) {
    // 已经学习多次：interval=15, EF=2.5, reps=3
    auto result = calculate(15, 2.5, 3, ReviewQuality::Again);
    
    // 完全不记得，重新开始
    EXPECT_EQ(result.interval, 1);
    EXPECT_EQ(result.repetitionCount, 0);
}

// ============================================
// 测试：EF 最小值限制
// ============================================
TEST_F(SM2AlgorithmTest, EF_HasMinimumValue) {
    // 多次回答 Again，EF 应该降低但不低于 1.3
    double ef = 2.5;
    
    for (int i = 0; i < 10; ++i) {
        auto result = calculate(1, ef, 1, ReviewQuality::Again);
        ef = result.easinessFactor;
    }
    
    EXPECT_GE(ef, 1.3);
}

// ============================================
// 测试：间隔指数增长
// ============================================
TEST_F(SM2AlgorithmTest, Interval_GrowsExponentially) {
    int interval = 6;
    double ef = 2.5;
    int reps = 2;
    
    QList<int> intervals;
    intervals.append(interval);
    
    // 模拟多次成功复习
    for (int i = 0; i < 5; ++i) {
        auto result = calculate(interval, ef, reps, ReviewQuality::Good);
        interval = result.interval;
        ef = result.easinessFactor;
        reps = result.repetitionCount;
        intervals.append(interval);
    }
    
    // 验证间隔递增
    for (int i = 1; i < intervals.size(); ++i) {
        EXPECT_GT(intervals[i], intervals[i-1]);
    }
    
    qDebug() << "Interval progression:" << intervals;
}

// ============================================
// 测试：不同质量的 EF 变化
// ============================================
TEST_F(SM2AlgorithmTest, EF_ChangesWithQuality) {
    double initialEF = 2.5;
    
    // Again (0): EF 大幅降低
    auto againResult = calculate(6, initialEF, 2, ReviewQuality::Again);
    EXPECT_LT(againResult.easinessFactor, initialEF);
    
    // Hard (2): EF 略微降低
    auto hardResult = calculate(6, initialEF, 2, ReviewQuality::Hard);
    EXPECT_LT(hardResult.easinessFactor, initialEF);
    EXPECT_GT(hardResult.easinessFactor, againResult.easinessFactor);
    
    // Good (3): EF 保持或略微增加
    auto goodResult = calculate(6, initialEF, 2, ReviewQuality::Good);
    EXPECT_NEAR(goodResult.easinessFactor, initialEF, 0.2);
    
    // Easy (5): EF 增加
    auto easyResult = calculate(6, initialEF, 2, ReviewQuality::Easy);
    EXPECT_GT(easyResult.easinessFactor, initialEF);
}

// ============================================
// 测试：真实学习场景模拟
// ============================================
TEST_F(SM2AlgorithmTest, RealWorldScenario_MasteringWord) {
    // 模拟一个单词从学习到掌握的过程
    int interval = 0;
    double ef = 2.5;
    int reps = 0;
    
    struct ReviewEvent {
        ReviewQuality quality;
        int expectedMinInterval;
    };
    
    QList<ReviewEvent> events = {
        {ReviewQuality::Good, 1},    // 第1次：1天
        {ReviewQuality::Good, 6},    // 第2次：6天
        {ReviewQuality::Good, 15},   // 第3次：12天
        {ReviewQuality::Hard, 10},   // 第4次：有点忘，间隔缩短
        {ReviewQuality::Good, 20},   // 第5次：恢复正常
        {ReviewQuality::Easy, 40}    // 第6次：很容易，间隔大幅增加
    };
    
    for (const ReviewEvent& event : events) {
        auto result = calculate(interval, ef, reps, event.quality);
        
        EXPECT_GE(result.interval, event.expectedMinInterval)
            << "Review " << (reps + 1) << " failed";
        
        interval = result.interval;
        ef = result.easinessFactor;
        reps = result.repetitionCount;
        
        qDebug() << "Review" << reps 
                 << ": interval=" << interval 
                 << ", EF=" << ef;
    }
    
    // 最终应该达到较长的间隔（表示已掌握）
    EXPECT_GE(interval, 30);
    EXPECT_GE(reps, 5);
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}