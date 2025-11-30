#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <iostream>

#include "application/services/book_service.h"
#include "application/services/study_service.h"
#include "application/services/sm2_scheduler.h"
#include "infrastructure/repositories/book_repository.h"
#include "infrastructure/repositories/word_repository.h"
#include "infrastructure/repositories/study_record_repository.h"
#include "infrastructure/repositories/review_schedule_repository.h"
#include "infrastructure/sqlite_adapter.h"

using namespace WordMaster::Application;
using namespace WordMaster::Infrastructure;
using namespace WordMaster::Domain;

/**
 * @brief WordMaster 命令行工具
 * 
 * 功能：
 * - 导入词库
 * - 查看词库列表
 * - 激活词库
 * - 查看词库统计
 * - 搜索单词
 */
class WordMasterCLI {
public:
    WordMasterCLI(const QString& dbPath) 
        : adapter_(dbPath)
    {
        if (!adapter_.open()) {
            qFatal("Failed to open database: %s", qPrintable(dbPath));
        }
        
        // 初始化数据库
        QString schemaPath = "../resources/database/001_initial_schema.sql";
        if (QFile::exists(schemaPath)) {
            adapter_.initializeDatabase(schemaPath);
        }
        
        // 创建仓储
        bookRepo_ = std::make_unique<BookRepository>(adapter_);
        wordRepo_ = std::make_unique<WordRepository>(adapter_);
        recordRepo_ = std::make_unique<StudyRecordRepository>(adapter_);
        scheduleRepo_ = std::make_unique<ReviewScheduleRepository>(adapter_);
        
        // 创建服务
        bookService_ = std::make_unique<BookService>(*bookRepo_, *wordRepo_);
        scheduler_ = std::make_unique<SM2Scheduler>(*scheduleRepo_);
        studyService_ = std::make_unique<StudyService>(*wordRepo_, *recordRepo_, *scheduler_);
    }
    
    // 导入词库
    void importBooks(const QString& metaJsonPath) {
        std::cout << "开始导入词库..." << std::endl;
        std::cout << "元数据文件: " << qPrintable(metaJsonPath) << std::endl;
        
        auto result = bookService_->importBooksFromMeta(metaJsonPath);
        
        std::cout << "\n导入结果:" << std::endl;
        std::cout << "  状态: " << (result.success ? "成功" : "失败") << std::endl;
        std::cout << "  消息: " << qPrintable(result.message) << std::endl;
        std::cout << "  导入词库数: " << result.importedBooks << std::endl;
        std::cout << "  导入单词数: " << result.importedWords << std::endl;
    }
    
    // 列出所有词库
    void listBooks() {
        QList<Book> books = bookService_->getAllBooks();
        
        if (books.isEmpty()) {
            std::cout << "暂无词库。请先导入词库。" << std::endl;
            return;
        }
        
        std::cout << "\n词库列表 (共 " << books.size() << " 个):" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        for (const Book& book : books) {
            std::cout << "\nID: " << qPrintable(book.id) << std::endl;
            std::cout << "名称: " << qPrintable(book.name) << std::endl;
            std::cout << "分类: " << qPrintable(book.category) << std::endl;
            std::cout << "单词数: " << book.wordCount << std::endl;
            std::cout << "激活: " << (book.isActive ? "是" : "否") << std::endl;
            
            if (!book.tags.isEmpty()) {
                std::cout << "标签: ";
                for (const QString& tag : book.tags) {
                    std::cout << qPrintable(tag) << " ";
                }
                std::cout << std::endl;
            }
            
            std::cout << std::string(80, '-') << std::endl;
        }
    }
    
    // 显示词库统计
    void showStatistics(const QString& bookId) {
        Book book = bookService_->getBookById(bookId);
        
        if (book.id.isEmpty()) {
            std::cout << "错误: 词库不存在: " << qPrintable(bookId) << std::endl;
            return;
        }
        
        auto stats = bookService_->getBookStatistics(bookId);
        
        std::cout << "\n词库统计:" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "词库: " << qPrintable(stats.bookName) << std::endl;
        std::cout << "总单词数: " << stats.totalWords << std::endl;
        std::cout << "已学习: " << stats.learnedWords << std::endl;
        std::cout << "已掌握: " << stats.masteredWords << std::endl;
        std::cout << "进度: " << (stats.progress * 100) << "%" << std::endl;
    }
    
    // 激活词库
    void activateBook(const QString& bookId) {
        if (bookService_->setActiveBook(bookId)) {
            std::cout << "成功激活词库: " << qPrintable(bookId) << std::endl;
        } else {
            std::cout << "激活失败: " << qPrintable(bookId) << std::endl;
        }
    }
    
    // 搜索单词
    void searchWord(const QString& word) {
        QList<Word> words = wordRepo_->searchByWord(word);
        
        if (words.isEmpty()) {
            std::cout << "未找到匹配的单词。" << std::endl;
            return;
        }
        
        std::cout << "\n搜索结果 (共 " << words.size() << " 个):" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        for (const Word& w : words) {
            std::cout << "\n单词: " << qPrintable(w.word) << std::endl;
            std::cout << "音标: " << qPrintable(w.phoneticUk) << std::endl;
            std::cout << "词库: " << qPrintable(w.bookId) << std::endl;
            std::cout << std::string(80, '-') << std::endl;
        }
    }
    
