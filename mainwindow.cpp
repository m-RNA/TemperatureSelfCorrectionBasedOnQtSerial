#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include "myserial.h"

void printSerialPortInitInfo(QSerialPort *sp)
{
    qDebug() << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialInfoUpdata();

    // 串口显示列表 安装事件过滤
    ui->cbSerial_Std->installEventFilter(this);
    ui->cbSerial_Dtm->installEventFilter(this);

    serial_Std = new QSerialPort (this);
    serial_Dtm = new QSerialPort (this);

    connect(this, &MainWindow::serialStateChange_Std, this, &MainWindow::uiLookUpdata_Std);
    connect(this, &MainWindow::serialStateChange_Dtm, this, &MainWindow::uiLookUpdata_Dtm);



}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) //鼠标点击事件
    {
        if (obj == ui->cbSerial_Std || ui->cbSerial_Dtm) // combox
        {
            // 更新界面串口列表信息
            serialInfoUpdata();
        }
    }

    return QWidget::eventFilter(obj, event);
}

// 更新界面串口信息
void MainWindow::serialInfoUpdata(void)
{
    QStringList serialNamePort;
    // 查询串口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // 将可用串口端口信息放到serialNamePort这里面
        serialNamePort << info.portName();
    }
    ui->cbSerial_Std->clear();
    ui->cbSerial_Std->addItems(serialNamePort);
    ui->cbSerial_Dtm->clear();
    ui->cbSerial_Dtm->addItems(serialNamePort);
}



