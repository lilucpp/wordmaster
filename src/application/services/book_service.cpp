#include "book_service.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace WordMaster {
namespace Application {

BookService::BookService(Domain::IBookRepository& bookRepo,
                        Domain::IWordRepository& wordRepo)
    : bookRepo_(bookRepo)
    , wordRepo_(wordRepo)
{
}

BookService::ImportResult BookService::importBooksFromMeta(
    const QString& metaJsonPath) 
{
    ImportResult result;
    
    // 解析词库元数据
    QList<Domain::Book> books = parseBookMetaJson(metaJsonPath);
    
    if (books.isEmpty()) {
        result.success = false;
        result.message = "未找到有效的词库数据";
        return result;
    }
    
    QFileInfo metaFileInfo(metaJsonPath);
    QDir metaDir = metaFileInfo.dir();
    
    // 遍历每个词库
    for (const Domain::Book& book : books) {
        // 检查词库是否已存在
        if (bookRepo_.exists(book.id)) {
            qDebug() << "Book already exists:" << book.id;
            continue;
        }
        
        // 保存词库元数据
        if (!bookRepo_.save(book)) {
            qWarning() << "Failed to save book:" << book.name;
            continue;
        }
        
        result.importedBooks++;
        
        // 导入单词数据
        QString bookJsonPath = metaDir.filePath(book.url);
        int importedWords = 0;
        
        if (importWordsFromJson(book.id, bookJsonPath, importedWords)) {
            result.importedWords += importedWords;
            qDebug() << "Imported" << importedWords << "words for book:" << book.name;
        } else {
            qWarning() << "Failed to import words for book:" << book.name;
        }
    }
    
    result.success = true;
    result.message = QString("成功导入 %1 个词库，共 %2 个单词")
        .arg(result.importedBooks)
        .arg(result.importedWords);
    
    return result;
}

QList<Domain::Book> BookService::getAllBooks() {
    return bookRepo_.getAll();
}

QList<Domain::Book> BookService::getBooksByCategory(const QString& category) {
    return bookRepo_.getByCategory(category);
}

Domain::Book BookService::getBookById(const QString& id) {
    return bookRepo_.getById(id);
}

Domain::Book BookService::getActiveBook() {
    return bookRepo_.getActiveBook();
}

bool BookService::setActiveBook(const QString& bookId) {
    if (!bookRepo_.exists(bookId)) {
        qWarning() << "Book not found:" << bookId;
        return false;
    }
    
    return bookRepo_.setActive(bookId, true);
}

bool BookService::deleteBook(const QString& bookId) {
    if (!bookRepo_.exists(bookId)) {
        return false;
    }
    
    // 先删除所有单词
    if (!wordRepo_.removeByBookId(bookId)) {
        qWarning() << "Failed to remove words for book:" << bookId;
        return false;
    }
    
    // 再删除词库
    return bookRepo_.remove(bookId);
}

BookService::BookStatistics BookService::getBookStatistics(const QString& bookId) {
    BookStatistics stats;
    
    Domain::Book book = bookRepo_.getById(bookId);
    if (book.id.isEmpty()) {
        return stats;
    }
    
    stats.bookId = book.id;
    stats.bookName = book.name;
    stats.totalWords = bookRepo_.getTotalWordCount(bookId);
    stats.learnedWords = bookRepo_.getLearnedWordCount(bookId);
    stats.masteredWords = bookRepo_.getMasteredWordCount(bookId);
    
    if (stats.totalWords > 0) {
        stats.progress = static_cast<double>(stats.learnedWords) / stats.totalWords;
    }
    
    return stats;
}

QList<BookService::BookStatistics> BookService::getAllBooksStatistics() {
    QList<BookStatistics> statsList;
    
    QList<Domain::Book> books = bookRepo_.getAll();
    for (const Domain::Book& book : books) {
        statsList.append(getBookStatistics(book.id));
    }
    
    return statsList;
}

bool BookService::importWordsFromJson(const QString& bookId,
                                       const QString& jsonPath,
                                       int& importedCount) 
{
    importedCount = 0;
    
    // 解析单词JSON
    QList<Domain::Word> words = parseWordsJson(bookId, jsonPath);
    
    if (words.isEmpty()) {
        qWarning() << "No words found in:" << jsonPath;
        return false;
    }
    
    // 批量保存
    if (!wordRepo_.saveBatch(words)) {
        qWarning() << "Failed to save words batch";
        return false;
    }
    
    importedCount = words.size();
    return true;
}

QList<Domain::Book> BookService::parseBookMetaJson(const QString& jsonPath) {
    QList<Domain::Book> books;
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << jsonPath;
        return books;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        qWarning() << "Invalid JSON format: expected array";
        return books;
    }
    
    QJsonArray booksArray = doc.array();
    
    for (const QJsonValue& value : booksArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject obj = value.toObject();
        
        Domain::Book book;
        book.id = obj["id"].toString();
        book.name = obj["name"].toString();
        book.description = obj["description"].toString();
        book.category = obj["category"].toString();
        book.url = obj["url"].toString();
        book.wordCount = obj["length"].toInt();
        book.language = obj["language"].toString("en");
        book.translateLanguage = obj["translateLanguage"].toString("zh-CN");
        
        // 解析tags数组
        if (obj.contains("tags") && obj["tags"].isArray()) {
            QJsonArray tagsArray = obj["tags"].toArray();
            for (const QJsonValue& tag : tagsArray) {
                book.tags.append(tag.toString());
            }
        }
        
        if (book.isValid()) {
            books.append(book);
        }
    }
    
    return books;
}

QList<Domain::Word> BookService::parseWordsJson(const QString& bookId,
                                                 const QString& jsonPath) 
{
    QList<Domain::Word> words;
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << jsonPath;
        return words;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        qWarning() << "Invalid JSON format: expected array";
        return words;
    }
    
    QJsonArray wordsArray = doc.array();
    
    for (const QJsonValue& value : wordsArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject obj = value.toObject();
        
        Domain::Word word;
        word.bookId = bookId;
        word.wordId = obj["id"].toInt();
        word.word = obj["word"].toString();
        word.phoneticUk = obj["phonetic0"].toString();
        word.phoneticUs = obj["phonetic1"].toString();
        
        // 序列化复杂字段为JSON字符串
        if (obj.contains("trans")) {
            word.translations = QJsonDocument(obj["trans"].toArray())
                .toJson(QJsonDocument::Compact);
        }
        
        if (obj.contains("sentences")) {
            word.sentences = QJsonDocument(obj["sentences"].toArray())
                .toJson(QJsonDocument::Compact);
        }
        
        if (obj.contains("phrases")) {
            word.phrases = QJsonDocument(obj["phrases"].toArray())
                .toJson(QJsonDocument::Compact);
        }
        
        if (obj.contains("synos")) {
            word.synonyms = QJsonDocument(obj["synos"].toArray())
                .toJson(QJsonDocument::Compact);
        }
        
        if (obj.contains("relWords")) {
            word.relatedWords = QJsonDocument(obj["relWords"].toObject())
                .toJson(QJsonDocument::Compact);
        }
        
        if (obj.contains("etymology")) {
            word.etymology = QJsonDocument(obj["etymology"].toArray())
                .toJson(QJsonDocument::Compact);
        }
        
        if (word.isValid()) {
            words.append(word);
        }
    }
    
    return words;
}

} // namespace Application
} // namespace WordMaster