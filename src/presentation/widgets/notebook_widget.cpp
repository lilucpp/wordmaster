#include "notebook_widget.h"

#include <QLabel>
#include <QTabBar>
#include <QVBoxLayout>

namespace {
const QString kAccent = "#2d8cff";
}

namespace WordMaster {
namespace Presentation {

NotebookWidget::NotebookWidget(Application::TagService *tagService,
                               Domain::IWordRepository *wordRepo,
                               QWidget *parent)
    : QWidget(parent), tagService_(tagService), wordRepo_(wordRepo) {
  setupUI();
}

void NotebookWidget::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(12);

  auto *titleLabel = new QLabel(QStringLiteral("我的词本"), this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: 700; color: #0f172a;");
  layout->addWidget(titleLabel);

  tabWidget_ = new QTabWidget(this);
  tabWidget_->setDocumentMode(true);
  tabWidget_->setStyleSheet(QString(R"(
        QTabWidget::pane { border: none; }
        QTabBar::tab {
            background: #e9eef5;
            color: #475569;
            border: none;
            border-radius: 10px;
            padding: 8px 16px;
            margin-right: 6px;
            font-weight: 600;
        }
        QTabBar::tab:selected {
            background: #dbeafe;
            color: #0f172a;
            font-weight: 700;
            border: 1px solid #bfdbfe;
        }
        QTabBar::tab:hover { background: #eef2ff; color: #0f172a; }
    )"));

  difficultList_ = new QListWidget();
  wrongList_ = new QListWidget();
  favoriteList_ = new QListWidget();

  QString listStyle = QString(R"(
        QListWidget {
            border: none;
            background: transparent;
        }
        QListWidget::item {
            padding: 12px 14px;
            margin: 6px 0;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            background: #ffffff;
            font-size: 14px;
            color: #0f172a;
        }
        QListWidget::item:hover { border-color: #cbd5e1; }
        QListWidget::item:selected {
            border: 1px solid %1;
            background: #f5f8ff;
            color: #0f172a;
        }
    )").arg(kAccent);

  difficultList_->setStyleSheet(listStyle);
  wrongList_->setStyleSheet(listStyle);
  favoriteList_->setStyleSheet(listStyle);

  tabWidget_->addTab(difficultList_, QStringLiteral("生词本"));
  tabWidget_->addTab(wrongList_, QStringLiteral("错词本"));
  tabWidget_->addTab(favoriteList_, QStringLiteral("收藏本"));

  layout->addWidget(tabWidget_);

  refresh();
}

void NotebookWidget::refresh() {
  loadWords(Domain::WordTag::TAG_DIFFICULT, difficultList_);
  loadWords(Domain::WordTag::TAG_WRONG, wrongList_);
  loadWords(Domain::WordTag::TAG_FAVORITE, favoriteList_);
}

void NotebookWidget::loadWords(const QString &tagType, QListWidget *list) {
  list->clear();

  auto wordIds = tagService_->getWordsByTag(tagType);

  if (wordIds.isEmpty()) {
    auto *item = new QListWidgetItem(QStringLiteral("暂无单词"));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setForeground(QColor("#94a3b8"));
    list->addItem(item);
    return;
  }

  auto words = wordRepo_->getByIds(wordIds);

  for (const auto &word : words) {
    QString text = QString("%1  %2").arg(word.word, word.phoneticUs);
    list->addItem(text);
  }
}

} // namespace Presentation
} // namespace WordMaster
