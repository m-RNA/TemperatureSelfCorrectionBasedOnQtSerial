#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include "myserial.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->start_Dtm->setTitle("待定仪器");
    ui->start_Std->setTitle("标准仪器");

}

MainWindow::~MainWindow()
{
    delete ui;
}

