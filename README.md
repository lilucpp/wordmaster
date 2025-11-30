# 英语单词记忆PC客户端 - 完整产品设计文档 v1.0

**文档信息**  

- **项目名称**：WordMaster（英语单词记忆助手）
- **技术栈**：Qt 5 + C++ + SQLite3
- **目标平台**：Windows (优先) / macOS / Linux
- **设计日期**：2025年11月
- **文档版本**：1.0

## 产品概述

### 产品定位

面向日常英语学习者的离线单词记忆工具，支持多词库管理、科学复习和学习进度追踪。

### 核心价值

- **离线可用**：无需网络，随时学习
- **科学记忆**：基于艾宾浩斯遗忘曲线的复习系统
- **灵活管理**：多词库支持，个性化学习计划
- **数据驱动**：详细学习统计，可视化进度

### 用户画像

- **主要用户**：大学生、考研/出国考试备考者
- **使用场景**：每日10-30分钟碎片化学习
- **使用频率**：每周至少3-5天

------

## 需求分析

### 功能需求（按优先级排序）

#### **P0 - 核心功能（MVP必需）**

1. ✅ 词库管理
   - 导入JSON格式词库（支持word.json元数据和具体词库文件）
   - 词库列表展示（分类、标签、单词数）
   - 词库选择和激活
2. ✅ 单词学习
   - 单词卡片展示（单词、音标、释义、例句）
   - 认识/不认识标记
   - 学习进度保存
3. ✅ 复习系统
   - 基于艾宾浩斯曲线的复习提醒
   - 今日待复习单词列表
   - 复习质量反馈（简单/一般/困难/遗忘）
4. ✅ 学习记录
   - 按词库记录学习进度
   - 已学习/已掌握/待复习状态追踪
   - 每日学习统计

#### **P1 - 重要功能（后续迭代）**

1. ✅ 生词本/错误本/收藏本
   - 标记生词、错词、收藏
   - 分类浏览和管理
   - 批量操作
2. ✅ 学习计划
   - 设置每日学习单词数
   - 自动计算完成目标所需天数
   - 动态调整学习进度
3. ✅ 单词查询
   - 全局搜索单词
   - 模糊匹配
   - 查看单词详情
4. ✅ 统计分析
   - 学习日历热力图
   - 词库掌握度概览
   - 学习时长统计

#### **P2 - 增强功能（可选）**

1. ⏳ 记忆测试

   - 拼写测试
   - 选择题
   - 听写模式

2. ⏳ 发音朗读

   - TTS语音合成
   - 播放控制

3. ⏳ 云同步

   （暂不实现）

   - 跨设备数据同步

### 非功能需求

| 需求类型     | 具体要求                             |
| ------------ | ------------------------------------ |
| **性能**     | 单词查询 < 100ms；界面响应 < 50ms    |
| **可用性**   | 学习流程 ≤ 3次点击；新手上手 < 5分钟 |
| **可靠性**   | 数据自动保存；异常恢复机制           |
| **可维护性** | 模块化设计；代码覆盖率 > 70%         |
| **扩展性**   | 支持新词库格式；支持插件机制         |

------

## 系统架构设计

### 总体架构



```
┌────────────────────────────────────────────────────────────┐
│                    Presentation Layer                      │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │  MainWindow  │  │  StudyWidget │  │ StatisticsWidget│  │
│  │  WordList    │  │  ReviewWidget│  │  SettingsWidget │  │
│  └──────────────┘  └──────────────┘  └─────────────────┘  │
├────────────────────────────────────────────────────────────┤
│                   Application Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │ WordService  │  │ StudyService │  │ ReviewScheduler │  │
│  │BookService   │  │ RecordService│  │ StatisticsService│ │
│  └──────────────┘  └──────────────┘  └─────────────────┘  │
├────────────────────────────────────────────────────────────┤
│                     Domain Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │ Word Entity  │  │ Book Entity  │  │  StudyRecord    │  │
│  │ ReviewPlan   │  │ UserPrefs    │  │  Statistics     │  │
│  └──────────────┘  └──────────────┘  └─────────────────┘  │
├────────────────────────────────────────────────────────────┤
│                 Infrastructure Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │WordRepository│  │BookRepository│  │ RecordRepository│  │
│  │SQLiteAdapter │  │JsonImporter  │  │  FileManager    │  │
│  └──────────────┘  └──────────────┘  └─────────────────┘  │
└────────────────────────────────────────────────────────────┘
```

### 设计原则

**SOLID 原则应用**：

- **S - 单一职责**：每个类只负责一个业务概念
- **O - 开闭原则**：通过接口扩展，不修改已有代码
- **L - 里氏替换**：所有Repository可被Mock替换
- **I - 接口隔离**：最小化接口，避免臃肿
- **D - 依赖倒置**：Service依赖抽象接口，不依赖具体实现

**其他原则**：

- **KISS**：保持实现简单直接
- **YAGNI**：不实现未来可能需要的功能
- **DRY**：避免重复代码

## 数据库设计

### ER 图

```
┌─────────────┐       ┌──────────────┐       ┌─────────────┐
│    books    │1────n │    words     │1────n │study_records│
└─────────────┘       └──────────────┘       └─────────────┘
                             │1                      
                             │                       
                             │n                      
                      ┌──────────────┐               
                      │review_schedule│               
                      └──────────────┘               
                             │1                      
                             │                       
                             │n                      
                      ┌──────────────┐               
                      │  word_tags   │               
                      └──────────────┘
```

### 表结构设计

#### **books（词库元数据表）**

```sql
CREATE TABLE books (
    id TEXT PRIMARY KEY,              -- 词库ID，如 "cet4"
    name TEXT NOT NULL,               -- 显示名称
    description TEXT,                 -- 描述
    category TEXT,                    -- 分类（中国考试/国际考试）
    tags TEXT,                        -- JSON数组，如 ["大学英语"]
    url TEXT NOT NULL,                -- JSON文件路径
    word_count INTEGER DEFAULT 0,     -- 总单词数
    language TEXT DEFAULT 'en',       -- 语言
    translate_language TEXT DEFAULT 'zh-CN',
    imported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT 0       -- 是否当前学习词库
);

CREATE INDEX idx_books_category ON books(category);
CREATE INDEX idx_books_is_active ON books(is_active);
```

#### **words（单词主表）**

