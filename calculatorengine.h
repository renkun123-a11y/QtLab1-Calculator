#ifndef CALCULATORENGINE_H
#define CALCULATORENGINE_H

#include <QObject>
#include <QString>

#include <optional>

#include "calckey.h"

// 计算器核心状态机。
//
// 这里只回答一个问题："按下这个键之后，显示区应该变成什么"。
// 它不接触任何界面控件，所以既能被 MainWindow 复用，也能被单元测试单独驱动。
//
// 用显式的 State 而不是若干布尔标志来描述输入阶段，是因为"同一个按键在不同阶段
// 含义不同"（例如数字键可能是开始新数、可能是第二个操作数、也可能是开始新的计算），
// 状态一旦写清楚，连续运算、除零、结果后继续输入这些边界情况就不会互相打架。
class CalculatorEngine : public QObject
{
    Q_OBJECT

public:
    // 输入阶段
    enum class State {
        EnteringFirstOperand,   // 正在输入第一个操作数
        OperatorPending,        // 已按下运算符，等待输入第二个操作数
        EnteringSecondOperand,  // 正在输入第二个操作数
        ShowingResult,          // 正在显示计算结果
        Error                   // 错误状态（除数为 0、负数开平方等）
    };
    Q_ENUM(State)

    explicit CalculatorEngine(QObject *parent = nullptr);

    // 唯一的按键入口：鼠标与键盘都通过它驱动计算器
    void press(Calc::Key key);

    // 复位到初始状态
    void clearAll();

    QString displayText() const;         // 主显示区文本
    QString expressionText() const { return m_expression; }  // 表达式栏文本
    State state() const { return m_state; }
    bool hasError() const { return m_state == State::Error; }

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
    void applyUnary(Calc::Key op);       // ± % 1/x x² √x

    // 按运算符计算结果；除数为 0 等非法情形会转入错误状态并返回 false
    bool applyBinary(Calc::Key op, double lhs, double rhs);

    // 回到"等待输入第一个操作数"的干净状态
    void startFreshCalculation();

    // 进入错误状态并给出提示文本
    void enterError(const QString &message);

    // 输入操作数过程中刷新表达式栏（如 "12 × 3"）
    void updateExpression();
    QString currentExpressionText() const;

    double entryValue() const;           // 当前输入文本对应的数值

    QString m_entry = QStringLiteral("0");  // 正在显示 / 输入的数字
    QString m_expression;                   // 表达式栏文本
    QString m_errorMessage;                 // 错误提示
    double m_lhs = 0.0;                     // 左操作数（也用于保存中间结果）
    std::optional<Calc::Key> m_pendingOp;   // 待执行的二元运算
    std::optional<Calc::Key> m_repeatOp;    // 上一次等号执行的运算，用于重复按等号
    double m_repeatRhs = 0.0;               // 重复运算时的右操作数
    State m_state = State::EnteringFirstOperand;
};

#endif // CALCULATORENGINE_H
