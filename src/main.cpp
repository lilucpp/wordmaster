#include <QApplication>
#include <QDebug>
#include "presentation/mainwindow.h"

int main(int argc, char *argv[]) {
    
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("WordMaster");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("WordMaster");
    
    qDebug() << "WordMaster starting...";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    
    // 创建并显示主窗口
    WordMaster::Presentation::MainWindow window;
    window.show();
    
    return app.exec();
}