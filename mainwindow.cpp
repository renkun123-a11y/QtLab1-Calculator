#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QTimer>

#include "calculatorengine.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_engine(new CalculatorEngine(this))
{
    ui->setupUi(this);
    uniformKeypadStretch();
    connectKeypad();

    connect(m_engine, &CalculatorEngine::displayChanged,
            this, &MainWindow::refreshDisplay);
    refreshDisplay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Ctrl / Alt / Meta 组合键留给系统快捷键（如 Ctrl+C 复制），不参与计算
    if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    const std::optional<Calc::Key> key = Calc::keyFromQtKey(event->key());
    if (!key.has_value()) {
        QMainWindow::keyPressEvent(event);   // 与计算无关的键交给基类
        return;
    }

    flashButton(*key);      // 键盘输入时让对应按键同步"按下"，鼠标与键盘表现一致
    dispatchKey(*key);      // 与鼠标点击走同一个入口
    event->accept();
}

void MainWindow::uniformKeypadStretch()
{
    for (int row = 0; row < 6; ++row)
        ui->keypadLayout->setRowStretch(row, 1);
    for (int column = 0; column < 4; ++column)
        ui->keypadLayout->setColumnStretch(column, 1);
}

void MainWindow::connectKeypad()
{
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        Calc::Key key{};
        const QString token = button->property("calcKey").toString();
        if (token.isEmpty() || !Calc::keyFromToken(token, &key))
            continue;   // 没有声明 calcKey 的普通按钮不参与计算

        connect(button, &QPushButton::clicked, this, [this, key] { dispatchKey(key); });
        m_buttonsByKey.insert(static_cast<int>(key), button);
    }
}

void MainWindow::dispatchKey(Calc::Key key)
{
    m_engine->press(key);
}

void MainWindow::flashButton(Calc::Key key)
{
    QPushButton *button = m_buttonsByKey.value(static_cast<int>(key), nullptr);
    if (!button)
        return;

    button->setDown(true);
    // 120ms 后恢复；用 QTimer 而不是阻塞等待，避免界面卡顿
    QTimer::singleShot(120, button, [button] { button->setDown(false); });
}

void MainWindow::refreshDisplay()
{
    const QString text = m_engine->displayText();
    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());   // 数字过长时始终显示末尾几位
    ui->expressionLabel->setText(m_engine->expressionText());
}