    // 显示词库中的单词样本
    void showWordSamples(const QString& bookId, int count = 10) {
        Book book = bookService_->getBookById(bookId);
        
        if (book.id.isEmpty()) {
            std::cout << "错误: 词库不存在: " << qPrintable(bookId) << std::endl;
            return;
        }
        
        QList<Word> words = wordRepo_->getByBookId(bookId, count, 0);
        
        if (words.isEmpty()) {
            std::cout << "该词库暂无单词。" << std::endl;
            return;
        }
        
        std::cout << "\n词库 " << qPrintable(book.name) << " 的前 " 
                  << words.size() << " 个单词:" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        for (int i = 0; i < words.size(); ++i) {
            const Word& w = words[i];
            std::cout << "\n" << (i + 1) << ". " << qPrintable(w.word) << std::endl;
            std::cout << "   音标: " << qPrintable(w.phoneticUk) 
                      << " / " << qPrintable(w.phoneticUs) << std::endl;
        }
    }
    
    // 删除词库
    void deleteBook(const QString& bookId) {
        Book book = bookService_->getBookById(bookId);
        
        if (book.id.isEmpty()) {
            std::cout << "错误: 词库不存在: " << qPrintable(bookId) << std::endl;
            return;
        }
        
        std::cout << "确定要删除词库 \"" << qPrintable(book.name) << "\" 吗? (y/N): ";
        std::string confirm;
        std::cin >> confirm;
        
        if (confirm == "y" || confirm == "Y") {
            if (bookService_->deleteBook(bookId)) {
                std::cout << "词库已删除。" << std::endl;
            } else {
                std::cout << "删除失败。" << std::endl;
            }
        } else {
            std::cout << "已取消。" << std::endl;
        }
    }

private:
    SQLiteAdapter adapter_;
    std::unique_ptr<BookRepository> bookRepo_;
    std::unique_ptr<WordRepository> wordRepo_;
    std::unique_ptr<StudyRecordRepository> recordRepo_;
    std::unique_ptr<ReviewScheduleRepository> scheduleRepo_;
    std::unique_ptr<BookService> bookService_;
    std::unique_ptr<SM2Scheduler> scheduler_;
    std::unique_ptr<StudyService> studyService_;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("WordMaster CLI");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("WordMaster 词库管理命令行工具");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 数据库路径选项
    QCommandLineOption dbOption(
        QStringList() << "d" << "database",
        "数据库文件路径 (默认: wordmaster.db)",
        "database",
        "wordmaster.db"
    );
    parser.addOption(dbOption);
    
    // 命令选项
    QCommandLineOption importOption(
        QStringList() << "i" << "import",
        "导入词库元数据JSON文件",
        "meta-json"
    );
    parser.addOption(importOption);
    
    QCommandLineOption listOption(
        QStringList() << "l" << "list",
        "列出所有词库"
    );
    parser.addOption(listOption);
    
    QCommandLineOption statsOption(
        QStringList() << "s" << "stats",
        "显示词库统计",
        "book-id"
    );
    parser.addOption(statsOption);
    
    QCommandLineOption activateOption(
        QStringList() << "a" << "activate",
        "激活词库",
        "book-id"
    );
    parser.addOption(activateOption);
    
    QCommandLineOption searchOption(
        QStringList() << "search",
        "搜索单词",
        "word"
    );
    parser.addOption(searchOption);
    
    QCommandLineOption samplesOption(
        QStringList() << "samples",
        "显示词库单词样本",
        "book-id"
    );
    parser.addOption(samplesOption);
    
    QCommandLineOption deleteOption(
        QStringList() << "delete",
        "删除词库",
        "book-id"
    );
    parser.addOption(deleteOption);
    
    parser.process(app);
    
    // 创建 CLI 工具实例
    QString dbPath = parser.value(dbOption);
    WordMasterCLI cli(dbPath);
    
    std::cout << "WordMaster CLI v1.0.0" << std::endl;
    std::cout << "数据库: " << qPrintable(dbPath) << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // 执行命令
    if (parser.isSet(importOption)) {
        QString metaPath = parser.value(importOption);
        cli.importBooks(metaPath);
    }
    else if (parser.isSet(listOption)) {
        cli.listBooks();
    }
    else if (parser.isSet(statsOption)) {
        QString bookId = parser.value(statsOption);
        cli.showStatistics(bookId);
    }
    else if (parser.isSet(activateOption)) {
        QString bookId = parser.value(activateOption);
        cli.activateBook(bookId);
    }
    else if (parser.isSet(searchOption)) {
        QString word = parser.value(searchOption);
        cli.searchWord(word);
    }
    else if (parser.isSet(samplesOption)) {
        QString bookId = parser.value(samplesOption);
        cli.showWordSamples(bookId, 10);
    }
    else if (parser.isSet(deleteOption)) {
        QString bookId = parser.value(deleteOption);
        cli.deleteBook(bookId);
    }
    else {
        parser.showHelp();
    }
    
    return 0;
}