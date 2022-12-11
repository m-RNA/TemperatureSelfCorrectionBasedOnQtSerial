#include "startcommunication.h"
#include "ui_startcommunication.h"
#include <QSerialPortInfo>

StartCommunication::StartCommunication(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartCommunication)
{
    ui->setupUi(this);

    serialInfoUpdata();

    // 串口显示列表 安装事件过滤
    ui->cbSerial->installEventFilter(this);

    qDebug() << deviceName << "Start";
    serial = new QSerialPort;

    connect(this, &StartCommunication::serialStateChange, this, &StartCommunication::uiLookUpdata);


}

bool StartCommunication::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) //鼠标点击事件
    {
        if (obj == ui->cbSerial) // combox
        {
            // 更新界面串口列表信息
            serialInfoUpdata();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void StartCommunication::setDeviceName(QString s)
{
    deviceName = s;
    ui->gbxSerialSetting->setTitle(deviceName);
}

// 更新界面串口信息
void StartCommunication::serialInfoUpdata(void)
{
    QStringList serialNamePort;
    // 查询串口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // 将可用串口端口信息放到serialNamePort这里面
        serialNamePort << info.portName();
    }
    ui->cbSerial->clear();
    ui->cbSerial->addItems(serialNamePort);
}


StartCommunication::~StartCommunication()
{
    if(serialState)    // 退出程序时，关闭使用中的串口
    {
        serial->close();     // 关闭串口
        serial->deleteLater();
        qDebug() << deviceName << "关闭串口";
    }
    delete ui;
}
void StartCommunication::on_btnSerialSwitch_clicked()
{
    qDebug() << deviceName << "点击串口开关";
    if (serialState == false)
    {
        serial->setPortName(ui->cbSerial->currentText());
        serial->setBaudRate(ui->cbBaudrate->currentText().toInt());

        //数据位
        switch(ui->cbDataBit->currentText().toInt())
        {
        case 5 : serial->setDataBits(QSerialPort::Data5); break;
        case 6 : serial->setDataBits(QSerialPort::Data6); break;
        case 7 : serial->setDataBits(QSerialPort::Data7); break;
        case 8 : serial->setDataBits(QSerialPort::Data8); break;
        default: break;
        }

        //校验位
        switch (ui->cbCheckBit->currentIndex())
        {
        case 0 :  serial->setParity(QSerialPort::NoParity); break;
        case 1 :  serial->setParity(QSerialPort::OddParity); break;
        case 2 :  serial->setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //停止位
        switch (ui->cbStopBit->currentIndex())
        {
        case 0 :  serial->setStopBits(QSerialPort::OneStop); break;
        case 1 :  serial->setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2 :  serial->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        // 流控
        switch (ui->cbFlowCtrl->currentIndex())
        {
        case 0 :  serial->setFlowControl(QSerialPort::NoFlowControl); break;
        case 1 :  serial->setFlowControl(QSerialPort::HardwareControl); break;
        case 2 :  serial->setFlowControl(QSerialPort::SoftwareControl); break;
        default: break;
        }
        printSerialPortInitInfo(serial);

        if (serial->open(QIODevice::ReadWrite) == true)
        {
            qDebug() << deviceName << "打开串口";

            serialState = true; // 串口状态 置开
        }
        else
        {
            qDebug() << deviceName << "串口打开失败！";
            serialState = false; // 串口状态 置关

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
        qDebug() << deviceName << "关闭串口";
        serialState = false; // 串口状态 置开
        serial->close();     // 关闭串口
    }

    emit serialStateChange(serialState);
}

void StartCommunication::uiLookUpdata(bool state)
{
    if(state)
    {
        ui->boxSerialSetting->hide();
        ui->cbSerial->setEnabled(false);
        ui->cbBaudrate->setEnabled(false);
        ui->btnSerialSwitch->setText("关闭");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/connect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
    }
    else
    {
        ui->boxSerialSetting->show();
        ui->cbSerial->setEnabled(true);
        ui->cbBaudrate->setEnabled(true);
        ui->btnSerialSwitch->setText("打开");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/disconnect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");
    }
}

/*  串口接收任务   */




