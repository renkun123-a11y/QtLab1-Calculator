#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>

#include "calckey.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CalculatorEngine;
class QPushButton;

// 计算器主窗口。
// 界面由 mainwindow.ui（Qt Designer 设计）描述，这里只负责"界面 <-> 计算核心"的接线。
//
// 鼠标点击与键盘按键都先翻译成 Calc::Key，再进入唯一的 dispatchKey()，
// 因此两种输入方式共用同一套处理逻辑，不存在"鼠标能用、键盘不对"的分叉。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    // 让 6x4 的按键网格均分行高与列宽（Designer 无法直接设置 QGridLayout 的均分系数）
    void uniformKeypadStretch();

    // 按动态属性 calcKey 把 24 个按键接到同一个分发入口，避免逐条 connect 的重复代码
    void connectKeypad();

    // 鼠标与键盘的统一入口
    void dispatchKey(Calc::Key key);

    // 键盘按下时让对应按键短暂显示"按下"效果，便于确认键位映射
    void flashButton(Calc::Key key);

    // 从 :/styles/app.qss 载入界面样式表
    void applyStyleSheet();

    // 把计算核心的状态显示到界面上
    void refreshDisplay();

    Ui::MainWindow *ui;
    CalculatorEngine *m_engine = nullptr;
    QHash<int, QPushButton *> m_buttonsByKey;   // 按键 -> 控件，用于键盘按下时的视觉反馈
};

#endif // MAINWINDOW_H
