#ifndef WORDMASTER_PRESENTATION_REVIEW_WIDGET_H
#define WORDMASTER_PRESENTATION_REVIEW_WIDGET_H

#include <QWidget>
#include "application/services/study_service.h"
#include "domain/repositories.h"

// 前向声明
class QLabel;
class QPushButton;
class QTextEdit;
class QProgressBar;

namespace WordMaster {
namespace Presentation {

/**
 * @brief 复习界面
 * 
 * 与学习界面类似，但增加了质量评分
 */
class ReviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReviewWidget(Application::StudyService* service,
                         Domain::IWordRepository* wordRepo,
                         QWidget* parent = nullptr);

    void setBookId(const QString& bookId);
    void startReviewSession();

private slots:
    void onShowTranslation();
    void onAgain();    // 完全不记得
    void onHard();     // 困难
    void onGood();     // 良好
    void onEasy();     // 简单

private:
    void setupUI();
    void loadCurrentWord();
    void updateProgress();
    void showSummary();
    void resetCard();
    void recordResult(bool known, int duration);

    Application::StudyService* service_;
    Domain::IWordRepository* wordRepo_;
    
    QString bookId_;
    Application::StudyService::StudySession session_;
    Domain::Word currentWord_;
    QTime wordStartTime_;
    bool translationVisible_;
    
    // UI 组件
    QWidget* cardWidget_;
    QWidget* emptyWidget_;
    QLabel* titleLabel_;
    QProgressBar* progressBar_;
    QLabel* wordLabel_;
    QLabel* phoneticLabel_;
    QTextEdit* translationText_;
    QPushButton* showButton_;
    QPushButton* againButton_;
    QPushButton* hardButton_;
    QPushButton* goodButton_;
    QPushButton* easyButton_;
    QLabel* statusLabel_;
};

} // namespace Presentation
} // namespace WordMaster

#endif // WORDMASTER_PRESENTATION_REVIEW_WIDGET_H