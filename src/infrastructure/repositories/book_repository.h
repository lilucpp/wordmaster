#ifndef WORDMASTER_INFRASTRUCTURE_BOOK_REPOSITORY_H
#define WORDMASTER_INFRASTRUCTURE_BOOK_REPOSITORY_H

#include "domain/repositories.h"
#include "infrastructure/sqlite_adapter.h"
#include <QJsonArray>

namespace WordMaster {
namespace Infrastructure {

/**
 * @brief Book 仓储实现
 * 
 * 职责：
 * - 词库元数据的持久化
 * - 词库查询和统计
 * - 标签序列化/反序列化
 */
class BookRepository : public Domain::IBookRepository {
public:
    explicit BookRepository(SQLiteAdapter& adapter);
    ~BookRepository() override = default;
    
    // 基本CRUD
    bool save(const Domain::Book& book) override;
    Domain::Book getById(const QString& id) override;
    QList<Domain::Book> getAll() override;
    bool remove(const QString& id) override;
    bool exists(const QString& id) override;
    
    // 查询
    QList<Domain::Book> getByCategory(const QString& category) override;
    Domain::Book getActiveBook() override;
    bool setActive(const QString& id, bool active) override;
    
    // 统计
    int getTotalWordCount(const QString& bookId) override;
    int getLearnedWordCount(const QString& bookId) override;
    int getMasteredWordCount(const QString& bookId) override;

private:
    SQLiteAdapter& adapter_;
    
    // 辅助方法：从 QSqlQuery 构建 Book 对象
    Domain::Book buildBookFromQuery(QSqlQuery& query);
    
    // 辅助方法：序列化标签数组为 JSON 字符串
    QString serializeTags(const QStringList& tags);
    
    // 辅助方法：反序列化 JSON 字符串为标签数组
    QStringList deserializeTags(const QString& json);
};

} // namespace Infrastructure
} // namespace WordMaster

#endif // WORDMASTER_INFRASTRUCTURE_BOOK_REPOSITORY_H