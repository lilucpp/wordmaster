#ifndef WORDMASTER_PRESENTATION_NOTEBOOK_WIDGET_H
#define WORDMASTER_PRESENTATION_NOTEBOOK_WIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTabWidget>
#include "application/services/tag_service.h"
#include "domain/repositories.h"

namespace WordMaster {
namespace Presentation {

class NotebookWidget : public QWidget {
    Q_OBJECT

public:
    explicit NotebookWidget(Application::TagService* tagService,
                           Domain::IWordRepository* wordRepo,
                           QWidget* parent = nullptr);

    void refresh();

private:
    void setupUI();
    void loadWords(const QString& tagType, QListWidget* list);

    Application::TagService* tagService_;
    Domain::IWordRepository* wordRepo_;
    
    QTabWidget* tabWidget_;
    QListWidget* difficultList_;
    QListWidget* wrongList_;
    QListWidget* favoriteList_;
};

} // namespace Presentation
} // namespace WordMaster

#endif
