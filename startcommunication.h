#ifndef STARTCOMMUNICATION_H
#define STARTCOMMUNICATION_H

#include <QWidget>
#include <QSerialPort>
#include <QDebug>

namespace Ui
{
    class StartCommunication;
}

class StartCommunication : public QWidget
{
    Q_OBJECT

public:
    explicit StartCommunication(QWidget *parent = nullptr);
    ~StartCommunication();

    bool eventFilter(QObject *obj, QEvent *event);

    void setDeviceName(QString s);

    bool state()
    {
        return serialState;
    }

public slots:

private slots:
    void on_btnSerialSwitch_clicked();
    void uiLookUpdate(bool state);

    void serialReadyRead_Slot();

    void serialRecvDataAnalyse(QByteArray rxData); // 解析数据
    void serialRecvTEditUpdate(QByteArray rxData); // 更新TextEdit

    void on_btnClearRecvTE_clicked();

    void on_btnSend_clicked();

    void on_btnClearSendTE_clicked();

    void on_cbPause_toggled(bool checked);

    void on_cbSendRegular_toggled(bool checked);

    void on_spbxRegularTime_valueChanged(int arg1);

signals:
    void serialStateChange(bool state);
    void serialPauseStateChange(bool state);
    void serialRecvData(QByteArray data);
    void RecvDataAnalyseFinish(double data); // 更新波形

private:
    Ui::StartCommunication *ui;

    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    QSerialPort *serial;
    QTimer *timerSend;
    bool serialState = false;    // 串口状态 true为开 false为关
    bool recvPauseState = false; // 暂停接收状态 true为暂停 false为正常
    // bool sendRegularState = false; // 定时发送状态 true为开启 false为关闭

    void serialInfoUpdate(void);

    void printSerialPortInitInfo(QSerialPort *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
    }
};

#endif // STARTCOMMUNICATION_H
