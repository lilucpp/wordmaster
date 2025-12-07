#include "review_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTime>

namespace WordMaster {
namespace Presentation {

ReviewWidget::ReviewWidget(Application::StudyService* service,
                          Domain::IWordRepository* wordRepo,
                          QWidget* parent)
    : QWidget(parent)
    , service_(service)
    , wordRepo_(wordRepo)
    , translationVisible_(false)
{
    setupUI();
}

void ReviewWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    
    // 标题和进度
    titleLabel_ = new QLabel("复习模式", this);
    titleLabel_->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel_);
    
    progressBar_ = new QProgressBar(this);
    mainLayout->addWidget(progressBar_);
    
    // 单词卡片（与学习界面相同）
    auto* cardWidget = new QWidget(this);
    cardWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
        }
    )");
    cardWidget->setMinimumHeight(400);
    
    auto* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setAlignment(Qt::AlignCenter);
    
    wordLabel_ = new QLabel("", this);
    wordLabel_->setStyleSheet("font-size: 48px; font-weight: bold; color: #1976d2;");
    wordLabel_->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(wordLabel_);
    
    phoneticLabel_ = new QLabel("", this);
    phoneticLabel_->setStyleSheet("font-size: 18px; color: #666;");
    phoneticLabel_->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(phoneticLabel_);
    
    cardLayout->addSpacing(30);
    
    showButton_ = new QPushButton("显示释义 ↓", this);
    showButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            padding: 10px 30px;
            border-radius: 5px;
            font-size: 16px;
        }
        QPushButton:hover { background-color: #45a049; }
    )");
    cardLayout->addWidget(showButton_, 0, Qt::AlignCenter);
    
    translationText_ = new QTextEdit(this);
    translationText_->setReadOnly(true);
    translationText_->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #e0e0e0;
            border-radius: 5px;
            padding: 15px;
            font-size: 14px;
            background-color: #f8f9fa;
        }
    )");
    translationText_->setVisible(false);
    cardLayout->addWidget(translationText_);
    
    mainLayout->addWidget(cardWidget);
    
    // 复习质量按钮（4个）
    auto* buttonLayout = new QHBoxLayout();
    
    againButton_ = new QPushButton("🔴 遗忘", this);
    againButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #da190b; }
    )");
    againButton_->setEnabled(false);
    
    hardButton_ = new QPushButton("🟡 困难", this);
    hardButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #ff9800;
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #f57c00; }
    )");
    hardButton_->setEnabled(false);
    
    goodButton_ = new QPushButton("🟢 良好", this);
    goodButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #45a049; }
    )");
    goodButton_->setEnabled(false);
    
    easyButton_ = new QPushButton("🔵 简单", this);
    easyButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #2196f3;
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #0b7dda; }
    )");
    easyButton_->setEnabled(false);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(againButton_);
    buttonLayout->addWidget(hardButton_);
    buttonLayout->addWidget(goodButton_);
    buttonLayout->addWidget(easyButton_);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // 提示信息
    auto* hintLabel = new QLabel("• 遗忘：1天后  • 困难：依然增长  • 良好：正常间隔  • 简单：大幅增长", this);
    hintLabel->setStyleSheet("color: #666; font-size: 12px;");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);
    
    statusLabel_ = new QLabel("请点击「开始复习」", this);
    statusLabel_->setStyleSheet("color: #666; font-size: 14px;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel_);
    
    // 连接信号
    connect(showButton_, &QPushButton::clicked, this, &ReviewWidget::onShowTranslation);
    connect(againButton_, &QPushButton::clicked, this, &ReviewWidget::onAgain);
    connect(hardButton_, &QPushButton::clicked, this, &ReviewWidget::onHard);
    connect(goodButton_, &QPushButton::clicked, this, &ReviewWidget::onGood);
    connect(easyButton_, &QPushButton::clicked, this, &ReviewWidget::onEasy);
}

void ReviewWidget::setBookId(const QString& bookId) {
    bookId_ = bookId;
    
    // 自动开始复习会话
    startReviewSession();
}

