#ifndef BLL_SERIAL_H
#define BLL_SERIAL_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QDebug>
#include "bll_serialrecvanalyse.h"
#include "bll_codeconverter.h"

typedef struct
{
    int returnCode = 0;
    QString msg = "";
} RES;

typedef struct
{
    QString portName;
    int baudRate;
    QSerialPort::Parity parity;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;

    int encodeMode = 0;
    int analyseMode = 0;
} Bll_SerialPortSetting;

class Bll_SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit Bll_SerialPort(QString name, QObject *parent = nullptr);
    ~Bll_SerialPort();

    void setDeviceName(QString name) { deviceName = name; }

    void init(const Bll_SerialPortSetting &setting, RES &res);

public slots:
    void slSendData(const QByteArray &);

private slots:
    void slReadyRead();

signals:
    void sgRecvData(QByteArray);

private:
    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    QSerialPort *serial = nullptr;

    void printSerialPortInitInfo(QSerialPort const *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate();
        qDebug() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
        // qDebug() << "Encode Mode:" << setting.encodeMode << "Analyse Mode:" << setting.analyseMode;
    }
};

#endif // BLL_SERIAL_H
