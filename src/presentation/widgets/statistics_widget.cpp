#include "statistics_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QProgressBar>

namespace WordMaster {
namespace Presentation {

StatisticsWidget::StatisticsWidget(Application::BookService* bookService,
                                  Domain::IStudyRecordRepository* recordRepo,
                                  QWidget* parent)
    : QWidget(parent)
    , bookService_(bookService)
    , recordRepo_(recordRepo)
{
    setupUI();
    loadStatistics();
}

void StatisticsWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // 标题
    titleLabel_ = new QLabel("学习统计", this);
    titleLabel_->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel_);
    
    // 今日统计
    auto* todayGroup = new QGroupBox("今日概览", this);
    todayGroup->setStyleSheet("QGroupBox { font-size: 16px; font-weight: bold; }");
    todayStatsWidget_ = new QWidget(todayGroup);
    auto* todayLayout = new QVBoxLayout(todayGroup);
    todayLayout->addWidget(todayStatsWidget_);
    mainLayout->addWidget(todayGroup);
    
    // 词库进度
    auto* booksGroup = new QGroupBox("词库进度", this);
    booksGroup->setStyleSheet("QGroupBox { font-size: 16px; font-weight: bold; }");
    booksProgressWidget_ = new QWidget(booksGroup);
    auto* booksLayout = new QVBoxLayout(booksGroup);
    booksLayout->addWidget(booksProgressWidget_);
    mainLayout->addWidget(booksGroup);
    
    mainLayout->addStretch();
}

void StatisticsWidget::loadStatistics() {
    // 今日统计
    auto todayRecords = recordRepo_->getTodayRecords();
    int learnCount = 0;
    int reviewCount = 0;
    int totalDuration = 0;
    
    for (const auto& record : todayRecords) {
        if (record.studyType == Domain::StudyRecord::Type::Learn) {
            learnCount++;
        } else if (record.studyType == Domain::StudyRecord::Type::Review) {
            reviewCount++;
        }
        totalDuration += record.studyDuration;
    }
    
    // 显示今日统计
    auto* todayLayout = new QHBoxLayout(todayStatsWidget_);
    
    auto* learnLabel = new QLabel(QString("📖 新学: %1").arg(learnCount));
    learnLabel->setStyleSheet("font-size: 18px; padding: 10px;");
    todayLayout->addWidget(learnLabel);
    
    auto* reviewLabel = new QLabel(QString("🔄 复习: %1").arg(reviewCount));
    reviewLabel->setStyleSheet("font-size: 18px; padding: 10px;");
    todayLayout->addWidget(reviewLabel);
    
    auto* durationLabel = new QLabel(QString("⏱️ 时长: %1分钟")
        .arg(totalDuration / 60));
    durationLabel->setStyleSheet("font-size: 18px; padding: 10px;");
    todayLayout->addWidget(durationLabel);
    
    todayLayout->addStretch();
    
    // 词库进度
    auto* booksLayout = new QVBoxLayout(booksProgressWidget_);
    auto statsList = bookService_->getAllBooksStatistics();
    
    for (const auto& stats : statsList) {
        auto* bookWidget = new QWidget();
        auto* bookLayout = new QVBoxLayout(bookWidget);
        
        // 词库名称和统计
        auto* infoLayout = new QHBoxLayout();
        auto* nameLabel = new QLabel(stats.bookName);
        nameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
        infoLayout->addWidget(nameLabel);
        
        auto* statsLabel = new QLabel(QString("%1 / %2 (掌握: %3)")
            .arg(stats.learnedWords)
            .arg(stats.totalWords)
            .arg(stats.masteredWords));
        statsLabel->setStyleSheet("color: #666;");
        infoLayout->addWidget(statsLabel);
        infoLayout->addStretch();
        
        bookLayout->addLayout(infoLayout);
        
        // 进度条
        auto* progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(static_cast<int>(stats.progress * 100));
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString("%1%").arg(static_cast<int>(stats.progress * 100)));
        progressBar->setStyleSheet(R"(
            QProgressBar {
                border: 1px solid #ddd;
                border-radius: 3px;
                text-align: center;
                height: 20px;
            }
            QProgressBar::chunk {
                background-color: #4caf50;
            }
        )");
        bookLayout->addWidget(progressBar);
        
        booksLayout->addWidget(bookWidget);
    }
    
    if (statsList.isEmpty()) {
        auto* emptyLabel = new QLabel("暂无数据");
        emptyLabel->setStyleSheet("color: #999;");
        booksLayout->addWidget(emptyLabel);
    }
}

void StatisticsWidget::refresh() {
    // 获取父布局的引用（在删除widget之前）
    QLayout* todayParentLayout = todayStatsWidget_->parentWidget()->layout();
    QLayout* booksParentLayout = booksProgressWidget_->parentWidget()->layout();
    
    // 从布局中移除并删除旧widget
    todayParentLayout->removeWidget(todayStatsWidget_);
    booksParentLayout->removeWidget(booksProgressWidget_);
    delete todayStatsWidget_;
    delete booksProgressWidget_;
    
    // 创建新widget
    todayStatsWidget_ = new QWidget();
    booksProgressWidget_ = new QWidget();
    
    // 重新添加到父布局
    todayParentLayout->addWidget(todayStatsWidget_);
    booksParentLayout->addWidget(booksProgressWidget_);
    
    // 重新加载数据
    loadStatistics();
}

} // namespace Presentation
} // namespace WordMaster