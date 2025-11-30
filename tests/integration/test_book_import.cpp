#include <gtest/gtest.h>
#include "application/services/book_service.h"
#include "infrastructure/repositories/book_repository.h"
#include "infrastructure/repositories/word_repository.h"
#include "tests/test_helpers.h"
#include <QFile>
#include <QDir>

using namespace WordMaster::Application;
using namespace WordMaster::Domain;
using namespace WordMaster::Infrastructure;
using namespace WordMaster::Testing;

/**
 * @brief BookService 集成测试 - 词库导入流程
 * 
 * 测试完整的导入流程：
 * 1. 解析元数据JSON
 * 2. 保存词库信息
 * 3. 解析单词JSON
 * 4. 批量保存单词
 */
class BookImportIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据库
        adapter = TestDatabaseHelper::createTestDatabase();
        ASSERT_TRUE(adapter->isOpen());
        ASSERT_TRUE(TestDatabaseHelper::initializeTestSchema(*adapter));
        
        // 创建仓储
        bookRepo = std::make_unique<BookRepository>(*adapter);
        wordRepo = std::make_unique<WordRepository>(*adapter);
        
        // 创建服务
        service = std::make_unique<BookService>(*bookRepo, *wordRepo);
        
        // 创建测试数据文件
        createTestDataFiles();
    }
    
    void TearDown() override {
        // 清理临时文件
        cleanupTestFiles();
        
        service.reset();
        wordRepo.reset();
        bookRepo.reset();
        adapter.reset();
    }
    
    void createTestDataFiles() {
        // 创建词库元数据JSON
        metaJsonPath = TestDataGenerator::writeToTempFile(
            TestDataGenerator::generateBookMetaJson(),
            "test_word_meta.json"
        );
        
        // 创建单词JSON
        wordsJsonPath = TestDataGenerator::writeToTempFile(
            TestDataGenerator::generateWordsJson(),
            "test_cet4_words.json"
        );
        
        ASSERT_FALSE(metaJsonPath.isEmpty());
        ASSERT_FALSE(wordsJsonPath.isEmpty());
    }
    
    void cleanupTestFiles() {
        if (!metaJsonPath.isEmpty()) {
            QFile::remove(metaJsonPath);
        }
        if (!wordsJsonPath.isEmpty()) {
            QFile::remove(wordsJsonPath);
        }
    }
    
    std::unique_ptr<SQLiteAdapter> adapter;
    std::unique_ptr<BookRepository> bookRepo;
    std::unique_ptr<WordRepository> wordRepo;
    std::unique_ptr<BookService> service;
    
    QString metaJsonPath;
    QString wordsJsonPath;
};

// ============================================
// 测试：完整的导入流程
// ============================================
TEST_F(BookImportIntegrationTest, CompleteImportFlow) {
    // Act - 导入词库
    auto result = service->importBooksFromMeta(metaJsonPath);
    
    // Assert - 验证导入结果
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.importedBooks, 0);
    EXPECT_GT(result.importedWords, 0);
    EXPECT_FALSE(result.message.isEmpty());
    
    qDebug() << "Import result:" << result.message;
}

// ============================================
// 测试：词库元数据正确保存
// ============================================
TEST_F(BookImportIntegrationTest, BookMetadataSaved) {
    // Act
    service->importBooksFromMeta(metaJsonPath);
    
    // Assert - 验证词库保存
    Book book = bookRepo->getById("test_cet4");
    
    EXPECT_EQ(book.id, "test_cet4");
    EXPECT_EQ(book.name, "Test CET-4");
    EXPECT_EQ(book.category, "中国考试");
    EXPECT_FALSE(book.tags.isEmpty());
    EXPECT_EQ(book.wordCount, 10);
}

// ============================================
// 测试：单词数据正确保存
// ============================================
TEST_F(BookImportIntegrationTest, WordsDataSaved) {
    // Act
    auto result = service->importBooksFromMeta(metaJsonPath);
    
    // Assert - 验证单词保存
    QList<Word> words = wordRepo->getByBookId("test_cet4");
    
    EXPECT_GT(words.size(), 0);
    EXPECT_EQ(words.size(), result.importedWords);
    
    // 验证第一个单词
    if (!words.isEmpty()) {
        Word firstWord = words[0];
        EXPECT_EQ(firstWord.bookId, "test_cet4");
        EXPECT_FALSE(firstWord.word.isEmpty());
        EXPECT_FALSE(firstWord.translations.isEmpty());
    }
}

