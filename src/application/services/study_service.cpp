#include "study_service.h"
#include <QUuid>
#include <QDebug>

namespace WordMaster {
namespace Application {

StudyService::StudyService(Domain::IWordRepository& wordRepo,
                           Domain::IStudyRecordRepository& recordRepo,
                           SM2Scheduler& scheduler)
    : wordRepo_(wordRepo)
    , recordRepo_(recordRepo)
    , scheduler_(scheduler)
{
}

StudyService::StudySession StudyService::startSession(
    const QString& bookId,
    StudySession::Type type,
    int maxWords)
{
    StudySession session;
    session.sessionId = QUuid::createUuid().toString();
    session.bookId = bookId;
    session.type = type;
    session.currentIndex = 0;
    session.startTime = QDateTime::currentDateTime();
    
    // 根据类型选择单词
    if (type == StudySession::NewWords) {
        // 学习新词：获取未学习的单词
        session.wordIds = scheduler_.getUnlearnedWords(bookId, maxWords);
        qDebug() << "Starting new words session:" << session.wordIds.size() << "words";
    } else {
        // 复习：获取今日待复习的单词
        session.wordIds = scheduler_.getTodayReviewWords(bookId);
        
        qDebug() << "Found" << session.wordIds.size() << "words to review for book:" << bookId;
        
        // 限制数量
        if (session.wordIds.size() > maxWords) {
            session.wordIds = session.wordIds.mid(0, maxWords);
        }
        
        qDebug() << "Starting review session:" << session.wordIds.size() << "words";
        
        // 调试：打印前5个单词ID
        if (!session.wordIds.isEmpty()) {
            qDebug() << "First word IDs:" << session.wordIds.mid(0, qMin(5, session.wordIds.size()));
        }
    }
    
    return session;
}

Domain::Word StudyService::getCurrentWord(const StudySession& session) {
    if (!session.hasNext()) {
        return Domain::Word();
    }
    
    int wordId = session.getCurrentWordId();
    return wordRepo_.getById(wordId);
}

bool StudyService::recordAndNext(StudySession& session, 
                                  const StudyResult& result) 
{
    // 记录学习结果
    if (!recordStudyResult(result, session.type)) {
        return false;
    }
    
    // 移动到下一个
    session.moveNext();
    
    return true;
}

StudyService::SessionSummary StudyService::endSession(
    const StudySession& session) 
{
    SessionSummary summary;
    
    // 获取本次会话的所有记录
    QList<Domain::StudyRecord> records = recordRepo_.getByBookId(session.bookId);
    
    // 统计本次会话的数据
    for (const Domain::StudyRecord& record : records) {
        // 只统计本次会话开始后的记录
        // 兼容 Qt 版本，取秒级时间戳
        qint64 studiedAtSecs=0;
        qint64 sessionStartSecs=0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        studiedAtSecs = record.studiedAt.toSecsSinceEpoch();
        sessionStartSecs = session.startTime.toSecsSinceEpoch();
#else
        studiedAtSecs = record.studiedAt.toTime_t();
        sessionStartSecs = session.startTime.toTime_t();
#endif

        if (studiedAtSecs >= sessionStartSecs) {
            // 检查是否是本次会话的单词
            if (session.wordIds.contains(record.wordId)) {
                summary.totalWords++;
                summary.totalDuration += record.studyDuration;
                
                if (record.result == Domain::StudyRecord::Result::Known) {
                    summary.knownWords++;
                } else {
                    summary.unknownWords++;
                }
            }
        }
    }
    
    qDebug() << "Session ended:" 
             << "total=" << summary.totalWords
             << ", known=" << summary.knownWords
             << ", unknown=" << summary.unknownWords
             << ", duration=" << summary.totalDuration << "s";
    
    return summary;
}

StudyService::TodayStats StudyService::getTodayStats(const QString& bookId) {
    TodayStats stats;
    
    stats.newWordsLearned = recordRepo_.getTodayLearnCount(bookId);
    stats.wordsReviewed = recordRepo_.getTodayReviewCount(bookId);
    stats.totalDuration = recordRepo_.getTotalStudyDuration(QDate::currentDate());
    
    return stats;
}

bool StudyService::recordStudyResult(const StudyResult& result,
                                      StudySession::Type sessionType)
{
    // 1. 创建学习记录
    Domain::StudyRecord record;
    record.wordId = result.wordId;
    record.bookId = result.bookId;
    record.studyType = (sessionType == StudySession::NewWords)
        ? Domain::StudyRecord::Type::Learn
        : Domain::StudyRecord::Type::Review;
    record.result = result.known 
        ? Domain::StudyRecord::Result::Known
        : Domain::StudyRecord::Result::Unknown;
    record.studyDuration = result.duration;
    
    // 保存学习记录
    if (!recordRepo_.save(record)) {
        qWarning() << "Failed to save study record";
        return false;
    }
    
    // 2. 更新复习计划
    if (sessionType == StudySession::NewWords) {
        // 学习新词：初始化复习计划
        scheduler_.initializeSchedule(result.wordId, result.bookId);
        
        // 根据结果更新复习计划
        Domain::ReviewQuality quality = result.known
            ? Domain::ReviewQuality::Good
            : Domain::ReviewQuality::Again;
        scheduler_.updateSchedule(result.wordId, quality);
    } else {
        // 复习：更新复习计划
        // 根据结果确定复习质量
        Domain::ReviewQuality quality;
        if (result.known) {
            // 认识：根据学习时长判断难度
            if (result.duration < 3) {
                quality = Domain::ReviewQuality::Easy;  // < 3秒，很简单
            } else if (result.duration < 10) {
                quality = Domain::ReviewQuality::Good;  // < 10秒，良好
            } else {
                quality = Domain::ReviewQuality::Hard;  // >= 10秒，有点难
            }
        } else {
            quality = Domain::ReviewQuality::Again;  // 不认识，重新开始
        }
        
        scheduler_.updateSchedule(result.wordId, quality);
    }
    
    qDebug() << "Recorded study result for word" << result.wordId 
             << "known:" << result.known;
    
    return true;
}

} // namespace Application
} // namespace WordMaster