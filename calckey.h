#ifndef CALCKEY_H
#define CALCKEY_H

#include <QString>

// 计算器按键的"统一令牌"。
//
// 鼠标点击和键盘按键最终都会被翻译成同一个 Calc::Key，再交给
// MainWindow::dispatchKey() 处理，因此两条输入路径只有"翻译"不同，
// 计算状态机的处理逻辑完全复用同一份。
namespace Calc {

enum class Key {
    // 数字 0-9（连续定义，便于用 digitValue() 换算）
    Digit0, Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,

    Dot,                              // 小数点
    Add, Subtract, Multiply, Divide,  // 四则运算符
    Equals,                           // =
    ClearAll,                         // C   全部清除
    ClearEntry,                       // CE  只清除当前输入
    Backspace,                        // ←   退格
    Negate,                           // ±   正负号
    Percent,                          // %
    Reciprocal,                       // 1/x
    Square,                           // x²
    Sqrt                              // √x
};

inline int digitValue(Key key)
{
    return static_cast<int>(key) - static_cast<int>(Key::Digit0);
}

inline bool isDigit(Key key)
{
    return digitValue(key) >= 0 && digitValue(key) <= 9;
}

inline bool isBinaryOperator(Key key)
{
    return key == Key::Add || key == Key::Subtract
        || key == Key::Multiply || key == Key::Divide;
}

// 运算符在表达式栏中显示的符号
QString operatorSymbol(Key key);

// UI 设计器里用动态属性 "calcKey" 保存的字符串令牌，与 Key 互转
QString keyToToken(Key key);
bool keyFromToken(const QString &token, Key *key);

} // namespace Calc

#endif // CALCKEY_H
