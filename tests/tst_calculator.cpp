// 计算器自动化测试
//
// 分两部分：
//   1) 计算核心（CalculatorEngine）的逻辑测试——直接驱动状态机，覆盖运算与各类边界输入；
//   2) 界面测试——用真实的 Qt 按键事件与按钮点击，验证"鼠标输入与键盘输入结果一致"。
//
// 运行方式：
//   tst_calculator.exe -platform offscreen         纯命令行跑完所有用例
//   set CALC_SHOT_DIR=docs/screenshots && tst_calculator.exe -platform offscreen
//                                                 额外导出报告用的界面截图

#include <QtTest>

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>

#include <memory>

#include "calckey.h"
#include "calculatorengine.h"
#include "mainwindow.h"

namespace {

// 逻辑测试用的字符表：数字、四则运算、小数点、等号，
// 以及用大写字母表示的功能键（B 退格 / C 全部清除 / E 清除当前 / N 正负号 / P 1÷x / S x² / R √x）
QString tokenForChar(QChar ch)
{
    if (ch.isDigit())
        return QString(ch);

    switch (ch.unicode()) {
    case '.': return QStringLiteral("dot");
    case '+': return QStringLiteral("add");
    case '-': return QStringLiteral("sub");
    case '*': return QStringLiteral("mul");
    case '/': return QStringLiteral("div");
    case '=': return QStringLiteral("eq");
    case '%': return QStringLiteral("pct");
    case 'B': return QStringLiteral("bs");
    case 'C': return QStringLiteral("c");
    case 'E': return QStringLiteral("ce");
    case 'N': return QStringLiteral("neg");
    case 'P': return QStringLiteral("inv");
    case 'S': return QStringLiteral("sq");
    case 'R': return QStringLiteral("sqrt");
    default:  return QString();
    }
}

// 界面测试用：用户为了输入某个字符，实际会按下的 Qt 键
int qtKeyForChar(QChar ch)
{
    if (ch.isDigit())
        return Qt::Key_0 + ch.digitValue();

    switch (ch.unicode()) {
    case '.': return Qt::Key_Period;
    case '+': return Qt::Key_Plus;
    case '-': return Qt::Key_Minus;
    case '*': return Qt::Key_Asterisk;
    case '/': return Qt::Key_Slash;
    case '=': return Qt::Key_Equal;
    case '%': return Qt::Key_Percent;
    case 'B': return Qt::Key_Backspace;
    case 'C': return Qt::Key_C;
    case 'E': return Qt::Key_E;
    case 'N': return Qt::Key_N;
    case 'P': return Qt::Key_P;
    case 'S': return Qt::Key_S;
    case 'R': return Qt::Key_R;
    default:  return 0;
    }
}

// 界面测试用：同一含义对应的按钮对象名
QString buttonNameForChar(QChar ch)
{
    if (ch.isDigit())
        return QStringLiteral("btnDigit") + ch;

    switch (ch.unicode()) {
    case '.': return QStringLiteral("btnDot");
    case '+': return QStringLiteral("btnAdd");
    case '-': return QStringLiteral("btnSubtract");
    case '*': return QStringLiteral("btnMultiply");
    case '/': return QStringLiteral("btnDivide");
    case '=': return QStringLiteral("btnEquals");
    case '%': return QStringLiteral("btnPercent");
    case 'B': return QStringLiteral("btnBackspace");
    case 'C': return QStringLiteral("btnClearAll");
    case 'E': return QStringLiteral("btnClearEntry");
    case 'N': return QStringLiteral("btnNegate");
    case 'P': return QStringLiteral("btnReciprocal");
    case 'S': return QStringLiteral("btnSquare");
    case 'R': return QStringLiteral("btnSqrt");
    default:  return QString();
    }
}

} // namespace

