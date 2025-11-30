#include <gtest/gtest.h>
#include "application/services/study_service.h"
#include "application/services/sm2_scheduler.h"
#include "infrastructure/repositories/word_repository.h"
#include "infrastructure/repositories/study_record_repository.h"
#include "infrastructure/repositories/review_schedule_repository.h"
#include "infrastructure/repositories/book_repository.h"
#include "tests/test_helpers.h"

using namespace WordMaster::Application;
using namespace WordMaster::Domain;
using namespace WordMaster::Infrastructure;
using namespace WordMaster::Testing;

/**
 * @brief 学习流程集成测试
 * 
 * 测试完整的学习流程：
 * 1. 开始学习会话
 * 2. 学习单词
 * 3. 记录结果
 * 4. 更新复习计划
 * 5. 会话总结
 */
class StudyFlowIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据库
        adapter = TestDatabaseHelper::createTestDatabase();
        ASSERT_TRUE(adapter->isOpen());
        ASSERT_TRUE(TestDatabaseHelper::initializeTestSchema(*adapter));
        
        // 创建仓储
        bookRepo = std::make_unique<BookRepository>(*adapter);
        wordRepo = std::make_unique<WordRepository>(*adapter);
        recordRepo = std::make_unique<StudyRecordRepository>(*adapter);
        scheduleRepo = std::make_unique<ReviewScheduleRepository>(*adapter);
        
        // 创建服务
        scheduler = std::make_unique<SM2Scheduler>(*scheduleRepo);
        service = std::make_unique<StudyService>(*wordRepo, *recordRepo, *scheduler);
        
        // 准备测试数据
        setupTestData();
    }
    
    void TearDown() override {
        service.reset();
        scheduler.reset();
        scheduleRepo.reset();
        recordRepo.reset();
        wordRepo.reset();
        bookRepo.reset();
        adapter.reset();
    }
    
    void setupTestData() {
        // 创建测试词库
        Book book;
        book.id = "test_cet4";
        book.name = "Test CET-4";
        book.url = "test.json";
        book.wordCount = 5;
        bookRepo->save(book);
        
        // 创建测试单词
        for (int i = 1; i <= 5; ++i) {
            Word word;
            word.bookId = "test_cet4";
            word.wordId = i;
            word.word = QString("word%1").arg(i);
            word.phoneticUk = QString("/word%1/").arg(i);
            word.phoneticUs = QString("/word%1/").arg(i);
            word.translations = "[{\"pos\":\"n.\",\"cn\":\"单词\"}]";
            word.sentences = "[]";
            word.phrases = "[]";
            word.synonyms = "[]";
            word.relatedWords = "{}";
            word.etymology = "[]";
            
            wordRepo->save(word);
        }
    }
    
    std::unique_ptr<SQLiteAdapter> adapter;
    std::unique_ptr<BookRepository> bookRepo;
    std::unique_ptr<WordRepository> wordRepo;
    std::unique_ptr<StudyRecordRepository> recordRepo;
    std::unique_ptr<ReviewScheduleRepository> scheduleRepo;
    std::unique_ptr<SM2Scheduler> scheduler;
    std::unique_ptr<StudyService> service;
};

// ============================================
// 测试：开始新词学习会话
// ============================================
TEST_F(StudyFlowIntegrationTest, StartNewWordsSession) {
    // Act
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        5
    );
    
    // Assert
    EXPECT_EQ(session.bookId, "test_cet4");
    EXPECT_EQ(session.type, StudyService::StudySession::NewWords);
    EXPECT_EQ(session.wordIds.size(), 5);
    EXPECT_EQ(session.currentIndex, 0);
    EXPECT_TRUE(session.hasNext());
    EXPECT_FALSE(session.hasPrevious());
}

// ============================================
// 测试：获取当前单词
// ============================================
TEST_F(StudyFlowIntegrationTest, GetCurrentWord) {
    // Arrange
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        5
    );
    
    // Act
    Word word = service->getCurrentWord(session);
    
    // Assert
    EXPECT_FALSE(word.word.isEmpty());
    EXPECT_EQ(word.bookId, "test_cet4");
}

