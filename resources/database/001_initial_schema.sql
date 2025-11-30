-- ============================================
-- WordMaster 数据库初始化脚本
-- Version: 1.0.0
-- ============================================

-- 1. 词库元数据表
CREATE TABLE IF NOT EXISTS books (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    category TEXT,
    tags TEXT,                              -- JSON数组
    url TEXT NOT NULL,
    word_count INTEGER DEFAULT 0,
    language TEXT DEFAULT 'en',
    translate_language TEXT DEFAULT 'zh-CN',
    imported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT 0
);

CREATE INDEX idx_books_category ON books(category);
CREATE INDEX idx_books_is_active ON books(is_active);

-- 2. 单词主表
CREATE TABLE IF NOT EXISTS words (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 自增ID作为主键
    book_id TEXT NOT NULL,
    word_id INTEGER NOT NULL,               -- JSON中的原始ID
    word TEXT NOT NULL,
    phonetic_uk TEXT,
    phonetic_us TEXT,
    translations TEXT,                      -- JSON: trans数组
    sentences TEXT,                         -- JSON: sentences数组
    phrases TEXT,                           -- JSON: phrases数组
    synonyms TEXT,                          -- JSON: synos数组
    related_words TEXT,                     -- JSON: relWords对象
    etymology TEXT,                         -- JSON: etymology数组
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE,
    UNIQUE(book_id, word_id)                -- 同一词库内word_id唯一
);

CREATE INDEX idx_words_word ON words(word);
CREATE INDEX idx_words_book_id ON words(book_id);
CREATE UNIQUE INDEX idx_words_book_word ON words(book_id, word);

-- 3. 学习记录表
CREATE TABLE IF NOT EXISTS study_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    word_id INTEGER NOT NULL,               -- 关联words表的自增ID
    book_id TEXT NOT NULL,
    study_type TEXT NOT NULL,               -- 'learn', 'review', 'test'
    result TEXT NOT NULL,                   -- 'known', 'unknown', 'correct', 'wrong'
    study_duration INTEGER DEFAULT 0,       -- 学习时长（秒）
    studied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
);

CREATE INDEX idx_study_records_word_id ON study_records(word_id);
CREATE INDEX idx_study_records_studied_at ON study_records(studied_at);
CREATE INDEX idx_study_records_book_id ON study_records(book_id);

-- 4. 复习计划表
CREATE TABLE IF NOT EXISTS review_schedule (
    word_id INTEGER PRIMARY KEY,            -- 关联words表
    book_id TEXT NOT NULL,
    next_review_date DATE NOT NULL,
    review_interval INTEGER DEFAULT 1,      -- 复习间隔（天）
    repetition_count INTEGER DEFAULT 0,     -- 已复习次数
    easiness_factor REAL DEFAULT 2.5,       -- SM-2难度系数
    last_review_date DATE,
    mastery_level INTEGER DEFAULT 0,        -- 0-未学 1-学习中 2-已掌握
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE,
    FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE
);

CREATE INDEX idx_review_next_date ON review_schedule(next_review_date);
CREATE INDEX idx_review_mastery ON review_schedule(mastery_level);
CREATE INDEX idx_review_book_id ON review_schedule(book_id);

-- 5. 单词标签表（生词本/错误本/收藏本）
CREATE TABLE IF NOT EXISTS word_tags (
    word_id INTEGER NOT NULL,
    tag_type TEXT NOT NULL,                 -- 'wrong', 'difficult', 'favorite'
    tagged_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(word_id, tag_type),
    FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE
);

CREATE INDEX idx_word_tags_type ON word_tags(tag_type);

-- 6. 用户设置表
CREATE TABLE IF NOT EXISTS user_preferences (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 插入默认配置
INSERT OR IGNORE INTO user_preferences (key, value) VALUES
    ('daily_new_words', '20'),
    ('daily_review_words', '50'),
    ('auto_pronounce', '1'),
    ('show_phonetic', '1'),
    ('theme', 'light');

-- 7. 创建视图：今日学习统计
CREATE VIEW IF NOT EXISTS v_today_stats AS
SELECT 
    book_id,
    COUNT(CASE WHEN study_type = 'learn' THEN 1 END) as new_words_count,
    COUNT(CASE WHEN study_type = 'review' THEN 1 END) as review_words_count,
    SUM(study_duration) as total_duration
FROM study_records
WHERE DATE(studied_at) = DATE('now')
GROUP BY book_id;

-- 8. 创建视图：词库进度统计
CREATE VIEW IF NOT EXISTS v_book_progress AS
SELECT 
    b.id as book_id,
    b.name as book_name,
    b.word_count as total_words,
    COUNT(DISTINCT rs.word_id) as learned_words,
    COUNT(CASE WHEN rs.mastery_level = 2 THEN 1 END) as mastered_words,
    ROUND(CAST(COUNT(DISTINCT rs.word_id) AS REAL) / b.word_count * 100, 2) as progress
FROM books b
LEFT JOIN review_schedule rs ON b.id = rs.book_id
GROUP BY b.id;

-- 完成
SELECT 'Database schema initialized successfully' as status;