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

void SerialSetting::setSettingIndex(const Ui_SerialSettingIndex &uiIndex)
{
    ui->cbSerial->setCurrentText(uiIndex.portName);
    ui->cbBaudrate->setCurrentText(uiIndex.baudRate);
    ui->cbDataBit->setCurrentIndex(uiIndex.dataBitsIndex);
    ui->cbCheckBit->setCurrentIndex(uiIndex.parityIndex);
    ui->cbStopBit->setCurrentIndex(uiIndex.stopBitsIndex);
    ui->cbFlowCtrl->setCurrentIndex(uiIndex.flowControlIndex);
    ui->cbEncode->setCurrentIndex(uiIndex.encodeModeIndex);
    ui->cbAnalyse->setCurrentIndex(uiIndex.analyseModeIndex);
}

void SerialSetting::getSettingIndex(Ui_SerialSettingIndex &uiIndex)
{
    uiIndex.portName = ui->cbSerial->currentText();
    uiIndex.portNameIndex = ui->cbSerial->currentIndex();
    uiIndex.baudRate = ui->cbBaudrate->currentText();
    uiIndex.dataBitsIndex = ui->cbDataBit->currentIndex();
    uiIndex.parityIndex = ui->cbCheckBit->currentIndex();
    uiIndex.stopBitsIndex = ui->cbStopBit->currentIndex();
    uiIndex.flowControlIndex = ui->cbFlowCtrl->currentIndex();
    uiIndex.encodeModeIndex = ui->cbEncode->currentIndex();
    uiIndex.analyseModeIndex = ui->cbAnalyse->currentIndex();
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
