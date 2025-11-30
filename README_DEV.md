# WordMaster 开发文档

## 快速开始

### 环境要求

- **操作系统**: Linux (Ubuntu 20.04+ 推荐)
- **编译器**: GCC 7+ 或 Clang 5+ (支持 C++17)
- **Qt**: Qt 5.12+ (推荐 5.15.2 LTS)
- **CMake**: 3.16+
- **Google Test**: 1.10+ (用于单元测试)

### 安装依赖 (Ubuntu/Debian)

```bash
# 基础开发工具
sudo apt update
sudo apt install -y build-essential cmake git

# Qt5
sudo apt install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

# Google Test
sudo apt install -y libgtest-dev

# 代码覆盖率工具（可选）
sudo apt install -y lcov
```

### 构建项目

```bash
# 克隆项目
git clone <repository-url>
cd WordMaster

# 赋予构建脚本执行权限
chmod +x scripts/build.sh

# Debug 构建（包含测试）
./scripts/build.sh

# Release 构建
./scripts/build.sh --release

# 清理构建
./scripts/build.sh --clean

# 生成覆盖率报告
./scripts/build.sh --coverage
```

### 运行测试

```bash
cd build
ctest --output-on-failure

# 或运行单个测试
./tests/test_sqlite_adapter
```

### 运行应用

```bash
cd build
./WordMaster
```

### 使用 CLI 工具测试真实数据

```bash
# 快速测试（推荐）
./scripts/quick_test.sh /path/to/1764429285470_recommend_word.json

# 或手动测试
cd build
./wordmaster_cli --import /path/to/1764429285470_recommend_word.json
./wordmaster_cli --list
./wordmaster_cli --stats cet4

# 验证数据完整性
./scripts/verify_data.sh build/wordmaster.db
```

**详细 CLI 使用说明：** 参见 [CLI_USAGE_GUIDE.md](CLI_USAGE_GUIDE.md)

---

## 项目结构

```
WordMaster/
├── CMakeLists.txt              # 根 CMake 配置
├── README.md                   # 用户文档
├── README_DEV.md              # 开发文档（本文件）
│
├── src/                        # 源代码
│   ├── main.cpp               # 主程序入口
│   │
│   ├── domain/                # 领域层
│   │   ├── entities.h         # 实体定义
│   │   └── repositories.h     # 仓储接口
│   │
│   ├── application/           # 应用层
│   │   └── services/          # 业务服务
│   │
│   ├── infrastructure/        # 基础设施层
│   │   ├── sqlite_adapter.h   # 数据库适配器
│   │   ├── sqlite_adapter.cpp
│   │   └── repositories/      # 仓储实现
│   │
│   └── presentation/          # 表示层
│       ├── mainwindow.h       # 主窗口
│       └── widgets/           # UI 组件
│
├── tests/                      # 测试代码
│   ├── CMakeLists.txt         # 测试 CMake 配置
│   ├── test_helpers.h         # 测试辅助工具
│   ├── test_helpers.cpp
│   │
│   ├── unit/                  # 单元测试
│   │   └── test_sqlite_adapter.cpp
│   │
│   └── integration/           # 集成测试
│
├── resources/                  # 资源文件
│   ├── database/              # 数据库脚本
│   │   └── 001_initial_schema.sql
│   ├── icons/                 # 图标
│   └── qss/                   # 样式表
│
└── scripts/                    # 构建脚本
    ├── build.sh               # 构建脚本
    └── package.sh             # 打包脚本（待实现）
```

---

## 架构设计

### 分层架构

```
┌─────────────────────────────────────────┐
│        Presentation Layer               │  UI 组件
│  (MainWindow, Widgets)                  │
├─────────────────────────────────────────┤
│        Application Layer                │  业务逻辑
│  (Services)                             │
├─────────────────────────────────────────┤
│        Domain Layer                     │  领域模型
│  (Entities, Interfaces)                 │
├─────────────────────────────────────────┤
│        Infrastructure Layer             │  技术实现
│  (SQLite, Repositories)                 │
└─────────────────────────────────────────┘
```

