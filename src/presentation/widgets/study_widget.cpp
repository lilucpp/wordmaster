#include "study_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTime>

namespace WordMaster {
namespace Presentation {

StudyWidget::StudyWidget(Application::StudyService* service,
                        Domain::IWordRepository* wordRepo,
                        QWidget* parent)
    : QWidget(parent)
    , service_(service)
    , wordRepo_(wordRepo)
    , translationVisible_(false)
{
    setupUI();
}

void StudyWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    
    // 标题和进度
    titleLabel_ = new QLabel("学习新单词", this);
    titleLabel_->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel_);
    
    progressBar_ = new QProgressBar(this);
    progressBar_->setTextVisible(true);
    mainLayout->addWidget(progressBar_);
    
    // 单词卡片
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
    
    // 单词
    wordLabel_ = new QLabel("", this);
    wordLabel_->setStyleSheet(R"(
        font-size: 48px;
        font-weight: bold;
        color: #1976d2;
    )");
    wordLabel_->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(wordLabel_);
    
    // 音标
    phoneticLabel_ = new QLabel("", this);
    phoneticLabel_->setStyleSheet("font-size: 18px; color: #666;");
    phoneticLabel_->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(phoneticLabel_);
    
    cardLayout->addSpacing(30);
    
    // 显示释义按钮
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
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    cardLayout->addWidget(showButton_, 0, Qt::AlignCenter);
    
    // 释义区域（初始隐藏）
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
    
    // 操作按钮
    auto* buttonLayout = new QHBoxLayout();
    
    unknownButton_ = new QPushButton("❌ 不认识", this);
    unknownButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            padding: 15px 40px;
            border-radius: 5px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #da190b;
        }
    )");
    unknownButton_->setEnabled(false);
    
    knownButton_ = new QPushButton("✅ 认识", this);
    knownButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            padding: 15px 40px;
            border-radius: 5px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    knownButton_->setEnabled(false);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(unknownButton_);
    buttonLayout->addWidget(knownButton_);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // 状态栏
    statusLabel_ = new QLabel("请点击「开始学习」", this);
    statusLabel_->setStyleSheet("color: #666; font-size: 14px;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel_);
    
    // 连接信号
    connect(showButton_, &QPushButton::clicked, this, &StudyWidget::onShowTranslation);
    connect(unknownButton_, &QPushButton::clicked, this, &StudyWidget::onUnknown);
    connect(knownButton_, &QPushButton::clicked, this, &StudyWidget::onKnown);
}

void StudyWidget::setBookId(const QString& bookId) {
    bookId_ = bookId;
}

void StudyWidget::startNewSession() {
    if (bookId_.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个词库");
        return;
    }
    
    // 开始新会话（每次20个单词）
    session_ = service_->startSession(
        bookId_,
        Application::StudyService::StudySession::NewWords,
        20
    );
    
    if (session_.wordIds.isEmpty()) {
        QMessageBox::information(this, "完成", "该词库所有单词已学完！");
        return;
    }
    
    updateProgress();
    loadCurrentWord();
}

void StudyWidget::loadCurrentWord() {
    if (!session_.hasNext()) {
        showSummary();
        return;
    }
    
    // 加载当前单词
    currentWord_ = service_->getCurrentWord(session_);
    
    if (currentWord_.word.isEmpty()) {
        QMessageBox::warning(this, "错误", "加载单词失败");
        return;
    }
    
    // 重置卡片
    resetCard();
    
    // 显示单词
    wordLabel_->setText(currentWord_.word);
    
    // 显示音标
    QString phonetic = currentWord_.phoneticUk;
    if (!currentWord_.phoneticUs.isEmpty()) {
        phonetic += " / " + currentWord_.phoneticUs;
    }
    phoneticLabel_->setText(phonetic);
    
    // 记录开始时间
    wordStartTime_ = QTime::currentTime();
    
    // 更新状态
    statusLabel_->setText("请先回忆该单词的含义，然后点击「显示释义」");
}

