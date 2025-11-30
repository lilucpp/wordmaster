#ifndef WORDMASTER_APPLICATION_BOOK_SERVICE_H
#define WORDMASTER_APPLICATION_BOOK_SERVICE_H

#include "domain/repositories.h"
#include "domain/entities.h"
#include <QString>
#include <QList>

namespace WordMaster {
namespace Application {

/**
 * @brief 词库管理服务
 * 
 * 职责：
 * - 导入词库和单词数据
 * - 词库查询和管理
 * - 统计信息计算
 */
class BookService {
public:
    /**
     * @brief 导入结果
     */
    struct ImportResult {
        bool success;
        QString message;
        int importedBooks;
        int importedWords;
        
        ImportResult() : success(false), importedBooks(0), importedWords(0) {}
    };
    
    /**
     * @brief 词库统计
     */
    struct BookStatistics {
        QString bookId;
        QString bookName;
        int totalWords;
        int learnedWords;
        int masteredWords;
        double progress;  // 0.0 - 1.0
        
        BookStatistics() : totalWords(0), learnedWords(0), 
                          masteredWords(0), progress(0.0) {}
    };
    
    explicit BookService(Domain::IBookRepository& bookRepo,
                        Domain::IWordRepository& wordRepo);
    
    ~BookService() = default;
    
    // 词库导入
    ImportResult importBooksFromMeta(const QString& metaJsonPath);
    
    // 词库查询
    QList<Domain::Book> getAllBooks();
    QList<Domain::Book> getBooksByCategory(const QString& category);
    Domain::Book getBookById(const QString& id);
    Domain::Book getActiveBook();
    
    // 词库管理
    bool setActiveBook(const QString& bookId);
    bool deleteBook(const QString& bookId);
    
    // 统计
    BookStatistics getBookStatistics(const QString& bookId);
    QList<BookStatistics> getAllBooksStatistics();

private:
    Domain::IBookRepository& bookRepo_;
    Domain::IWordRepository& wordRepo_;
    
    // 导入单个词库的单词数据
    bool importWordsFromJson(const QString& bookId, 
                            const QString& jsonPath,
                            int& importedCount);
    
    // 解析词库元数据JSON
    QList<Domain::Book> parseBookMetaJson(const QString& jsonPath);
    
    // 解析单词JSON
    QList<Domain::Word> parseWordsJson(const QString& bookId, 
                                       const QString& jsonPath);
};

} // namespace Application
} // namespace WordMaster

#endif // WORDMASTER_APPLICATION_BOOK_SERVICE_H