#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("川渝地区轨道交通客流数据分析展示系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Railway Analysis Team");
    
    // Set modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Set application icon (if available)
    // app.setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
} 