```sql
CREATE TABLE words (
    id INTEGER PRIMARY KEY,           -- 单词ID（来自JSON）
    book_id TEXT NOT NULL,            -- 所属词库
    word TEXT NOT NULL,               -- 单词
    phonetic_uk TEXT,                 -- 英式音标
    phonetic_us TEXT,                 -- 美式音标
    translations TEXT,                -- JSON，词性和释义
    sentences TEXT,                   -- JSON，例句数组
    phrases TEXT,                     -- JSON，短语数组
    synonyms TEXT,                    -- JSON，同义词
    related_words TEXT,               -- JSON，派生词
    etymology TEXT,                   -- JSON，词源
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE,
    UNIQUE(book_id, id)              -- 同一词库内ID唯一
);

CREATE INDEX idx_words_word ON words(word);
CREATE INDEX idx_words_book_id ON words(book_id);
CREATE UNIQUE INDEX idx_words_book_word ON words(book_id, word);
```

**字段说明**：

- `translations`：存储trans数组，例如：

```json
  [{"pos": "n.", "cn": "含酒精饮品，酒；酒精，乙醇；醇"}]
```

- `sentences`：存储sentences数组
- `phrases`、`synonyms`、`related_words`、`etymology`：同理

#### **study_records（学习记录表）**

```sql
CREATE TABLE study_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    word_id INTEGER NOT NULL,         -- 关联words表的自增ID
    book_id TEXT NOT NULL,            -- 冗余字段，便于统计
    study_type TEXT NOT NULL,         -- 'learn', 'review', 'test'
    result TEXT NOT NULL,             -- 'known', 'unknown', 'correct', 'wrong'
    study_duration INTEGER DEFAULT 0, -- 学习时长（秒）
    studied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(word_id) REFERENCES words(ROWID) ON DELETE CASCADE,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
);

CREATE INDEX idx_study_records_word_id ON study_records(word_id);
CREATE INDEX idx_study_records_studied_at ON study_records(studied_at);
CREATE INDEX idx_study_records_book_id ON study_records(book_id);
```

#### **review_schedule（复习计划表）**

```sql
CREATE TABLE review_schedule (
    word_id INTEGER PRIMARY KEY,      -- 关联words表
    book_id TEXT NOT NULL,
    next_review_date DATE NOT NULL,   -- 下次复习日期
    review_interval INTEGER DEFAULT 1,-- 复习间隔（天）
    repetition_count INTEGER DEFAULT 0,-- 已复习次数
    easiness_factor REAL DEFAULT 2.5, -- SM-2算法难度系数
    last_review_date DATE,            -- 上次复习日期
    mastery_level INTEGER DEFAULT 0,  -- 0-未学 1-学习中 2-已掌握
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(word_id) REFERENCES words(ROWID) ON DELETE CASCADE,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
);

CREATE INDEX idx_review_next_date ON review_schedule(next_review_date);
CREATE INDEX idx_review_mastery ON review_schedule(mastery_level);
```

**掌握度定义**：

- `0 - 未学习`：从未学习过
- `1 - 学习中`：已学习但未达到掌握标准（复习次数 < 5 或间隔 < 30天）
- `2 - 已掌握`：复习次数 ≥ 5 且间隔 ≥ 30天

#### **word_tags（单词标签表）**

```sql
CREATE TABLE word_tags (
    word_id INTEGER NOT NULL,         -- 关联words表
    tag_type TEXT NOT NULL,           -- 'wrong', 'difficult', 'favorite'
    tagged_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(word_id, tag_type),
    FOREIGN KEY(word_id) REFERENCES words(ROWID) ON DELETE CASCADE
);

CREATE INDEX idx_word_tags_type ON word_tags(tag_type);
```

**标签类型**：

- `wrong`：错误本
- `difficult`：生词本
- `favorite`：收藏本

#### **user_preferences（用户设置表）**

```sql
CREATE TABLE user_preferences (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 预设配置项
INSERT INTO user_preferences (key, value) VALUES
    ('daily_new_words', '20'),        -- 每日新词数
    ('daily_review_words', '50'),     -- 每日复习数
    ('auto_pronounce', '1'),          -- 自动发音
    ('show_phonetic', '1'),           -- 显示音标
    ('theme', 'light');               -- 主题
```

## 核心模块详细设计

### 词库管理模块（BookManagement）

#### **职责**

- 导入和解析JSON词库文件
- 管理词库元数据
- 词库激活/停用

#### **核心类设计**

```cpp
// ============ Domain Layer ============
class Book {
public:
    QString id;
    QString name;
    QString description;
    QString category;
    QStringList tags;
    QString url;
    int wordCount;
    QString language;
    QString translateLanguage;
    QDateTime importedAt;
    bool isActive;
};

// ============ Repository Interface ============
class IBookRepository {
public:
    virtual ~IBookRepository() = default;
    
    // CRUD操作
    virtual bool save(const Book& book) = 0;
    virtual Book getById(const QString& id) = 0;
    virtual QList<Book> getAll() = 0;
    virtual QList<Book> getByCategory(const QString& category) = 0;
    virtual bool updateActive(const QString& id, bool active) = 0;
    virtual bool remove(const QString& id) = 0;
    
    // 统计
    virtual int getTotalWordCount(const QString& bookId) = 0;
    virtual int getLearnedWordCount(const QString& bookId) = 0;
    virtual int getMasteredWordCount(const QString& bookId) = 0;
};

// ============ Application Service ============
class BookService {
public:
    explicit BookService(IBookRepository& bookRepo,
                        IWordRepository& wordRepo);
    
    // 导入词库
    struct ImportResult {
        bool success;
        QString message;
        int importedWords;
    };
    ImportResult importBook(const QString& metaJsonPath);
    
    // 查询
    QList<Book> getAllBooks();
    QList<Book> getBooksByCategory(const QString& category);
    Book getActiveBook();
    
    // 管理
    bool setActiveBook(const QString& bookId);
    bool deleteBook(const QString& bookId);
    
    // 统计
    struct BookStatistics {
        int totalWords;
        int learnedWords;
        int masteredWords;
        double progress; // 0.0 - 1.0
    };
    BookStatistics getBookStatistics(const QString& bookId);
    
private:
    IBookRepository& bookRepo_;
    IWordRepository& wordRepo_;
    
    bool importWordsFromJson(const QString& bookId, 
                            const QString& jsonPath);
};
```

#### **JSON导入流程**

