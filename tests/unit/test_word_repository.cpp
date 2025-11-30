#include <gtest/gtest.h>
#include "infrastructure/repositories/word_repository.h"
#include "infrastructure/repositories/book_repository.h"
#include "tests/test_helpers.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace WordMaster::Domain;
using namespace WordMaster::Infrastructure;
using namespace WordMaster::Testing;

/**
 * @brief WordRepository 单元测试
 */
class WordRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        adapter = TestDatabaseHelper::createTestDatabase();
        ASSERT_TRUE(adapter->isOpen());
        ASSERT_TRUE(TestDatabaseHelper::initializeTestSchema(*adapter));
        
        // 先创建词库
        bookRepo = std::make_unique<BookRepository>(*adapter);
        Book book;
        book.id = "test_cet4";
        book.name = "Test CET-4";
        book.url = "test.json";
        book.wordCount = 10;
        bookRepo->save(book);
        
        repository = std::make_unique<WordRepository>(*adapter);
    }
    
    void TearDown() override {
        repository.reset();
        bookRepo.reset();
        adapter.reset();
    }
    
    // 创建测试用的 Word 对象
    Word createTestWord(int wordId = 1, const QString& word = "test") {
        Word w;
        w.bookId = "test_cet4";
        w.wordId = wordId;
        w.word = word;
        w.phoneticUk = "/test/";
        w.phoneticUs = "/test/";
        
        // 构建简单的 translations JSON
        QJsonArray trans;
        QJsonObject trans1;
        trans1["pos"] = "n.";
        trans1["cn"] = "测试";
        trans.append(trans1);
        w.translations = QJsonDocument(trans).toJson(QJsonDocument::Compact);
        
        w.sentences = "[]";
        w.phrases = "[]";
        w.synonyms = "[]";
        w.relatedWords = "{}";
        w.etymology = "[]";
        
        return w;
    }
    
    std::unique_ptr<SQLiteAdapter> adapter;
    std::unique_ptr<BookRepository> bookRepo;
    std::unique_ptr<WordRepository> repository;
};

// ============================================
// 测试：保存和获取
// ============================================
TEST_F(WordRepositoryTest, SaveAndGetById) {
    // Arrange
    Word word = createTestWord();
    
    // Act
    bool saved = repository->save(word);
    ASSERT_TRUE(saved);
    
    // 获取自增ID
    auto query = adapter->query("SELECT id FROM words WHERE word = 'test'");
    ASSERT_TRUE(query.next());
    int id = query.value("id").toInt();
    
    Word retrieved = repository->getById(id);
    
    // Assert
    EXPECT_EQ(retrieved.word, "test");
    EXPECT_EQ(retrieved.bookId, "test_cet4");
    EXPECT_EQ(retrieved.wordId, 1);
    EXPECT_EQ(retrieved.phoneticUk, "/test/");
}

// ============================================
// 测试：保存无效对象
// ============================================
TEST_F(WordRepositoryTest, SaveInvalidWord) {
    // Arrange
    Word invalidWord;
    invalidWord.word = "";  // 无效：单词为空
    
    // Act & Assert
    EXPECT_FALSE(repository->save(invalidWord));
}

// ============================================
// 测试：按词库ID获取
// ============================================
TEST_F(WordRepositoryTest, GetByBookId) {
    // Arrange
    Word word1 = createTestWord(1, "apple");
    Word word2 = createTestWord(2, "banana");
    Word word3 = createTestWord(3, "cherry");
    
    repository->save(word1);
    repository->save(word2);
    repository->save(word3);
    
    // Act
    QList<Word> words = repository->getByBookId("test_cet4");
    
    // Assert
    EXPECT_EQ(words.size(), 3);
}

// ============================================
// 测试：分页查询
// ============================================
TEST_F(WordRepositoryTest, GetByBookIdWithPagination) {
    // Arrange
    for (int i = 1; i <= 10; ++i) {
        Word word = createTestWord(i, QString("word%1").arg(i));
        repository->save(word);
    }
    
    // Act
    QList<Word> page1 = repository->getByBookId("test_cet4", 5, 0);
    QList<Word> page2 = repository->getByBookId("test_cet4", 5, 5);
    
    // Assert
    EXPECT_EQ(page1.size(), 5);
    EXPECT_EQ(page2.size(), 5);
    EXPECT_NE(page1[0].word, page2[0].word);
}

