#include "notebook_widget.h"
#include <QVBoxLayout>
#include <QLabel>

namespace WordMaster {
namespace Presentation {

NotebookWidget::NotebookWidget(Application::TagService* tagService,
                              Domain::IWordRepository* wordRepo,
                              QWidget* parent)
    : QWidget(parent)
    , tagService_(tagService)
    , wordRepo_(wordRepo)
{
    setupUI();
}

void NotebookWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    auto* titleLabel = new QLabel("我的词本", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    layout->addWidget(titleLabel);
    
    tabWidget_ = new QTabWidget(this);
    
    difficultList_ = new QListWidget();
    wrongList_ = new QListWidget();
    favoriteList_ = new QListWidget();
    
    QString listStyle = R"(
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
    )";
    
    difficultList_->setStyleSheet(listStyle);
    wrongList_->setStyleSheet(listStyle);
    favoriteList_->setStyleSheet(listStyle);
    
    tabWidget_->addTab(difficultList_, "📝 生词本");
    tabWidget_->addTab(wrongList_, "❌ 错误本");
    tabWidget_->addTab(favoriteList_, "⭐ 收藏本");
    
    layout->addWidget(tabWidget_);
    
    refresh();
}

void NotebookWidget::refresh() {
    loadWords(Domain::WordTag::TAG_DIFFICULT, difficultList_);
    loadWords(Domain::WordTag::TAG_WRONG, wrongList_);
    loadWords(Domain::WordTag::TAG_FAVORITE, favoriteList_);
}

void NotebookWidget::loadWords(const QString& tagType, QListWidget* list) {
    list->clear();
    
    auto wordIds = tagService_->getWordsByTag(tagType);
    
    if (wordIds.isEmpty()) {
        auto* item = new QListWidgetItem("暂无单词");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(QColor("#999"));
        list->addItem(item);
        return;
    }
    
    auto words = wordRepo_->getByIds(wordIds);
    
    for (const auto& word : words) {
        QString text = QString("%1  %2").arg(word.word, word.phoneticUk);
        list->addItem(text);
    }
}

} // namespace Presentation
} // namespace WordMaster