#include "mainwindow.h"
#include "widgets/book_list_widget.h"
#include "widgets/study_widget.h"
#include "widgets/review_widget.h"
#include "widgets/statistics_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QStatusBar>

using namespace WordMaster::Infrastructure;
using namespace WordMaster::Application;

namespace WordMaster {
namespace Presentation {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget_(new QWidget(this))
    , navigationList_(new QListWidget(this))
    , contentStack_(new QStackedWidget(this))
{
    setWindowTitle("WordMaster - 英语单词记忆助手");
    resize(1200, 800);
    
    initializeDatabase();
    setupUI();
    setupConnections();
    loadInitialData();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // 主布局：左右分栏
    auto* mainLayout = new QHBoxLayout(centralWidget_);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 左侧导航
    setupNavigation();
    mainLayout->addWidget(navigationList_);
    
    // 右侧内容区
    setupContentArea();
    mainLayout->addWidget(contentStack_);
    
    setCentralWidget(centralWidget_);
}

void MainWindow::setupNavigation() {
    navigationList_->setMaximumWidth(200);
    navigationList_->setStyleSheet(R"(
        QListWidget {
            background-color: #2c3e50;
            color: white;
            border: none;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 15px 20px;
            border-bottom: 1px solid #34495e;
        }
        QListWidget::item:selected {
            background-color: #3498db;
        }
        QListWidget::item:hover {
            background-color: #34495e;
        }
    )");
    
    // 添加导航项
    navigationList_->addItem("📚 词库管理");
    navigationList_->addItem("📖 学习");
    navigationList_->addItem("🔄 复习");
    navigationList_->addItem("📊 统计");
    
    navigationList_->setCurrentRow(0);
}

void MainWindow::setupContentArea() {
    // 创建各个页面
    bookListWidget_ = new BookListWidget(bookService_.get(), this);
    studyWidget_ = new StudyWidget(studyService_.get(), wordRepo_.get(), this);
    reviewWidget_ = new ReviewWidget(studyService_.get(), wordRepo_.get(), this);
    statsWidget_ = new StatisticsWidget(bookService_.get(), recordRepo_.get(), this);
    
    // 添加到堆栈
    contentStack_->addWidget(bookListWidget_);
    contentStack_->addWidget(studyWidget_);
    contentStack_->addWidget(reviewWidget_);
    contentStack_->addWidget(statsWidget_);
}

void MainWindow::setupConnections() {
    // 导航切换
    connect(navigationList_, &QListWidget::currentRowChanged,
            this, &MainWindow::onNavigationClicked);
    
    // 词库选择
    connect(bookListWidget_, &BookListWidget::bookSelected,
            this, &MainWindow::onBookSelected);
    
    // 导入词库
    connect(bookListWidget_, &BookListWidget::importRequested,
            this, &MainWindow::onImportBooks);
    
    // 开始学习
    connect(bookListWidget_, &BookListWidget::studyRequested,
            this, &MainWindow::onStartStudy);
}

void MainWindow::initializeDatabase() {
    // 数据库路径
    QString dataPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
    );
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/wordmaster.db";
    
    // 创建适配器
    adapter_ = std::make_unique<SQLiteAdapter>(dbPath);
    if (!adapter_->open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库");
        qApp->quit();
        return;
    }
    
    // 初始化数据库模式
    QString schemaPath = ":/resources/database/001_initial_schema.sql";
    
    if (!adapter_->initializeDatabase(schemaPath)) {
        qWarning() << "Database initialization may have failed";
    }
    
    // 创建仓储
    bookRepo_ = std::make_unique<BookRepository>(*adapter_);
    wordRepo_ = std::make_unique<WordRepository>(*adapter_);
    recordRepo_ = std::make_unique<StudyRecordRepository>(*adapter_);
    scheduleRepo_ = std::make_unique<ReviewScheduleRepository>(*adapter_);
    
    // 创建服务
    bookService_ = std::make_unique<BookService>(*bookRepo_, *wordRepo_);
    scheduler_ = std::make_unique<SM2Scheduler>(*scheduleRepo_);
    studyService_ = std::make_unique<StudyService>(*wordRepo_, *recordRepo_, *scheduler_);
}

void MainWindow::loadInitialData() {
    // 加载激活的词库
    auto activeBook = bookService_->getActiveBook();
    if (!activeBook.id.isEmpty()) {
        currentBookId_ = activeBook.id;
    }
}

void MainWindow::onNavigationClicked(int index) {
    contentStack_->setCurrentIndex(index);
    
    // 刷新页面数据
    switch (index) {
        case 0: // 词库管理
            bookListWidget_->refresh();
            break;
        case 1: // 学习
            if (!currentBookId_.isEmpty()) {
                studyWidget_->setBookId(currentBookId_);
            }
            break;
        case 2: // 复习
            if (!currentBookId_.isEmpty()) {
                reviewWidget_->setBookId(currentBookId_);
            } else {
                // 尝试获取激活的词库
                auto activeBook = bookService_->getActiveBook();
                if (!activeBook.id.isEmpty()) {
                    currentBookId_ = activeBook.id;
                    reviewWidget_->setBookId(currentBookId_);
                }
            }
            break;
        case 3: // 统计
            statsWidget_->refresh();
            break;
    }
}

void MainWindow::onBookSelected(const QString& bookId) {
    currentBookId_ = bookId;
    
    // 激活词库
    if (bookService_->setActiveBook(bookId)) {
        statusBar()->showMessage(QString("已激活词库: %1").arg(bookId), 3000);
    }
}

void MainWindow::onImportBooks() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择词库元数据文件",
        QDir::homePath(),
        "JSON Files (*.json)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 导入
    auto result = bookService_->importBooksFromMeta(fileName);
    
    if (result.success) {
        QMessageBox::information(
            this,
            "导入成功",
            QString("成功导入 %1 个词库，共 %2 个单词")
                .arg(result.importedBooks)
                .arg(result.importedWords)
        );
        
        // 刷新列表
        bookListWidget_->refresh();
    } else {
        QMessageBox::warning(this, "导入失败", result.message);
    }
}

void MainWindow::onStartStudy() {
    if (currentBookId_.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个词库");
        return;
    }
    
    // 切换到学习页面
    navigationList_->setCurrentRow(1);
    studyWidget_->startNewSession();
}

void MainWindow::onStartReview() {
    if (currentBookId_.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个词库");
        return;
    }
    
    // 切换到复习页面
    navigationList_->setCurrentRow(2);
    reviewWidget_->startReviewSession();
}

} // namespace Presentation
} // namespace WordMaster