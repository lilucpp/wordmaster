#include "study_widget.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTime>
#include <QVBoxLayout>

namespace {
const QString kAccent = "#2d8cff";
const QString kDanger = "#ef4444";
const QString kSuccess = "#16a34a";
const QString kMuted = "#94a3b8";
}

namespace WordMaster {
namespace Presentation {

StudyWidget::StudyWidget(Application::StudyService *service,
                         Domain::IWordRepository *wordRepo,
                         Application::TagService *tagService, QWidget *parent)
    : QWidget(parent), service_(service), wordRepo_(wordRepo),
      tagService_(tagService), translationVisible_(false) {
  setupUI();
}

void StudyWidget::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(18);

  titleLabel_ = new QLabel(QStringLiteral("学习新单词"), this);
  titleLabel_->setStyleSheet(
      "font-size: 24px; font-weight: 700; color: #0f172a;");
  mainLayout->addWidget(titleLabel_);

  progressBar_ = new QProgressBar(this);
  progressBar_->setTextVisible(true);
  progressBar_->setStyleSheet(QString(R"(
        QProgressBar {
            background: #e9eef5;
            border: none;
            border-radius: 8px;
            padding: 0 6px;
            height: 14px;
            color: #0f172a;
            font-weight: 600;
        }
        QProgressBar::chunk {
            background: %1;
            border-radius: 8px;
        }
    )").arg(kAccent));
  mainLayout->addWidget(progressBar_);

  cardWidget_ = new QWidget(this);
  cardWidget_->setObjectName("wordCard");
  cardWidget_->setStyleSheet(R"(
        QWidget#wordCard {
            background: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 14px;
        }
    )");
  cardWidget_->setMinimumHeight(380);

  auto *shadow = new QGraphicsDropShadowEffect(cardWidget_);
  shadow->setBlurRadius(18);
  shadow->setOffset(0, 8);
  shadow->setColor(QColor(15, 23, 42, 22));
  cardWidget_->setGraphicsEffect(shadow);

  auto *cardLayout = new QVBoxLayout(cardWidget_);
  cardLayout->setAlignment(Qt::AlignCenter);
  cardLayout->setContentsMargins(24, 24, 24, 24);
  cardLayout->setSpacing(12);

  wordLabel_ = new QLabel("", this);
  wordLabel_->setStyleSheet(R"(
        font-size: 48px;
        font-weight: 800;
        color: #0f172a;
    )");
  wordLabel_->setAlignment(Qt::AlignCenter);
  cardLayout->addWidget(wordLabel_);

  phoneticLabel_ = new QLabel("", this);
  phoneticLabel_->setStyleSheet("font-size: 18px; color: #475569;");
  phoneticLabel_->setAlignment(Qt::AlignCenter);
  cardLayout->addWidget(phoneticLabel_);

  cardLayout->addSpacing(20);

