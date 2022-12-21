#ifndef BLL_SERIAL_H
#define BLL_SERIAL_H

#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSerialPort>
#include <QDebug>
#include "bll_serialrecvanalyse.h"
#include "interactchart.h"

typedef struct _Bll_SerialPortSetting
{
    QString portName;
    int baudRate;
    QSerialPort::Parity parity;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
} Bll_SerialPortSetting;

typedef struct _resStruct
{
    int returnCode = 0;
    QString msg = "";
} RES;

class Bll_SerialPort : public QObject // , public QRunnable
{
    Q_OBJECT
public:
    Bll_SerialRecvAnalyse *recvAnalyse = nullptr;

    explicit Bll_SerialPort(QString name, QObject *parent = nullptr);
    ~Bll_SerialPort();

    // void run() override;
    void setDeviceName(QString name) { deviceName = name; }
    void setChartAddr(InteractChart *addr) { chartAddr = addr; }
    void init(const Bll_SerialPortSetting setting, RES &res);

public slots:
    void slSendData(QString);

private slots:
    void slReadyRead();

signals:
    void sgRecvData(QByteArray);

private:
    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    QSerialPort *serial = nullptr;
    QThread *threadSerial = nullptr;
    QThread *threadAnalyse = nullptr;
    InteractChart *chartAddr = nullptr;
    // Bll_SerialSend *bll_SerialSend = nullptr;

    void printSerialPortInitInfo(QSerialPort const *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
    }
};

// class Bll_SerialSend : public QObject, public QRunnable
// {
//     Q_OBJECT
// public:
//     explicit Bll_SerialSend(QObject *parent = nullptr);
//     ~Bll_SerialSend();

//     void run() override;

//     void bll_SetSerialAddr(QSerialPort *serialAddr) { serial = serialAddr; }

// public slots:
//     void slBll_SendData(QString sendData) { data = sendData; }

// private:
//     QSerialPort *serial;
//     QString data;
// };

#endif // BLL_SERIAL_H
