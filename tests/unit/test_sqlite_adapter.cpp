#include <gtest/gtest.h>
#include "infrastructure/sqlite_adapter.h"
#include <QSqlQuery>
#include <QVariant>
#include <QFile>

using namespace WordMaster::Infrastructure;

/**
 * @brief SQLiteAdapter 单元测试
 * 
 * 测试目标：
 * 1. 连接管理
 * 2. 基本查询
 * 3. 事务处理
 * 4. 错误处理
 */
class SQLiteAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 使用内存数据库进行测试
        adapter = std::make_unique<SQLiteAdapter>(":memory:");
    }
    
    void TearDown() override {
        adapter.reset();
    }
    
    std::unique_ptr<SQLiteAdapter> adapter;
};

// ============================================
// 测试：打开和关闭连接
// ============================================
TEST_F(SQLiteAdapterTest, OpenAndClose) {
    // 初始状态应该未打开
    EXPECT_FALSE(adapter->isOpen());
    
    // 打开连接
    EXPECT_TRUE(adapter->open());
    EXPECT_TRUE(adapter->isOpen());
    
    // 关闭连接
    adapter->close();
    EXPECT_FALSE(adapter->isOpen());
    
    // 重复打开应该成功
    EXPECT_TRUE(adapter->open());
    EXPECT_TRUE(adapter->isOpen());
}

// ============================================
// 测试：执行SQL语句
// ============================================
TEST_F(SQLiteAdapterTest, ExecuteSQL) {
    ASSERT_TRUE(adapter->open());
    
    // 创建表
    QString createTableSQL = R"(
        CREATE TABLE test_table (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        )
    )";
    
    EXPECT_TRUE(adapter->execute(createTableSQL));
    
    // 插入数据
    QString insertSQL = "INSERT INTO test_table (name) VALUES ('test')";
    EXPECT_TRUE(adapter->execute(insertSQL));
    
    // 查询数据
    auto query = adapter->query("SELECT * FROM test_table");
    EXPECT_TRUE(query.next());
    EXPECT_EQ(query.value("name").toString(), QString("test"));
}

// ============================================
// 测试：预处理语句
// ============================================
TEST_F(SQLiteAdapterTest, PrepareStatement) {
    ASSERT_TRUE(adapter->open());
    
    // 创建表
    adapter->execute(R"(
        CREATE TABLE test_users (
            id INTEGER PRIMARY KEY,
            username TEXT NOT NULL,
            age INTEGER
        )
    )");
    
    // 准备插入语句
    auto query = adapter->prepare(
        "INSERT INTO test_users (username, age) VALUES (?, ?)"
    );
    
    EXPECT_TRUE(query.lastError().isValid() == false);
    
    // 绑定参数并执行
    query.addBindValue("alice");
    query.addBindValue(25);
    EXPECT_TRUE(query.exec());
    
    // 验证插入
    auto selectQuery = adapter->query("SELECT COUNT(*) as cnt FROM test_users");
    EXPECT_TRUE(selectQuery.next());
    EXPECT_EQ(selectQuery.value("cnt").toInt(), 1);
}

// ============================================
// 测试：事务处理 - 提交
// ============================================
TEST_F(SQLiteAdapterTest, TransactionCommit) {
    ASSERT_TRUE(adapter->open());
    
    // 创建表
    adapter->execute(R"(
        CREATE TABLE test_items (
            id INTEGER PRIMARY KEY,
            value TEXT
        )
    )");
    
    // 开始事务
    EXPECT_TRUE(adapter->beginTransaction());
    
    // 插入数据
    adapter->execute("INSERT INTO test_items (value) VALUES ('item1')");
    adapter->execute("INSERT INTO test_items (value) VALUES ('item2')");
    
    // 提交事务
    EXPECT_TRUE(adapter->commit());
    
    // 验证数据已提交
    auto query = adapter->query("SELECT COUNT(*) as cnt FROM test_items");
    EXPECT_TRUE(query.next());
    EXPECT_EQ(query.value("cnt").toInt(), 2);
}

