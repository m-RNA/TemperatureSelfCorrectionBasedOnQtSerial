#ifndef BLL_SERIAL_H
#define BLL_SERIAL_H

#include <QObject>
#include <QThread>
// #include <QThreadPool>
// #include <QRunnable>
#include <QSerialPort>
#include <QDebug>

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
    explicit Bll_SerialPort(QString name, const Bll_SerialPortSetting &setting, RES &res, QObject *parent = nullptr);
    ~Bll_SerialPort();

    // void run() override;
    void setDeviceName(QString name) { deviceName = name; }
    int init(const Bll_SerialPortSetting);

public slots:

    void slSendData(QString);

private slots:
    void slReadyRead();

    // void slRecvDataAnalyse(QByteArray rxData); // 解析数据

signals:
    void sgRecvData(QByteArray);
    // void RecvDataAnalyseFinish(double data); // 更新波形

private:
    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    QSerialPort *serial = nullptr;
    QThread *thread = nullptr;

    void printSerialPortInitInfo(QSerialPort const *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
    }
};

#endif // BLL_SERIAL_H
