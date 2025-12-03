#include "mainwindow.h"
#include "widgets/book_list_widget.h"
#include "widgets/notebook_widget.h"
#include "widgets/review_widget.h"
#include "widgets/statistics_widget.h"
#include "widgets/study_widget.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>

using namespace WordMaster::Infrastructure;
using namespace WordMaster::Application;

namespace WordMaster {
namespace Presentation {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), centralWidget_(new QWidget(this)),
      navigationList_(new QListWidget(this)),
      contentStack_(new QStackedWidget(this)) {
  setWindowTitle(QStringLiteral("WordMaster - 英语单词记忆助手"));
  resize(1200, 800);

  initializeDatabase();
  setupUI();
  setupConnections();
  loadInitialData();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
  setStyleSheet(R"(
        QWidget {
            font-family: "Segoe UI", "Microsoft YaHei", "Noto Sans SC", sans-serif;
            color: #0f172a;
            background-color: #f7f9fc;
        }
        QStackedWidget {
            background-color: #f7f9fc;
        }
    )");

  auto *mainLayout = new QHBoxLayout(centralWidget_);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  setupNavigation();

  auto *navContainer = new QWidget(this);
  navContainer->setFixedWidth(220);
  navContainer->setStyleSheet(
      "background-color: #0f172a; border-right: 1px solid #162133;");

  auto *navLayout = new QVBoxLayout(navContainer);
  navLayout->setContentsMargins(18, 20, 18, 20);
  navLayout->setSpacing(16);

  auto *brandLabel = new QLabel("WordMaster", navContainer);
  brandLabel->setStyleSheet(
      "font-size: 20px; font-weight: 700; color: #e2e8f0; letter-spacing: 0.5px;");
  navLayout->addWidget(brandLabel);

  navLayout->addWidget(navigationList_);
  navLayout->addStretch();

  mainLayout->addWidget(navContainer);

  setupContentArea();
  mainLayout->addWidget(contentStack_);

  setCentralWidget(centralWidget_);
}

void MainWindow::setupNavigation() {
  navigationList_->setFrameShape(QFrame::NoFrame);
  navigationList_->setIconSize(QSize(18, 18));
  navigationList_->setSpacing(6);
  navigationList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  navigationList_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  navigationList_->setStyleSheet(R"(
        QListWidget {
            background-color: transparent;
            color: #cbd5e1;
            border: none;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 12px 14px;
            margin: 2px 0;
            border-radius: 10px;
        }
        QListWidget::item:selected {
            background-color: #f1f5f9;
            color: #0f172a;
            border-left: 4px solid #2d8cff;
            padding-left: 10px;
            font-weight: 600;
        }
        QListWidget::item:hover {
            background-color: rgba(255,255,255,0.06);
            color: #e2e8f0;
        }
    )");

  auto addItem = [this](QStyle::StandardPixmap icon, const QString &text) {
    auto *item = new QListWidgetItem(style()->standardIcon(icon), text);
    item->setSizeHint(QSize(item->sizeHint().width(), 46));
    navigationList_->addItem(item);
  };

  addItem(QStyle::SP_DirIcon, QStringLiteral("词库管理"));
  addItem(QStyle::SP_MediaPlay, QStringLiteral("学习"));
  addItem(QStyle::SP_BrowserReload, QStringLiteral("复习"));
  addItem(QStyle::SP_FileDialogDetailedView, QStringLiteral("我的词本"));
  addItem(QStyle::SP_ComputerIcon, QStringLiteral("统计"));

  navigationList_->setCurrentRow(0);
}

void MainWindow::setupContentArea() {
  bookListWidget_ = new BookListWidget(bookService_.get(), this);
  studyWidget_ = new StudyWidget(studyService_.get(), wordRepo_.get(),
                                 tagService_.get(), this);
  reviewWidget_ = new ReviewWidget(studyService_.get(), wordRepo_.get(), this);
  notebookWidget_ =
      new NotebookWidget(tagService_.get(), wordRepo_.get(), this);
  statsWidget_ =
      new StatisticsWidget(bookService_.get(), recordRepo_.get(), this);

  contentStack_->addWidget(bookListWidget_);
  contentStack_->addWidget(studyWidget_);
  contentStack_->addWidget(reviewWidget_);
  contentStack_->addWidget(notebookWidget_);
  contentStack_->addWidget(statsWidget_);
}

void MainWindow::setupConnections() {
  connect(navigationList_, &QListWidget::currentRowChanged, this,
          &MainWindow::onNavigationClicked);

  connect(bookListWidget_, &BookListWidget::bookSelected, this,
          &MainWindow::onBookSelected);

  connect(bookListWidget_, &BookListWidget::importRequested, this,
          &MainWindow::onImportBooks);

  connect(bookListWidget_, &BookListWidget::studyRequested, this,
          &MainWindow::onStartStudy);
}

void MainWindow::initializeDatabase() {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(dataPath);
  QString dbPath = dataPath + "/wordmaster.db";

  adapter_ = std::make_unique<SQLiteAdapter>(dbPath);
  if (!adapter_->open()) {
    QMessageBox::critical(this, QStringLiteral("错误"),
                          QStringLiteral("无法打开数据库"));
    qApp->quit();
    return;
  }

  QString schemaPath = ":/resources/database/001_initial_schema.sql";

  if (!adapter_->initializeDatabase(schemaPath)) {
    qWarning() << "Database initialization may have failed";
  }

  bookRepo_ = std::make_unique<BookRepository>(*adapter_);
  wordRepo_ = std::make_unique<WordRepository>(*adapter_);
  recordRepo_ = std::make_unique<StudyRecordRepository>(*adapter_);
  scheduleRepo_ = std::make_unique<ReviewScheduleRepository>(*adapter_);
  tagRepo_ = std::make_unique<WordTagRepository>(*adapter_);

  bookService_ = std::make_unique<BookService>(*bookRepo_, *wordRepo_);
  scheduler_ = std::make_unique<SM2Scheduler>(*scheduleRepo_);
  studyService_ =
      std::make_unique<StudyService>(*wordRepo_, *recordRepo_, *scheduler_);
  tagService_ = std::make_unique<TagService>(*tagRepo_);
}

void MainWindow::loadInitialData() {
  auto activeBook = bookService_->getActiveBook();
  if (!activeBook.id.isEmpty()) {
    currentBookId_ = activeBook.id;
  }
}

void MainWindow::onNavigationClicked(int index) {
  contentStack_->setCurrentIndex(index);

  switch (index) {
  case 0:
    bookListWidget_->refresh();
    break;
  case 1:
    if (!currentBookId_.isEmpty()) {
      studyWidget_->setBookId(currentBookId_);
    }
    break;
  case 2:
    if (!currentBookId_.isEmpty()) {
      reviewWidget_->setBookId(currentBookId_);
    } else {
      auto activeBook = bookService_->getActiveBook();
      if (!activeBook.id.isEmpty()) {
        currentBookId_ = activeBook.id;
        reviewWidget_->setBookId(currentBookId_);
      }
    }
    break;
  case 3:
    notebookWidget_->refresh();
    break;
  case 4:
    statsWidget_->refresh();
    break;
  }
}

void MainWindow::onBookSelected(const QString &bookId) {
  currentBookId_ = bookId;

  if (bookService_->setActiveBook(bookId)) {
    statusBar()->showMessage(QStringLiteral("已激活词库 %1").arg(bookId), 3000);
  }
}

void MainWindow::onImportBooks() {
  QString fileName = QFileDialog::getOpenFileName(
      this, QStringLiteral("选择词库元数据文件"), QDir::homePath(),
      QStringLiteral("JSON Files (*.json)"));

  if (fileName.isEmpty()) {
    return;
  }

  auto result = bookService_->importBooksFromMeta(fileName);

  if (result.success) {
    QMessageBox::information(this, QStringLiteral("导入成功"),
                             QStringLiteral("成功导入 %1 个词库，共 %2 个单词")
                                 .arg(result.importedBooks)
                                 .arg(result.importedWords));

    bookListWidget_->refresh();
  } else {
    QMessageBox::warning(this, QStringLiteral("导入失败"), result.message);
  }
}

void MainWindow::onStartStudy() {
  if (currentBookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  navigationList_->setCurrentRow(1);
  studyWidget_->startNewSession();
}

void MainWindow::onStartReview() {
  if (currentBookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  navigationList_->setCurrentRow(2);
  reviewWidget_->startReviewSession();
}

} // namespace Presentation
} // namespace WordMaster