class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // ---------- 计算核心 ----------
    void fourOperations();
    void decimalInput();
    void duplicateDecimalPoint();
    void leadingDecimalPoint();
    void backspaceEditing();
    void clearEntryKeepsOperator();
    void clearAllResets();
    void divisionByZero();
    void chainedOperations();
    void consecutiveOperators();
    void continueAfterResult();
    void repeatedEquals();
    void unaryOperations();
    void invalidUnaryInput();
    void entryLengthLimit();
    void numberFormatting();

    // ---------- 界面：按钮与键盘 ----------
    void keypadButtonsAreComplete();
    void keyboardMatchesMouse();
    void mixedMouseAndKeyboard();
    void keyboardEditingKeys();
    void modifierKeysAreIgnored();
    void errorStateRecoverableFromKeyboard();

    // ---------- 报告截图 ----------
    void visualSnapshots();

private:
    void type(const QString &sequence);
    QString display() const;

    void pressKeys(QWidget *window, const QString &sequence);
    void clickButtons(MainWindow *window, const QString &sequence);

    std::unique_ptr<CalculatorEngine> m_engine;
};

void TestCalculator::init()
{
    m_engine = std::make_unique<CalculatorEngine>();
}

void TestCalculator::cleanup()
{
    m_engine.reset();
}

void TestCalculator::type(const QString &sequence)
{
    for (const QChar ch : sequence) {
        const QString token = tokenForChar(ch);
        QVERIFY2(!token.isEmpty(), qPrintable(QStringLiteral("无效测试字符: %1").arg(ch)));

        Calc::Key key{};
        QVERIFY2(Calc::keyFromToken(token, &key), qPrintable(QStringLiteral("无效令牌: %1").arg(token)));
        m_engine->press(key);
    }
}

QString TestCalculator::display() const
{
    return m_engine->displayText();
}

// ------------------------------------------------------------------ 四则运算与小数

void TestCalculator::fourOperations()
{
    type(QStringLiteral("12+7="));
    QCOMPARE(display(), QStringLiteral("19"));

    type(QStringLiteral("C9-4="));
    QCOMPARE(display(), QStringLiteral("5"));

    type(QStringLiteral("C6*7="));
    QCOMPARE(display(), QStringLiteral("42"));

    type(QStringLiteral("C8/2="));
    QCOMPARE(display(), QStringLiteral("4"));
}

void TestCalculator::decimalInput()
{
    type(QStringLiteral("C1.5+2.25="));
    QCOMPARE(display(), QStringLiteral("3.75"));

    // 浮点误差不能泄漏到显示区
    type(QStringLiteral("C0.1+0.2="));
    QCOMPARE(display(), QStringLiteral("0.3"));

    type(QStringLiteral("C2.5*4="));
    QCOMPARE(display(), QStringLiteral("10"));
}

void TestCalculator::duplicateDecimalPoint()
{
    // 同一个操作数只能有一个小数点，重复的小数点被忽略，后续数字仍然有效
    type(QStringLiteral("C1..5"));
    QCOMPARE(display(), QStringLiteral("1.5"));

    type(QStringLiteral("C5.5.5"));
    QCOMPARE(display(), QStringLiteral("5.55"));

    // 运算符之后的第二个操作数同样只允许一个小数点
    type(QStringLiteral("C1+2..3="));
    QCOMPARE(display(), QStringLiteral("3.3"));
}

void TestCalculator::leadingDecimalPoint()
{
    type(QStringLiteral("C.5"));
    QCOMPARE(display(), QStringLiteral("0.5"));

    type(QStringLiteral("C.5*.2="));
    QCOMPARE(display(), QStringLiteral("0.1"));
}

// ------------------------------------------------------------------ 退格与清除

void TestCalculator::backspaceEditing()
{
    type(QStringLiteral("C123BB"));
    QCOMPARE(display(), QStringLiteral("1"));

    type(QStringLiteral("CB"));
    QCOMPARE(display(), QStringLiteral("0"));       // 删到空时回到 0

    type(QStringLiteral("C12.5B"));
    QCOMPARE(display(), QStringLiteral("12."));     // 小数部分删完仍保留小数点

    // 等待输入第二个操作数时退格不应破坏左操作数
    type(QStringLiteral("C12+"));
    type(QStringLiteral("B"));
    QCOMPARE(display(), QStringLiteral("12"));
}

