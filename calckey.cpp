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

} // namespace Calc
