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
    QString deviceName;
    QSerialPort *serial;
    bool serialState = false;

    void serialInfoUpdata(void);

    void serialInit(QSerialPort sp);

    void printSerialPortInitInfo(QSerialPort *sp)
    {
        qDebug() << deviceName << sp->portName() << sp->baudRate() << sp->dataBits() << sp->parity() << sp->stopBits() << sp->flowControl();
    }
};

#endif // STARTCOMMUNICATION_H
