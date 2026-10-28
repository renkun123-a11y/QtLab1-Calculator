#include "calculatorengine.h"

#include <cmath>

namespace {
// 单个操作数允许输入的最多字符数，避免无限追加把显示区撑爆
constexpr int kMaxEntryLength = 16;
} // namespace

CalculatorEngine::CalculatorEngine(QObject *parent)
    : QObject(parent)
{
}

void CalculatorEngine::press(Calc::Key key)
{
    if (Calc::isDigit(key)) {
        inputDigit(Calc::digitValue(key));
        emit displayChanged();
        return;
    }
    if (Calc::isBinaryOperator(key)) {
        inputOperator(key);
        emit displayChanged();
        return;
    }

    switch (key) {
    case Calc::Key::Dot:        inputDot();        break;
    case Calc::Key::Equals:     inputEquals();     break;
    case Calc::Key::Backspace:  inputBackspace();   break;
    case Calc::Key::ClearEntry: inputClearEntry();  break;
    case Calc::Key::ClearAll:   clearAll();         break;
    case Calc::Key::Negate:
    case Calc::Key::Percent:
    case Calc::Key::Reciprocal:
    case Calc::Key::Square:
    case Calc::Key::Sqrt:
        applyUnary(key);
        break;
    default:
        break;   // 数字键与二元运算符已在上面处理
    }

    emit displayChanged();
}

void CalculatorEngine::clearAll()
{
    startFreshCalculation();
}

QString CalculatorEngine::displayText() const
{
    return m_state == State::Error ? m_errorMessage : m_entry;
}

// ---------------------------------------------------------------- 输入数字 / 小数点

void CalculatorEngine::inputDigit(int digit)
{
    const QString text = QString::number(digit);

    if (m_state == State::Error || m_state == State::ShowingResult) {
        // 出错后、出结果后按数字键：都视为开始一次新的计算
        startFreshCalculation();
    } else if (m_state == State::OperatorPending) {
        // 运算符之后按数字键：开始输入第二个操作数
        m_state = State::EnteringSecondOperand;
        m_entry = text;
        updateExpression();
        return;
    }

    // EnteringFirstOperand / EnteringSecondOperand：继续在当前操作数后追加
    if (m_entry == QLatin1String("0"))
        m_entry = text;                         // 去掉多余的前导 0
    else if (m_entry == QLatin1String("-0"))
        m_entry = QLatin1Char('-') + text;
    else if (m_entry.size() < kMaxEntryLength)
        m_entry += text;                        // 超出长度上限则忽略本次输入

    updateExpression();
}

void CalculatorEngine::inputDot()
{
    if (m_state == State::Error || m_state == State::ShowingResult)
        startFreshCalculation();

    if (m_state == State::OperatorPending) {
        m_state = State::EnteringSecondOperand;
        m_entry = QStringLiteral("0.");
        updateExpression();
        return;
    }

    if (m_entry.contains(QLatin1Char('.')))
        return;                                 // 同一个操作数只允许一个小数点

    if (m_entry == QLatin1String("-0"))
        m_entry = QStringLiteral("-0.");
    else
        m_entry += QLatin1Char('.');

    updateExpression();
}

// ---------------------------------------------------------------- 运算符 / 等号

void CalculatorEngine::inputOperator(Calc::Key op)
{
    if (m_state == State::Error)
        return;                                 // 错误状态只接受 C / CE

    switch (m_state) {
    case State::EnteringFirstOperand:
    case State::ShowingResult:
        m_lhs = entryValue();                   // 当前显示值成为左操作数
        break;
    case State::EnteringSecondOperand: {
        if (!m_pendingOp.has_value())
            break;
        // 连续运算：先把上一步算出来当作新的左操作数，再记下新运算符
        if (!applyBinary(*m_pendingOp, m_lhs, entryValue()))
            return;                             // 除数为 0 等，已进入错误状态
        break;
    }
    case State::OperatorPending:
    default:
        break;                                  // 连续按运算符：以最后一次按下的为准
    }

    m_pendingOp = op;
    m_repeatOp.reset();                         // 手动输入了运算符，不再重复上一次等号运算
    m_state = State::OperatorPending;
    updateExpression();
}

