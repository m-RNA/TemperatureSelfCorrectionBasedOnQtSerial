#include "bll_serial.h"

#include <QElapsedTimer>

Bll_SerialPort::Bll_SerialPort(QString name, const Bll_SerialPortSetting &setting, RES &res, QObject *parent) : QObject(parent)
{
    // setAutoDelete(false); // 关掉:放到线程池后自动析构
    serial = new QSerialPort();

    this->setDeviceName(name);
    recvAnalyse = new Bll_SerialRecvAnalyse;
    connect(this, &Bll_SerialPort::sgRecvData, recvAnalyse, &Bll_SerialRecvAnalyse::working);

    // QThreadPool::globalInstance()->start(recvAnalyse);
    res.returnCode = init(setting);
    if (res.returnCode < 0)
    {
        res.msg = serial->errorString();
        return;
    }
}

Bll_SerialPort::~Bll_SerialPort()
{
    // 任务对象和线程不需要维护，线程池会维护
    // disconnect(this, &Bll_SerialPort::sgRecvData, recvAnalyse, &Bll_SerialRecvAnalyse::slBll_GetRowRecvData);
    recvAnalyse->deleteLater();

    if (serial->isOpen()) // 退出程序时，关闭使用中的串口
    {
        serial->close(); // 关闭串口
        qDebug() << deviceName << "关闭串口";
    }

    if (threadAnalyse)
    {
        threadAnalyse->quit();
        // threadAnalyse->wait();
        threadAnalyse->deleteLater();
    }

    if (threadSerial)
    {
        threadSerial->quit();
        // threadSerial->wait();
        threadSerial->deleteLater();
    }

    serial->deleteLater();

    qDebug() << "Bll_SerialPort Destroyed!";
}

// void Bll_SerialPort::run()
//{
//     QThread::exec();
// }

/// @brief 初始化我的串口（任务对象） 开启串口
/// @param Bll_SerialPortSetting setting
/// @return 异常返回-1 正常返回0
int Bll_SerialPort::init(const Bll_SerialPortSetting setting)
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
        // serialState = false; // 串口状态 置关
        emit;
        return -1;
    }

    // 正常打开
    qDebug() << deviceName << "打开串口";
    // serialState = true; // 串口状态 置开

    emit;

    // 创建线程
    threadSerial = new QThread();
    threadAnalyse = new QThread();

    serial->moveToThread(threadSerial);
    this->moveToThread(threadSerial);
    qDebug() << "主线程ID：" << QThread::currentThread();

    recvAnalyse->moveToThread(threadAnalyse);

    threadSerial->start();
    threadAnalyse->start();
    // QThreadPool::globalInstance()->start(this);

    // 连接 readyRead信号 与 串口接收槽函数
    connect(serial, &QSerialPort::readyRead, this, &Bll_SerialPort::slReadyRead, Qt::QueuedConnection);

    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
    return 0;
}

void Bll_SerialPort::slSendData(QString text)
{
    serial->write(text.toLocal8Bit().data());
    // qDebug() << deviceName << "串口发送线程ID" << QThread::currentThread();
    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
}

//   串口接收任务
void Bll_SerialPort::slReadyRead()
{
    // qDebug() << "slReadyRead 线程ID：" << QThread::currentThread();

    QByteArray rowRxBuf; // 最新接收数据
    rowRxBuf = serial->readAll();

    emit sgRecvData(rowRxBuf);

    // qDebug() << deviceName << "串口接收线程ID" << QThread::currentThread();
    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
}

// Bll_SerialSend::Bll_SerialSend(QObject *parent) : QObject(parent), QRunnable()
// {
// }

// Bll_SerialSend::~Bll_SerialSend()
// {
//     qDebug() << "Bll_SerialSend Destroyed!";
// }

// void Bll_SerialSend::run()
// {
//     serial->write(data.toLocal8Bit().data());
//     qDebug() << "串口发送线程ID" << QThread::currentThread();
//     qDebug() << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
// }