void ReviewWidget::startReviewSession() {
    if (bookId_.isEmpty()) {
        wordLabel_->setText("请先选择词库");
        phoneticLabel_->clear();
        statusLabel_->setText("在词库管理页面选择一个词库");
        return;
    }
    
    session_ = service_->startSession(
        bookId_,
        Application::StudyService::StudySession::Review,
        50  // 每次复习最多50个
    );
    
    if (session_.wordIds.isEmpty()) {
        wordLabel_->setText("暂无需要复习的单词");
        phoneticLabel_->clear();
        statusLabel_->setText("今天没有需要复习的单词，继续学习新单词吧！");
        return;
    }
    
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
        QMessageBox::warning(this, "错误", "加载单词失败");
        return;
    }
    
    resetCard();
    wordLabel_->setText(currentWord_.word);
    
    QString phonetic = currentWord_.phoneticUk;
    if (!currentWord_.phoneticUs.isEmpty()) {
        phonetic += " / " + currentWord_.phoneticUs;
    }
    phoneticLabel_->setText(phonetic);
    
    wordStartTime_ = QTime::currentTime();
    statusLabel_->setText("请先回忆该单词的含义");
}

void ReviewWidget::resetCard() {
    translationVisible_ = false;
    translationText_->setVisible(false);
    showButton_->setVisible(true);
    againButton_->setEnabled(false);
    hardButton_->setEnabled(false);
    goodButton_->setEnabled(false);
    easyButton_->setEnabled(false);
}

void ReviewWidget::onShowTranslation() {
    if (translationVisible_) return;
    
    translationVisible_ = true;
    
    // 解析显示释义（与学习界面相同）
    QString content;
    
    QJsonDocument transDoc = QJsonDocument::fromJson(currentWord_.translations.toUtf8());
    if (transDoc.isArray()) {
        QJsonArray transArray = transDoc.array();
        content += "<h3>释义：</h3><ul>";
        for (const QJsonValue& val : transArray) {
            QJsonObject obj = val.toObject();
            content += QString("<li><b>%1</b> %2</li>")
                .arg(obj["pos"].toString(), obj["cn"].toString());
        }
        content += "</ul>";
    }
    
    translationText_->setHtml(content);
    translationText_->setVisible(true);
    showButton_->setVisible(false);
    
    // 启用质量按钮
    againButton_->setEnabled(true);
    hardButton_->setEnabled(true);
    goodButton_->setEnabled(true);
    easyButton_->setEnabled(true);
    
    statusLabel_->setText("请根据回忆的难易程度选择");
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
    recordResult(false, duration);  // 遗忘 = 不认识
}

void ReviewWidget::onHard() {
    int duration = 15;  // Hard: 假设15秒
    recordResult(true, duration);
}

void ReviewWidget::onGood() {
    int duration = 8;  // Good: 假设8秒
    recordResult(true, duration);
}

void ReviewWidget::onEasy() {
    int duration = 2;  // Easy: 假设2秒
    recordResult(true, duration);
}

void ReviewWidget::updateProgress() {
    progressBar_->setMaximum(session_.getTotal());
    progressBar_->setValue(session_.getProgress());
    progressBar_->setFormat(QString("%1 / %2")
        .arg(session_.getProgress()).arg(session_.getTotal()));
    
    titleLabel_->setText(QString("复习模式 (%1/%2)")
        .arg(session_.getProgress()).arg(session_.getTotal()));
}

void ReviewWidget::showSummary() {
    auto summary = service_->endSession(session_);
    
    QString message = QString(
        "复习完成！\n\n"
        "复习单词数：%1\n"
        "掌握良好：%2\n"
        "需要加强：%3\n"
    ).arg(summary.totalWords)
     .arg(summary.knownWords)
     .arg(summary.unknownWords);
    
    QMessageBox::information(this, "复习完成", message);
    
    wordLabel_->setText("复习完成");
    phoneticLabel_->clear();
    statusLabel_->setText("点击「开始复习」继续");
}

} // namespace Presentation
} // namespace WordMaster