// ============================================
// 测试：完整学习流程
// ============================================
TEST_F(StudyFlowIntegrationTest, CompleteStudyFlow) {
    // 1. 开始会话
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        5
    );
    
    ASSERT_EQ(session.wordIds.size(), 5);
    
    // 2. 学习所有单词
    int knownCount = 0;
    int unknownCount = 0;
    
    while (session.hasNext()) {
        Word word = service->getCurrentWord(session);
        ASSERT_FALSE(word.word.isEmpty());
        
        // 模拟学习结果（交替认识/不认识）
        StudyService::StudyResult result;
        result.wordId = word.id;
        result.bookId = "test_cet4";
        result.known = (session.currentIndex % 2 == 0);  // 偶数认识，奇数不认识
        result.duration = 5;
        
        if (result.known) {
            knownCount++;
        } else {
            unknownCount++;
        }
        
        // 记录并移动到下一个
        ASSERT_TRUE(service->recordAndNext(session, result));
    }
    
    // 3. 结束会话
    auto summary = service->endSession(session);
    
    // Assert
    EXPECT_EQ(summary.totalWords, 5);
    EXPECT_EQ(summary.knownWords, knownCount);
    EXPECT_EQ(summary.unknownWords, unknownCount);
    EXPECT_GT(summary.totalDuration, 0);
}

// ============================================
// 测试：学习记录保存
// ============================================
TEST_F(StudyFlowIntegrationTest, StudyRecordsSaved) {
    // Arrange
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        3
    );
    
    // Act - 学习3个单词
    for (int i = 0; i < 3; ++i) {
        Word word = service->getCurrentWord(session);
        
        StudyService::StudyResult result;
        result.wordId = word.id;
        result.bookId = "test_cet4";
        result.known = true;
        result.duration = 5;
        
        service->recordAndNext(session, result);
    }
    
    // Assert - 验证学习记录
    auto records = recordRepo->getTodayRecords();
    EXPECT_EQ(records.size(), 3);
    
    for (const StudyRecord& record : records) {
        EXPECT_EQ(record.bookId, "test_cet4");
        EXPECT_EQ(record.studyType, StudyRecord::Type::Learn);
        EXPECT_EQ(record.result, StudyRecord::Result::Known);
    }
}

// ============================================
// 测试：复习计划初始化
// ============================================
TEST_F(StudyFlowIntegrationTest, ReviewScheduleInitialized) {
    // Arrange
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        3
    );
    
    // Act - 学习3个单词
    for (int i = 0; i < 3; ++i) {
        Word word = service->getCurrentWord(session);
        
        StudyService::StudyResult result;
        result.wordId = word.id;
        result.bookId = "test_cet4";
        result.known = true;
        result.duration = 5;
        
        service->recordAndNext(session, result);
    }
    
    // Assert - 验证复习计划
    for (int wordId : session.wordIds) {
        EXPECT_TRUE(scheduleRepo->exists(wordId));
        
        ReviewPlan plan = scheduleRepo->get(wordId);
        EXPECT_EQ(plan.bookId, "test_cet4");
        EXPECT_GT(plan.reviewInterval, 0);
        EXPECT_EQ(plan.masteryLevel, ReviewPlan::MasteryLevel::Learning);
    }
}