```cpp
BookService::ImportResult BookService::importBook(
    const QString& metaJsonPath) 
{
    ImportResult result{false, "", 0};
    
    // 1. 解析word.json获取词库元数据
    QFile file(metaJsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.message = "无法打开文件";
        return result;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray booksArray = doc.array();
    
    // 2. 遍历每个词库
    for (const QJsonValue& value : booksArray) {
        QJsonObject obj = value.toObject();
        
        Book book;
        book.id = obj["id"].toString();
        book.name = obj["name"].toString();
        book.description = obj["description"].toString();
        book.category = obj["category"].toString();
        
        // 解析tags数组
        QJsonArray tagsArray = obj["tags"].toArray();
        for (const QJsonValue& tag : tagsArray) {
            book.tags.append(tag.toString());
        }
        
        book.url = obj["url"].toString();
        book.wordCount = obj["length"].toInt();
        book.language = obj["language"].toString();
        book.translateLanguage = obj["translateLanguage"].toString();
        
        // 3. 保存词库元数据
        if (!bookRepo_.save(book)) {
            result.message = QString("保存词库元数据失败: %1")
                .arg(book.name);
            continue;
        }
        
        // 4. 导入具体单词数据
        QString bookJsonPath = QFileInfo(metaJsonPath).dir()
            .filePath(book.url);
        if (!importWordsFromJson(book.id, bookJsonPath)) {
            result.message = QString("导入单词失败: %1")
                .arg(book.name);
            continue;
        }
        
        result.importedWords += book.wordCount;
    }
    
    result.success = true;
    result.message = QString("成功导入 %1 个单词")
        .arg(result.importedWords);
    return result;
}

bool BookService::importWordsFromJson(
    const QString& bookId,
    const QString& jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray wordsArray = doc.array();
    
    // 批量插入（使用事务提升性能）
    wordRepo_.beginTransaction();
    
    for (const QJsonValue& value : wordsArray) {
        QJsonObject obj = value.toObject();
        
        Word word;
        word.id = obj["id"].toInt();
        word.bookId = bookId;
        word.word = obj["word"].toString();
        word.phoneticUk = obj["phonetic0"].toString();
        word.phoneticUs = obj["phonetic1"].toString();
        
        // 序列化复杂字段为JSON字符串
        word.translations = QJsonDocument(obj["trans"].toArray())
            .toJson(QJsonDocument::Compact);
        word.sentences = QJsonDocument(obj["sentences"].toArray())
            .toJson(QJsonDocument::Compact);
        word.phrases = QJsonDocument(obj["phrases"].toArray())
            .toJson(QJsonDocument::Compact);
        word.synonyms = QJsonDocument(obj["synos"].toArray())
            .toJson(QJsonDocument::Compact);
        word.relatedWords = QJsonDocument(obj["relWords"].toObject())
            .toJson(QJsonDocument::Compact);
        word.etymology = QJsonDocument(obj["etymology"].toArray())
            .toJson(QJsonDocument::Compact);
        
        if (!wordRepo_.save(word)) {
            wordRepo_.rollback();
            return false;
        }
    }
    
    wordRepo_.commit();
    return true;
}
```

### 5.2 单词学习模块（StudyManagement）

#### **5.2.1 职责**

- 管理学习会话
- 单词展示和翻转
- 学习结果记录

#### **核心类设计**

```cpp
// ============ Domain Layer ============
class StudySession {
public:
    QString sessionId;
    QString bookId;
    QList<int> wordIds;        // 本次学习的单词ID列表
    int currentIndex;          // 当前单词索引
    QDateTime startTime;
    QDateTime endTime;
    
    enum SessionType {
        NewWords,              // 学习新词
        Review,                // 复习
        Test                   // 测试
    };
    SessionType type;
};

class StudyResult {
public:
    int wordId;
    QString bookId;
    enum Quality {
        Unknown = 0,           // 不认识
        Known = 1,             // 认识
        Correct = 2,           // 答对（测试模式）
        Wrong = 3              // 答错（测试模式）
    };
    Quality quality;
    int duration;              // 本单词学习时长（秒）
};

// ============ Application Service ============
class StudyService {
public:
    explicit StudyService(
        IWordRepository& wordRepo,
        IStudyRecordRepository& recordRepo,
        IReviewScheduler& scheduler);
    
    // 开始学习会话
    struct SessionConfig {
        QString bookId;
        StudySession::SessionType type;
        int maxWords;          // 最多学习多少个单词
    };
    StudySession startSession(const SessionConfig& config);
    
    // 获取当前单词
    Word getCurrentWord(const StudySession& session);
    
    // 记录学习结果并移动到下一个
    bool recordAndNext(StudySession& session, 
                      const StudyResult& result);
    
    // 结束会话
    struct SessionSummary {
        int totalWords;
        int knownWords;
        int unknownWords;
        int totalDuration;     // 总时长（秒）
    };
    SessionSummary endSession(const StudySession& session);
    
private:
    IWordRepository& wordRepo_;
    IStudyRecordRepository& recordRepo_;
    IReviewScheduler& scheduler_;
    
    // 选择待学习单词
    QList<int> selectNewWords(const QString& bookId, int count);
    QList<int> selectReviewWords(const QString& bookId, int count);
};
```

#### **学习流程**