void MainWindow::on_btnSerialSwitch_Std_clicked()
{
    qDebug("STD串口开关");
    if (serialState_Std == false)
    {
        serial_Std->setPortName(ui->cbSerial_Std->currentText());
        serial_Std->setBaudRate(ui->cbBaudrate_Std->currentText().toInt());

        //数据位
        switch(ui->cbDataBit_Std->currentText().toInt())
        {
        case 5 : serial_Std->setDataBits(QSerialPort::Data5); break;
        case 6 : serial_Std->setDataBits(QSerialPort::Data6); break;
        case 7 : serial_Std->setDataBits(QSerialPort::Data7); break;
        case 8 : serial_Std->setDataBits(QSerialPort::Data8); break;
        default: break;
        }

        //校验位
        switch (ui->cbCheckBit_Std->currentIndex())
        {
        case 0 :  serial_Std->setParity(QSerialPort::NoParity); break;
        case 1 :  serial_Std->setParity(QSerialPort::OddParity); break;
        case 2 :  serial_Std->setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //停止位
        switch (ui->cbStopBit_Std->currentIndex())
        {
        case 0 :  serial_Std->setStopBits(QSerialPort::OneStop); break;
        case 1 :  serial_Std->setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2 :  serial_Std->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        // 流控
        switch (ui->cbFlowCtrl_Std->currentIndex())
        {
        case 0 :  serial_Std->setFlowControl(QSerialPort::NoFlowControl); break;
        case 1 :  serial_Std->setFlowControl(QSerialPort::HardwareControl); break;
        case 2 :  serial_Std->setFlowControl(QSerialPort::SoftwareControl); break;
        default: break;
        }
        printSerialPortInitInfo(serial_Std);

        if (serial_Std->open(QIODevice::ReadWrite) == true)
        {
            qDebug("打开STD串口");

            serialState_Std = true; // 串口状态 置开
        }
        else
        {
            qDebug("STD串口打开失败！");
            serialState_Std = false; // 串口状态 置关

            QMessageBox::critical(this, "标准仪器串口打开失败!",
                                  "请检查:\n\n\
线缆是否松动?\n\
串口号是否正确?\n\
串口是否被序占用?\n\
是否有串口读写权限?");
        }
    }
    else
    {
        qDebug("关闭STD串口");
        serialState_Std = false; // 串口状态 置开
        serial_Std->close();     // 关闭串口
    }

    emit serialStateChange_Std(serialState_Std);
}

void MainWindow::on_btnSerialSwitch_Dtm_clicked()
{
    qDebug("Dtm串口开关");
    if (serialState_Dtm == false)
    {
        serial_Dtm->setPortName(ui->cbSerial_Dtm->currentText());
        serial_Dtm->setBaudRate(ui->cbBaudrate_Dtm->currentText().toInt());

        //数据位
        switch(ui->cbDataBit_Dtm->currentText().toInt())
        {
        case 5 : serial_Dtm->setDataBits(QSerialPort::Data5); break;
        case 6 : serial_Dtm->setDataBits(QSerialPort::Data6); break;
        case 7 : serial_Dtm->setDataBits(QSerialPort::Data7); break;
        case 8 : serial_Dtm->setDataBits(QSerialPort::Data8); break;
        default: break;
        }

        //校验位
        switch (ui->cbCheckBit_Dtm->currentIndex())
        {
        case 0 :  serial_Dtm->setParity(QSerialPort::NoParity); break;
        case 1 :  serial_Dtm->setParity(QSerialPort::OddParity); break;
        case 2 :  serial_Dtm->setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //停止位
        switch (ui->cbStopBit_Dtm->currentIndex())
        {
        case 0 :  serial_Dtm->setStopBits(QSerialPort::OneStop); break;
        case 1 :  serial_Dtm->setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2 :  serial_Dtm->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        // 流控
        switch (ui->cbFlowCtrl_Dtm->currentIndex())
        {
        case 0 :  serial_Dtm->setFlowControl(QSerialPort::NoFlowControl); break;
        case 1 :  serial_Dtm->setFlowControl(QSerialPort::HardwareControl); break;
        case 2 :  serial_Dtm->setFlowControl(QSerialPort::SoftwareControl); break;
        default: break;
        }
        printSerialPortInitInfo(serial_Dtm);

        if (serial_Dtm->open(QIODevice::ReadWrite) == true)
        {
            qDebug("打开Dtm串口");

            serialState_Dtm = true; // 串口状态 置开
        }
        else
        {
            qDebug("Dtm串口打开失败！");
            serialState_Dtm = false; // 串口状态 置关

            QMessageBox::critical(this, "待定仪器串口打开失败!",
                                  "请检查:\n\n\
线缆是否松动?\n\
串口号是否正确?\n\
串口是否被序占用?\n\
是否有串口读写权限?");
        }
    }
    else
    {
        qDebug("关闭Dtm串口");
        serialState_Dtm = false; // 串口状态 置开
        serial_Dtm->close();     // 关闭串口
    }

    emit serialStateChange_Dtm(serialState_Dtm);
}


void MainWindow::uiLookUpdata_Std(bool state)
{
    if(state)
    {
        ui->boxSerialSetting_Std->hide();
        ui->cbSerial_Std->setEnabled(false);
        ui->cbBaudrate_Std->setEnabled(false);
        ui->ledText_Std->setText("在线");
        ui->btnSerialSwitch_Std->setText("关闭");
        ui->btnSerialSwitch_Std->setIcon(QIcon(":/icon/connect.ico"));
        ui->led1_Std->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
        ui->led2_Std->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
    }
    else
    {
        ui->boxSerialSetting_Std->show();
        ui->cbSerial_Std->setEnabled(true);
        ui->cbBaudrate_Std->setEnabled(true);
        ui->ledText_Std->setText("离线");
        ui->btnSerialSwitch_Std->setText("打开");
        ui->btnSerialSwitch_Std->setIcon(QIcon(":/icon/disconnect.ico"));
        ui->led1_Std->setStyleSheet("border-radius:7px;background-color: red;");
        ui->led2_Std->setStyleSheet("border-radius:7px;background-color: red;");
    }
}

void MainWindow::uiLookUpdata_Dtm(bool state)
{
    if(state)
    {
        ui->boxSerialSetting_Dtm->hide();
        ui->cbSerial_Dtm->setEnabled(false);
        ui->cbBaudrate_Dtm->setEnabled(false);
        ui->ledText_Dtm->setText("在线");
        ui->btnSerialSwitch_Dtm->setText("关闭");
        ui->btnSerialSwitch_Dtm->setIcon(QIcon(":/icon/connect.ico"));
        ui->led1_Dtm->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
        ui->led2_Dtm->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
    }
    else
    {
        ui->boxSerialSetting_Dtm->show();
        ui->cbSerial_Dtm->setEnabled(true);
        ui->cbBaudrate_Dtm->setEnabled(true);
        ui->ledText_Dtm->setText("离线");
        ui->btnSerialSwitch_Dtm->setText("打开");
        ui->btnSerialSwitch_Dtm->setIcon(QIcon(":/icon/disconnect.ico"));
        ui->led1_Dtm->setStyleSheet("border-radius:7px;background-color: red;");
        ui->led2_Dtm->setStyleSheet("border-radius:7px;background-color: red;");
    }
}
