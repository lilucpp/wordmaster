#include "statistics_widget.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

namespace {
const QString kAccent = "#2d8cff";
const QString kSuccess = "#16a34a";
const QString kMuted = "#94a3b8";
}

namespace WordMaster {
namespace Presentation {

StatisticsWidget::StatisticsWidget(Application::BookService *bookService,
                                   Domain::IStudyRecordRepository *recordRepo,
                                   QWidget *parent)
    : QWidget(parent), bookService_(bookService), recordRepo_(recordRepo) {
  setupUI();
  loadStatistics();
}

void StatisticsWidget::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(18);

  titleLabel_ = new QLabel(QStringLiteral("学习统计"), this);
  titleLabel_->setStyleSheet(
      "font-size: 26px; font-weight: 700; color: #0f172a;");
  mainLayout->addWidget(titleLabel_);

  auto *todayGroup = new QGroupBox(QStringLiteral("今日概览"), this);
  todayGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 16px;
            font-weight: 700;
            color: #0f172a;
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 14px;
            padding: 16px;
            margin-top: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
    )");
  todayStatsWidget_ = new QWidget(todayGroup);
  auto *todayLayout = new QVBoxLayout(todayGroup);
  todayLayout->setContentsMargins(8, 16, 8, 8);
  todayLayout->addWidget(todayStatsWidget_);
  mainLayout->addWidget(todayGroup);

  auto *booksGroup = new QGroupBox(QStringLiteral("词库进度"), this);
  booksGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 16px;
            font-weight: 700;
            color: #0f172a;
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 14px;
            padding: 16px;
            margin-top: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
    )");
  booksProgressWidget_ = new QWidget(booksGroup);
  auto *booksLayout = new QVBoxLayout(booksGroup);
  booksLayout->setContentsMargins(8, 16, 8, 8);
  booksLayout->addWidget(booksProgressWidget_);
  mainLayout->addWidget(booksGroup);

  mainLayout->addStretch();
}

void StatisticsWidget::loadStatistics() {
  auto todayRecords = recordRepo_->getTodayRecords();
  int learnCount = 0;
  int reviewCount = 0;
  int totalDuration = 0;

  for (const auto &record : todayRecords) {
    if (record.studyType == Domain::StudyRecord::Type::Learn) {
      learnCount++;
    } else if (record.studyType == Domain::StudyRecord::Type::Review) {
      reviewCount++;
    }
    totalDuration += record.studyDuration;
  }

  auto *todayLayout = new QHBoxLayout(todayStatsWidget_);
  todayLayout->setSpacing(12);

  auto makeStatCard = [](const QString &title, const QString &value,
                         const QString &badgeColor) {
    auto *card = new QWidget();
    card->setStyleSheet(QString(R"(
            QWidget {
                background: #f8fafc;
                border: 1px solid #e2e8f0;
                border-radius: 12px;
            }
        )"));
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(6);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "font-size: 13px; color: #475569; font-weight: 600;");

    auto *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet("font-size: 28px; font-weight: 800; "
                              "color: #0f172a;");

    auto *badge = new QLabel(" ");
    badge->setFixedHeight(4);
    badge->setStyleSheet(
        QString("background: %1; border-radius: 2px;").arg(badgeColor));

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(badge);
    layout->addStretch();
    return card;
  };

  todayLayout->addWidget(makeStatCard(QStringLiteral("新学"), QString::number(learnCount), kAccent));
  todayLayout->addWidget(makeStatCard(QStringLiteral("复习"), QString::number(reviewCount), "#f59e0b"));
  todayLayout->addWidget(
      makeStatCard(QStringLiteral("专注分钟"), QString::number(totalDuration / 60),
                   kSuccess));
  todayLayout->addStretch();

  auto *booksLayout = new QVBoxLayout(booksProgressWidget_);
  booksLayout->setSpacing(10);
  auto statsList = bookService_->getAllBooksStatistics();

  for (const auto &stats : statsList) {
    auto *bookWidget = new QWidget();
    bookWidget->setStyleSheet(
        "background: #ffffff; border: 1px solid #e2e8f0; "
        "border-radius: 12px;");
    auto *bookLayout = new QVBoxLayout(bookWidget);
    bookLayout->setContentsMargins(12, 12, 12, 12);
    bookLayout->setSpacing(6);

    auto *infoLayout = new QHBoxLayout();
    auto *nameLabel = new QLabel(stats.bookName);
    nameLabel->setStyleSheet("font-size: 15px; font-weight: 700; color: #0f172a;");
    infoLayout->addWidget(nameLabel);

    auto *statsLabel =
        new QLabel(QStringLiteral("已学 %1 / %2 · 掌握 %3")
                       .arg(stats.learnedWords)
                       .arg(stats.totalWords)
                       .arg(stats.masteredWords));
    statsLabel->setStyleSheet("color: #475569; font-size: 13px;");
    infoLayout->addWidget(statsLabel);
    infoLayout->addStretch();

    bookLayout->addLayout(infoLayout);

    auto *progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    int progress = static_cast<int>(stats.progress * 100);
    progressBar->setValue(progress);
    progressBar->setTextVisible(true);
    progressBar->setFormat(QString("%1%").arg(progress));
    progressBar->setStyleSheet(QString(R"(
            QProgressBar {
                border: none;
                background: #e9eef5;
                border-radius: 8px;
                height: 14px;
                color: #0f172a;
                font-weight: 600;
            }
            QProgressBar::chunk {
                background: %1;
                border-radius: 8px;
            }
        )").arg(kAccent));
    bookLayout->addWidget(progressBar);

    booksLayout->addWidget(bookWidget);
  }

  if (statsList.isEmpty()) {
    auto *emptyLabel = new QLabel(QStringLiteral("暂无数据"));
    emptyLabel->setStyleSheet(QString("color: %1;").arg(kMuted));
    booksLayout->addWidget(emptyLabel);
  }
}

void StatisticsWidget::refresh() {
  QLayout *todayParentLayout = todayStatsWidget_->parentWidget()->layout();
  QLayout *booksParentLayout = booksProgressWidget_->parentWidget()->layout();

  todayParentLayout->removeWidget(todayStatsWidget_);
  booksParentLayout->removeWidget(booksProgressWidget_);
  delete todayStatsWidget_;
  delete booksProgressWidget_;

  todayStatsWidget_ = new QWidget();
  booksProgressWidget_ = new QWidget();

  todayParentLayout->addWidget(todayStatsWidget_);
  booksParentLayout->addWidget(booksProgressWidget_);

  loadStatistics();
}

} // namespace Presentation
} // namespace WordMaster
