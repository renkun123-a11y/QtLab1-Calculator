#include <QApplication>
#include <QLabel>
#include <QMainWindow>

// 实验 1：带键盘事件的计算器
// 当前阶段仅搭建工程骨架，确认 qmake / MinGW 编译链可用，界面随后实现。
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("带键盘事件的计算器"));

    auto *placeholder = new QLabel(QStringLiteral("工程骨架（界面待实现）"), &window);
    placeholder->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(placeholder);

    window.resize(420, 660);
    window.show();
    return app.exec();
}
