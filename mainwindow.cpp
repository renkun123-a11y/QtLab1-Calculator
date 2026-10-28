#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    uniformKeypadStretch();
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
