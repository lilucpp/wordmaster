#ifndef WORDMASTER_PRESENTATION_BOOK_LIST_WIDGET_H
#define WORDMASTER_PRESENTATION_BOOK_LIST_WIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "application/services/book_service.h"

namespace WordMaster {
namespace Presentation {

/**
 * @brief 词库列表组件
 */
class BookListWidget : public QWidget {
    Q_OBJECT

public:
    explicit BookListWidget(Application::BookService* service, 
                           QWidget* parent = nullptr);

    void refresh();

signals:
    void bookSelected(const QString& bookId);
    void importRequested();
    void studyRequested(const QString& bookId);

private slots:
    void onBookItemClicked(QListWidgetItem* item);
    void onImportClicked();
    void onStudyClicked();
    void onDeleteClicked();

private:
    void setupUI();
    void loadBooks();
    QWidget* createBookCard(const Domain::Book& book,
                           const Application::BookService::BookStatistics& stats);

    Application::BookService* service_;
    
    // UI 组件
    QListWidget* bookList_;
    QPushButton* importButton_;
    QPushButton* refreshButton_;
    QLabel* titleLabel_;
    
    QString selectedBookId_;
};

} // namespace Presentation
} // namespace WordMaster

#endif // WORDMASTER_PRESENTATION_BOOK_LIST_WIDGET_H