// ============================================
// 测试：单词内容正确解析
// ============================================
TEST_F(BookImportIntegrationTest, WordContentParsed) {
    // Act
    service->importBooksFromMeta(metaJsonPath);
    
    // Assert - 验证特定单词
    Word testWord = wordRepo->getByBookAndWord("test_cet4", "test");
    
    EXPECT_EQ(testWord.word, "test");
    EXPECT_FALSE(testWord.phoneticUk.isEmpty());
    EXPECT_FALSE(testWord.translations.isEmpty());
    
    // 验证JSON字段不为空
    EXPECT_NE(testWord.translations, "[]");
    
    qDebug() << "Test word phonetic:" << testWord.phoneticUk;
    qDebug() << "Test word translations:" << testWord.translations;
}

// ============================================
// 测试：获取所有词库
// ============================================
TEST_F(BookImportIntegrationTest, GetAllBooks) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // Act
    QList<Book> books = service->getAllBooks();
    
    // Assert
    EXPECT_GT(books.size(), 0);
}

// ============================================
// 测试：按分类获取词库
// ============================================
TEST_F(BookImportIntegrationTest, GetBooksByCategory) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // Act
    QList<Book> chineseExams = service->getBooksByCategory("中国考试");
    
    // Assert
    EXPECT_GT(chineseExams.size(), 0);
}

// ============================================
// 测试：设置激活词库
// ============================================
TEST_F(BookImportIntegrationTest, SetActiveBook) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // Act
    bool success = service->setActiveBook("test_cet4");
    
    // Assert
    ASSERT_TRUE(success);
    
    Book activeBook = service->getActiveBook();
    EXPECT_EQ(activeBook.id, "test_cet4");
    EXPECT_TRUE(activeBook.isActive);
}

// ============================================
// 测试：词库统计
// ============================================
TEST_F(BookImportIntegrationTest, BookStatistics) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // Act
    auto stats = service->getBookStatistics("test_cet4");
    
    // Assert
    EXPECT_EQ(stats.bookId, "test_cet4");
    EXPECT_EQ(stats.bookName, "Test CET-4");
    EXPECT_GT(stats.totalWords, 0);
    EXPECT_EQ(stats.learnedWords, 0);  // 未开始学习
    EXPECT_EQ(stats.masteredWords, 0);
    EXPECT_EQ(stats.progress, 0.0);
}

// ============================================
// 测试：删除词库
// ============================================
TEST_F(BookImportIntegrationTest, DeleteBook) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // Act
    bool deleted = service->deleteBook("test_cet4");
    
    // Assert
    ASSERT_TRUE(deleted);
    
    // 验证词库已删除
    Book book = service->getBookById("test_cet4");
    EXPECT_TRUE(book.id.isEmpty());
    
    // 验证单词也已删除（级联删除）
    QList<Word> words = wordRepo->getByBookId("test_cet4");
    EXPECT_EQ(words.size(), 0);
}

// ============================================
// 测试：重复导入同一词库（应跳过）
// ============================================
TEST_F(BookImportIntegrationTest, SkipDuplicateImport) {
    // Act - 第一次导入
    auto result1 = service->importBooksFromMeta(metaJsonPath);
    
    // Act - 第二次导入
    auto result2 = service->importBooksFromMeta(metaJsonPath);
    
    // Assert
    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
    EXPECT_GT(result1.importedBooks, 0);
    EXPECT_EQ(result2.importedBooks, 0);  // 应该跳过已存在的
}

// ============================================
// 测试：导入不存在的文件
// ============================================
TEST_F(BookImportIntegrationTest, ImportNonExistentFile) {
    // Act
    auto result = service->importBooksFromMeta("/nonexistent/path.json");
    
    // Assert
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.importedBooks, 0);
    EXPECT_EQ(result.importedWords, 0);
}

// ============================================
// 测试：批量操作的事务性
// ============================================
TEST_F(BookImportIntegrationTest, BatchOperationTransactional) {
    // Arrange
    service->importBooksFromMeta(metaJsonPath);
    
    // 获取导入的单词数
    QList<Word> words = wordRepo->getByBookId("test_cet4");
    int originalCount = words.size();
    
    // 模拟失败场景：尝试保存无效单词
    QList<Word> invalidWords;
    Word invalid;
    invalid.word = "";  // 无效
    invalidWords.append(invalid);
    
    // Act
    bool saved = wordRepo->saveBatch(invalidWords);
    
    // Assert - 失败后不应影响原有数据
    EXPECT_FALSE(saved);
    
    words = wordRepo->getByBookId("test_cet4");
    EXPECT_EQ(words.size(), originalCount);
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}