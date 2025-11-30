#include <QApplication>
#include <QDebug>
#include "infrastructure/sqlite_adapter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("WordMaster");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("WordMaster");
    
    qDebug() << "WordMaster starting...";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    
    // 测试数据库连接
    WordMaster::Infrastructure::SQLiteAdapter adapter("wordmaster.db");
    if (adapter.open()) {
        qDebug() << "Database opened successfully";
        
        // 初始化数据库
        if (adapter.initializeDatabase("../resources/database/001_initial_schema.sql")) {
            qDebug() << "Database initialized successfully";
        } else {
            qWarning() << "Failed to initialize database";
        }
    } else {
        qWarning() << "Failed to open database";
        return 1;
    }
    
    qDebug() << "Application initialized. Ready to start GUI...";
    
    // TODO: 显示主窗口
    // MainWindow window;
    // window.show();
    
    return 0; // 暂时直接退出，下一步实现GUI
}