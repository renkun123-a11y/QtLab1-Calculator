#include "calckey.h"

#include <QHash>
#include <QStringList>

namespace Calc {

namespace {

// 令牌表：UI 动态属性 calcKey 的取值 <-> 计算器按键
const QHash<QString, Key> &tokenTable()
{
    static const QHash<QString, Key> table = {
        {QStringLiteral("0"), Key::Digit0},
        {QStringLiteral("1"), Key::Digit1},
        {QStringLiteral("2"), Key::Digit2},
        {QStringLiteral("3"), Key::Digit3},
        {QStringLiteral("4"), Key::Digit4},
        {QStringLiteral("5"), Key::Digit5},
        {QStringLiteral("6"), Key::Digit6},
        {QStringLiteral("7"), Key::Digit7},
        {QStringLiteral("8"), Key::Digit8},
        {QStringLiteral("9"), Key::Digit9},
        {QStringLiteral("dot"), Key::Dot},
        {QStringLiteral("add"), Key::Add},
        {QStringLiteral("sub"), Key::Subtract},
        {QStringLiteral("mul"), Key::Multiply},
        {QStringLiteral("div"), Key::Divide},
        {QStringLiteral("eq"), Key::Equals},
        {QStringLiteral("c"), Key::ClearAll},
        {QStringLiteral("ce"), Key::ClearEntry},
        {QStringLiteral("bs"), Key::Backspace},
        {QStringLiteral("neg"), Key::Negate},
        {QStringLiteral("pct"), Key::Percent},
        {QStringLiteral("inv"), Key::Reciprocal},
        {QStringLiteral("sq"), Key::Square},
        {QStringLiteral("sqrt"), Key::Sqrt}
    };
    return table;
}

} // namespace

QString operatorSymbol(Key key)
{
    switch (key) {
    case Key::Add:      return QStringLiteral("+");
    case Key::Subtract: return QStringLiteral("\u2212");   // 数学减号 −，比连字符 - 好看
    case Key::Multiply: return QStringLiteral("\u00d7");   // ×
    case Key::Divide:   return QStringLiteral("\u00f7");   // ÷
    default:            return QString();
    }
}

QString keyToToken(Key key)
{
    for (auto it = tokenTable().cbegin(); it != tokenTable().cend(); ++it) {
        if (it.value() == key)
            return it.key();
    }
    return QString();
}

bool keyFromToken(const QString &token, Key *key)
{
    const auto it = tokenTable().constFind(token);
    if (it == tokenTable().cend())
        return false;
    if (key)
        *key = it.value();
    return true;
}

std::optional<Key> keyFromQtKey(int qtKey)
{
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
        // 主键盘数字键与小键盘数字键到达 Qt 后的键值相同，这里一并处理
        return static_cast<Key>(static_cast<int>(Key::Digit0) + (qtKey - Qt::Key_0));
    }

    switch (qtKey) {
    case Qt::Key_Plus:      return Key::Add;
    case Qt::Key_Minus:     return Key::Subtract;
    case Qt::Key_Asterisk:  return Key::Multiply;
    case Qt::Key_X:         return Key::Multiply;   // 键盘上 x 也当乘号
    case Qt::Key_Slash:     return Key::Divide;
    case Qt::Key_Period:    return Key::Dot;
    case Qt::Key_Comma:     return Key::Dot;        // 部分键盘布局的小数点键
    case Qt::Key_Equal:     return Key::Equals;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return Key::Equals;
    case Qt::Key_Backspace: return Key::Backspace;
    case Qt::Key_Delete:    return Key::ClearEntry;
    case Qt::Key_Escape:    return Key::ClearAll;
    case Qt::Key_C:         return Key::ClearAll;
    case Qt::Key_E:         return Key::ClearEntry;
    case Qt::Key_Percent:   return Key::Percent;
    case Qt::Key_P:         return Key::Reciprocal;
    case Qt::Key_S:         return Key::Square;
    case Qt::Key_R:         return Key::Sqrt;
    case Qt::Key_N:         return Key::Negate;
    case Qt::Key_F9:        return Key::Negate;     // 与 Windows 计算器保持一致
    default:                return std::nullopt;
    }
}

} // namespace Calc
