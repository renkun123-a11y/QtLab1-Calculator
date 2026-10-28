#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 计算器主窗口。
// 界面本身由 mainwindow.ui（Qt Designer 设计）描述，这里只负责把界面和行为连起来。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // 让 6x4 的按键网格均分行高与列宽（Designer 无法直接设置 QGridLayout 的均分系数）
    void uniformKeypadStretch();

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
