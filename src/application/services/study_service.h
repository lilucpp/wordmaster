#ifndef WORDMASTER_APPLICATION_STUDY_SERVICE_H
#define WORDMASTER_APPLICATION_STUDY_SERVICE_H

#include "domain/repositories.h"
#include "domain/entities.h"
#include "sm2_scheduler.h"
#include <QDateTime>

namespace WordMaster {
namespace Application {

/**
 * @brief 学习服务
 * 
 * 职责：
 * - 管理学习会话
 * - 单词展示流程
 * - 学习结果记录
 * - 复习调度集成
 */
class StudyService {
public:
    /**
     * @brief 学习会话
     */
    struct StudySession {
        QString sessionId;         // 会话ID
        QString bookId;            // 词库ID
        QList<int> wordIds;        // 本次学习的单词ID列表
        int currentIndex;          // 当前单词索引
        QDateTime startTime;       // 开始时间
        
        enum Type {
            NewWords,              // 学习新词
            Review                 // 复习
        };
        Type type;
        
        StudySession() : currentIndex(0), type(NewWords) {}
        
        bool hasNext() const {
            return currentIndex < wordIds.size();
        }
        
        bool hasPrevious() const {
            return currentIndex > 0;
        }
        
        int getCurrentWordId() const {
            if (currentIndex >= 0 && currentIndex < wordIds.size()) {
                return wordIds[currentIndex];
            }
            return 0;
        }
        
        void moveNext() {
            if (hasNext()) {
                currentIndex++;
            }
        }
        
        void movePrevious() {
            if (hasPrevious()) {
                currentIndex--;
            }
        }
        
        int getProgress() const {
            return currentIndex;
        }
        
        int getTotal() const {
            return wordIds.size();
        }
    };
    
    /**
     * @brief 学习结果
     */
    struct StudyResult {
        int wordId;
        QString bookId;
        bool known;                // true=认识, false=不认识
        int duration;              // 本单词学习时长（秒）
        
        StudyResult() : wordId(0), known(false), duration(0) {}
    };
    
    /**
     * @brief 会话总结
     */
    struct SessionSummary {
        int totalWords;
        int knownWords;
        int unknownWords;
        int totalDuration;         // 总时长（秒）
        
        SessionSummary() : totalWords(0), knownWords(0), 
                          unknownWords(0), totalDuration(0) {}
    };
    
    explicit StudyService(Domain::IWordRepository& wordRepo,
                         Domain::IStudyRecordRepository& recordRepo,
                         SM2Scheduler& scheduler);
    
    /**
     * @brief 开始学习会话
     * @param bookId 词库ID
     * @param type 会话类型
     * @param maxWords 最多学习多少个单词
     * @return 学习会话
     */
    StudySession startSession(const QString& bookId,
                             StudySession::Type type,
                             int maxWords);
    
    /**
     * @brief 获取当前单词
     * @param session 学习会话
     * @return 单词对象
     */
    Domain::Word getCurrentWord(const StudySession& session);
    
    /**
     * @brief 记录学习结果并移动到下一个
     * @param session 学习会话
     * @param result 学习结果
     * @return 是否成功
     */
    bool recordAndNext(StudySession& session, const StudyResult& result);
    
    /**
     * @brief 结束会话
     * @param session 学习会话
     * @return 会话总结
     */
    SessionSummary endSession(const StudySession& session);
    
    /**
     * @brief 获取今日学习统计
     * @param bookId 词库ID
     */
    struct TodayStats {
        int newWordsLearned;
        int wordsReviewed;
        int totalDuration;
        
        TodayStats() : newWordsLearned(0), wordsReviewed(0), totalDuration(0) {}
    };
    TodayStats getTodayStats(const QString& bookId);

private:
    Domain::IWordRepository& wordRepo_;
    Domain::IStudyRecordRepository& recordRepo_;
    SM2Scheduler& scheduler_;
    
    // 记录学习结果的内部实现
    bool recordStudyResult(const StudyResult& result, 
                          StudySession::Type sessionType);
};

} // namespace Application
} // namespace WordMaster

#endif // WORDMASTER_APPLICATION_STUDY_SERVICE_H