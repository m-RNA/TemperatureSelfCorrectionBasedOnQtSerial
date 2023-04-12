#include "serialsetting.h"
#include "ui_serialsetting.h"


SerialSetting::SerialSetting(QWidget *parent) : QWidget(parent),
                                                ui(new Ui::SerialSetting)
{
    ui->setupUi(this);
}

SerialSetting::~SerialSetting()
{
    delete ui;
}

void SerialSetting::setSettingIndex(const Ui_SerialSettingIndex &uiIndex)
{
    // ui->cbSerial->setCurrentText(uiIndex.portName);
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
    // uiIndex.portName = ui->cbSerial->currentText();
    // uiIndex.portNameIndex = ui->cbSerial->currentIndex();
    uiIndex.baudRate = ui->cbBaudrate->currentText();
    uiIndex.dataBitsIndex = ui->cbDataBit->currentIndex();
    uiIndex.parityIndex = ui->cbCheckBit->currentIndex();
    uiIndex.stopBitsIndex = ui->cbStopBit->currentIndex();
    uiIndex.flowControlIndex = ui->cbFlowCtrl->currentIndex();
    uiIndex.encodeModeIndex = ui->cbEncode->currentIndex();
    uiIndex.analyseModeIndex = ui->cbAnalyse->currentIndex();
}

void SerialSetting::setSerialPortName(const QString &serialName)
{
    ui->lbSerial->setText(serialName);
}