void TestCalculator::clearEntryKeepsOperator()
{
    type(QStringLiteral("C12+34E"));
    QCOMPARE(display(), QStringLiteral("0"));
    QCOMPARE(m_engine->expressionText(), QStringLiteral("12 +"));

    type(QStringLiteral("5="));
    QCOMPARE(display(), QStringLiteral("17"));      // 运算符与左操作数都还在
}

void TestCalculator::clearAllResets()
{
    type(QStringLiteral("C12+34C"));
    QCOMPARE(display(), QStringLiteral("0"));
    QCOMPARE(m_engine->expressionText(), QString());

    type(QStringLiteral("7="));
    QCOMPARE(display(), QStringLiteral("7"));
}

// ------------------------------------------------------------------ 边界与异常

void TestCalculator::divisionByZero()
{
    type(QStringLiteral("C8/0="));
    QVERIFY(m_engine->hasError());
    QCOMPARE(display(), QString::fromUtf8("除数不能为 0"));
    QCOMPARE(m_engine->expressionText(), QString::fromUtf8("8 ÷ 0 ="));

    // 错误状态下不接受运算符
    type(QStringLiteral("+"));
    QVERIFY(m_engine->hasError());

    // 全部清除后可以继续计算
    type(QStringLiteral("C2+2="));
    QCOMPARE(display(), QStringLiteral("4"));
    QVERIFY(!m_engine->hasError());

    // 出错后直接按数字也可以重新开始
    type(QStringLiteral("C8/0=5"));
    QCOMPARE(display(), QStringLiteral("5"));
}

void TestCalculator::chainedOperations()
{
    type(QStringLiteral("C1+2+"));
    QCOMPARE(display(), QStringLiteral("3"));       // 先给出中间结果

    type(QStringLiteral("3="));
    QCOMPARE(display(), QStringLiteral("6"));

    type(QStringLiteral("C10-2-3="));
    QCOMPARE(display(), QStringLiteral("5"));

    type(QStringLiteral("C2*3+4="));
    QCOMPARE(display(), QStringLiteral("10"));

    type(QStringLiteral("C12/3/2="));
    QCOMPARE(display(), QStringLiteral("2"));
}

void TestCalculator::consecutiveOperators()
{
    // 连续按运算符以最后一次为准
    type(QStringLiteral("C1+*2="));
    QCOMPARE(display(), QStringLiteral("2"));
    QCOMPARE(m_engine->expressionText(), QString::fromUtf8("1 × 2 ="));

    type(QStringLiteral("C8//2="));
    QCOMPARE(display(), QStringLiteral("4"));

    type(QStringLiteral("C9+-+-2="));
    QCOMPARE(display(), QStringLiteral("7"));
}

void TestCalculator::continueAfterResult()
{
    type(QStringLiteral("C2+3="));
    QCOMPARE(display(), QStringLiteral("5"));

    // 结果后输数字：开始一次新的计算，而不是接在结果后面
    type(QStringLiteral("4"));
    QCOMPARE(display(), QStringLiteral("4"));

    type(QStringLiteral("*5="));
    QCOMPARE(display(), QStringLiteral("20"));

    // 结果后直接按运算符：结果作为新的左操作数
    type(QStringLiteral("C2+3=*2="));
    QCOMPARE(display(), QStringLiteral("10"));

    // 结果后按小数点同样开始新计算
    type(QStringLiteral("C2+3=.5"));
    QCOMPARE(display(), QStringLiteral("0.5"));
}

void TestCalculator::repeatedEquals()
{
    type(QStringLiteral("C2+3=="));
    QCOMPARE(display(), QStringLiteral("8"));       // 5 + 3

    type(QStringLiteral("C2*3=="));
    QCOMPARE(display(), QStringLiteral("18"));      // 6 × 3

    // 缺第二个操作数时按等号，用左操作数补齐
    type(QStringLiteral("C5+="));
    QCOMPARE(display(), QStringLiteral("10"));
}