void CalculatorEngine::inputEquals()
{
    if (m_state == State::Error)
        return;

    if (m_state == State::EnteringFirstOperand) {
        // 只输入了一个数就按等号：结果就是它自己
        m_state = State::ShowingResult;
        m_expression = m_entry + QStringLiteral(" =");
        m_repeatOp.reset();
        return;
    }

    double lhs = m_lhs;
    double rhs = 0.0;
    Calc::Key op = Calc::Key::Equals;

    switch (m_state) {
    case State::ShowingResult:
        if (!m_repeatOp.has_value())
            return;                             // 没有可以重复的运算
        op = *m_repeatOp;
        lhs = entryValue();
        rhs = m_repeatRhs;                      // 连按等号：重复"上一次运算"
        break;
    case State::OperatorPending:
        if (!m_pendingOp.has_value())
            return;
        op = *m_pendingOp;
        rhs = m_lhs;                            // 缺第二个操作数时用左操作数：5 + = 得 10
        break;
    case State::EnteringSecondOperand:
        if (!m_pendingOp.has_value())
            return;
        op = *m_pendingOp;
        rhs = entryValue();
        break;
    default:
        return;
    }

    const QString lhsText = formatNumber(lhs);
    const QString rhsText = formatNumber(rhs);

    m_pendingOp.reset();
    m_state = State::ShowingResult;
    m_repeatOp = op;
    m_repeatRhs = rhs;
    m_expression = lhsText + QLatin1Char(' ') + Calc::operatorSymbol(op)
                 + QLatin1Char(' ') + rhsText + QStringLiteral(" =");

    // 计算失败时 applyBinary 会把表达式改成出错的那一步并转入错误状态
    applyBinary(op, lhs, rhs);
}

// ---------------------------------------------------------------- 退格 / 清除

void CalculatorEngine::inputBackspace()
{
    if (m_state != State::EnteringFirstOperand && m_state != State::EnteringSecondOperand)
        return;   // 结果、等待输入、错误状态下没有可删的字符

    if (m_entry.size() <= 1) {
        m_entry = QStringLiteral("0");
    } else {
        m_entry.chop(1);                        // 删除最后一个字符
        if (m_entry == QLatin1String("-"))
            m_entry = QStringLiteral("0");      // 只剩负号时回到 0
    }

    updateExpression();
}

void CalculatorEngine::inputClearEntry()
{
    switch (m_state) {
    case State::Error:
    case State::ShowingResult:
        // 错误状态、结果状态下的 CE 等同于重新开始
        clearAll();
        break;
    case State::OperatorPending:
        // 等待第二个操作数时按 CE：清掉即将输入的内容，运算符与左操作数保留
        m_entry = QStringLiteral("0");
        break;
    case State::EnteringFirstOperand:
    case State::EnteringSecondOperand:
        m_entry = QStringLiteral("0");
        updateExpression();
        break;
    }
}

// ---------------------------------------------------------------- 一元运算

void CalculatorEngine::applyUnary(Calc::Key op)
{
    if (m_state == State::Error)
        return;

    if (m_state == State::OperatorPending) {
        // 还没输入第二个操作数：一元运算作用在"即将输入的 0"上
        m_entry = QStringLiteral("0");
        m_state = State::EnteringSecondOperand;
    }

    if (op == Calc::Key::Negate) {
        // 正负号只翻转符号，不改变输入阶段，因此可以继续往后输入数字
        if (m_entry == QLatin1String("0") || m_entry == QLatin1String("-0"))
            return;
        if (m_entry.startsWith(QLatin1Char('-')))
            m_entry.remove(0, 1);
        else
            m_entry.prepend(QLatin1Char('-'));
        updateExpression();
        return;
    }

    const State stateBefore = m_state;
    const double value = entryValue();
    const QString valueText = formatNumber(value);

    double result = 0.0;
    QString expression;

    switch (op) {
    case Calc::Key::Percent:
        result = value / 100.0;
        expression = valueText + QLatin1Char('%');
        break;
    case Calc::Key::Reciprocal:
        if (value == 0.0) {
            m_expression = QStringLiteral("1/(") + valueText + QStringLiteral(") =");
            enterError(QStringLiteral("除数不能为 0"));
            return;
        }
        result = 1.0 / value;
        expression = QStringLiteral("1/(") + valueText + QLatin1Char(')');
        break;
    case Calc::Key::Square:
        result = value * value;
        expression = QLatin1Char('(') + valueText + QStringLiteral(")\u00b2");
        break;
    case Calc::Key::Sqrt:
        if (value < 0.0) {
            m_expression = QStringLiteral("\u221a(") + valueText + QStringLiteral(") =");
            enterError(QStringLiteral("负数不能开平方"));
            return;
        }
        result = std::sqrt(value);
        expression = QStringLiteral("\u221a(") + valueText + QLatin1Char(')');
        break;
    default:
        return;
    }

    if (!std::isfinite(result)) {
        enterError(QStringLiteral("数值超出范围"));
        return;
    }

    m_entry = formatNumber(result);
    m_lhs = result;                             // 一元运算的结果可直接参与后续运算

    if (stateBefore == State::EnteringSecondOperand && m_pendingOp.has_value()) {
        // 对第二个操作数做一元运算：保留待执行的二元运算，等号仍能算出结果
        m_state = State::EnteringSecondOperand;
        m_expression = currentExpressionText();
    } else {
        m_state = State::ShowingResult;
        m_expression = expression + QStringLiteral(" =");
    }
}

