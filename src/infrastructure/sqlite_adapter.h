#ifndef WORDMASTER_INFRASTRUCTURE_SQLITE_ADAPTER_H
#define WORDMASTER_INFRASTRUCTURE_SQLITE_ADAPTER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

/**
 * @brief SQLite 数据库适配器
 * 
 * 职责：
 * - 管理数据库连接
 * - 提供查询执行接口
 * - 处理事务
 * - 错误处理
 */
class SQLiteAdapter {
public:
    /**
     * @brief 构造函数
     * @param dbPath 数据库文件路径，":memory:" 表示内存数据库
     */
    explicit SQLiteAdapter(const QString& dbPath);
    
    /**
     * @brief 析构函数 - 自动关闭连接
     */
    ~SQLiteAdapter();
    
    // 禁用拷贝
    SQLiteAdapter(const SQLiteAdapter&) = delete;
    SQLiteAdapter& operator=(const SQLiteAdapter&) = delete;
    
    /**
     * @brief 打开数据库连接
     * @return 成功返回true
     */
    bool open();
    
    /**
     * @brief 关闭数据库连接
     */
    void close();
    
    /**
     * @brief 检查连接是否打开
     */
    bool isOpen() const;
    
    /**
     * @brief 执行SQL语句（不返回结果）
     * @param sql SQL语句
     * @return 成功返回true
     */
    bool execute(const QString& sql);
    
    /**
     * @brief 执行SQL查询（返回结果）
     * @param sql SQL语句
     * @return QSqlQuery对象
     */
    QSqlQuery query(const QString& sql);
    
    /**
     * @brief 准备SQL语句
     * @param sql 带占位符的SQL语句
     * @return QSqlQuery对象
     */
    QSqlQuery prepare(const QString& sql);
    
    /**
     * @brief 开始事务
     */
    bool beginTransaction();
    
    /**
     * @brief 提交事务
     */
    bool commit();
    
    /**
     * @brief 回滚事务
     */
    bool rollback();
    
    /**
     * @brief 获取最后插入的行ID
     */
    int lastInsertId() const;
    
    /**
     * @brief 获取上次操作影响的行数
     */
    int affectedRows() const;
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const;
    
    /**
     * @brief 初始化数据库（执行迁移脚本）
     * @param migrationFile SQL迁移文件路径
     * @return 成功返回true
     */
    bool initializeDatabase(const QString& migrationFile);
    
    /**
     * @brief 获取数据库连接（用于直接操作）
     */
    QSqlDatabase& getConnection();

private:
    QString dbPath_;
    QString connectionName_;
    QSqlDatabase db_;
    QSqlQuery lastQuery_;
};

} // namespace Infrastructure
} // namespace WordMaster

#endif // WORDMASTER_INFRASTRUCTURE_SQLITE_ADAPTER_H