#include <QApplication>
#include <QFont>
#include "mainwindow.h"

// 实验 1：带键盘事件的计算器
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 统一中文界面字体，避免不同系统上中英文混排字重不一致
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