void StudyWidget::resetCard() {
    translationVisible_ = false;
    translationText_->setVisible(false);
    showButton_->setVisible(true);
    unknownButton_->setEnabled(false);
    knownButton_->setEnabled(false);
}

void StudyWidget::onShowTranslation() {
    if (translationVisible_) {
        return;
    }
    
    translationVisible_ = true;
    
    // 解析并显示释义
    QString content;
    
    // 释义
    QJsonDocument transDoc = QJsonDocument::fromJson(currentWord_.translations.toUtf8());
    if (transDoc.isArray()) {
        QJsonArray transArray = transDoc.array();
        content += "<h3>释义：</h3><ul>";
        for (const QJsonValue& val : transArray) {
            QJsonObject obj = val.toObject();
            QString pos = obj["pos"].toString();
            QString cn = obj["cn"].toString();
            content += QString("<li><b>%1</b> %2</li>").arg(pos, cn);
        }
        content += "</ul>";
    }
    
    // 例句
    QJsonDocument sentDoc = QJsonDocument::fromJson(currentWord_.sentences.toUtf8());
    if (sentDoc.isArray()) {
        QJsonArray sentArray = sentDoc.array();
        if (!sentArray.isEmpty()) {
            content += "<h3>例句：</h3>";
            for (const QJsonValue& val : sentArray) {
                QJsonObject obj = val.toObject();
                QString en = obj["c"].toString();
                QString cn = obj["cn"].toString();
                content += QString("<p><i>%1</i><br/>%2</p>").arg(en, cn);
            }
        }
    }
    
    translationText_->setHtml(content);
    translationText_->setVisible(true);
    showButton_->setVisible(false);
    
    // 启用按钮
    unknownButton_->setEnabled(true);
    knownButton_->setEnabled(true);
    
    statusLabel_->setText("请标记是否认识该单词");
}

void StudyWidget::onKnown() {
    // 计算学习时长
    int duration = wordStartTime_.secsTo(QTime::currentTime());
    
    // 记录结果
    Application::StudyService::StudyResult result;
    result.wordId = currentWord_.id;
    result.bookId = bookId_;
    result.known = true;
    result.duration = duration;
    
    service_->recordAndNext(session_, result);
    
    // 更新进度并加载下一个
    updateProgress();
    loadCurrentWord();
}

void StudyWidget::onUnknown() {
    // 计算学习时长
    int duration = wordStartTime_.secsTo(QTime::currentTime());
    
    // 记录结果
    Application::StudyService::StudyResult result;
    result.wordId = currentWord_.id;
    result.bookId = bookId_;
    result.known = false;
    result.duration = duration;
    
    service_->recordAndNext(session_, result);
    
    // 更新进度并加载下一个
    updateProgress();
    loadCurrentWord();
}

void StudyWidget::onPrevious() {
    // TODO: 实现上一个功能
}

void StudyWidget::onNext() {
    // TODO: 实现跳过功能
}

void StudyWidget::updateProgress() {
    int progress = session_.getProgress();
    int total = session_.getTotal();
    
    progressBar_->setMaximum(total);
    progressBar_->setValue(progress);
    progressBar_->setFormat(QString("%1 / %2").arg(progress).arg(total));
    
    titleLabel_->setText(QString("学习新单词 (%1/%2)").arg(progress).arg(total));
}

void StudyWidget::showSummary() {
    auto summary = service_->endSession(session_);
    
    QString message = QString(
        "本次学习完成！\n\n"
        "学习单词数：%1\n"
        "认识：%2\n"
        "不认识：%3\n"
        "学习时长：%4 秒"
    ).arg(summary.totalWords)
     .arg(summary.knownWords)
     .arg(summary.unknownWords)
     .arg(summary.totalDuration);
    
    QMessageBox::information(this, "学习完成", message);
    
    // 重置界面
    wordLabel_->setText("学习完成");
    phoneticLabel_->clear();
    translationText_->clear();
    translationText_->setVisible(false);
    showButton_->setVisible(false);
    unknownButton_->setEnabled(false);
    knownButton_->setEnabled(false);
    statusLabel_->setText("点击「开始学习」继续学习");
}

} // namespace Presentation
} // namespace WordMaster