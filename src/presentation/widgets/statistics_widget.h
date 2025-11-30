#ifndef WORDMASTER_PRESENTATION_STATISTICS_WIDGET_H
#define WORDMASTER_PRESENTATION_STATISTICS_WIDGET_H

#include <QWidget>
#include <QLabel>
#include "application/services/book_service.h"
#include "domain/repositories.h"

namespace WordMaster {
namespace Presentation {

/**
 * @brief 统计界面（简化版）
 * 
 * 显示：
 * - 今日学习统计
 * - 词库进度
 */
class StatisticsWidget : public QWidget {
    Q_OBJECT

public:
    explicit StatisticsWidget(Application::BookService* bookService,
                             Domain::IStudyRecordRepository* recordRepo,
                             QWidget* parent = nullptr);

    void refresh();

private:
    void setupUI();
    void loadStatistics();

    Application::BookService* bookService_;
    Domain::IStudyRecordRepository* recordRepo_;
    
    // UI 组件
    QLabel* titleLabel_;
    QWidget* todayStatsWidget_;
    QWidget* booksProgressWidget_;
};

} // namespace Presentation
} // namespace WordMaster

#endif // WORDMASTER_PRESENTATION_STATISTICS_WIDGET_H