void TestCalculator::unaryOperations()
{
    type(QStringLiteral("C16R"));
    QCOMPARE(display(), QStringLiteral("4"));       // √16

    type(QStringLiteral("C4S"));
    QCOMPARE(display(), QStringLiteral("16"));      // 4²

    type(QStringLiteral("C4P"));
    QCOMPARE(display(), QStringLiteral("0.25"));    // 1/4

    type(QStringLiteral("C50%"));
    QCOMPARE(display(), QStringLiteral("0.5"));

    type(QStringLiteral("C5N"));
    QCOMPARE(display(), QStringLiteral("-5"));

    type(QStringLiteral("C5NN"));
    QCOMPARE(display(), QStringLiteral("5"));       // 两次取负抵消

    // 负号之后可以继续输入数字
    type(QStringLiteral("C1N2"));
    QCOMPARE(display(), QStringLiteral("-12"));
}

void TestCalculator::invalidUnaryInput()
{
    type(QStringLiteral("C1NR"));
    QVERIFY(m_engine->hasError());
    QCOMPARE(display(), QString::fromUtf8("负数不能开平方"));

    type(QStringLiteral("C0P"));
    QVERIFY(m_engine->hasError());
    QCOMPARE(display(), QString::fromUtf8("除数不能为 0"));
}

void TestCalculator::entryLengthLimit()
{
    type(QStringLiteral("C") + QString(20, QLatin1Char('7')));
    QCOMPARE(display().size(), 16);
    QCOMPARE(display(), QStringLiteral("7777777777777777"));
}

void TestCalculator::numberFormatting()
{
    QCOMPARE(CalculatorEngine::formatNumber(0.1 + 0.2), QStringLiteral("0.3"));
    QCOMPARE(CalculatorEngine::formatNumber(1.0 / 3.0), QStringLiteral("0.333333333333"));
    QCOMPARE(CalculatorEngine::formatNumber(-0.0), QStringLiteral("0"));
    QCOMPARE(CalculatorEngine::formatNumber(2.0), QStringLiteral("2"));
    QCOMPARE(CalculatorEngine::formatNumber(1e17), QStringLiteral("1e+17"));
    QCOMPARE(CalculatorEngine::formatNumber(-12.5), QStringLiteral("-12.5"));
}

// ------------------------------------------------------------------ 界面：按钮与键盘

void TestCalculator::keypadButtonsAreComplete()
{
    MainWindow window;

    const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
    QCOMPARE(buttons.size(), 24);

    QSet<QString> tokens;
    for (QPushButton *button : buttons) {
        const QString token = button->property("calcKey").toString();
        QVERIFY2(!token.isEmpty(),
                 qPrintable(QStringLiteral("控件 %1 缺少 calcKey 属性").arg(button->objectName())));
        Calc::Key key{};
        QVERIFY2(Calc::keyFromToken(token, &key),
                 qPrintable(QStringLiteral("控件 %1 的 calcKey 令牌无效: %2")
                            .arg(button->objectName(), token)));
        QVERIFY2(!tokens.contains(token),
                 qPrintable(QStringLiteral("令牌重复: %1").arg(token)));
        tokens.insert(token);

        // 样式表按 buttonRole 分类，缺了会让界面回到默认外观
        QVERIFY2(!button->property("buttonRole").toString().isEmpty(),
                 qPrintable(QStringLiteral("控件 %1 缺少 buttonRole 属性").arg(button->objectName())));
    }
    QCOMPARE(tokens.size(), 24);
}

void TestCalculator::pressKeys(QWidget *window, const QString &sequence)
{
    for (const QChar ch : sequence) {
        const int key = qtKeyForChar(ch);
        QVERIFY2(key != 0, qPrintable(QStringLiteral("无效测试字符: %1").arg(ch)));
        QTest::keyClick(window, static_cast<Qt::Key>(key));
    }
}

