#ifndef WORDMASTER_PRESENTATION_MAINWINDOW_H
#define WORDMASTER_PRESENTATION_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <memory>

#include "application/services/book_service.h"
#include "application/services/study_service.h"
#include "application/services/sm2_scheduler.h"
#include "infrastructure/sqlite_adapter.h"
#include "infrastructure/repositories/book_repository.h"
#include "infrastructure/repositories/word_repository.h"
#include "infrastructure/repositories/study_record_repository.h"
#include "infrastructure/repositories/review_schedule_repository.h"

namespace WordMaster {
namespace Presentation {

// 前向声明
class BookListWidget;
class StudyWidget;
class ReviewWidget;
class StatisticsWidget;

/**
 * @brief 主窗口
 * 
 * 布局：左侧导航 + 右侧内容区
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNavigationClicked(int index);
    void onBookSelected(const QString& bookId);
    void onImportBooks();
    void onStartStudy();
    void onStartReview();

private:
    void setupUI();
    void setupNavigation();
    void setupContentArea();
    void setupConnections();
    void initializeDatabase();
    void loadInitialData();

    // UI 组件
    QWidget* centralWidget_;
    QListWidget* navigationList_;
    QStackedWidget* contentStack_;
    
    // 内容页面
    BookListWidget* bookListWidget_;
    StudyWidget* studyWidget_;
    ReviewWidget* reviewWidget_;
    StatisticsWidget* statsWidget_;
    
    // 数据库和服务
    std::unique_ptr<Infrastructure::SQLiteAdapter> adapter_;
    std::unique_ptr<Infrastructure::BookRepository> bookRepo_;
    std::unique_ptr<Infrastructure::WordRepository> wordRepo_;
    std::unique_ptr<Infrastructure::StudyRecordRepository> recordRepo_;
    std::unique_ptr<Infrastructure::ReviewScheduleRepository> scheduleRepo_;
    
    std::unique_ptr<Application::BookService> bookService_;
    std::unique_ptr<Application::SM2Scheduler> scheduler_;
    std::unique_ptr<Application::StudyService> studyService_;
    
    // 状态
    QString currentBookId_;
};

} // namespace Presentation
} // namespace WordMaster

#endif // WORDMASTER_PRESENTATION_MAINWINDOW_H