```cpp
StudySession StudyService::startSession(const SessionConfig& config) {
    StudySession session;
    session.sessionId = QUuid::createUuid().toString();
    session.bookId = config.bookId;
    session.type = config.type;
    session.currentIndex = 0;
    session.startTime = QDateTime::currentDateTime();
    
    // 根据类型选择单词
    if (config.type == StudySession::NewWords) {
        session.wordIds = selectNewWords(config.bookId, 
                                        config.maxWords);
    } else if (config.type == StudySession::Review) {
        session.wordIds = selectReviewWords(config.bookId, 
                                           config.maxWords);
    }
    
    return session;
}

QList<int> StudyService::selectNewWords(
    const QString& bookId, 
    int count) 
{
    // 查询未学习的单词（review_schedule中不存在的单词）
    QString sql = R"(
        SELECT w.ROWID 
        FROM words w
        LEFT JOIN review_schedule rs ON w.ROWID = rs.word_id
        WHERE w.book_id = ? AND rs.word_id IS NULL
        ORDER BY w.id
        LIMIT ?
    )";
    
    return wordRepo_.executeQuery(sql, {bookId, count});
}

QList<int> StudyService::selectReviewWords(
    const QString& bookId,
    int count)
{
    // 查询今日需要复习的单词
    QString sql = R"(
        SELECT word_id
        FROM review_schedule
        WHERE book_id = ?
          AND next_review_date <= date('now')
          AND mastery_level < 2
        ORDER BY next_review_date ASC
        LIMIT ?
    )";
    
    return wordRepo_.executeQuery(sql, {bookId, count});
}

bool StudyService::recordAndNext(
    StudySession& session,
    const StudyResult& result)
{
    // 1. 记录学习结果
    StudyRecord record;
    record.wordId = result.wordId;
    record.bookId = result.bookId;
    record.studyType = (session.type == StudySession::NewWords) 
        ? "learn" : "review";
    record.result = (result.quality == StudyResult::Known) 
        ? "known" : "unknown";
    record.studyDuration = result.duration;
    
    if (!recordRepo_.save(record)) {
        return false;
    }
    
    // 2. 更新复习计划
    ReviewQuality quality = (result.quality == StudyResult::Known)
        ? ReviewQuality::Good
        : ReviewQuality::Again;
    scheduler_.updateSchedule(result.wordId, quality);
    
    // 3. 移动到下一个单词
    session.currentIndex++;
    
    return true;
}
```

### 复习调度模块（ReviewScheduling）

#### **SuperMemo SM-2 算法**

**算法原理**：

- 根据回答质量调整复习间隔
- 间隔呈指数增长
- 难度系数动态调整

**质量评分（0-5）**：

- 0 - 完全不记得（Again）
- 1 - 记得但很困难（Hard）
- 2 - 记得但有些困难（Hard）
- 3 - 记得，思考后能回忆（Good）
- 4 - 记得很清楚（Good）
- 5 - 完美记住（Easy）

#### **核心类设计**

```cpp
// ============ Domain Layer ============
class ReviewPlan {
public:
    int wordId;
    QString bookId;
    QDate nextReviewDate;
    int reviewInterval;        // 天数
    int repetitionCount;
    double easinessFactor;     // EF系数
    QDate lastReviewDate;
    int masteryLevel;
};

enum class ReviewQuality {
    Again = 0,                 // 完全不记得
    Hard = 2,                  // 困难
    Good = 3,                  // 良好
    Easy = 5                   // 简单
};

// ============ Repository Interface ============
class IReviewScheduleRepository {
public:
    virtual ~IReviewScheduleRepository() = default;
    
    virtual bool save(const ReviewPlan& plan) = 0;
    virtual ReviewPlan get(int wordId) = 0;
    virtual bool exists(int wordId) = 0;
    virtual QList<int> getTodayReviewWords(const QString& bookId) = 0;
    virtual QList<int> getOverdueWords(const QString& bookId) = 0;
};

// ============ Application Service ============
class SM2ReviewScheduler : public IReviewScheduler {
public:
    explicit SM2ReviewScheduler(
        IReviewScheduleRepository& repo);
    
    // 获取今日复习单词
    QList<int> getTodayReviewWords(
        const QString& bookId) override;
    
    // 更新复习计划
    void updateSchedule(int wordId, 
                       ReviewQuality quality) override;
    
    // 初始化新单词的复习计划
    void initializeSchedule(int wordId, 
                          const QString& bookId);
    
private:
    IReviewScheduleRepository& repo_;
    
    // SM-2算法核心计算
    struct SM2Result {
        int interval;
        double easinessFactor;
        int repetitionCount;
    };
    SM2Result calculateSM2(int currentInterval,
                          double currentEF,
                          int repetitionCount,
                          ReviewQuality quality);
};
```

#### **SM-2 算法实现**

```cpp
void SM2ReviewScheduler::updateSchedule(
    int wordId,
    ReviewQuality quality)
{
    ReviewPlan plan = repo_.get(wordId);
    
    // 如果是新单词，初始化
    if (!repo_.exists(wordId)) {
        initializeSchedule(wordId, plan.bookId);
        plan = repo_.get(wordId);
    }
    
    // 应用SM-2算法
    SM2Result result = calculateSM2(
        plan.reviewInterval,
        plan.easinessFactor,
        plan.repetitionCount,
        quality
    );
    
    // 更新复习计划
    plan.lastReviewDate = QDate::currentDate();
    plan.reviewInterval = result.interval;
    plan.easinessFactor = result.easinessFactor;
    plan.repetitionCount = result.repetitionCount;
    plan.nextReviewDate = QDate::currentDate()
        .addDays(result.interval);
    
    // 更新掌握度
    if (plan.repetitionCount >= 5 && plan.reviewInterval >= 30) {
        plan.masteryLevel = 2;  //已掌握
    } else if (plan.repetitionCount > 0) {
    plan.masteryLevel = 1;  // 学习中
    }
    repo_.save(plan);
}

SM2ReviewScheduler::SM2Result SM2ReviewScheduler::calculateSM2(
int currentInterval,
double currentEF,
int repetitionCount,
ReviewQuality quality)
{
  SM2Result result;
  int q = static_cast<int>(quality);
	// 1. 计算新的EF值
  // EF' = EF + (0.1 - (5-q) * (0.08 + (5-q) * 0.02))
  double newEF = currentEF + (0.1 - (5 - q) * (0.08 + (5 - q) * 0.02));
  result.easinessFactor = qMax(1.3, newEF);  // EF最小值为1.3

  // 2. 计算新的间隔
  if (q < 3) {
      // 回答质量差，重新开始
      result.interval = 1;
      result.repetitionCount = 0;
  } else {
      // 回答质量好，增加间隔
      result.repetitionCount = repetitionCount + 1;

      if (result.repetitionCount == 1) {
          result.interval = 1;
      } else if (result.repetitionCount == 2) {
          result.interval = 6;
      } else {
          // I(n) = I(n-1) * EF
          result.interval = qRound(currentInterval * result.easinessFactor);
      }
  }

  return result;
}

void SM2ReviewScheduler::initializeSchedule(
int wordId,
const QString& bookId)
{
  ReviewPlan plan;
  plan.wordId = wordId;
  plan.bookId = bookId;
  plan.nextReviewDate = QDate::currentDate();
  plan.reviewInterval = 1;
  plan.repetitionCount = 0;
  plan.easinessFactor = 2.5;
  plan.masteryLevel = 0;
  repo_.save(plan);
}

```

---

---

### 统计分析模块（Statistics）

