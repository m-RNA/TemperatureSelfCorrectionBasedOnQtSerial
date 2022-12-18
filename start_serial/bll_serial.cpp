#include "bll_serial.h"
#include <QElapsedTimer>

Bll_SerialPort::Bll_SerialPort(QString name, const Bll_SerialPortSetting &setting, RES &res, QObject *parent) : QObject(parent)
{
    // setAutoDelete(false); // 关掉:放到线程池后自动析构
    serial = new QSerialPort();
    this->setDeviceName(name);
    res.returnCode = init(setting);
    if (res.returnCode < 0)
    {
        res.msg = serial->errorString();
        return;
    }
}

Bll_SerialPort::~Bll_SerialPort()
{
    if (serial->isOpen()) // 退出程序时，关闭使用中的串口
    {
        serial->close(); // 关闭串口
        qDebug() << deviceName << "关闭串口";
    }
    serial->deleteLater();

    if (thread)
    {
        thread->quit();
        // thread->wait();
        thread->deleteLater();
    }
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
    thread = new QThread();
    serial->moveToThread(thread);
    this->moveToThread(thread);
    thread->start();
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

/*   串口接收任务   */
void Bll_SerialPort::slReadyRead()
{
    QByteArray rowRxBuf; // 最新接收数据
    rowRxBuf = serial->readAll();
    emit sgRecvData(rowRxBuf);
    // qDebug() << deviceName << "串口接收线程ID" << QThread::currentThread();
    // qDebug() << deviceName << "活跃线程数" << QThreadPool::globalInstance()->activeThreadCount();
}

// void Bll_SerialPort::serialRecvDataAnalyse(QByteArray rxData)
// {
//     QByteArray rxFrame;
//     static QByteArray staticTemp; // 静态中间变量
//     int startIndex = -1;

//     staticTemp.append(rxData);                 // 读取串口，附在 staticTemp 之后
//     startIndex = staticTemp.lastIndexOf("\n"); // 获取"\n"的索引

//     if (startIndex >= 0)
//     {
//         rxFrame.append(staticTemp.left(startIndex - 1)); // 去除"\r\n"
//         staticTemp.remove(0, startIndex + 1);            // 移除"\n"与"\n"之前的内容
//     }

//     if (rxFrame.isEmpty())
//         return;

//     qDebug() << deviceName << "RxFrame:" << rxFrame;
//     // 解析数据
//     // {text}23.3
//     // printf("temp=%f\r\n",rtd);
//     int title_index_left;
//     title_index_left = rxFrame.indexOf("}");
//     rxData = rxFrame.replace("{", "").left(title_index_left - 1);
//     qDebug() << deviceName << "Title:" << QString(rxData);
//     int titleLength = rxData.length();
//     double data;
//     data = QString(rxFrame.right(rxFrame.length() - 1 - titleLength)).toDouble();
//     qDebug() << deviceName << "Temp:" << data;

//     emit RecvDataAnalyseFinish(data);
// }
