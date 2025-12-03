#include "review_widget.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>

namespace {
const QString kAccent = "#2d8cff";
const QString kDanger = "#ef4444";
const QString kCaution = "#f59e0b";
const QString kSuccess = "#16a34a";
const QString kMuted = "#94a3b8";
}

namespace WordMaster {
namespace Presentation {

ReviewWidget::ReviewWidget(Application::StudyService *service,
                           Domain::IWordRepository *wordRepo, QWidget *parent)
    : QWidget(parent), service_(service), wordRepo_(wordRepo),
      translationVisible_(false) {
  setupUI();
}

void ReviewWidget::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(18);

  titleLabel_ = new QLabel(QStringLiteral("复习模式"), this);
  titleLabel_->setStyleSheet(
      "font-size: 24px; font-weight: 700; color: #0f172a;");
  mainLayout->addWidget(titleLabel_);

  progressBar_ = new QProgressBar(this);
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
  cardWidget_->setObjectName("reviewCard");
  cardWidget_->setStyleSheet(R"(
        QWidget#reviewCard {
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
  wordLabel_->setStyleSheet(
      "font-size: 48px; font-weight: 800; color: #0f172a;");
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

  auto *emptyIcon = new QLabel(QStringLiteral("📒"), this);
  emptyIcon->setStyleSheet("font-size: 46px;");
  emptyIcon->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyIcon);

  auto *emptyText = new QLabel(QStringLiteral("暂无需要复习的单词"), this);
  emptyText->setStyleSheet("font-size: 18px; color: #0f172a;");
  emptyText->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptyText);

  auto *emptySub =
      new QLabel(QStringLiteral("请先在「学习」中完成新词学习"), this);
  emptySub->setStyleSheet("font-size: 14px; color: #94a3b8;");
  emptySub->setAlignment(Qt::AlignCenter);
  emptyLayout->addWidget(emptySub);

  mainLayout->addWidget(emptyWidget_);

  cardWidget_->setVisible(false);
  emptyWidget_->setVisible(true);

  auto *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(12);

  againButton_ = new QPushButton(QStringLiteral("忘记"), this);
  againButton_->setCursor(Qt::PointingHandCursor);
  againButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 12px 18px;
            border-radius: 12px;
            font-weight: 700;
        }
        QPushButton:hover { background: #d92d20; }
        QPushButton:pressed { background: #b42318; }
    )").arg(kDanger));
  againButton_->setEnabled(false);

  hardButton_ = new QPushButton(QStringLiteral("困难"), this);
  hardButton_->setCursor(Qt::PointingHandCursor);
  hardButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 12px 18px;
            border-radius: 12px;
            font-weight: 700;
        }
        QPushButton:hover { background: #d97706; }
        QPushButton:pressed { background: #b45309; }
    )").arg(kCaution));
  hardButton_->setEnabled(false);

  goodButton_ = new QPushButton(QStringLiteral("良好"), this);
  goodButton_->setCursor(Qt::PointingHandCursor);
  goodButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 12px 18px;
            border-radius: 12px;
            font-weight: 700;
        }
        QPushButton:hover { background: #2375e1; }
        QPushButton:pressed { background: #1e63c0; }
    )").arg(kAccent));
  goodButton_->setEnabled(false);

  easyButton_ = new QPushButton(QStringLiteral("简单"), this);
  easyButton_->setCursor(Qt::PointingHandCursor);
  easyButton_->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            padding: 12px 18px;
            border-radius: 12px;
            font-weight: 700;
        }
        QPushButton:hover { background: #16a34a; }
        QPushButton:pressed { background: #15803d; }
    )").arg(kSuccess));
  easyButton_->setEnabled(false);

  buttonLayout->addStretch();
  buttonLayout->addWidget(againButton_);
  buttonLayout->addWidget(hardButton_);
  buttonLayout->addWidget(goodButton_);
  buttonLayout->addWidget(easyButton_);
  buttonLayout->addStretch();

  mainLayout->addLayout(buttonLayout);

  statusLabel_ = new QLabel(QStringLiteral("先回忆含义，再选择记忆难度"), this);
  statusLabel_->setStyleSheet(
      QString("color: %1; font-size: 14px;").arg(kMuted));
  statusLabel_->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(statusLabel_);

  connect(showButton_, &QPushButton::clicked, this,
          &ReviewWidget::onShowTranslation);
  connect(againButton_, &QPushButton::clicked, this, &ReviewWidget::onAgain);
  connect(hardButton_, &QPushButton::clicked, this, &ReviewWidget::onHard);
  connect(goodButton_, &QPushButton::clicked, this, &ReviewWidget::onGood);
  connect(easyButton_, &QPushButton::clicked, this, &ReviewWidget::onEasy);
}