#### **核心类设计**
```cpp
class StatisticsService {
public:
    explicit StatisticsService(
        IStudyRecordRepository& recordRepo,
        IReviewScheduleRepository& scheduleRepo);
    
    // 每日统计
    struct DailyStats {
        QDate date;
        int newWords;          // 新学单词数
        int reviewWords;       // 复习单词数
        int studyMinutes;      // 学习时长（分钟）
    };
    QList<DailyStats> getDailyStats(const QDate& startDate,
                                   const QDate& endDate);
    
    // 词库统计
    struct BookProgress {
        QString bookId;
        QString bookName;
        int totalWords;
        int learnedWords;      // 学习中+已掌握
        int masteredWords;     // 已掌握
        double progress;       // 0.0 - 1.0
    };
    QList<BookProgress> getAllBooksProgress();
    
    // 学习热力图数据
    struct HeatmapData {
        QDate date;
        int intensity;         // 0-4，学习强度
    };
    QList<HeatmapData> getHeatmapData(int recentDays);
    
    // 今日概览
    struct TodayOverview {
        int newWordsToLearn;   // 计划学习新词数
        int wordsToReview;     // 待复习数
        int studiedToday;      // 今日已学
        int reviewedToday;     // 今日已复习
        int streak;            // 连续学习天数
    };
    TodayOverview getTodayOverview();
    
private:
    IStudyRecordRepository& recordRepo_;
    IReviewScheduleRepository& scheduleRepo_;
};
```

---

###  标签管理模块（TagManagement）

#### **核心类设计**
```cpp
class TagService {
public:
    explicit TagService(IWordTagRepository& tagRepo);
    
    // 添加/移除标签
    bool addTag(int wordId, const QString& tagType);
    bool removeTag(int wordId, const QString& tagType);
    bool hasTag(int wordId, const QString& tagType);
    
    // 查询
    QList<int> getWordsByTag(const QString& tagType);
    QStringList getWordTags(int wordId);
    
    // 批量操作
    bool addTags(const QList<int>& wordIds, 
                const QString& tagType);
    bool removeTags(const QList<int>& wordIds,
                   const QString& tagType);
    
    // 统计
    int getTagCount(const QString& tagType);
    
    // 预定义标签类型
    static const QString TAG_WRONG;       // "wrong"
    static const QString TAG_DIFFICULT;   // "difficult"
    static const QString TAG_FAVORITE;    // "favorite"
    
private:
    IWordTagRepository& tagRepo_;
};
```

---

## 用户界面设计

###  主窗口布局

```txt
┌────────────────────────────────────────────────────────────┐
│  WordMaster                                    □  －  ×    │
├────────────────────────────────────────────────────────────┤
│ ┌────────┐                                                 │
│ │ 📚词库  │  ┌────────────────────────────────────────┐   │
│ │        │  │                                        │   │
│ │ 📖学习  │  │                                        │   │
│ │        │  │                                        │   │
│ │ 🔄复习  │  │          主内容区                      │   │
│ │        │  │                                        │   │
│ │ 📊统计  │  │                                        │   │
│ │        │  │                                        │   │
│ │ 📝本子  │  │                                        │   │
│ │        │  │                                        │   │
│ │ ⚙️设置  │  └────────────────────────────────────────┘   │
│ └────────┘                                                 │
└────────────────────────────────────────────────────────────┘
```

###  核心界面设计

#### **词库管理界面**

```txt
┌─────────────────────────────────────────────────────────┐
│  词库管理                           [+导入词库] [刷新]   │
├─────────────────────────────────────────────────────────┤
│ 分类: [全部▼] [中国考试] [国际考试] [其他]             │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ ☑️ CET-4                         [当前词库]      │   │
│  │    大学英语四级词库                              │   │
│  │    📊 2607词  ✅ 1200  📚 800  📈 46%           │   │
│  │    [开始学习] [查看详情] [删除]                  │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ ☐ 雅思听力 Day5                                  │   │
│  │    xdf听力 雅思中级直通车 day5                   │   │
│  │    📊 38词  ✅ 0  📚 0  📈 0%                    │   │
│  │    [开始学习] [查看详情] [删除]                  │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**图例说明**：
- ☑️：当前激活词库
- 📊：总单词数
- ✅：已学习数
- 📚：已掌握数
- 📈：进度百分比

#### **单词学习界面**

```txt
┌─────────────────────────────────────────────────────────┐
│  学习模式: [新词学习▼]          进度: 15/20  75%        │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                      [🔊 发音]                          │
│                                                         │
│                      alcohol                            │
│                   /ˈælkəhɒl/                            │
│                                                         │
│                  [显示释义 ↓]                           │
│                                                         │
│  ┌───────────────────────────────────────────────┐     │
│  │ 翻转后显示:                                    │     │
│  │                                                │     │
│  │ n. 含酒精饮品，酒；酒精，乙醇；醇              │     │
│  │                                                │     │
│  │ 例句:                                          │     │
│  │ Wine contains about 10% alcohol.               │     │
│  │ 葡萄酒含有约10%的酒精。                        │     │
│  │                                                │     │
│  │ 短语:                                          │     │
│  │ • ethyl alcohol  乙醇；酒精                    │     │
│  │ • alcohol content  酒精含量                    │     │
│  └───────────────────────────────────────────────┘     │
│                                                         │
│  [❌ 不认识]  [⭐ 收藏]  [✅ 认识]                      │
│                                                         │
│               [上一个]  [下一个]                        │
└─────────────────────────────────────────────────────────┘

```

**交互流程**：
1. 初始状态：只显示单词和音标
2. 点击"显示释义"：展开释义、例句等
3. 点击"认识/不认识"：记录结果，自动进入下一个单词
4. 支持快捷键：Space（翻转）、←/→（上/下一个）、1（不认识）、2（认识）

#### **复习界面**

```txt
┌─────────────────────────────────────────────────────────┐
│  复习模式                       今日待复习: 25词        │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                      [🔊 发音]                          │
│                                                         │
│                    democracy                            │
│                  /dɪˈmɒkrəsi/                           │
│                                                         │
│                  [显示释义 ↓]                           │
│                                                         │
│  ┌───────────────────────────────────────────────┐     │
│  │ n. 民主，民主制度；民主国家                    │     │
│  └───────────────────────────────────────────────┘     │
│                                                         │
│  回答质量:                                              │
│  [🔴 遗忘]  [🟡 困难]  [🟢 良好]  [🔵 简单]            │
│                                                         │
│  下次复习: 根据选择动态显示                             │
│  • 遗忘: 1天后    • 困难: 3天后                        │
│  • 良好: 7天后    • 简单: 15天后                       │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

