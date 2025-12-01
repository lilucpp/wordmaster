#include "book_list_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QMessageBox>

namespace WordMaster {
namespace Presentation {

BookListWidget::BookListWidget(Application::BookService* service, 
                               QWidget* parent)
    : QWidget(parent)
    , service_(service)
    , bookList_(new QListWidget(this))
    , importButton_(new QPushButton("导入词库", this))
    , refreshButton_(new QPushButton("刷新", this))
    , titleLabel_(new QLabel("词库管理", this))
{
    setupUI();
    loadBooks();
}

void BookListWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // 标题栏
    auto* headerLayout = new QHBoxLayout();
    titleLabel_->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();
    headerLayout->addWidget(refreshButton_);
    headerLayout->addWidget(importButton_);
    
    mainLayout->addLayout(headerLayout);
    
    // 词库列表
    bookList_->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #ddd;
            background-color: #f8f9fa;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #e9ecef;
            background-color: white;
            margin: 5px;
            border-radius: 5px;
        }
        QListWidget::item:selected {
            background-color: #e3f2fd;
            border: 2px solid #2196f3;
        }
        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )");
    bookList_->setSelectionMode(QAbstractItemView::SingleSelection);
    bookList_->setSpacing(5);
    
    mainLayout->addWidget(bookList_);
    
    // 连接信号
    connect(bookList_, &QListWidget::itemClicked,
            this, &BookListWidget::onBookItemClicked);
    connect(importButton_, &QPushButton::clicked,
            this, &BookListWidget::onImportClicked);
    connect(refreshButton_, &QPushButton::clicked,
            this, &BookListWidget::refresh);
}

void BookListWidget::loadBooks() {
    bookList_->clear();
    
    auto books = service_->getAllBooks();
    
    if (books.isEmpty()) {
        auto* item = new QListWidgetItem("暂无词库，请点击「导入词库」按钮导入");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(QColor("#999"));
        bookList_->addItem(item);
        return;
    }
    
    for (const auto& book : books) {
        auto stats = service_->getBookStatistics(book.id);
        
        // 创建列表项
        auto* item = new QListWidgetItem(bookList_);
        item->setData(Qt::UserRole, book.id);
        
        // 创建自定义卡片
        auto* card = createBookCard(book, stats);
        item->setSizeHint(card->sizeHint());
        
        bookList_->addItem(item);
        bookList_->setItemWidget(item, card);
    }
}

QWidget* BookListWidget::createBookCard(
    const Domain::Book& book,
    const Application::BookService::BookStatistics& stats)
{
    auto* card = new QWidget();
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15);  // 增加边距防止重叠
    layout->setSpacing(8);  // 增加间距
    
    // 标题行
    auto* titleLayout = new QHBoxLayout();
    
    auto* nameLabel = new QLabel(book.name);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    titleLayout->addWidget(nameLabel);
    
    if (book.isActive) {
        auto* activeLabel = new QLabel("✓ 当前词库");
        activeLabel->setStyleSheet(R"(
            background-color: #4caf50;
            color: white;
            padding: 3px 10px;
            border-radius: 3px;
            font-size: 12px;
        )");
        titleLayout->addWidget(activeLabel);
    }
    
    titleLayout->addStretch();
    
    // 开始学习按钮
    auto* studyBtn = new QPushButton("开始学习");
    studyBtn->setMinimumHeight(32);  // 确保按钮高度足够
    studyBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2196f3;
            color: white;
            border: none;
            padding: 8px 15px;
            border-radius: 3px;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #1976d2;
        }
    )");
    connect(studyBtn, &QPushButton::clicked, [this, book]() {
        emit studyRequested(book.id);
    });
    titleLayout->addWidget(studyBtn);
    
    layout->addLayout(titleLayout);
    
    // 描述和分类
    auto* infoLayout = new QHBoxLayout();
    
    auto* categoryLabel = new QLabel(QString("📂 %1").arg(book.category));
    categoryLabel->setStyleSheet("color: #666; font-size: 14px;");
    infoLayout->addWidget(categoryLabel);
    
    if (!book.tags.isEmpty()) {
        auto* tagLabel = new QLabel(QString("🏷️ %1").arg(book.tags.join(", ")));
        tagLabel->setStyleSheet("color: #666; font-size: 14px;");
        infoLayout->addWidget(tagLabel);
    }
    
    infoLayout->addStretch();
    layout->addLayout(infoLayout);
    
    // 统计信息
    auto* statsLayout = new QHBoxLayout();
    
    auto* totalLabel = new QLabel(QString("📚 总数: %1").arg(stats.totalWords));
    totalLabel->setStyleSheet("font-size: 14px;");
    statsLayout->addWidget(totalLabel);
    
    auto* learnedLabel = new QLabel(QString("✅ 已学: %1").arg(stats.learnedWords));
    learnedLabel->setStyleSheet("color: #4caf50; font-size: 14px;");
    statsLayout->addWidget(learnedLabel);
    
    auto* masteredLabel = new QLabel(QString("⭐ 掌握: %1").arg(stats.masteredWords));
    masteredLabel->setStyleSheet("color: #ff9800; font-size: 14px;");
    statsLayout->addWidget(masteredLabel);
    
    statsLayout->addStretch();
    layout->addLayout(statsLayout);
    
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
    layout->addWidget(progressBar);
    
    return card;
}

void BookListWidget::refresh() {
    loadBooks();
}

void BookListWidget::onBookItemClicked(QListWidgetItem* item) {
    QString bookId = item->data(Qt::UserRole).toString();
    if (!bookId.isEmpty()) {
        selectedBookId_ = bookId;
        emit bookSelected(bookId);
    }
}

void BookListWidget::onImportClicked() {
    emit importRequested();
}

void BookListWidget::onStudyClicked() {
    if (selectedBookId_.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个词库");
        return;
    }
    
    emit studyRequested(selectedBookId_);
}

void BookListWidget::onDeleteClicked() {
    if (selectedBookId_.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个词库");
        return;
    }
    
    auto reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除该词库吗？此操作不可恢复。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (service_->deleteBook(selectedBookId_)) {
            QMessageBox::information(this, "成功", "词库已删除");
            selectedBookId_.clear();
            refresh();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

} // namespace Presentation
} // namespace WordMaster