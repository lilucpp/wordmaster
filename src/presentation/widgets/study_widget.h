#ifndef WORDMASTER_PRESENTATION_STUDY_WIDGET_H
#define WORDMASTER_PRESENTATION_STUDY_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include "application/services/study_service.h"
#include "domain/repositories.h"

namespace WordMaster {
namespace Presentation {

/**
 * @brief 学习界面
 * 
 * 功能：
 * - 单词卡片展示
 * - 翻转显示释义
 * - 认识/不认识标记
 */
class StudyWidget : public QWidget {
    Q_OBJECT

public:
    explicit StudyWidget(Application::StudyService* service,
                        Domain::IWordRepository* wordRepo,
                        QWidget* parent = nullptr);

    void setBookId(const QString& bookId);
    void startNewSession();

private slots:
    void onShowTranslation();
    void onKnown();
    void onUnknown();
    void onPrevious();
    void onNext();

private:
    void setupUI();
    void loadCurrentWord();
    void updateProgress();
    void showSummary();
    void resetCard();

    Application::StudyService* service_;
    Domain::IWordRepository* wordRepo_;
    
    // 当前状态
    QString bookId_;
    Application::StudyService::StudySession session_;
    Domain::Word currentWord_;
    QTime wordStartTime_;
    bool translationVisible_;
    
    // UI 组件
    QLabel* titleLabel_;
    QProgressBar* progressBar_;
    QLabel* wordLabel_;
    QLabel* phoneticLabel_;
    QTextEdit* translationText_;
    QPushButton* showButton_;
    QPushButton* unknownButton_;
    QPushButton* knownButton_;
    QPushButton* prevButton_;
    QPushButton* nextButton_;
    QLabel* statusLabel_;
};

} // namespace Presentation
} // namespace WordMaster

#endif // WORDMASTER_PRESENTATION_STUDY_WIDGET_H