void TestCalculator::clickButtons(MainWindow *window, const QString &sequence)
{
    for (const QChar ch : sequence) {
        const QString name = buttonNameForChar(ch);
        QVERIFY2(!name.isEmpty(), qPrintable(QStringLiteral("无效测试字符: %1").arg(ch)));

        auto *button = window->findChild<QPushButton *>(name);
        QVERIFY2(button, qPrintable(QStringLiteral("找不到按钮: %1").arg(name)));
        button->click();
    }
}

void TestCalculator::keyboardMatchesMouse()
{
    MainWindow window;
    auto *displayEdit = window.findChild<QLineEdit *>(QStringLiteral("displayEdit"));
    QVERIFY(displayEdit);

    const QStringList sequences = {
        QStringLiteral("12.5*4="),
        QStringLiteral("1+2+3="),
        QStringLiteral("8/0="),
        QStringLiteral("123BB"),
        QStringLiteral("C1.5+2.25="),
        QStringLiteral("2+3=="),
        QStringLiteral("0.1+0.2="),
        QStringLiteral("16R"),
        QStringLiteral("5S"),
        QStringLiteral("50%"),
    };

    for (const QString &sequence : sequences) {
        // 鼠标：先清空，再依次点击按钮
        clickButtons(&window, QStringLiteral("C"));
        clickButtons(&window, sequence);
        const QString byMouse = displayEdit->text();

        // 键盘：同样的清空与输入序列
        pressKeys(&window, QStringLiteral("C"));
        pressKeys(&window, sequence);
        const QString byKeyboard = displayEdit->text();

        QVERIFY2(byMouse == byKeyboard,
                 qPrintable(QStringLiteral("序列 \"%1\" 鼠标得到 [%2]，键盘得到 [%3]")
                            .arg(sequence, byMouse, byKeyboard)));
    }
}

void TestCalculator::mixedMouseAndKeyboard()
{
    MainWindow window;
    auto *displayEdit = window.findChild<QLineEdit *>(QStringLiteral("displayEdit"));
    QVERIFY(displayEdit);

    clickButtons(&window, QStringLiteral("1"));      // 鼠标
    pressKeys(&window, QStringLiteral("2"));         // 键盘
    clickButtons(&window, QStringLiteral("+"));      // 鼠标
    pressKeys(&window, QStringLiteral("3"));         // 键盘
    clickButtons(&window, QStringLiteral("="));      // 鼠标

    QCOMPARE(displayEdit->text(), QStringLiteral("15"));
}

void TestCalculator::keyboardEditingKeys()
{
    MainWindow window;
    auto *displayEdit = window.findChild<QLineEdit *>(QStringLiteral("displayEdit"));
    auto *expressionLabel = window.findChild<QLabel *>(QStringLiteral("expressionLabel"));
    QVERIFY(displayEdit);
    QVERIFY(expressionLabel);

    pressKeys(&window, QStringLiteral("123"));
    QCOMPARE(displayEdit->text(), QStringLiteral("123"));

    QTest::keyClick(&window, Qt::Key_Backspace);
    QCOMPARE(displayEdit->text(), QStringLiteral("12"));

    // Delete 等价于 CE：只清当前输入，表达式保留
    pressKeys(&window, QStringLiteral("+"));
    pressKeys(&window, QStringLiteral("34"));
    QTest::keyClick(&window, Qt::Key_Delete);
    QCOMPARE(displayEdit->text(), QStringLiteral("0"));
    QCOMPARE(expressionLabel->text(), QStringLiteral("12 +"));

    // Esc 等价于 C：全部清除
    QTest::keyClick(&window, Qt::Key_Escape);
    QCOMPARE(displayEdit->text(), QStringLiteral("0"));
    QCOMPARE(expressionLabel->text(), QString());

    // 回车等价于等号
    pressKeys(&window, QStringLiteral("7*6"));
    QTest::keyClick(&window, Qt::Key_Return);
    QCOMPARE(displayEdit->text(), QStringLiteral("42"));
}