### 依赖关系

- **Presentation → Application → Domain ← Infrastructure**
- 依赖倒置：Application 依赖 Domain 的接口，Infrastructure 实现这些接口
- 松耦合：通过接口隔离各层

---

## 开发规范

### 编码风格

- 遵循 C++17 标准
- 类名使用 PascalCase: `BookService`
- 函数名使用 camelCase: `getActiveBook()`
- 成员变量使用下划线后缀: `dbPath_`
- 常量使用 UPPER_SNAKE_CASE: `TAG_WRONG`

### 命名约定

- **接口**: `I` 前缀，如 `IBookRepository`
- **实体**: 简单名词，如 `Book`, `Word`
- **服务**: `Service` 后缀，如 `BookService`
- **仓储**: `Repository` 后缀，如 `BookRepository`
- **测试**: `Test` 后缀，如 `BookServiceTest`

### Git 提交规范

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Type:**
- `feat`: 新功能
- `fix`: 修复 bug
- `docs`: 文档更新
- `style`: 代码格式
- `refactor`: 重构
- `test`: 测试相关
- `chore`: 构建/工具变更

**示例:**
```
feat(database): add SQLite adapter

- Implement connection management
- Add transaction support
- Include error handling

Closes #1
```

---

## TDD 开发流程

### 1. 编写失败的测试

```cpp
TEST(BookServiceTest, ImportBook) {
    // Arrange
    MockBookRepository bookRepo;
    BookService service(bookRepo);
    
    // Act
    auto result = service.importBook("test.json");
    
    // Assert
    EXPECT_TRUE(result.success);
}
```

### 2. 实现最少代码

```cpp
BookService::ImportResult BookService::importBook(
    const QString& path) 
{
    ImportResult result;
    result.success = true;
    return result;
}
```

### 3. 运行测试

```bash
cd build
./tests/test_book_service
```

### 4. 重构优化

- 提取重复代码
- 优化命名
- 添加注释

### 5. 重复循环

---

## 数据库操作

### 执行查询

```cpp
SQLiteAdapter adapter("database.db");
adapter.open();

// 简单查询
auto query = adapter.query("SELECT * FROM books");
while (query.next()) {
    QString name = query.value("name").toString();
}

// 预处理语句
auto stmt = adapter.prepare(
    "INSERT INTO books (id, name) VALUES (?, ?)"
);
stmt.addBindValue("cet4");
stmt.addBindValue("CET-4");
stmt.exec();
```

### 事务处理

```cpp
adapter.beginTransaction();

try {
    // 执行多个操作
    adapter.execute("INSERT ...");
    adapter.execute("UPDATE ...");
    
    adapter.commit();
} catch (...) {
    adapter.rollback();
}
```

---

## 常见问题

### Q: 编译时找不到 Qt 头文件

**A:** 确保已安装 Qt5 开发包：
```bash
sudo apt install qtbase5-dev
```

### Q: 测试失败：找不到 gtest

**A:** 安装 Google Test：
```bash
sudo apt install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib
```

### Q: 数据库文件位置

**A:** 
- 开发环境：`build/wordmaster.db`
- 生产环境：`~/.local/share/WordMaster/wordmaster.db`

---

## 下一步开发

### 迭代 2: 词库导入（当前）

- [ ] BookRepository 实现
- [ ] WordRepository 实现
- [ ] BookService 实现
- [ ] JSON 解析和导入
- [ ] 集成测试

### 迭代 3: 学习功能

- [ ] StudyService 实现
- [ ] 学习会话管理
- [ ] 学习记录持久化
- [ ] 基础 UI 原型

### 迭代 4: 复习系统

- [ ] SM-2 算法实现
- [ ] ReviewScheduler 服务
- [ ] 复习流程集成

---

## 联系方式

如有问题，请提交 Issue 或 Pull Request。

**Happy Coding! 🚀**