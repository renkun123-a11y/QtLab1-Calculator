#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::refreshDisplay()
{
    const QString text = m_engine->displayText();
    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());   // 数字过长时始终显示末尾几位
    ui->expressionLabel->setText(m_engine->expressionText());
}
