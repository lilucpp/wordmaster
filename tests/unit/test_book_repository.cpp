#include <gtest/gtest.h>
#include "infrastructure/repositories/book_repository.h"
#include "tests/test_helpers.h"
#include <memory>

using namespace WordMaster::Domain;
using namespace WordMaster::Infrastructure;
using namespace WordMaster::Testing;

/**
 * @brief BookRepository 单元测试
 */
class BookRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        adapter = TestDatabaseHelper::createTestDatabase();
        ASSERT_TRUE(adapter->isOpen());
        ASSERT_TRUE(TestDatabaseHelper::initializeTestSchema(*adapter));
        
        repository = std::make_unique<BookRepository>(*adapter);
    }
    
    void TearDown() override {
        repository.reset();
        adapter.reset();
    }
    
    // 创建测试用的 Book 对象
    Book createTestBook(const QString& id = "test_cet4") {
        Book book;
        book.id = id;
        book.name = "Test CET-4";
        book.description = "Test book for CET-4";
        book.category = "中国考试";
        book.tags = QStringList{"大学英语", "四级"};
        book.url = "test_cet4.json";
        book.wordCount = 2607;
        book.language = "en";
        book.translateLanguage = "zh-CN";
        book.isActive = false;
        return book;
    }
    
    std::unique_ptr<SQLiteAdapter> adapter;
    std::unique_ptr<BookRepository> repository;
};

// ============================================
// 测试：保存和获取
// ============================================
TEST_F(BookRepositoryTest, SaveAndGetById) {
    // Arrange
    Book book = createTestBook();
    
    // Act
    bool saved = repository->save(book);
    ASSERT_TRUE(saved);
    
    Book retrieved = repository->getById("test_cet4");
    
    // Assert
    EXPECT_EQ(retrieved.id, book.id);
    EXPECT_EQ(retrieved.name, book.name);
    EXPECT_EQ(retrieved.description, book.description);
    EXPECT_EQ(retrieved.category, book.category);
    EXPECT_EQ(retrieved.tags.size(), 2);
    EXPECT_EQ(retrieved.tags[0], "大学英语");
    EXPECT_EQ(retrieved.wordCount, 2607);
    EXPECT_EQ(retrieved.isActive, false);
}

// ============================================
// 测试：保存无效对象
// ============================================
TEST_F(BookRepositoryTest, SaveInvalidBook) {
    // Arrange
    Book invalidBook;
    invalidBook.id = "";  // 无效：ID为空
    
    // Act & Assert
    EXPECT_FALSE(repository->save(invalidBook));
}

// ============================================
// 测试：更新已存在的词库
// ============================================
TEST_F(BookRepositoryTest, UpdateExistingBook) {
    // Arrange
    Book book = createTestBook();
    repository->save(book);
    
    // Act - 修改并保存
    book.name = "Updated Name";
    book.wordCount = 3000;
    bool updated = repository->save(book);
    
    // Assert
    ASSERT_TRUE(updated);
    
    Book retrieved = repository->getById("test_cet4");
    EXPECT_EQ(retrieved.name, "Updated Name");
    EXPECT_EQ(retrieved.wordCount, 3000);
}

// ============================================
// 测试：获取所有词库
// ============================================
TEST_F(BookRepositoryTest, GetAll) {
    // Arrange
    Book book1 = createTestBook("cet4");
    Book book2 = createTestBook("cet6");
    book2.name = "CET-6";
    
    repository->save(book1);
    repository->save(book2);
    
    // Act
    QList<Book> books = repository->getAll();
    
    // Assert
    EXPECT_EQ(books.size(), 2);
}

// ============================================
// 测试：按分类查询
// ============================================
TEST_F(BookRepositoryTest, GetByCategory) {
    // Arrange
    Book book1 = createTestBook("cet4");
    book1.category = "中国考试";
    
    Book book2 = createTestBook("toefl");
    book2.category = "国际考试";
    
    repository->save(book1);
    repository->save(book2);
    
    // Act
    QList<Book> chineseExams = repository->getByCategory("中国考试");
    QList<Book> internationalExams = repository->getByCategory("国际考试");
    
    // Assert
    EXPECT_EQ(chineseExams.size(), 1);
    EXPECT_EQ(chineseExams[0].id, "cet4");
    
    EXPECT_EQ(internationalExams.size(), 1);
    EXPECT_EQ(internationalExams[0].id, "toefl");
}

// ============================================
// 测试：删除词库
// ============================================
TEST_F(BookRepositoryTest, Remove) {
    // Arrange
    Book book = createTestBook();
    repository->save(book);
    
    // Act
    bool removed = repository->remove("test_cet4");
    
    // Assert
    ASSERT_TRUE(removed);
    EXPECT_FALSE(repository->exists("test_cet4"));
    
    Book retrieved = repository->getById("test_cet4");
    EXPECT_TRUE(retrieved.id.isEmpty());
}

// ============================================
// 测试：检查存在性
// ============================================
TEST_F(BookRepositoryTest, Exists) {
    // Arrange
    Book book = createTestBook();
    repository->save(book);
    
    // Act & Assert
    EXPECT_TRUE(repository->exists("test_cet4"));
    EXPECT_FALSE(repository->exists("nonexistent"));
}

// ============================================
// 测试：设置激活状态
// ============================================
TEST_F(BookRepositoryTest, SetActive) {
    // Arrange
    Book book1 = createTestBook("cet4");
    Book book2 = createTestBook("cet6");
    
    repository->save(book1);
    repository->save(book2);
    
    // Act - 激活第一个词库
    bool success = repository->setActive("cet4", true);
    ASSERT_TRUE(success);
    
    // Assert
    Book activeBook = repository->getActiveBook();
    EXPECT_EQ(activeBook.id, "cet4");
    EXPECT_TRUE(activeBook.isActive);
    
    // Act - 激活第二个词库（应自动取消第一个）
    repository->setActive("cet6", true);
    
    // Assert
    activeBook = repository->getActiveBook();
    EXPECT_EQ(activeBook.id, "cet6");
    
    Book book1Retrieved = repository->getById("cet4");
    EXPECT_FALSE(book1Retrieved.isActive);
}

// ============================================
// 测试：获取激活词库（无激活）
// ============================================
TEST_F(BookRepositoryTest, GetActiveBookWhenNone) {
    // Act
    Book activeBook = repository->getActiveBook();
    
    // Assert
    EXPECT_TRUE(activeBook.id.isEmpty());
}

// ============================================
// 测试：获取总单词数
// ============================================
TEST_F(BookRepositoryTest, GetTotalWordCount) {
    // Arrange
    Book book = createTestBook();
    book.wordCount = 2607;
    repository->save(book);
    
    // Act
    int count = repository->getTotalWordCount("test_cet4");
    
    // Assert
    EXPECT_EQ(count, 2607);
}

// ============================================
// 测试：标签序列化/反序列化
// ============================================
TEST_F(BookRepositoryTest, TagsSerialization) {
    // Arrange
    Book book = createTestBook();
    book.tags = QStringList{"标签1", "标签2", "标签3"};
    
    // Act
    repository->save(book);
    Book retrieved = repository->getById("test_cet4");
    
    // Assert
    EXPECT_EQ(retrieved.tags.size(), 3);
    EXPECT_EQ(retrieved.tags[0], "标签1");
    EXPECT_EQ(retrieved.tags[1], "标签2");
    EXPECT_EQ(retrieved.tags[2], "标签3");
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}