#### **统计界面**

```txt
┌─────────────────────────────────────────────────────────┐
│  学习统计                      时间范围: [最近30天▼]    │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────┐   │
│  │ 今日概览                                        │   │
│  │ ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐   │   │
│  │ │ 新学   │ │ 复习   │ │ 学习   │ │ 连续   │   │   │
│  │ │  15    │ │  25    │ │ 30min  │ │ 7天    │   │   │
│  │ └────────┘ └────────┘ └────────┘ └────────┘   │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ 学习热力图 (GitHub风格)                         │   │
│  │                                                 │   │
│  │ 一 ░░▓░░░░▓▓░░░░░                              │   │
│  │ 二 ░▓▓░░▓░░░▓░░░░                              │   │
│  │ 三 ▓░░▓░░▓░░░░░░░                              │   │
│  │ 四 ░░▓▓░░░▓░░▓░░░                              │   │
│  │ 五 ░▓░░▓░▓░░░░░░░                              │   │
│  │ 六 ▓░░░░▓░░▓░░░░░                              │   │
│  │ 日 ░░▓░░░▓░░░▓░░░                              │   │
│  │                                                 │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ 词库进度                                        │   │
│  │                                                 │   │
│  │ CET-4          ████████░░  80% (2085/2607)     │   │
│  │ 雅思听力Day5   ██░░░░░░░░  20% (8/38)          │   │
│  │                                                 │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

#### **本子管理界面**

```txt
┌─────────────────────────────────────────────────────────┐
│  [错误本] [生词本] [收藏本]                             │
├─────────────────────────────────────────────────────────┤
│ 搜索: [____________]                       [批量删除]   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ alcohol      /ˈælkəhɒl/              [🔊] [×]  │   │
│  │ n. 含酒精饮品，酒；酒精，乙醇                   │   │
│  │ 添加时间: 2025-11-20  来源: CET-4               │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ democracy    /dɪˈmɒkrəsi/            [🔊] [×]  │   │
│  │ n. 民主，民主制度；民主国家                     │   │
│  │ 添加时间: 2025-11-19  来源: CET-4               │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│                      ... 更多单词 ...                   │
│                                                         │
│                    [1] [2] [3] ... [10]                 │
└─────────────────────────────────────────────────────────┘

```

---

## 开发计划

### 阶段划分（按TDD方式迭代）

#### **第一阶段：基础框架（2周）**

**目标**：搭建可运行的最小系统

**任务清单**：

- [ ] 项目初始化（CMake + Qt5）
- [ ] 数据库设计和迁移脚本
- [ ] SQLite适配器实现
- [ ] 单元测试框架搭建
- [ ] 主窗口框架和导航

**验收标准**：
- ✅ 数据库表创建成功
- ✅ 能执行基本SQL查询
- ✅ 主界面显示正常
- ✅ 单元测试运行通过

**核心代码示例**：

```cpp
// tests/test_sqlite_adapter.cpp
TEST(SQLiteAdapterTest, CreateConnection) {
    SQLiteAdapter adapter(":memory:");
    EXPECT_TRUE(adapter.isOpen());
}

TEST(SQLiteAdapterTest, ExecuteQuery) {
    SQLiteAdapter adapter(":memory:");
    QString sql = "CREATE TABLE test (id INTEGER PRIMARY KEY)";
    EXPECT_TRUE(adapter.executeQuery(sql));
}

// src/infrastructure/sqlite_adapter.h
class SQLiteAdapter {
public:
    explicit SQLiteAdapter(const QString& dbPath);
    ~SQLiteAdapter();
    
    bool isOpen() const;
    QSqlDatabase& getConnection();
    bool executeQuery(const QString& sql);
    QSqlQuery prepareQuery(const QString& sql);
    
    bool beginTransaction();
    bool commit();
    bool rollback();
    
private:
    QSqlDatabase db_;
    QString connectionName_;
};
```

---

#### **第二阶段：词库管理（2周）**

**目标**：完成词库导入和管理功能

**TDD流程**：

**1. 编写失败的测试**：
```cpp
// tests/test_book_service.cpp
TEST(BookServiceTest, ImportBookFromJson) {
    MockBookRepository bookRepo;
    MockWordRepository wordRepo;
    BookService service(bookRepo, wordRepo);
    
    auto result = service.importBook("testdata/word.json");
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.importedWords, 0);
}

TEST(BookServiceTest, GetBookStatistics) {
    // ... 测试统计功能
}
```

**2. 实现最少代码**：
```cpp
// src/application/book_service.cpp
BookService::ImportResult BookService::importBook(
    const QString& metaJsonPath) 
{
    // 实现导入逻辑
    // ...
    return result;
}
```

**3. 验证测试通过**：
```bash
$ ctest --output-on-failure
Test project /build
    Start 1: BookServiceTest.ImportBookFromJson
1/1 Test #1: BookServiceTest.ImportBookFromJson .....   Passed
```

**4. 必要时重构**：
```cpp
// 提取JSON解析逻辑到单独的类
class JsonParser {
public:
    static QList<Book> parseBookMetadata(const QString& path);
    static QList<Word> parseWords(const QString& path);
};
```

**任务清单**：
- [ ] Book实体和Repository接口定义
- [ ] JSON导入器实现（测试驱动）
- [ ] BookService实现
- [ ] 词库管理UI实现
- [ ] 集成测试

**验收标准**：
- ✅ 能成功导入word.json和词库文件
- ✅ 单词数据正确存储到数据库
- ✅ UI能显示词库列表
- ✅ 能查看词库统计信息

---

#### **第三阶段：学习功能（2周）**

**目标**：实现单词卡片学习流程

**TDD流程**：
```cpp
// tests/test_study_service.cpp
TEST(StudyServiceTest, StartNewWordsSession) {
    MockWordRepository wordRepo;
    MockStudyRecordRepository recordRepo;
    MockReviewScheduler scheduler;
    
    StudyService service(wordRepo, recordRepo, scheduler);
    
    StudySession::Config config;
    config.bookId = "cet4";
    config.type = StudySession::NewWords;
    config.maxWords = 20;
    
    auto session = service.startSession(config);
    
    EXPECT_EQ(session.bookId, "cet4");
    EXPECT_EQ(session.wordIds.size(), 20);
}

