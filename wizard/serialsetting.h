#ifndef SERIALSETTING_H
#define SERIALSETTING_H

#include <QWidget>
#include <QSerialPort>

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

namespace Ui
{
    class SerialSetting;
}

class SerialSetting : public QWidget
{
    Q_OBJECT

public:
    explicit SerialSetting(QWidget *parent = nullptr);
    ~SerialSetting();
    void getSettings(Bll_SerialPortSetting &settings);
    void setSerialIndex(const int index);
    void setAnalyseIndex(const int index);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_cbSerial_activated(int index);

signals:
    void serialPortChanged(const QString &);

private:
    Ui::SerialSetting *ui;
    void updateSerialPortInfo();
};

#endif // SERIALSETTING_H
