#ifndef CALCULATORENGINE_H
#define CALCULATORENGINE_H

#include <QObject>
#include <QString>

#include "calckey.h"

// 计算器核心状态机。
//
// 这里只回答一个问题："按下这个键之后，显示区应该变成什么"。
// 它不接触任何界面控件，所以既能被 MainWindow 复用，也能被单元测试单独驱动。
class CalculatorEngine : public QObject
{
    Q_OBJECT

public:
    explicit CalculatorEngine(QObject *parent = nullptr);

    // 唯一的按键入口：鼠标与键盘都通过它驱动计算器
    void press(Calc::Key key);

    // 复位到初始状态
    void clearAll();

    QString displayText() const { return m_entry; }        // 主显示区
    QString expressionText() const { return m_expression; } // 上方表达式栏

    // 结果格式化：去掉浮点误差与多余的 0
    static QString formatNumber(double value);

signals:
    void displayChanged();

private:
    void inputDigit(int digit);
    void inputDot();
    void inputOperator(Calc::Key op);
    void inputEquals();
    void inputBackspace();
    void inputClearEntry();

    // 输入操作数过程中，表达式栏应显示的文本（如 "12 × 3"）
    QString currentExpressionText() const;

    double compute(double lhs, Calc::Key op, double rhs) const;

    QString m_entry = QStringLiteral("0"); // 正在显示 / 输入的数字
    QString m_expression;                  // 表达式栏文本
    double m_lhs = 0.0;                    // 左操作数
    Calc::Key m_pendingOperator = Calc::Key::Add;
    bool m_hasPendingOperator = false;
    bool m_startNewEntry = false;          // 下一个数字键是否要另起一个数
};

#endif // CALCULATORENGINE_H