TEST(StudyServiceTest, RecordStudyResult) {
    // ... 测试学习结果记录
}
```

**任务清单**：
- [ ] StudySession和StudyResult实体
- [ ] StudyService实现（TDD）
- [ ] 学习UI实现（单词卡片）
- [ ] 键盘快捷键支持
- [ ] 学习记录持久化

**验收标准**：
- ✅ 能开始学习会话
- ✅ 单词卡片正确显示
- ✅ 学习结果正确记录
- ✅ 进度正确更新

---

#### **第四阶段：复习系统（2周）**

**目标**：实现SM-2复习算法

**TDD流程**：
```cpp
// tests/test_sm2_scheduler.cpp
TEST(SM2SchedulerTest, CalculateIntervalForQuality3) {
    SM2ReviewScheduler scheduler(mockRepo);
    
    auto result = scheduler.calculateSM2(
        1,      // currentInterval
        2.5,    // currentEF
        0,      // repetitionCount
        ReviewQuality::Good  // quality = 3
    );
    
    EXPECT_EQ(result.interval, 1);  // 第一次复习间隔为1天
    EXPECT_EQ(result.repetitionCount, 1);
    EXPECT_NEAR(result.easinessFactor, 2.5, 0.1);
}

TEST(SM2SchedulerTest, CalculateIntervalForQuality5) {
    // 测试"简单"的情况
}

TEST(SM2SchedulerTest, ResetOnBadQuality) {
    // 测试回答错误时重置间隔
}
```

**任务清单**：
- [ ] ReviewPlan实体和Repository
- [ ] SM-2算法实现（TDD）
- [ ] ReviewScheduler服务
- [ ] 复习UI实现
- [ ] 今日复习列表

**验收标准**：
- ✅ SM-2算法计算正确
- ✅ 复习计划正确更新
- ✅ 今日待复习单词列表准确
- ✅ 复习质量反馈生效

---

#### **第五阶段：统计和标签（2周）**

**目标**：实现学习统计和标签管理

**任务清单**：
- [ ] StatisticsService实现
- [ ] 热力图组件开发
- [ ] 进度条和图表
- [ ] TagService实现
- [ ] 错误本/生词本/收藏本UI

**验收标准**：
- ✅ 统计数据准确
- ✅ 热力图正确显示
- ✅ 标签功能正常

---

#### **第六阶段：优化和测试（1周）**

**任务清单**：
- [ ] 性能优化（数据库索引）
- [ ] UI/UX优化
- [ ] 异常处理完善
- [ ] 用户手册编写
- [ ] 打包发布脚本

---

### 技术债务管理

**待优化项**：

1. **数据库性能**：大词库查询优化
2. **内存管理**：图片和缓存策略
3. **UI响应性**：耗时操作异步化
4. **代码覆盖率**：目标 > 70%

---

## 关键技术决策

### 为什么使用JSON存储复杂字段？

**问题**：单词的例句、短语等是数组结构

**方案对比**：

| 方案            | 优点           | 缺点           | 决策 |
| --------------- | -------------- | -------------- | ---- |
| 关系表          | 规范化，易查询 | 表多，JOIN复杂 | ❌    |
| JSON字段        | 简单，灵活     | 查询能力弱     | ✅    |
| Protocol Buffer | 高效，类型安全 | 增加复杂度     | ❌    |

**选择理由**：
- 复杂字段仅用于展示，无需查询
- SQLite3原生支持JSON
- 简化数据模型

### 为什么选择SM-2而非SM-18？

**对比**：

| 算法  | 复杂度 | 效果 | 实现成本 | 决策       |
| ----- | ------ | ---- | -------- | ---------- |
| SM-2  | 简单   | 良好 | 低       | ✅          |
| SM-18 | 复杂   | 更好 | 高       | ❌          |
| Anki  | 中等   | 很好 | 中       | ⏳ 未来考虑 |

**选择理由**：
- MVP阶段优先简单可靠
- SM-2已被广泛验证
- 后续可无缝升级到Anki算法

### 为什么用Qt Widgets而非QML？

**对比**：

| 技术       | 适用场景      | 学习曲线 | 性能      | 决策 |
| ---------- | ------------- | -------- | --------- | ---- |
| Qt Widgets | 传统桌面应用  | 平缓     | 稳定      | ✅    |
| QML        | 现代UI/移动端 | 陡峭     | 高（GPU） | ❌    |

**选择理由**：
- 项目需求以功能为主，UI要求不高
- Widgets生态成熟，组件丰富
- 便于快速迭代

### 数据库事务策略

**批量导入使用事务**：
```cpp
wordRepo_.beginTransaction();
for (const Word& word : words) {
    wordRepo_.save(word);
}
wordRepo_.commit();
```

**单次操作不使用事务**：
```cpp
// 学习记录单条插入，无需事务
recordRepo_.save(record);
```

---

## 风险管理

### 风险识别

| 风险           | 概率 | 影响 | 应对措施              |
| -------------- | ---- | ---- | --------------------- |
| JSON格式不一致 | 高   | 中   | 健壮的解析器+错误提示 |
| 大词库性能问题 | 中   | 高   | 分页+索引优化         |
| 复习算法不适配 | 中   | 中   | 支持参数调整          |
| 跨平台兼容性   | 低   | 高   | CI自动化测试          |
| 数据丢失       | 低   | 高   | 自动备份机制          |

### 应对策略

**1. JSON解析健壮性**：
```cpp
// 防御式编程
QJsonObject obj = value.toObject();
word.id = obj.value("id").toInt(0);  // 默认值
if (obj.contains("phonetic0")) {
    word.phoneticUk = obj["phonetic0"].toString();
}
```

**2. 性能优化**：
```sql
-- 建立索引
CREATE INDEX idx_words_word ON words(word);
CREATE INDEX idx_review_next_date ON review_schedule(next_review_date);

-- 分页查询
SELECT * FROM words LIMIT 100 OFFSET 0;
```

**3. 数据备份**：
```cpp
class BackupService {
public:
    bool createBackup(const QString& targetPath);
    bool restoreBackup(const QString& backupPath);
private:
    // 定期自动备份到 ~/WordMaster/backups/
};
```

---

## 测试策略

### 测试金字塔

```txt
        /\
       /  \     E2E Tests (10%)
      /────\    - 完整学习流程测试
     /      \   
    /────────\  Integration Tests (30%)
   /          \ - Service + Repository集成
  /────────────
 /              \ Unit Tests (60%)
