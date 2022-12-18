#ifndef STARTCOMMUNICATION_H
#define STARTCOMMUNICATION_H

#include <QWidget>
#include <QSerialPort>
#include <QDebug>
#include "bll_serial.h"

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
        return serialPortState;
    }

public slots:

private slots:
    void on_btnSerialSwitch_clicked();

    void slSerialPortRecvData(QByteArray rxData); // 更新TextEdit

    void on_btnClearRecvTE_clicked();

    void on_btnSend_clicked();

    void on_btnClearSendTE_clicked();

    void on_cbPause_toggled(bool checked);

    void on_cbSendRegular_toggled(bool checked);

    void on_spbxRegularTime_valueChanged(int arg1);

signals:
    void sgSerialPortSendData(QString);

private:
    Ui::StartCommunication *ui;

    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    QTimer *timerSendRegular = nullptr;

    bool serialPortState = false; // 串口状态 true为开 false为关
    bool recvPauseState = false;  // 暂停接收状态 true为暂停 false为正常

    // 任务对象
    Bll_SerialPort *bll_SerialPort = nullptr;

    void updateSerialPortInfo(void);
    void setSerialPortCtrlState(bool state);
    int getSerialPortSetting(Bll_SerialPortSetting &setting);
};

#endif // STARTCOMMUNICATION_H