// ---------------------------------------------------------------- 内部工具

bool CalculatorEngine::applyBinary(Calc::Key op, double lhs, double rhs)
{
    double result = 0.0;

    switch (op) {
    case Calc::Key::Add:
        result = lhs + rhs;
        break;
    case Calc::Key::Subtract:
        result = lhs - rhs;
        break;
    case Calc::Key::Multiply:
        result = lhs * rhs;
        break;
    case Calc::Key::Divide:
        if (rhs == 0.0) {
            // 除数为 0：不能交给 IEEE 754 去得到 inf，直接进入可恢复的错误状态
            m_expression = formatNumber(lhs) + QLatin1Char(' ')
                         + Calc::operatorSymbol(op) + QLatin1Char(' ')
                         + formatNumber(rhs) + QStringLiteral(" =");
            enterError(QStringLiteral("除数不能为 0"));
            return false;
        }
        result = lhs / rhs;
        break;
    default:
        return false;
    }

    if (!std::isfinite(result)) {
        m_expression = formatNumber(lhs) + QLatin1Char(' ')
                     + Calc::operatorSymbol(op) + QLatin1Char(' ')
                     + formatNumber(rhs) + QStringLiteral(" =");
        enterError(QStringLiteral("数值超出范围"));
        return false;
    }

    m_lhs = result;
    m_entry = formatNumber(result);
    return true;
}

void CalculatorEngine::startFreshCalculation()
{
    m_entry = QStringLiteral("0");
    m_expression.clear();
    m_errorMessage.clear();
    m_lhs = 0.0;
    m_pendingOp.reset();
    m_repeatOp.reset();
    m_repeatRhs = 0.0;
    m_state = State::EnteringFirstOperand;
}

void CalculatorEngine::enterError(const QString &message)
{
    m_errorMessage = message;
    m_pendingOp.reset();
    m_repeatOp.reset();
    m_state = State::Error;
}

void CalculatorEngine::updateExpression()
{
    if (m_state == State::ShowingResult || m_state == State::Error)
        return;   // 这两种状态的表达式由产生结果 / 错误的分支负责设置

    m_expression = currentExpressionText();
}

QString CalculatorEngine::currentExpressionText() const
{
    if (!m_pendingOp.has_value())
        return QString();

    QString text = formatNumber(m_lhs) + QLatin1Char(' ')
                 + Calc::operatorSymbol(*m_pendingOp);
    if (m_state == State::EnteringSecondOperand)
        text += QLatin1Char(' ') + m_entry;   // 第二个操作数边输入边显示
    return text;
}

double CalculatorEngine::entryValue() const
{
    bool ok = false;
    const double value = m_entry.toDouble(&ok);
    return ok ? value : 0.0;
}

QString CalculatorEngine::formatNumber(double value)
{
    if (!std::isfinite(value))
        return QStringLiteral("0");
    if (value == 0.0)
        return QStringLiteral("0");   // 同时把 -0.0 归一成 "0"

    const double magnitude = std::fabs(value);
    if (magnitude >= 1e-9 && magnitude < 1e16) {
        QString text = QString::number(value, 'f', 12);
        while (text.endsWith(QLatin1Char('0')))
            text.chop(1);
        if (text.endsWith(QLatin1Char('.')))
            text.chop(1);
        if (text == QLatin1String("-0"))
            text = QStringLiteral("0");
        return text;
    }
    return QString::number(value, 'g', 15);
}
