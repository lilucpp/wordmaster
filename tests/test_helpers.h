#ifndef WORDMASTER_TEST_HELPERS_H
#define WORDMASTER_TEST_HELPERS_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <memory>
#include "infrastructure/sqlite_adapter.h"

namespace WordMaster {
namespace Testing {

/**
 * @brief 测试数据库助手
 * 
 * 提供测试用的数据库初始化和数据生成
 */
class TestDatabaseHelper {
public:
    /**
     * @brief 创建测试数据库（内存）
     */
    static std::unique_ptr<Infrastructure::SQLiteAdapter> createTestDatabase() {
        auto adapter = std::make_unique<Infrastructure::SQLiteAdapter>(":memory:");
        adapter->open();
        return adapter;
    }
    
    /**
     * @brief 初始化测试数据库模式
     */
    static bool initializeTestSchema(Infrastructure::SQLiteAdapter& adapter) {
        // 简化的测试模式
        QString schema = R"(
            CREATE TABLE books (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                description TEXT,
                category TEXT,
                tags TEXT,
                url TEXT NOT NULL,
                word_count INTEGER DEFAULT 0,
                language TEXT DEFAULT 'en',
                translate_language TEXT DEFAULT 'zh-CN',
                imported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                is_active BOOLEAN DEFAULT 0
            );
            
            CREATE TABLE words (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                book_id TEXT NOT NULL,
                word_id INTEGER NOT NULL,
                word TEXT NOT NULL,
                phonetic_uk TEXT,
                phonetic_us TEXT,
                translations TEXT,
                sentences TEXT,
                phrases TEXT,
                synonyms TEXT,
                related_words TEXT,
                etymology TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE,
                UNIQUE(book_id, word_id)
            );
            
            CREATE TABLE study_records (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_id INTEGER NOT NULL,
                book_id TEXT NOT NULL,
                study_type TEXT NOT NULL,
                result TEXT NOT NULL,
                study_duration INTEGER DEFAULT 0,
                studied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE,
                FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
            );
            
            CREATE TABLE review_schedule (
                word_id INTEGER PRIMARY KEY,
                book_id TEXT NOT NULL,
                next_review_date DATE NOT NULL,
                review_interval INTEGER DEFAULT 1,
                repetition_count INTEGER DEFAULT 0,
                easiness_factor REAL DEFAULT 2.5,
                last_review_date DATE,
                mastery_level INTEGER DEFAULT 0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE,
                FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
            );
            
            CREATE TABLE word_tags (
                word_id INTEGER NOT NULL,
                tag_type TEXT NOT NULL,
                tagged_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                PRIMARY KEY(word_id, tag_type),
                FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE
            );
        )";

        // 拆分一条条执行
        QStringList sqlList = schema.split(";");
        for (auto sql : sqlList) {
            if (!sql.trimmed().isEmpty()) {
                if (!adapter.execute(sql)) {
                    return false;
                }
            }
        }
        return true;
        
    }
};

/**
 * @brief JSON 测试数据生成器
 */
class TestDataGenerator {
public:
    /**
     * @brief 生成测试用的词库元数据JSON
     */
    static QString generateBookMetaJson() {
        QJsonArray books;
        
        QJsonObject book1;
        book1["id"] = "test_cet4";
        book1["name"] = "Test CET-4";
        book1["description"] = "Test book for CET-4";
        book1["category"] = "中国考试";
        book1["tags"] = QJsonArray{"大学英语"};
        book1["url"] = "test_cet4_words.json";
        book1["length"] = 10;
        book1["language"] = "en";
        book1["translateLanguage"] = "zh-CN";
        
        books.append(book1);
        
        return QJsonDocument(books).toJson(QJsonDocument::Compact);
    }
    
    /**
     * @brief 生成测试用的单词JSON
     */
    static QString generateWordsJson() {
        QJsonArray words;
        
        // 单词1: test
        QJsonObject word1;
        word1["id"] = 1;
        word1["word"] = "test";
        word1["phonetic0"] = "/test/";
        word1["phonetic1"] = "/test/";
        
        QJsonArray trans1;
        QJsonObject trans1_1;
        trans1_1["pos"] = "n.";
        trans1_1["cn"] = "测试，试验";
        trans1.append(trans1_1);
        word1["trans"] = trans1;
        
        QJsonArray sentences1;
        QJsonObject sent1_1;
        sent1_1["c"] = "This is a test.";
        sent1_1["cn"] = "这是一个测试。";
        sentences1.append(sent1_1);
        word1["sentences"] = sentences1;
        
        word1["phrases"] = QJsonArray();
        word1["synos"] = QJsonArray();
        word1["relWords"] = QJsonObject();
        word1["etymology"] = QJsonArray();
        
        words.append(word1);
        
        // 单词2: example
        QJsonObject word2;
        word2["id"] = 2;
        word2["word"] = "example";
        word2["phonetic0"] = "/ɪɡˈzɑːmpl/";
        word2["phonetic1"] = "/ɪɡˈzæmpl/";
        
        QJsonArray trans2;
        QJsonObject trans2_1;
        trans2_1["pos"] = "n.";
        trans2_1["cn"] = "例子，实例";
        trans2.append(trans2_1);
        word2["trans"] = trans2;
        
        word2["sentences"] = QJsonArray();
        word2["phrases"] = QJsonArray();
        word2["synos"] = QJsonArray();
        word2["relWords"] = QJsonObject();
        word2["etymology"] = QJsonArray();
        
        words.append(word2);
        
        return QJsonDocument(words).toJson(QJsonDocument::Compact);
    }
    
    /**
     * @brief 将JSON写入临时文件
     */
    static QString writeToTempFile(const QString& content, const QString& filename) {
        QString tempPath = QString("/tmp/%1").arg(filename);
        QFile file(tempPath);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return tempPath;
        }
        
        return QString();
    }
};

} // namespace Testing
} // namespace WordMaster

#endif // WORDMASTER_TEST_HELPERS_H