void ReviewWidget::setBookId(const QString &bookId) { bookId_ = bookId; }

void ReviewWidget::startReviewSession() {
  if (bookId_.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("请先选择一个词库"));
    return;
  }

  session_ = service_->startSession(
      bookId_, Application::StudyService::StudySession::Review, 50);

  if (session_.wordIds.isEmpty()) {
    wordLabel_->setText(QStringLiteral("暂无需要复习的单词"));
    phoneticLabel_->clear();
    statusLabel_->setText(
        QStringLiteral("今天没有需要复习的单词，继续学习新单词吧！"));

    cardWidget_->setVisible(false);
    emptyWidget_->setVisible(true);
    return;
  }

  emptyWidget_->setVisible(false);
  cardWidget_->setVisible(true);

  updateProgress();
  loadCurrentWord();
}

void ReviewWidget::loadCurrentWord() {
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

void ReviewWidget::resetCard() {
  translationVisible_ = false;
  translationText_->setVisible(false);
  translationText_->clear();
  showButton_->setVisible(true);
  againButton_->setEnabled(false);
  hardButton_->setEnabled(false);
  goodButton_->setEnabled(false);
  easyButton_->setEnabled(false);
}

void ReviewWidget::onShowTranslation() {
  if (translationVisible_)
    return;

  translationVisible_ = true;

  QString content;

  QJsonDocument transDoc =
      QJsonDocument::fromJson(currentWord_.translations.toUtf8());
  if (transDoc.isArray()) {
    QJsonArray transArray = transDoc.array();
    content += "<h3 style='margin:0 0 8px 0;'>释义</h3><ul>";
    for (const QJsonValue &val : transArray) {
      QJsonObject obj = val.toObject();
      content += QString("<li><b>%1</b> %2</li>")
                     .arg(obj["pos"].toString(), obj["cn"].toString());
    }
    content += "</ul>";
  }

  translationText_->setHtml(content);
  translationText_->setVisible(true);
  showButton_->setVisible(false);

  againButton_->setEnabled(true);
  hardButton_->setEnabled(true);
  goodButton_->setEnabled(true);
  easyButton_->setEnabled(true);

  statusLabel_->setText(QStringLiteral("请根据记忆难度选择"));
}

void ReviewWidget::recordResult(bool known, int duration) {
  Application::StudyService::StudyResult result;
  result.wordId = currentWord_.id;
  result.bookId = bookId_;
  result.known = known;
  result.duration = duration;

  service_->recordAndNext(session_, result);
  updateProgress();
  loadCurrentWord();
}

void ReviewWidget::onAgain() {
  int duration = wordStartTime_.secsTo(QTime::currentTime());
  recordResult(false, duration);
}

void ReviewWidget::onHard() {
  int duration = 15;
  recordResult(true, duration);
}

void ReviewWidget::onGood() {
  int duration = 8;
  recordResult(true, duration);
}

void ReviewWidget::onEasy() {
  int duration = 2;
  recordResult(true, duration);
}

void ReviewWidget::updateProgress() {
  progressBar_->setMaximum(session_.getTotal());
  progressBar_->setValue(session_.getProgress());
  progressBar_->setFormat(
      QString("%1 / %2").arg(session_.getProgress()).arg(session_.getTotal()));

  titleLabel_->setText(QStringLiteral("复习模式 (%1/%2)")
                           .arg(session_.getProgress())
                           .arg(session_.getTotal()));
}

void ReviewWidget::showSummary() {
  auto summary = service_->endSession(session_);

  QString message = QStringLiteral("复习完成！\n\n"
                                   "复习单词数：%1\n"
                                   "掌握良好：%2\n"
                                   "需要加强：%3\n")
                        .arg(summary.totalWords)
                        .arg(summary.knownWords)
                        .arg(summary.unknownWords);

  QMessageBox::information(this, QStringLiteral("复习完成"), message);

  wordLabel_->setText(QStringLiteral("复习完成"));
  phoneticLabel_->clear();
  statusLabel_->setText(QStringLiteral("点击「开始复习」继续"));

  cardWidget_->setVisible(false);
  emptyWidget_->setVisible(true);
}

} // namespace Presentation
} // namespace WordMaster
