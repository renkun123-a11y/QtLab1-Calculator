#include "calculatorengine.h"

#include <cmath>

CalculatorEngine::CalculatorEngine(QObject *parent)
    : QObject(parent)
{
}

void CalculatorEngine::press(Calc::Key key)
{
    if (Calc::isDigit(key)) {
        inputDigit(Calc::digitValue(key));
    } else if (Calc::isBinaryOperator(key)) {
        inputOperator(key);
    } else if (key == Calc::Key::Dot) {
        inputDot();
    } else if (key == Calc::Key::Equals) {
        inputEquals();
    } else if (key == Calc::Key::Backspace) {
        inputBackspace();
    } else if (key == Calc::Key::ClearEntry) {
        inputClearEntry();
    } else if (key == Calc::Key::ClearAll) {
        clearAll();
    }
    // TODO: 正负号、百分号等一元运算在后续阶段实现

    emit displayChanged();
}

void CalculatorEngine::clearAll()
{
    m_entry = QStringLiteral("0");
    m_expression.clear();
    m_lhs = 0.0;
    m_hasPendingOperator = false;
    m_startNewEntry = false;
}

void CalculatorEngine::inputDigit(int digit)
{
    const QString text = QString::number(digit);

    if (m_startNewEntry || m_entry == QLatin1String("0")) {
        m_entry = text;              // 另起一个数，或去掉多余的前导 0
        m_startNewEntry = false;
    } else {
        m_entry += text;
    }

    m_expression = currentExpressionText();
}

void CalculatorEngine::inputDot()
{
    if (m_startNewEntry) {
        m_entry = QStringLiteral("0.");   // 直接按小数点，自动补前导 0
        m_startNewEntry = false;
    } else if (m_entry.contains(QLatin1Char('.'))) {
        return;                           // 同一个操作数只允许一个小数点，重复输入直接忽略
    } else {
        m_entry += QLatin1Char('.');
    }

    m_expression = currentExpressionText();
}

void CalculatorEngine::inputOperator(Calc::Key op)
{
    if (m_hasPendingOperator && m_startNewEntry) {
        // 还没输入第二个操作数就又按了运算符（如 1 + ×）：
        // 以最后一次按下的运算符为准，不重复计算
        m_pendingOperator = op;
        m_expression = currentExpressionText();
        return;
    }

    m_lhs = m_entry.toDouble();      // 当前显示值成为左操作数
    m_pendingOperator = op;
    m_hasPendingOperator = true;
    m_startNewEntry = true;
    m_expression = currentExpressionText();
}

void CalculatorEngine::inputEquals()
{
    if (!m_hasPendingOperator)
        return;

    const double rhs = m_entry.toDouble();
    const QString lhsText = formatNumber(m_lhs);
    const QString rhsText = m_entry;

    const double result = compute(m_lhs, m_pendingOperator, rhs);

    m_entry = formatNumber(result);
    m_expression = lhsText + QLatin1Char(' ') + Calc::operatorSymbol(m_pendingOperator)
                 + QLatin1Char(' ') + rhsText + QStringLiteral(" =");
    m_hasPendingOperator = false;
    m_startNewEntry = true;
}

void CalculatorEngine::inputBackspace()
{
    if (m_startNewEntry)
        return;                      // 还没有开始输入操作数，没有可删的字符

    if (m_entry.size() <= 1) {
        m_entry = QStringLiteral("0");
    } else {
        m_entry.chop(1);             // 删掉最后一个字符
        if (m_entry == QLatin1String("-"))
            m_entry = QStringLiteral("0");
    }

    m_expression = currentExpressionText();
}

void CalculatorEngine::inputClearEntry()
{
    m_entry = QStringLiteral("0");   // 只清除当前输入，左操作数与运算符保持不变
    m_startNewEntry = false;

    m_expression = currentExpressionText();
}

QString CalculatorEngine::currentExpressionText() const
{
    if (!m_hasPendingOperator)
        return QString();

    QString text = formatNumber(m_lhs) + QLatin1Char(' ')
                 + Calc::operatorSymbol(m_pendingOperator);
    if (!m_startNewEntry)
        text += QLatin1Char(' ') + m_entry;   // 第二个操作数边输入边显示
    return text;
}

double CalculatorEngine::compute(double lhs, Calc::Key op, double rhs) const
{
    switch (op) {
    case Calc::Key::Add:      return lhs + rhs;
    case Calc::Key::Subtract: return lhs - rhs;
    case Calc::Key::Multiply: return lhs * rhs;
    case Calc::Key::Divide:   return lhs / rhs;
    default:                  return rhs;
    }
}

QString CalculatorEngine::formatNumber(double value)
{
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
