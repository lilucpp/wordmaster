#include "review_schedule_repository.h"
#include <QDebug>

namespace WordMaster {
namespace Infrastructure {

ReviewScheduleRepository::ReviewScheduleRepository(SQLiteAdapter& adapter)
    : adapter_(adapter)
{
}

bool ReviewScheduleRepository::save(const Domain::ReviewPlan& plan) {
    QString sql = R"(
        INSERT OR REPLACE INTO review_schedule 
        (word_id, book_id, next_review_date, review_interval, 
         repetition_count, easiness_factor, last_review_date, 
         mastery_level, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(plan.wordId);
    query.addBindValue(plan.bookId);
    query.addBindValue(plan.nextReviewDate.toString(Qt::ISODate));
    query.addBindValue(plan.reviewInterval);
    query.addBindValue(plan.repetitionCount);
    query.addBindValue(plan.easinessFactor);
    query.addBindValue(plan.lastReviewDate.isValid() 
        ? plan.lastReviewDate.toString(Qt::ISODate) 
        : QVariant());
    query.addBindValue(Domain::ReviewPlan::masteryLevelToInt(plan.masteryLevel));
    
    if (!query.exec()) {
        qWarning() << "Failed to save review plan:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Domain::ReviewPlan ReviewScheduleRepository::get(int wordId) {
    QString sql = "SELECT * FROM review_schedule WHERE word_id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query review plan:" << query.lastError().text();
        return Domain::ReviewPlan();
    }
    
    if (query.next()) {
        return buildPlanFromQuery(query);
    }
    
    return Domain::ReviewPlan();
}

bool ReviewScheduleRepository::remove(int wordId) {
    QString sql = "DELETE FROM review_schedule WHERE word_id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete review plan:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool ReviewScheduleRepository::exists(int wordId) {
    QString sql = "SELECT COUNT(*) as cnt FROM review_schedule WHERE word_id = ?";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(wordId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt() > 0;
    }
    
    return false;
}

QList<int> ReviewScheduleRepository::getTodayReviewWords(const QString& bookId) {
    QList<int> wordIds;
    
    QString sql = R"(
        SELECT word_id FROM review_schedule
        WHERE book_id = ?
          AND next_review_date <= DATE('now')
          AND mastery_level < 2
        ORDER BY next_review_date ASC, repetition_count ASC
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query today review words:" << query.lastError().text();
        return wordIds;
    }
    
    while (query.next()) {
        wordIds.append(query.value("word_id").toInt());
    }
    
    qDebug() << "Today review words for" << bookId << ":" << wordIds.size();
    
    return wordIds;
}

QList<int> ReviewScheduleRepository::getOverdueWords(const QString& bookId) {
    QList<int> wordIds;
    
    QString sql = R"(
        SELECT word_id FROM review_schedule
        WHERE book_id = ?
          AND next_review_date < DATE('now')
          AND mastery_level < 2
        ORDER BY next_review_date ASC
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query overdue words:" << query.lastError().text();
        return wordIds;
    }
    
    while (query.next()) {
        wordIds.append(query.value("word_id").toInt());
    }
    
    return wordIds;
}

QList<int> ReviewScheduleRepository::getUnlearnedWords(
    const QString& bookId, 
    int limit) 
{
    QList<int> wordIds;
    
    QString sql = R"(
        SELECT w.id FROM words w
        LEFT JOIN review_schedule rs ON w.id = rs.word_id
        WHERE w.book_id = ?
          AND rs.word_id IS NULL
        ORDER BY w.word_id
    )";
    
    if (limit > 0) {
        sql += QString(" LIMIT %1").arg(limit);
    }
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query unlearned words:" << query.lastError().text();
        return wordIds;
    }
    
    while (query.next()) {
        wordIds.append(query.value("id").toInt());
    }
    
    return wordIds;
}

int ReviewScheduleRepository::getLearnedCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(*) as cnt FROM review_schedule
        WHERE book_id = ?
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

int ReviewScheduleRepository::getMasteredCount(const QString& bookId) {
    QString sql = R"(
        SELECT COUNT(*) as cnt FROM review_schedule
        WHERE book_id = ? AND mastery_level = 2
    )";
    
    auto query = adapter_.prepare(sql);
    query.addBindValue(bookId);
    
    if (query.exec() && query.next()) {
        return query.value("cnt").toInt();
    }
    
    return 0;
}

int ReviewScheduleRepository::getTodayReviewCount(const QString& bookId) {
    return getTodayReviewWords(bookId).size();
}

Domain::ReviewPlan ReviewScheduleRepository::buildPlanFromQuery(QSqlQuery& query) {
    Domain::ReviewPlan plan;
    
    plan.wordId = query.value("word_id").toInt();
    plan.bookId = query.value("book_id").toString();
    plan.nextReviewDate = QDate::fromString(
        query.value("next_review_date").toString(), 
        Qt::ISODate
    );
    plan.reviewInterval = query.value("review_interval").toInt();
    plan.repetitionCount = query.value("repetition_count").toInt();
    plan.easinessFactor = query.value("easiness_factor").toDouble();
    
    QString lastReviewStr = query.value("last_review_date").toString();
    if (!lastReviewStr.isEmpty()) {
        plan.lastReviewDate = QDate::fromString(lastReviewStr, Qt::ISODate);
    }
    
    plan.masteryLevel = Domain::ReviewPlan::intToMasteryLevel(
        query.value("mastery_level").toInt()
    );
    plan.createdAt = query.value("created_at").toDateTime();
    plan.updatedAt = query.value("updated_at").toDateTime();
    
    return plan;
}

} // namespace Infrastructure
} // namespace WordMaster