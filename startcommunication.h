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

    void setTitle(QString s);

public slots:

private slots:
    void on_btnSerialSwitch_clicked();
    void uiLookUpdata(bool state);

signals:
    void serialStateChange(bool state);

private:
    Ui::StartCommunication *ui;
    QString deviceName = "未知仪器"; // 需要初始化变量，不然会程序会异常退出 参考B站：BV1U14y1K7Po
    bool serialState = false;

    void serialInfoUpdata(void);

    void serialInit(QSerialPort sp);

    void printSerialPortInitInfo(QSerialPort *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
    }
};

#endif // STARTCOMMUNICATION_H