// ============================================
// 测试：搜索单词
// ============================================
TEST_F(WordRepositoryTest, SearchByWord) {
    // Arrange
    Word word1 = createTestWord(1, "apple");
    Word word2 = createTestWord(2, "application");
    Word word3 = createTestWord(3, "banana");
    
    repository->save(word1);
    repository->save(word2);
    repository->save(word3);
    
    // Act
    QList<Word> results = repository->searchByWord("app");
    
    // Assert
    EXPECT_EQ(results.size(), 2);
    EXPECT_TRUE(results[0].word.contains("app") || 
                results[1].word.contains("app"));
}

// ============================================
// 测试：按词库和单词名获取
// ============================================
TEST_F(WordRepositoryTest, GetByBookAndWord) {
    // Arrange
    Word word = createTestWord(1, "unique");
    repository->save(word);
    
    // Act
    Word retrieved = repository->getByBookAndWord("test_cet4", "unique");
    
    // Assert
    EXPECT_EQ(retrieved.word, "unique");
    EXPECT_EQ(retrieved.bookId, "test_cet4");
}

// ============================================
// 测试：删除单词
// ============================================
TEST_F(WordRepositoryTest, Remove) {
    // Arrange
    Word word = createTestWord();
    repository->save(word);
    
    auto query = adapter->query("SELECT id FROM words WHERE word = 'test'");
    ASSERT_TRUE(query.next());
    int id = query.value("id").toInt();
    
    // Act
    bool removed = repository->remove(id);
    
    // Assert
    ASSERT_TRUE(removed);
    EXPECT_FALSE(repository->exists(id));
}

// ============================================
// 测试：批量保存
// ============================================
TEST_F(WordRepositoryTest, SaveBatch) {
    // Arrange
    QList<Word> words;
    for (int i = 1; i <= 100; ++i) {
        words.append(createTestWord(i, QString("word%1").arg(i)));
    }
    
    // Act
    bool saved = repository->saveBatch(words);
    
    // Assert
    ASSERT_TRUE(saved);
    
    QList<Word> retrieved = repository->getByBookId("test_cet4");
    EXPECT_EQ(retrieved.size(), 100);
}

// ============================================
// 测试：批量保存事务回滚
// ============================================
TEST_F(WordRepositoryTest, SaveBatchRollbackOnError) {
    // Arrange
    QList<Word> words;
    words.append(createTestWord(1, "valid1"));
    
    Word invalid;
    invalid.word = "";  // 无效单词
    words.append(invalid);
    
    words.append(createTestWord(3, "valid2"));
    
    // Act
    bool saved = repository->saveBatch(words);
    
    // Assert
    EXPECT_FALSE(saved);
    
    // 验证所有单词都未保存（事务回滚）
    QList<Word> retrieved = repository->getByBookId("test_cet4");
    EXPECT_EQ(retrieved.size(), 0);
}

// ============================================
// 测试：按词库删除
// ============================================
TEST_F(WordRepositoryTest, RemoveByBookId) {
    // Arrange
    for (int i = 1; i <= 10; ++i) {
        repository->save(createTestWord(i, QString("word%1").arg(i)));
    }
    
    // Act
    bool removed = repository->removeByBookId("test_cet4");
    
    // Assert
    ASSERT_TRUE(removed);
    
    QList<Word> words = repository->getByBookId("test_cet4");
    EXPECT_EQ(words.size(), 0);
}

// ============================================
// 测试：按多个ID获取
// ============================================
TEST_F(WordRepositoryTest, GetByIds) {
    // Arrange
    Word word1 = createTestWord(1, "first");
    Word word2 = createTestWord(2, "second");
    Word word3 = createTestWord(3, "third");
    
    repository->save(word1);
    repository->save(word2);
    repository->save(word3);
    
    // 获取ID列表
    QList<int> ids;
    auto query = adapter->query("SELECT id FROM words ORDER BY word_id");
    while (query.next()) {
        ids.append(query.value("id").toInt());
    }
    
    // Act
    QList<Word> words = repository->getByIds(ids);
    
    // Assert
    EXPECT_EQ(words.size(), 3);
}

// ============================================
// 测试：事务管理
// ============================================
TEST_F(WordRepositoryTest, TransactionManagement) {
    // Arrange
    Word word1 = createTestWord(1, "trans1");
    Word word2 = createTestWord(2, "trans2");
    
    // Act
    ASSERT_TRUE(repository->beginTransaction());
    
    repository->save(word1);
    repository->save(word2);
    
    ASSERT_TRUE(repository->commit());
    
    // Assert
    QList<Word> words = repository->getByBookId("test_cet4");
    EXPECT_EQ(words.size(), 2);
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}