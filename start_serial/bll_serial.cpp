#include "bll_serial.h"

#include <QElapsedTimer>

Bll_SerialPort::Bll_SerialPort(QString name, QObject *parent) : QObject(parent)
{
    // setAutoDelete(false); // 关掉:放到线程池后自动析构
    this->setDeviceName(name);
    serial = new QSerialPort();
    recvAnalyse = new Bll_SerialRecvAnalyse;

    connect(serial, &QSerialPort::readyRead, this, &Bll_SerialPort::slReadyRead, Qt::QueuedConnection);
    connect(this, &Bll_SerialPort::sgRecvData, recvAnalyse, &Bll_SerialRecvAnalyse::working);
}

Bll_SerialPort::~Bll_SerialPort()
{
    recvAnalyse->deleteLater();

    if (serial->isOpen()) // 退出程序时，关闭使用中的串口
    {
        serial->close(); // 关闭串口
        qDebug() << deviceName << "关闭串口";
    }
    serial->deleteLater();
    qDebug() << "Bll_SerialPort Destroyed!";

    // 任务对象和线程不需要维护，线程池会维护
    // 自己维护的会导致程序异常退出
}

/// @brief 初始化我的串口（任务对象） 开启串口
/// @param Bll_SerialPortSetting setting
/// @return 异常返回-1 正常返回0
void Bll_SerialPort::init(const Bll_SerialPortSetting setting, RES &res)
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

    // 创建串口接收线程
    threadSerial = new QThread();
    this->moveToThread(threadSerial);
    serial->moveToThread(threadSerial);
    threadSerial->start();

    // 设置解码模式
    recvAnalyse->setAnalyseMode(setting.analyseMode);
    if (setting.analyseMode != 0)
    {
        // 创建串口接收数据分析线程
        threadAnalyse = new QThread();
        recvAnalyse->moveToThread(threadAnalyse);
        threadAnalyse->start();
    }

    res.returnCode = 0;
}

void Bll_SerialPort::slSendData(QByteArray data)
{
    serial->write(data);

    // qDebug() << deviceName << "串口发送线程ID" << QThread::currentThread();
    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
}

//   串口接收任务
void Bll_SerialPort::slReadyRead()
{
    QByteArray rowRxBuf; // 最新接收数据
    rowRxBuf = serial->readAll();

    emit sgRecvData(rowRxBuf);

    // qDebug() << "slReadyRead 线程ID：" << QThread::currentThread();
    // qDebug() << deviceName << "串口接收线程ID" << QThread::currentThread();
    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
}
