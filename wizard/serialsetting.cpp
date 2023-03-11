#include "serialsetting.h"
#include "ui_serialsetting.h"
#include <QSerialPortInfo>
#include <QDebug>

SerialSetting::SerialSetting(QWidget *parent) : QWidget(parent),
                                                ui(new Ui::SerialSetting)
{
    ui->setupUi(this);
    updateSerialPortInfo();
}

/// @brief 初始化 串口combo box 扫描更新界面串口端口信息
void SerialSetting::updateSerialPortInfo()
{
    QStringList serialNamePort;
    // 查询串口端口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        serialNamePort << info.portName();
    }
    ui->cbSerial->clear();
    ui->cbSerial->addItems(serialNamePort);
}

SerialSetting::~SerialSetting()
{
    delete ui;
}

/// @brief 从ui界面获取串口设置参数，并对传入的参数赋值
/// @param setting
void SerialSetting::getSettings(Bll_SerialPortSetting &setting)
{
    QString portName = ui->cbSerial->currentText();
    if (portName.isEmpty())
        return;

    setting.portName = portName;
    setting.baudRate = ui->cbBaudrate->currentText().toInt();

    // 数据位
    switch (ui->cbDataBit->currentText().toInt())
    {
    case 5:
        setting.dataBits = QSerialPort::Data5;
        break;
    case 6:
        setting.dataBits = QSerialPort::Data6;
        break;
    case 7:
        setting.dataBits = QSerialPort::Data7;
        break;
    case 8:
        setting.dataBits = QSerialPort::Data8;
        break;
    default:

        break;
    }

    // 校验位
    switch (ui->cbCheckBit->currentIndex())
    {
    case 0:
        setting.parity = QSerialPort::NoParity;
        break;
    case 1:
        setting.parity = QSerialPort::OddParity;
        break;
    case 2:
        setting.parity = QSerialPort::EvenParity;
        break;
    default:
        break;
    }

    // 停止位
    switch (ui->cbStopBit->currentIndex())
    {
    case 0:
        setting.stopBits = QSerialPort::OneStop;
        break;
    case 1:
        setting.stopBits = QSerialPort::OneAndHalfStop;
        break;
    case 2:
        setting.stopBits = QSerialPort::TwoStop;
        break;
    default:
        break;
    }

    // 流控
    switch (ui->cbFlowCtrl->currentIndex())
    {
    case 0:
        setting.flowControl = QSerialPort::NoFlowControl;
        break;
    case 1:
        setting.flowControl = QSerialPort::HardwareControl;
        break;
    case 2:
        setting.flowControl = QSerialPort::SoftwareControl;
        break;
    default:
        break;
    }

    // 编码格式
    setting.encodeMode = ui->cbEncode->currentIndex();

    // 解码格式
    setting.analyseMode = ui->cbAnalyse->currentIndex();
}

/// @brief 事件过滤器
/// @param obj
/// @param event
/// @return
bool SerialSetting::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) // 鼠标点击事件
    {
        if (obj == ui->cbSerial) // cBox
        {
            if (ui->cbSerial->isEnabled())
            {
                updateSerialPortInfo();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SerialSetting::on_cbSerial_activated(int index)
{
    Q_UNUSED(index);
    // qDebug() << ui->cbSerial->currentText();
    emit serialPortChanged(ui->cbSerial->currentText());
}

void SerialSetting::setSerialIndex(const int index)
{
    ui->cbSerial->setCurrentIndex(index);
}

void SerialSetting::setAnalyseIndex(const int index)
{
    ui->cbAnalyse->setCurrentIndex(index);
}
