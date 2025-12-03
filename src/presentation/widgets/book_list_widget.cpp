#include "book_list_widget.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressBar>
#include <QVBoxLayout>

namespace {
const QString kAccent = "#2d8cff";
const QString kSuccess = "#16a34a";
const QString kWarning = "#f59e0b";
const QString kMuted = "#64748b";
}

namespace WordMaster {
namespace Presentation {

BookListWidget::BookListWidget(Application::BookService *service,
                               QWidget *parent)
    : QWidget(parent), service_(service), bookList_(new QListWidget(this)),
      importButton_(new QPushButton(QStringLiteral("导入词库"), this)),
      refreshButton_(new QPushButton(QStringLiteral("刷新"), this)),
      titleLabel_(new QLabel(QStringLiteral("词库管理"), this)) {
  setupUI();
  loadBooks();
}

void BookListWidget::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(16);

  auto *headerLayout = new QHBoxLayout();
  titleLabel_->setStyleSheet(
      "font-size: 26px; font-weight: 700; color: #0f172a;");
  headerLayout->addWidget(titleLabel_);
  headerLayout->addStretch();

  refreshButton_->setCursor(Qt::PointingHandCursor);
  refreshButton_->setStyleSheet(R"(
        QPushButton {
            background: #ffffff;
            color: #334155;
            border: 1px solid #e2e8f0;
            padding: 10px 18px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover { border-color: #cbd5e1; background: #f8fafc; }
        QPushButton:pressed { background: #e2e8f0; }
    )");

  importButton_->setCursor(Qt::PointingHandCursor);
  importButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 10px 22px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 700;
            min-width: 110px;
        }
        QPushButton:hover { background: #2375e1; }
        QPushButton:pressed { background: #1e63c0; }
    )").arg(kAccent));

  headerLayout->addWidget(refreshButton_);
  headerLayout->addWidget(importButton_);

  mainLayout->addLayout(headerLayout);

  bookList_->setStyleSheet(R"(
        QListWidget {
            border: none;
            background: transparent;
        }
        QListWidget::item {
            margin: 10px 0;
        }
    )");
  bookList_->setSelectionMode(QAbstractItemView::SingleSelection);
  bookList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  bookList_->setSpacing(0);

  mainLayout->addWidget(bookList_);

  connect(bookList_, &QListWidget::itemClicked, this,
          &BookListWidget::onBookItemClicked);
  connect(importButton_, &QPushButton::clicked, this,
          &BookListWidget::onImportClicked);
  connect(refreshButton_, &QPushButton::clicked, this,
          &BookListWidget::refresh);
}

void BookListWidget::loadBooks() {
  bookList_->clear();

  auto books = service_->getAllBooks();

  if (books.isEmpty()) {
    auto *item = new QListWidgetItem(QStringLiteral("暂无词库，请点击「导入词库」按钮导入"));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setForeground(QColor(kMuted));
    bookList_->addItem(item);
    return;
  }

  for (const auto &book : books) {
    auto stats = service_->getBookStatistics(book.id);

    auto *item = new QListWidgetItem(bookList_);
    item->setData(Qt::UserRole, book.id);

    auto *card = createBookCard(book, stats);
    item->setSizeHint(card->sizeHint());

    bookList_->addItem(item);
    bookList_->setItemWidget(item, card);
  }
}

QWidget *BookListWidget::createBookCard(
    const Domain::Book &book,
    const Application::BookService::BookStatistics &stats) {
  auto *card = new QWidget();
  card->setObjectName("bookCard");
  card->setStyleSheet(R"(
        QWidget#bookCard {
            background: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 14px;
        }
    )");

  auto *shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(18);
  shadow->setOffset(0, 6);
  shadow->setColor(QColor(15, 23, 42, 25));
  card->setGraphicsEffect(shadow);

  auto *layout = new QVBoxLayout(card);
  layout->setContentsMargins(18, 18, 18, 18);
  layout->setSpacing(10);

  auto *titleLayout = new QHBoxLayout();

  auto *nameLabel = new QLabel(book.name);
  nameLabel->setStyleSheet(
      "font-size: 18px; font-weight: 700; color: #0f172a;");
  titleLayout->addWidget(nameLabel);

  if (book.isActive) {
    auto *activeLabel = new QLabel(QStringLiteral("当前词库"));
    activeLabel->setStyleSheet(QString(R"(
            QLabel {
                background: #e0f2ff;
                color: %1;
                padding: 4px 10px;
                border-radius: 10px;
                font-size: 12px;
                font-weight: 700;
            }
        )").arg(kAccent));
    titleLayout->addWidget(activeLabel);
  }

  titleLayout->addStretch();

  auto *studyBtn = new QPushButton(QStringLiteral("开始学习"));
  studyBtn->setMinimumHeight(34);
  studyBtn->setCursor(Qt::PointingHandCursor);
  studyBtn->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 8px 18px;
            border-radius: 10px;
            font-weight: 700;
        }
        QPushButton:hover { background: #2375e1; }
        QPushButton:pressed { background: #1e63c0; }
    )").arg(kAccent));
  connect(studyBtn, &QPushButton::clicked,
          [this, book]() { emit studyRequested(book.id); });
  titleLayout->addWidget(studyBtn);

  layout->addLayout(titleLayout);

  auto chip = [](const QString &text, const QString &bg, const QString &fg) {
    auto *label = new QLabel(text);
    label->setStyleSheet(QString(
        "QLabel { background: %1; color: %2; padding: 4px 8px; border-radius: 8px; font-size: 12px; }")
                             .arg(bg, fg));
    return label;
  };

  auto *infoLayout = new QHBoxLayout();
  infoLayout->setSpacing(10);

  if (!book.category.isEmpty()) {
    infoLayout->addWidget(
        chip(QStringLiteral("分类 · %1").arg(book.category),
             "#f1f5f9", "#0f172a"));
  }

  if (!book.tags.isEmpty()) {
    infoLayout->addWidget(
        chip(QStringLiteral("标签 · %1").arg(book.tags.join(", ")),
             "#f8fafc", "#0f172a"));
  }

  infoLayout->addStretch();
  layout->addLayout(infoLayout);

  auto *statsLayout = new QHBoxLayout();
  statsLayout->setSpacing(14);

  auto *totalLabel =
      new QLabel(QStringLiteral("总词数：%1").arg(stats.totalWords));
  totalLabel->setStyleSheet("font-size: 14px; color: #0f172a;");
  statsLayout->addWidget(totalLabel);

  auto *learnedLabel =
      new QLabel(QStringLiteral("已学：%1").arg(stats.learnedWords));
  learnedLabel->setStyleSheet(
      QString("font-size: 14px; color: %1;").arg(kSuccess));
  statsLayout->addWidget(learnedLabel);

  auto *masteredLabel =
      new QLabel(QStringLiteral("掌握：%1").arg(stats.masteredWords));
  masteredLabel->setStyleSheet(
      QString("font-size: 14px; color: %1;").arg(kWarning));
  statsLayout->addWidget(masteredLabel);

  statsLayout->addStretch();
  layout->addLayout(statsLayout);

  auto *progressLayout = new QHBoxLayout();
  progressLayout->setSpacing(8);

  auto *progressBar = new QProgressBar();
  progressBar->setRange(0, 100);
  int progressValue = static_cast<int>(stats.progress * 100);
  progressBar->setValue(progressValue);
  progressBar->setTextVisible(false);
  progressBar->setFixedHeight(10);
  progressBar->setStyleSheet(QString(R"(
        QProgressBar {
            background: #e9eef5;
            border: none;
            border-radius: 6px;
            padding: 0;
        }
        QProgressBar::chunk {
            background: %1;
            border-radius: 6px;
        }
    )").arg(kAccent));

  auto *progressText = new QLabel(
      QStringLiteral("%1%").arg(progressValue));
  progressText->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");

  progressLayout->addWidget(progressBar, 1);
  progressLayout->addWidget(progressText);

  layout->addLayout(progressLayout);

  return card;
}

void BookListWidget::refresh() { loadBooks(); }

void BookListWidget::onBookItemClicked(QListWidgetItem *item) {
  QString bookId = item->data(Qt::UserRole).toString();
  if (!bookId.isEmpty()) {
    selectedBookId_ = bookId;
    emit bookSelected(bookId);
  }
}

void BookListWidget::onImportClicked() { emit importRequested(); }

void BookListWidget::onStudyClicked() {
  if (selectedBookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  emit studyRequested(selectedBookId_);
}

void BookListWidget::onDeleteClicked() {
  if (selectedBookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  auto reply =
      QMessageBox::question(this, QStringLiteral("确认删除"),
                            QStringLiteral("确定要删除该词库吗？此操作不可恢复。"),
                            QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    if (service_->deleteBook(selectedBookId_)) {
      QMessageBox::information(this, QStringLiteral("成功"),
                               QStringLiteral("词库已删除"));
      selectedBookId_.clear();
      refresh();
    } else {
      QMessageBox::warning(this, QStringLiteral("错误"),
                           QStringLiteral("删除失败"));
    }
  }
}

} // namespace Presentation
} // namespace WordMaster
