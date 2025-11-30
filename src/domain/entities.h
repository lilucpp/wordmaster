#ifndef WORDMASTER_DOMAIN_ENTITIES_H
#define WORDMASTER_DOMAIN_ENTITIES_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDate>

namespace WordMaster {
namespace Domain {

// ============================================
// Book Entity - 词库实体
// ============================================
struct Book {
    QString id;                     // 词库ID，如 "cet4"
    QString name;                   // 显示名称
    QString description;            // 描述
    QString category;               // 分类
    QStringList tags;               // 标签数组
    QString url;                    // JSON文件路径
    int wordCount;                  // 总单词数
    QString language;               // 语言
    QString translateLanguage;      // 翻译语言
    QDateTime importedAt;           // 导入时间
    bool isActive;                  // 是否当前激活
    
    Book() : wordCount(0), isActive(false) {}
    
    bool isValid() const {
        return !id.isEmpty() && !name.isEmpty() && !url.isEmpty();
    }
};

// ============================================
// Word Entity - 单词实体
// ============================================
struct Word {
    int id;                         // 数据库自增ID
    QString bookId;                 // 所属词库
    int wordId;                     // JSON中的原始ID
    QString word;                   // 单词
    QString phoneticUk;             // 英式音标
    QString phoneticUs;             // 美式音标
    QString translations;           // JSON字符串：trans数组
    QString sentences;              // JSON字符串：sentences数组
    QString phrases;                // JSON字符串：phrases数组
    QString synonyms;               // JSON字符串：synos数组
    QString relatedWords;           // JSON字符串：relWords对象
    QString etymology;              // JSON字符串：etymology数组
    QDateTime createdAt;            // 创建时间
    
    Word() : id(0), wordId(0) {}
    
    bool isValid() const {
        return !word.isEmpty() && !bookId.isEmpty();
    }
};

// ============================================
// StudyRecord Entity - 学习记录实体
// ============================================
struct StudyRecord {
    int id;                         // 记录ID
    int wordId;                     // 单词ID
    QString bookId;                 // 词库ID
    
    enum class Type {
        Learn,                      // 学习新词
        Review,                     // 复习
        Test                        // 测试
    };
    Type studyType;
    
    enum class Result {
        Known,                      // 认识
        Unknown,                    // 不认识
        Correct,                    // 答对（测试）
        Wrong                       // 答错（测试）
    };
    Result result;
    
    int studyDuration;              // 学习时长（秒）
    QDateTime studiedAt;            // 学习时间
    
    StudyRecord() : id(0), wordId(0), studyType(Type::Learn), 
                    result(Result::Unknown), studyDuration(0) {}
    
    static QString typeToString(Type type) {
        switch(type) {
            case Type::Learn: return "learn";
            case Type::Review: return "review";
            case Type::Test: return "test";
            default: return "learn";
        }
    }
    
    static Type stringToType(const QString& str) {
        if (str == "review") return Type::Review;
        if (str == "test") return Type::Test;
        return Type::Learn;
    }
    
    static QString resultToString(Result res) {
        switch(res) {
            case Result::Known: return "known";
            case Result::Unknown: return "unknown";
            case Result::Correct: return "correct";
            case Result::Wrong: return "wrong";
            default: return "unknown";
        }
    }
    
    static Result stringToResult(const QString& str) {
        if (str == "known") return Result::Known;
        if (str == "correct") return Result::Correct;
        if (str == "wrong") return Result::Wrong;
        return Result::Unknown;
    }
};

// ============================================
// ReviewPlan Entity - 复习计划实体
// ============================================
struct ReviewPlan {
    int wordId;                     // 单词ID
    QString bookId;                 // 词库ID
    QDate nextReviewDate;           // 下次复习日期
    int reviewInterval;             // 复习间隔（天）
    int repetitionCount;            // 已复习次数
    double easinessFactor;          // SM-2难度系数
    QDate lastReviewDate;           // 上次复习日期
    
    enum class MasteryLevel {
        NotLearned = 0,             // 未学习
        Learning = 1,               // 学习中
        Mastered = 2                // 已掌握
    };
    MasteryLevel masteryLevel;
    
    QDateTime createdAt;            // 创建时间
    QDateTime updatedAt;            // 更新时间
    
    ReviewPlan() : wordId(0), reviewInterval(1), repetitionCount(0),
                   easinessFactor(2.5), masteryLevel(MasteryLevel::NotLearned) {}
    
    static int masteryLevelToInt(MasteryLevel level) {
        return static_cast<int>(level);
    }
    
    static MasteryLevel intToMasteryLevel(int level) {
        if (level == 1) return MasteryLevel::Learning;
        if (level == 2) return MasteryLevel::Mastered;
        return MasteryLevel::NotLearned;
    }
};

// ============================================
// ReviewQuality Enum - 复习质量评分
// ============================================
enum class ReviewQuality {
    Again = 0,                      // 完全不记得
    Hard = 3,                       // 困难
    Good = 4,                       // 良好
    Easy = 5                        // 简单
};

// ============================================
// WordTag Entity - 单词标签实体
// ============================================
struct WordTag {
    int wordId;                     // 单词ID
    QString tagType;                // 标签类型
    QDateTime taggedAt;             // 标记时间
    
    // 预定义标签类型
    static const QString TAG_WRONG;      // "wrong"
    static const QString TAG_DIFFICULT;  // "difficult"
    static const QString TAG_FAVORITE;   // "favorite"
    
    WordTag() : wordId(0) {}
};

// 静态常量定义
inline const QString WordTag::TAG_WRONG = "wrong";
inline const QString WordTag::TAG_DIFFICULT = "difficult";
inline const QString WordTag::TAG_FAVORITE = "favorite";

// ============================================
// UserPreference Entity - 用户设置实体
// ============================================
struct UserPreference {
    QString key;                    // 配置键
    QString value;                  // 配置值
    QDateTime updatedAt;            // 更新时间
    
    UserPreference() {}
    UserPreference(const QString& k, const QString& v) 
        : key(k), value(v) {}
};

} // namespace Domain
} // namespace WordMaster

#endif // WORDMASTER_DOMAIN_ENTITIES_H