# WordMaster 构建和测试指南

## 快速开始

### 1. 克隆项目

```bash
git clone <repository-url>
cd WordMaster
```

### 2. 项目结构

```
WordMaster/
├── src/
│   ├── main.cpp
│   ├── domain/
│   │   ├── entities.h
│   │   └── repositories.h
│   ├── infrastructure/
│   │   ├── sqlite_adapter.h/cpp
│   │   └── repositories/
│   │       ├── book_repository.h/cpp
│   │       └── word_repository.h/cpp
│   └── application/
│       └── services/
│           └── book_service.h/cpp
├── tests/
│   ├── test_helpers.h/cpp
│   ├── unit/
│   │   ├── test_sqlite_adapter.cpp
│   │   ├── test_book_repository.cpp
│   │   └── test_word_repository.cpp
│   └── integration/
│       └── test_book_import.cpp
└── resources/
    └── database/
        └── 001_initial_schema.sql
```

### 3. 构建项目

```bash
# 赋予构建脚本执行权限
chmod +x scripts/build.sh

# Debug 构建（默认）
./scripts/build.sh

# Release 构建
./scripts/build.sh --release

# 清理后重新构建
./scripts/build.sh --clean

# 生成代码覆盖率报告
./scripts/build.sh --coverage
```

### 4. 运行测试

```bash
cd build

# 运行所有测试
ctest --output-on-failure

# 运行特定测试
./tests/test_sqlite_adapter
./tests/test_book_repository
./tests/test_word_repository
./tests/test_book_import

# 查看测试详细输出
ctest -V
```

---

## 已实现的功能测试

### 单元测试

#### 1. SQLite Adapter 测试
```bash
./tests/test_sqlite_adapter
```

**测试内容：**
- ✅ 数据库连接打开/关闭
- ✅ SQL 语句执行
- ✅ 预处理语句
- ✅ 事务提交/回滚
- ✅ 外键约束
- ✅ 数据库初始化

#### 2. BookRepository 测试
```bash
./tests/test_book_repository
```

**测试内容：**
- ✅ 词库保存和查询
- ✅ 按分类查询
- ✅ 激活状态管理
- ✅ 标签序列化/反序列化
- ✅ 词库删除
- ✅ 统计信息查询

#### 3. WordRepository 测试
```bash
./tests/test_word_repository
```

**测试内容：**
- ✅ 单词保存和查询
- ✅ 按词库ID查询
- ✅ 分页查询
- ✅ 单词搜索
- ✅ 批量操作
- ✅ 事务管理

### 集成测试

#### 4. 词库导入集成测试
```bash
./tests/test_book_import
```

**测试内容：**
- ✅ 完整导入流程
- ✅ 元数据解析和保存
- ✅ 单词数据解析和保存
- ✅ 词库查询和管理
- ✅ 激活状态设置
- ✅ 词库统计
- ✅ 词库删除（级联）
- ✅ 重复导入处理
- ✅ 错误处理
- ✅ 事务一致性

---

## 测试数据

### 测试用 JSON 数据

测试使用动态生成的 JSON 数据，位于 `tests/test_helpers.h`：

**词库元数据示例：**
```json
[
  {
    "id": "test_cet4",
    "name": "Test CET-4",
    "description": "Test book for CET-4",
    "category": "中国考试",
    "tags": ["大学英语"],
    "url": "test_cet4_words.json",
    "length": 10,
    "language": "en",
    "translateLanguage": "zh-CN"
  }
]
```

**单词数据示例：**
```json
[
  {
    "id": 1,
    "word": "test",
    "phonetic0": "/test/",
    "phonetic1": "/test/",
    "trans": [
      {
        "pos": "n.",
        "cn": "测试，试验"
      }
    ],
    "sentences": [
      {
        "c": "This is a test.",
        "cn": "这是一个测试。"
      }
    ]
  }
]
```

---

## 实际数据导入测试

### 使用真实 CET4 数据

如果您有真实的词库数据文件（如问题中提供的 `1764429285470_recommend_word.json` 和 `CET4_T.json`），可以进行实际导入测试：

```bash
# 1. 将数据文件放在同一目录
cp 1764429285470_recommend_word.json /tmp/word_meta.json
cp CET4_T.json /tmp/

# 2. 运行应用程序
cd build
./WordMaster

# 3. 或者编写简单的测试程序
```

**简单测试代码：**
```cpp
#include "application/services/book_service.h"
#include "infrastructure/repositories/book_repository.h"
#include "infrastructure/repositories/word_repository.h"
#include "infrastructure/sqlite_adapter.h"

int main() {
    // 初始化数据库
    SQLiteAdapter adapter("test.db");
    adapter.open();
    adapter.initializeDatabase("../resources/database/001_initial_schema.sql");
    
    // 创建仓储和服务
    BookRepository bookRepo(adapter);
    WordRepository wordRepo(adapter);
    BookService service(bookRepo, wordRepo);
    
    // 导入真实数据
    auto result = service.importBooksFromMeta("/tmp/word_meta.json");
    
    qDebug() << "Import result:" << result.message;
    qDebug() << "Books imported:" << result.importedBooks;
    qDebug() << "Words imported:" << result.importedWords;
    
    return 0;
}
```

---

## 测试覆盖率

### 生成覆盖率报告

```bash
# 使用覆盖率选项构建
./scripts/build.sh --coverage

# 报告生成在
build/coverage_html/index.html
```

### 当前覆盖率状态

**目标：**
- 核心算法: 100%
- Service 层: > 80%
- Repository 层: > 70%
- 总体: > 70%

---

## 调试技巧

### 1. 启用详细日志

```cpp
// 在测试中添加
QLoggingCategory::setFilterRules("*.debug=true");
```

### 2. 查看 SQL 日志

```bash
# 设置环境变量
export QT_LOGGING_RULES="*.sql.debug=true"
./tests/test_book_import
```

### 3. 使用 GDB 调试

```bash
gdb ./tests/test_book_import
(gdb) run
(gdb) bt  # 查看堆栈
```

### 4. 内存泄漏检查

```bash
valgrind --leak-check=full ./tests/test_book_import
```

---

## 常见问题

### Q: 测试编译失败

**A:** 检查依赖是否完整安装：
```bash
sudo apt install qtbase5-dev libgtest-dev
```

### Q: 数据库锁定错误

**A:** 确保测试使用内存数据库 (":memory:")，避免文件锁

### Q: JSON 解析失败

**A:** 验证 JSON 格式：
```bash
cat test.json | jq .
```

---

## 下一步

✅ **已完成：**
- 数据库基础设施
- Repository 实现
- 词库导入服务
- 单元测试和集成测试

🚧 **进行中：**
- 学习功能实现
- 复习系统实现
- UI 界面开发

📋 **待实现：**
- 标签管理
- 统计分析
- 设置管理

---

## 反馈

如遇到问题，请提交 Issue 并附上：
1. 错误信息
2. 构建日志
3. 测试输出

Happy Testing! 🚀