// ============================================
// 测试：事务处理 - 回滚
// ============================================
TEST_F(SQLiteAdapterTest, TransactionRollback) {
    ASSERT_TRUE(adapter->open());
    
    // 创建表
    adapter->execute(R"(
        CREATE TABLE test_data (
            id INTEGER PRIMARY KEY,
            value TEXT
        )
    )");
    
    // 插入初始数据
    adapter->execute("INSERT INTO test_data (value) VALUES ('initial')");
    
    // 开始事务
    EXPECT_TRUE(adapter->beginTransaction());
    
    // 插入更多数据
    adapter->execute("INSERT INTO test_data (value) VALUES ('temp1')");
    adapter->execute("INSERT INTO test_data (value) VALUES ('temp2')");
    
    // 回滚事务
    EXPECT_TRUE(adapter->rollback());
    
    // 验证只有初始数据
    auto query = adapter->query("SELECT COUNT(*) as cnt FROM test_data");
    EXPECT_TRUE(query.next());
    EXPECT_EQ(query.value("cnt").toInt(), 1);
}

// ============================================
// 测试：获取最后插入ID
// ============================================
TEST_F(SQLiteAdapterTest, LastInsertId) {
    ASSERT_TRUE(adapter->open());
    
    adapter->execute(R"(
        CREATE TABLE test_autoincrement (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            data TEXT
        )
    )");
    
    adapter->execute("INSERT INTO test_autoincrement (data) VALUES ('test')");
    
    int lastId = adapter->lastInsertId();
    EXPECT_GT(lastId, 0);
}

// ============================================
// 测试：影响的行数
// ============================================
TEST_F(SQLiteAdapterTest, AffectedRows) {
    ASSERT_TRUE(adapter->open());
    
    adapter->execute(R"(
        CREATE TABLE test_update (
            id INTEGER PRIMARY KEY,
            status TEXT
        )
    )");
    
    // 插入多行
    adapter->execute("INSERT INTO test_update (status) VALUES ('pending')");
    adapter->execute("INSERT INTO test_update (status) VALUES ('pending')");
    adapter->execute("INSERT INTO test_update (status) VALUES ('done')");
    
    // 更新部分行
    adapter->execute("UPDATE test_update SET status = 'completed' WHERE status = 'pending'");
    
    int affected = adapter->affectedRows();
    EXPECT_EQ(affected, 2);
}

// ============================================
// 测试：外键约束
// ============================================
TEST_F(SQLiteAdapterTest, ForeignKeyConstraint) {
    ASSERT_TRUE(adapter->open());
    
    // 创建父表和子表
    adapter->execute(R"(
        CREATE TABLE parent (
            id INTEGER PRIMARY KEY,
            name TEXT
        )
    )");
    
    adapter->execute(R"(
        CREATE TABLE child (
            id INTEGER PRIMARY KEY,
            parent_id INTEGER,
            FOREIGN KEY(parent_id) REFERENCES parent(id) ON DELETE CASCADE
        )
    )");
    
    // 插入父记录
    adapter->execute("INSERT INTO parent (id, name) VALUES (1, 'Parent1')");
    
    // 插入子记录
    adapter->execute("INSERT INTO child (parent_id) VALUES (1)");
    
    // 尝试插入无效的外键（应该失败）
    bool result = adapter->execute("INSERT INTO child (parent_id) VALUES (999)");
    EXPECT_FALSE(result);
}

// ============================================
// 测试：数据库初始化
// ============================================
TEST_F(SQLiteAdapterTest, InitializeDatabase) {
    ASSERT_TRUE(adapter->open());
    
    // 创建临时迁移文件
    QString migrationSQL = R"(
        CREATE TABLE books (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL
        );
        
        INSERT INTO books (id, name) VALUES ('test', 'Test Book');
    )";
    
    // 写入临时文件
    QString tempFile = "/tmp/test_migration.sql";
    QFile file(tempFile);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << migrationSQL;
    file.close();
    
    // 执行迁移
    EXPECT_TRUE(adapter->initializeDatabase(tempFile));
    
    // 验证表已创建且数据已插入
    auto query = adapter->query("SELECT * FROM books WHERE id = 'test'");
    EXPECT_TRUE(query.next());
    EXPECT_EQ(query.value("name").toString(), QString("Test Book"));
    
    // 清理
    QFile::remove(tempFile);
}

// ============================================
// 主函数
// ============================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}