// ============================================
// 测试：复习会话
// ============================================
TEST_F(StudyFlowIntegrationTest, ReviewSession) {
    // 1. 先学习几个单词
    auto learnSession = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        3
    );
    
    for (int i = 0; i < 3; ++i) {
        Word word = service->getCurrentWord(learnSession);
        
        StudyService::StudyResult result;
        result.wordId = word.id;
        result.bookId = "test_cet4";
        result.known = true;
        result.duration = 5;
        
        service->recordAndNext(learnSession, result);
    }
    
    // 验证：今天学习后，复习计划应该是明天
    for (int wordId : learnSession.wordIds) {
        ReviewPlan plan = scheduleRepo->get(wordId);
        EXPECT_EQ(plan.nextReviewDate, QDate::currentDate().addDays(1));
    }
    
    // 2. 今天不应该有待复习的单词
    auto reviewSessionToday = service->startSession(
        "test_cet4",
        StudyService::StudySession::Review,
        10
    );
    
    // Assert - 今天刚学的，今天不需要复习
    EXPECT_EQ(reviewSessionToday.wordIds.size(), 0);
    
    // 3. 手动修改复习日期为今天（模拟时间流逝）
    for (int wordId : learnSession.wordIds) {
        ReviewPlan plan = scheduleRepo->get(wordId);
        plan.nextReviewDate = QDate::currentDate();  // 改为今天
        scheduleRepo->save(plan);
    }
    
    // 4. 现在应该有待复习的单词了
    auto reviewSession = service->startSession(
        "test_cet4",
        StudyService::StudySession::Review,
        10
    );
    
    // Assert
    EXPECT_EQ(reviewSession.type, StudyService::StudySession::Review);
    EXPECT_EQ(reviewSession.wordIds.size(), 3);
}

// ============================================
// 测试：今日统计
// ============================================
TEST_F(StudyFlowIntegrationTest, TodayStatistics) {
    // Arrange - 学习一些单词
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        3
    );
    
    for (int i = 0; i < 3; ++i) {
        Word word = service->getCurrentWord(session);
        
        StudyService::StudyResult result;
        result.wordId = word.id;
        result.bookId = "test_cet4";
        result.known = true;
        result.duration = 5;
        
        service->recordAndNext(session, result);
    }
    
    // Act
    auto stats = service->getTodayStats("test_cet4");
    
    // Assert
    EXPECT_EQ(stats.newWordsLearned, 3);
    EXPECT_EQ(stats.wordsReviewed, 0);
    EXPECT_GT(stats.totalDuration, 0);
}

// ============================================
// 测试：会话导航
// ============================================
TEST_F(StudyFlowIntegrationTest, SessionNavigation) {
    // Arrange
    auto session = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        5
    );
    
    // Act & Assert
    EXPECT_EQ(session.getProgress(), 0);
    EXPECT_EQ(session.getTotal(), 5);
    
    session.moveNext();
    EXPECT_EQ(session.getProgress(), 1);
    EXPECT_TRUE(session.hasPrevious());
    
    session.movePrevious();
    EXPECT_EQ(session.getProgress(), 0);
    EXPECT_FALSE(session.hasPrevious());
}

// ============================================
// 测试：复习质量对复习间隔的影响
// ============================================
TEST_F(StudyFlowIntegrationTest, ReviewQualityAffectsInterval) {
    // 1. 学习一个单词
    auto learnSession = service->startSession(
        "test_cet4",
        StudyService::StudySession::NewWords,
        1
    );
    
    Word word = service->getCurrentWord(learnSession);
    int wordId = word.id;
    
    StudyService::StudyResult learnResult;
    learnResult.wordId = wordId;
    learnResult.bookId = "test_cet4";
    learnResult.known = true;
    learnResult.duration = 5;
    
    service->recordAndNext(learnSession, learnResult);
    
    // 获取初始复习计划
    ReviewPlan initialPlan = scheduleRepo->get(wordId);
    int initialInterval = initialPlan.reviewInterval;
    
    // 2. 复习（表现良好）
    auto reviewSession = service->startSession(
        "test_cet4",
        StudyService::StudySession::Review,
        10
    );
    
    StudyService::StudyResult reviewResult;
    reviewResult.wordId = wordId;
    reviewResult.bookId = "test_cet4";
    reviewResult.known = true;
    reviewResult.duration = 2;  // 很快，说明很简单
    
    service->recordAndNext(reviewSession, reviewResult);
    
    // 3. 验证间隔增加
    ReviewPlan updatedPlan = scheduleRepo->get(wordId);
    EXPECT_GT(updatedPlan.reviewInterval, initialInterval);
    EXPECT_GT(updatedPlan.repetitionCount, initialPlan.repetitionCount);
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}