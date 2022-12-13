#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"

#include <QSerialPortInfo>
#include <QSerialPort>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->start_Dtm->setDeviceName("待定仪器");
    ui->start_Std->setDeviceName("标准仪器");

    connect(ui->start_Std, &StartCommunication::serialStateChange, this, [=](bool state){
        if(state){
            ui->ledText_Std->setText("在线");
            ui->led2_Std->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
        }
        else{
            ui->ledText_Std->setText("离线");
            ui->led2_Std->setStyleSheet("border-radius:7px;background-color: red;");
        }
    });

    connect(ui->start_Dtm, &StartCommunication::serialStateChange, this, [=](bool state){
        if(state){
            ui->ledText_Dtm->setText("在线");
            ui->led2_Dtm->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
        }
        else{
            ui->ledText_Dtm->setText("离线");
            ui->led2_Dtm->setStyleSheet("border-radius:7px;background-color: red;");
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