  showButton_ = new QPushButton(QStringLiteral("显示释义 →"), this);
  showButton_->setCursor(Qt::PointingHandCursor);
  showButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 12px 32px;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 700;
        }
        QPushButton:hover { background: #2375e1; }
        QPushButton:pressed { background: #1e63c0; }
    )").arg(kAccent));
  cardLayout->addWidget(showButton_, 0, Qt::AlignCenter);

  translationText_ = new QTextEdit(this);
  translationText_->setReadOnly(true);
  translationText_->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 16px;
            font-size: 14px;
            background-color: #f8fafc;
        }
    )");
  translationText_->setVisible(false);
  cardLayout->addWidget(translationText_);

  mainLayout->addWidget(cardWidget_);

  emptyWidget_ = new QWidget(this);
  auto *emptyLayout = new QVBoxLayout(emptyWidget_);
  emptyLayout->setContentsMargins(24, 24, 24, 24);
  emptyLayout->setSpacing(6);

  auto *emptyIcon = new QLabel(QStringLiteral("📚"), this);
  emptyIcon->setStyleSheet("font-size: 46px;");
  emptyIcon->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyIcon);

  auto *emptyText =
      new QLabel(QStringLiteral("请从左侧「词库管理」选择一个词库开始学习"), this);
  emptyText->setStyleSheet("font-size: 18px; color: #0f172a;");
  emptyText->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyText);

  auto *emptySub =
      new QLabel(QStringLiteral("导入或选择词库后即可开始当日学习。"), this);
  emptySub->setStyleSheet("font-size: 14px; color: #94a3b8;");
  emptySub->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptySub);

  mainLayout->addWidget(emptyWidget_);

  cardWidget_->setVisible(false);
  emptyWidget_->setVisible(true);

  auto *tagLayout = new QHBoxLayout();

  difficultButton_ = new QPushButton(QStringLiteral("生词本"), this);
  difficultButton_->setCheckable(true);
  difficultButton_->setCursor(Qt::PointingHandCursor);
  difficultButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: #ffffff;
            border: 1px solid #e2e8f0;
            padding: 8px 14px;
            border-radius: 10px;
            color: #475569;
            font-weight: 600;
        }
        QPushButton:checked {
            background: #fffbeb;
            border-color: #fbbf24;
            color: #92400e;
        }
    )"));

  favoriteButton_ = new QPushButton(QStringLiteral("收藏"), this);
  favoriteButton_->setCheckable(true);
  favoriteButton_->setCursor(Qt::PointingHandCursor);
  favoriteButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: #ffffff;
            border: 1px solid #e2e8f0;
            padding: 8px 14px;
            border-radius: 10px;
            color: #475569;
            font-weight: 600;
        }
        QPushButton:checked {
            background: #fef3c7;
            border-color: #f59e0b;
            color: #92400e;
        }
    )"));

  tagLayout->addStretch();
  tagLayout->addWidget(difficultButton_);
  tagLayout->addWidget(favoriteButton_);
  tagLayout->addStretch();

  mainLayout->addLayout(tagLayout);

  auto *buttonLayout = new QHBoxLayout();

  unknownButton_ = new QPushButton(QStringLiteral("不认识"), this);
  unknownButton_->setCursor(Qt::PointingHandCursor);
  unknownButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 14px 36px;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 700;
        }
        QPushButton:hover { background: #d92d20; }
        QPushButton:pressed { background: #b42318; }
    )").arg(kDanger));
  unknownButton_->setEnabled(false);

  knownButton_ = new QPushButton(QStringLiteral("认识"), this);
  knownButton_->setCursor(Qt::PointingHandCursor);
  knownButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 14px 36px;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 700;
        }
        QPushButton:hover { background: #16a34a; }
        QPushButton:pressed { background: #15803d; }
    )").arg(kSuccess));
  knownButton_->setEnabled(false);

  buttonLayout->addStretch();
  buttonLayout->addWidget(unknownButton_);
  buttonLayout->addWidget(knownButton_);
  buttonLayout->addStretch();

  mainLayout->addLayout(buttonLayout);

  statusLabel_ = new QLabel(QStringLiteral("点击「开始学习」后按记忆情况标记"), this);
  statusLabel_->setStyleSheet(
      QString("color: %1; font-size: 14px;").arg(kMuted));
  statusLabel_->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(statusLabel_);

  connect(showButton_, &QPushButton::clicked, this,
          &StudyWidget::onShowTranslation);
  connect(unknownButton_, &QPushButton::clicked, this, &StudyWidget::onUnknown);
  connect(knownButton_, &QPushButton::clicked, this, &StudyWidget::onKnown);
  connect(difficultButton_, &QPushButton::clicked, this,
          &StudyWidget::onToggleDifficult);
  connect(favoriteButton_, &QPushButton::clicked, this,
          &StudyWidget::onToggleFavorite);
}

void StudyWidget::setBookId(const QString &bookId) { bookId_ = bookId; }

void StudyWidget::startNewSession() {
  if (bookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  session_ = service_->startSession(
      bookId_, Application::StudyService::StudySession::NewWords, 5);

  if (session_.wordIds.isEmpty()) {
    QMessageBox::information(this, QStringLiteral("完成"),
                             QStringLiteral("该词库所有单词已学习完毕"));
    return;
  }

  emptyWidget_->setVisible(false);
  cardWidget_->setVisible(true);

  updateProgress();
  loadCurrentWord();
}

void StudyWidget::loadCurrentWord() {
  if (!session_.hasNext()) {
    showSummary();
    return;
  }

  currentWord_ = service_->getCurrentWord(session_);

  if (currentWord_.word.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("错误"),
                         QStringLiteral("加载单词失败"));
    return;
  }

  resetCard();
  wordLabel_->setText(currentWord_.word);
  phoneticLabel_->setText(currentWord_.phoneticUs);

  wordStartTime_ = QTime::currentTime();
  statusLabel_->setText(QStringLiteral("请先回忆该单词的含义"));
}

void StudyWidget::resetCard() {
  translationVisible_ = false;
  translationText_->setVisible(false);
  translationText_->clear();
  showButton_->setVisible(true);
  unknownButton_->setEnabled(false);
  knownButton_->setEnabled(false);
}