/────────────────\ - 算法、实体、工具类
```

### 单元测试示例
```cpp
// tests/domain/test_sm2_algorithm.cpp
class SM2AlgorithmTest : public ::testing::Test {
protected:
    SM2ReviewScheduler scheduler_;
};

TEST_F(SM2AlgorithmTest, FirstReview_Quality3_ReturnsInterval1) {
    auto result = scheduler_.calculateSM2(0, 2.5, 0, ReviewQuality::Good);
EXPECT_EQ(result.interval, 1);
  
TEST_F(SM2AlgorithmTest, SecondReview_Quality3_ReturnsInterval6) {
  auto result = scheduler_.calculateSM2(1, 2.5, 1,
  ReviewQuality::Good);
  EXPECT_EQ(result.interval, 6);
}
TEST_F(SM2AlgorithmTest, WrongAnswer_ResetsInterval) {
  auto result = scheduler_.calculateSM2(10, 2.5, 5,
  ReviewQuality::Again);
  EXPECT_EQ(result.interval, 1);
  EXPECT_EQ(result.repetitionCount, 0);
}
```

### 集成测试示例
```cpp
// tests/integration/test_study_flow.cpp
TEST(StudyFlowIntegrationTest, CompleteStudySession) {
    // 1. 导入词库
    BookService bookService(realBookRepo, realWordRepo);
    auto importResult = bookService.importBook("testdata/word.json");
    ASSERT_TRUE(importResult.success);
    
    // 2. 开始学习
    StudyService studyService(realWordRepo, realRecordRepo, 
                             realScheduler);
    auto session = studyService.startSession({
        .bookId = "cet4",
        .type = StudySession::NewWords,
        .maxWords = 5
    });
    ASSERT_EQ(session.wordIds.size(), 5);
    
    // 3. 学习所有单词
    for (int i = 0; i < session.wordIds.size(); i++) {
        Word word = studyService.getCurrentWord(session);
        ASSERT_FALSE(word.word.isEmpty());
        
        studyService.recordAndNext(session, {
            .wordId = word.rowId,
            .bookId = "cet4",
            .quality = StudyResult::Known,
            .duration = 10
        });
    }
    
    // 4. 验证学习记录
    auto summary = studyService.endSession(session);
    EXPECT_EQ(summary.totalWords, 5);
    
    // 5. 验证复习计划
    auto todayReview = realScheduler.getTodayReviewWords("cet4");
    EXPECT_EQ(todayReview.size(), 5);  // 今天学的明天要复习
}
```

### 测试覆盖率目标
```bash
# 使用gcov + lcov生成覆盖率报告
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
$ make
$ ctest
$ lcov --capture --directory . --output-file coverage.info
$ genhtml coverage.info --output-directory coverage_html

# 目标覆盖率
- 核心算法（SM-2）: 100%
- Service层: > 80%
- Repository层: > 70%
- 总体: > 70%
```

---

## 部署和打包

### 项目结构

```txt
WordMaster/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── docs/
│   ├── design.md
│   └── user_manual.md
├── src/
│   ├── main.cpp
│   ├── domain/
│   │   ├── entities/
│   │   └── interfaces/
│   ├── application/
│   │   └── services/
│   ├── infrastructure/
│   │   ├── repositories/
│   │   └── sqlite_adapter.cpp
│   └── presentation/
│       ├── mainwindow.cpp
│       └── widgets/
├── tests/
│   ├── unit/
│   ├── integration/
│   └── testdata/
├── resources/
│   ├── icons/
│   ├── qss/
│   └── database/
│       └── migrations/
└── scripts/
├── build.sh
└── package.sh
```

### CMake配置
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(WordMaster VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt5
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql 
    Multimedia
)

# 源文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/infrastructure/sqlite_adapter.cpp
    src/application/book_service.cpp
    src/application/study_service.cpp
    src/application/sm2_scheduler.cpp
    # ... 更多源文件
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    Qt5::Multimedia
)

# 测试
enable_testing()
add_subdirectory(tests)

# 安装
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
```

### Windows打包
```bash
# scripts/package_windows.bat
@echo off
set QT_DIR=C:\Qt\5.15.2\msvc2019_64

REM 编译
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release

REM 拷贝依赖
%QT_DIR%\bin\windeployqt.exe Release\WordMaster.exe

REM 打包
"C:\Program Files\NSIS\makensis.exe" ..\installer.nsi
```

### macOS打包
```bash
# scripts/package_macos.sh
#!/bin/bash
QT_DIR=~/Qt/5.15.2/clang_64

# 编译
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

# 创建App Bundle
macdeployqt WordMaster.app -dmg

# 代码签名（可选）
# codesign --deep --force --verify --verbose \
#   --sign "Developer ID" WordMaster.app
```

---

## 未来扩展

### 短期计划（3-6个月）

1. **发音朗读**：集成TTS引擎
2. **记忆测试**：拼写、选择题模式
3. **主题切换**：深色模式支持
4. **快捷键自定义**

### 中期计划（6-12个月）

1. **云同步**：跨设备数据同步
2. **词汇扩展**：支持自定义词库
3. **社区分享**：词库分享平台
4. **AI助手**：例句生成、释义优化

### 长期愿景（1年+）

1. **移动端**：iOS/Android版本
2. **浏览器插件**：划词记忆
3. **社区功能**：学习打卡、排行榜
4. **多语言支持**：日语、韩语等

---

## 总结

### 核心设计原则

✅ **单一职责**：每个类只做一件事  
✅ **依赖倒置**：面向接口编程  
✅ **测试驱动**：先写测试再写代码  
✅ **迭代开发**：小步快跑，持续交付  

### 质量保证

- **代码审查**：每次提交前自审
- **自动化测试**：CI/CD流水线
- **性能监控**：关键操作耗时统计
- **用户反馈**：快速响应迭代

### 下一步行动

请您确认：
1. ✅ 对整体设计方案是否满意？
2. ✅ 是否需要调整功能优先级？
3. ✅ 是否有其他特殊需求？

**我将基于您的反馈：**
- 开始第一阶段代码实现
- 提供详细的类图和时序图
- 编写核心模块的代码示例
- 制定详细的开发任务清单

---

**让我们一起打造专业、好用的单词记忆工具！** 🚀📚