#ifndef WORDMASTER_INFRASTRUCTURE_WORD_REPOSITORY_H
#define WORDMASTER_INFRASTRUCTURE_WORD_REPOSITORY_H

#include "domain/repositories.h"
#include "infrastructure/sqlite_adapter.h"

namespace WordMaster {
namespace Infrastructure {

/**
 * @brief Word 仓储实现
 * 
 * 职责：
 * - 单词数据的持久化
 * - 单词查询和搜索
 * - 批量操作和事务管理
 */
class WordRepository : public Domain::IWordRepository {
public:
    explicit WordRepository(SQLiteAdapter& adapter);
    ~WordRepository() override = default;
    
    // 基本CRUD
    bool save(const Domain::Word& word) override;
    Domain::Word getById(int id) override;
    QList<Domain::Word> getByIds(const QList<int>& ids) override;
    bool remove(int id) override;
    bool exists(int id) override;
    
    // 查询
    QList<Domain::Word> getByBookId(const QString& bookId, 
                                     int limit = -1, 
                                     int offset = 0) override;
    QList<Domain::Word> searchByWord(const QString& word) override;
    Domain::Word getByBookAndWord(const QString& bookId, 
                                  const QString& word) override;
    
    // 批量操作
    bool saveBatch(const QList<Domain::Word>& words) override;
    bool removeByBookId(const QString& bookId) override;
    
    // 事务支持
    bool beginTransaction() override;
    bool commit() override;
    bool rollback() override;

private:
    SQLiteAdapter& adapter_;
    
    // 辅助方法：从 QSqlQuery 构建 Word 对象
    Domain::Word buildWordFromQuery(QSqlQuery& query);
};

} // namespace Infrastructure
} // namespace WordMaster

#endif // WORDMASTER_INFRASTRUCTURE_WORD_REPOSITORY_H