void StudyWidget::onShowTranslation() {
  if (translationVisible_) {
    return;
  }

  translationVisible_ = true;

  QString content;

  QJsonDocument transDoc =
      QJsonDocument::fromJson(currentWord_.translations.toUtf8());
  if (transDoc.isArray()) {
    QJsonArray transArray = transDoc.array();
    content += "<h3 style='margin:0 0 8px 0;'>释义</h3><ul>";
    for (const QJsonValue &val : transArray) {
      QJsonObject obj = val.toObject();
      QString pos = obj["pos"].toString();
      QString cn = obj["cn"].toString();
      content += QString("<li><b>%1</b> %2</li>").arg(pos, cn);
    }
    content += "</ul>";
  }

  QJsonDocument sentDoc =
      QJsonDocument::fromJson(currentWord_.sentences.toUtf8());
  if (sentDoc.isArray()) {
    QJsonArray sentArray = sentDoc.array();
    if (!sentArray.isEmpty()) {
      content += "<h3 style='margin:12px 0 8px 0;'>例句</h3>";
      for (const QJsonValue &val : sentArray) {
        QJsonObject obj = val.toObject();
        QString en = obj["c"].toString();
        QString cn = obj["cn"].toString();
        content += QString("<p style='margin:6px 0;'><i>%1</i><br/>%2</p>")
                       .arg(en, cn);
      }
    }
  }

  translationText_->setHtml(content);
  translationText_->setVisible(true);
  showButton_->setVisible(false);

  unknownButton_->setEnabled(true);
  knownButton_->setEnabled(true);

  statusLabel_->setText(QStringLiteral("请标记是否认识该单词"));
}

void StudyWidget::onKnown() {
  int duration = wordStartTime_.secsTo(QTime::currentTime());

  Application::StudyService::StudyResult result;
  result.wordId = currentWord_.id;
  result.bookId = bookId_;
  result.known = true;
  result.duration = duration;

  service_->recordAndNext(session_, result);

  updateProgress();
  loadCurrentWord();
}

void StudyWidget::onUnknown() {
  int duration = wordStartTime_.secsTo(QTime::currentTime());

  Application::StudyService::StudyResult result;
  result.wordId = currentWord_.id;
  result.bookId = bookId_;
  result.known = false;
  result.duration = duration;

  service_->recordAndNext(session_, result);

  updateProgress();
  loadCurrentWord();
}

void StudyWidget::onPrevious() {
  // TODO: 实现上一个功能
}

void StudyWidget::onNext() {
  // TODO: 实现跳过功能
}

void StudyWidget::onToggleDifficult() {
  if (currentWord_.id > 0) {
    tagService_->toggleTag(currentWord_.id, Domain::WordTag::TAG_DIFFICULT);
  }
}

void StudyWidget::onToggleFavorite() {
  if (currentWord_.id > 0) {
    tagService_->toggleTag(currentWord_.id, Domain::WordTag::TAG_FAVORITE);
  }
}

void StudyWidget::updateProgress() {
  int progress = session_.getProgress();
  int total = session_.getTotal();

  progressBar_->setMaximum(total);
  progressBar_->setValue(progress);
  progressBar_->setFormat(QString("%1 / %2").arg(progress).arg(total));

  titleLabel_->setText(QStringLiteral("学习新单词 (%1/%2)").arg(progress).arg(total));
}

void StudyWidget::showSummary() {
  auto summary = service_->endSession(session_);

  QString message = QStringLiteral("本次学习完成！\n\n"
                                   "学习单词数：%1\n"
                                   "认识：%2\n"
                                   "不认识：%3\n"
                                   "学习时长：%4 秒")
                        .arg(summary.totalWords)
                        .arg(summary.knownWords)
                        .arg(summary.unknownWords)
                        .arg(summary.totalDuration);

  QMessageBox::information(this, QStringLiteral("学习完成"), message);

  wordLabel_->setText(QStringLiteral("学习完成"));
  phoneticLabel_->clear();
  translationText_->clear();
  translationText_->setVisible(false);
  showButton_->setVisible(false);
  unknownButton_->setEnabled(false);
  knownButton_->setEnabled(false);
  statusLabel_->setText(QStringLiteral("点击「开始学习」继续"));

  cardWidget_->setVisible(false);
  emptyWidget_->setVisible(true);
}

} // namespace Presentation
} // namespace WordMaster
