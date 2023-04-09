#ifndef SERIALSETTING_H
#define SERIALSETTING_H

#include <QWidget>
#include <QSerialPort>

typedef struct
{
    QString portName;
    int portNameIndex;
    QString baudRate;
    int dataBitsIndex;
    int parityIndex;
    int stopBitsIndex;
    int flowControlIndex;
    int encodeModeIndex;
    int analyseModeIndex;
} Ui_SerialSettingIndex;

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

    void getSettingIndex(Ui_SerialSettingIndex &uiIndex);
    void setSettingIndex(const Ui_SerialSettingIndex &uiIndex);

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
