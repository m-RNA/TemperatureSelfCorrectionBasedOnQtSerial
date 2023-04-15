#include "bll_serial.h"

Bll_SerialPort::Bll_SerialPort(QString name, QObject *parent) : QObject(parent)
{
    // setAutoDelete(false); // 关掉:放到线程池后自动析构
    this->setDeviceName(name);
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &Bll_SerialPort::slReadyRead);
}

Bll_SerialPort::~Bll_SerialPort()
{
    if (serial->isOpen()) // 退出程序时，关闭使用中的串口
    {
        serial->close(); // 关闭串口
        qDebug() << deviceName << "关闭串口";
    }
    qDebug() << "Bll_SerialPort Destroyed!";
}

/// @brief 初始化我的串口（任务对象） 开启串口
/// @param Bll_SerialPortSetting setting
/// @return 异常返回-1 正常返回0
void Bll_SerialPort::init(const Bll_SerialPortSetting &setting, RES &res)
{
    serial->setPortName(setting.portName);
    serial->setBaudRate(setting.baudRate);
    serial->setDataBits(setting.dataBits);
    serial->setParity(setting.parity);
    serial->setStopBits(setting.stopBits);
    serial->setFlowControl(setting.flowControl);
    printSerialPortInitInfo(serial);

    if (serial->open(QIODevice::ReadWrite) == false) // 打开失败
    {
        qDebug() << deviceName << "串口打开失败！" << serial->errorString();
        res.returnCode = -1;
        res.msg = serial->errorString();
        return;
    }

    // 正常打开
    qDebug() << deviceName << "打开串口";

    res.returnCode = 0;
}

// 串口发送任务
void Bll_SerialPort::slSendData(const QByteArray &data)
{
    serial->write(data);

    // qDebug() << deviceName << "串口发送线程ID" << QThread::currentThreadId();
}

//   串口接收任务
void Bll_SerialPort::slReadyRead()
{
    emit sgRecvData(serial->readAll()); // 最新接收数据

    // qDebug() << deviceName << "串口接收线程ID" << QThread::currentThreadId();
}