void TestCalculator::modifierKeysAreIgnored()
{
    MainWindow window;
    auto *displayEdit = window.findChild<QLineEdit *>(QStringLiteral("displayEdit"));
    QVERIFY(displayEdit);

    pressKeys(&window, QStringLiteral("12"));
    QCOMPARE(displayEdit->text(), QStringLiteral("12"));

    // Ctrl+C 应留给系统复制，而不是当作"清除"
    QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier);
    QCOMPARE(displayEdit->text(), QStringLiteral("12"));

    // Ctrl+Alt 组合键同理不影响输入
    QTest::keyClick(&window, Qt::Key_5, Qt::AltModifier);
    QCOMPARE(displayEdit->text(), QStringLiteral("12"));
}

void TestCalculator::errorStateRecoverableFromKeyboard()
{
    MainWindow window;
    auto *displayEdit = window.findChild<QLineEdit *>(QStringLiteral("displayEdit"));
    QVERIFY(displayEdit);

    pressKeys(&window, QStringLiteral("8/0="));
    QCOMPARE(displayEdit->text(), QString::fromUtf8("除数不能为 0"));

    // 错误状态下运算符无效
    pressKeys(&window, QStringLiteral("+*"));
    QCOMPARE(displayEdit->text(), QString::fromUtf8("除数不能为 0"));

    // Esc 恢复后可以正常计算
    QTest::keyClick(&window, Qt::Key_Escape);
    pressKeys(&window, QStringLiteral("6*7="));
    QCOMPARE(displayEdit->text(), QStringLiteral("42"));
}

// ------------------------------------------------------------------ 报告截图

void TestCalculator::visualSnapshots()
{
    const QString dir = qEnvironmentVariable("CALC_SHOT_DIR");
    if (dir.isEmpty())
        QSKIP("未设置 CALC_SHOT_DIR，跳过界面截图导出");

    QVERIFY(QDir().mkpath(dir));

    MainWindow window;
    window.resize(420, 660);
    window.show();
    QTest::qWait(120);

    const auto shot = [&window, &dir](const QString &name) {
        QTest::qWait(240);   // 等 flashButton() 的 120ms 恢复完成，截图不残留按下状态
        const QString path = dir + QLatin1Char('/') + name + QStringLiteral(".png");
        QVERIFY2(window.grab().save(path), qPrintable(QStringLiteral("保存失败: %1").arg(path)));
    };

    shot(QStringLiteral("01-interface"));

    clickButtons(&window, QStringLiteral("12.5*4="));
    shot(QStringLiteral("02-decimal-multiply"));

    clickButtons(&window, QStringLiteral("C845+996"));
    shot(QStringLiteral("03-typing-expression"));

    clickButtons(&window, QStringLiteral("C8/0="));
    shot(QStringLiteral("04-divide-by-zero"));

    clickButtons(&window, QStringLiteral("C1+2+3="));
    shot(QStringLiteral("05-chained-operations"));

    clickButtons(&window, QStringLiteral("C123B"));
    shot(QStringLiteral("06-backspace"));

    clickButtons(&window, QStringLiteral("C"));
    pressKeys(&window, QStringLiteral("845+996="));
    shot(QStringLiteral("07-keyboard-result"));

    // 报告用：异常输入与编辑状态
    clickButtons(&window, QStringLiteral("C1..5"));
    shot(QStringLiteral("08-duplicate-dot"));

    clickButtons(&window, QStringLiteral("C12+34E"));
    shot(QStringLiteral("09-ce-keeps-operator"));

    clickButtons(&window, QStringLiteral("C1NR"));
    shot(QStringLiteral("10-negative-sqrt"));

    clickButtons(&window, QStringLiteral("C1+2+"));
    shot(QStringLiteral("11-chained-middle"));
}

QTEST_MAIN(TestCalculator)

#include "tst_calculator.moc"
