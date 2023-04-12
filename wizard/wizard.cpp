#include "wizard.h"
#include "ui_wizard.h"
#include <QSerialPortInfo>
#include <QDate>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include "config.h"

Wizard::Wizard(const Ui_SerialSettingIndex &settings_Std, const Ui_SerialSettingIndex &settings_Dtm, QWidget *parent) : QWizard(parent),
                                                                                                                        ui(new Ui::Wizard)
{
    ui->setupUi(this);
    // 设置为当前日期
    ui->dateEdit->setDate(QDate::currentDate());
    // 去掉问号，只保留关闭
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // 更新串口信息
    updateSerialPortInfo('a');

    // 设置界面
    loadUiSettings();
    ui->cbSerial_Std->setCurrentIndex(settings_Std.portNameIndex);
    ui->cbSerial_Dtm->setCurrentIndex(settings_Dtm.portNameIndex);
    ui->ss_Std->setSettingIndex(settings_Std);
    ui->ss_Dtm->setSettingIndex(settings_Dtm);
    ui->ss_Std->setSerialPortName(settings_Std.portName);
    ui->ss_Dtm->setSerialPortName(settings_Dtm.portName);

    // 当向导完成时，从向导界面获取配置信息
    connect(this, &Wizard::accepted, this, &Wizard::getInfo);
}

Wizard::~Wizard()
{
    delete ui;
}

void Wizard::loadUiSettings()
{
    QSettings settings(CONFIG_FILE_NAME, QSettings::IniFormat);
    settings.beginGroup("WizardBaseInfo");
    ui->lePlace->setText(settings.value("Place", "中北大学工程训练中心").toString());
    ui->spbxTemp->setValue(settings.value("Temp", 21).toInt());
    ui->spbxRH->setValue(settings.value("RH", 52).toInt());
    ui->leOperator->setText(settings.value("Operator", "老陈皮").toString());
    ui->leID_Std->setText(settings.value("ID_Std", "道万").toString());
    ui->leID_Dtm->setText(settings.value("ID_Dtm", "待定").toString());

    bool autoSave = settings.value("AutoCollect", true).toBool();
    ui->twTarget->setEnabled(autoSave);
    ui->cbAutoCollect->setCurrentIndex(autoSave);
    settings.endGroup();
}

void Wizard::saveUiSettings()
{
    QSettings settings(CONFIG_FILE_NAME, QSettings::IniFormat);
    settings.beginGroup("WizardBaseInfo");
    settings.setValue("Place", ui->lePlace->text());
    settings.setValue("Temp", ui->spbxTemp->value());
    settings.setValue("RH", ui->spbxRH->value());
    settings.setValue("Operator", ui->leOperator->text());
    settings.setValue("ID_Std", ui->leID_Std->text());
    settings.setValue("ID_Dtm", ui->leID_Dtm->text());
    settings.setValue("AutoCollect", ui->cbAutoCollect->currentIndex());
    settings.endGroup();
}



void Wizard::getInfo()
{
    wizardInfo.baseInfo.clear();
    wizardInfo.baseInfo << ui->lePlace->text()
                        << ui->spbxTemp->text()
                        << ui->spbxRH->text()
                        << ui->dateEdit->text()
                        << ui->leOperator->text()
                        << ui->leID_Std->text()
                        << ui->leID_Dtm->text();
    qDebug() << wizardInfo.baseInfo;

    ui->ss_Std->getSettingIndex(wizardInfo.ssIndex_Std);
    ui->ss_Dtm->getSettingIndex(wizardInfo.ssIndex_Dtm);

    wizardInfo.ssIndex_Std.portNameIndex = ui->cbSerial_Std->currentIndex();
    wizardInfo.ssIndex_Dtm.portNameIndex = ui->cbSerial_Dtm->currentIndex();

    qDebug() << wizardInfo.ssIndex_Std.portName << wizardInfo.ssIndex_Std.portNameIndex << wizardInfo.ssIndex_Std.baudRate << wizardInfo.ssIndex_Std.dataBitsIndex << wizardInfo.ssIndex_Std.parityIndex << wizardInfo.ssIndex_Std.stopBitsIndex << wizardInfo.ssIndex_Std.flowControlIndex << wizardInfo.ssIndex_Std.encodeModeIndex << wizardInfo.ssIndex_Std.analyseModeIndex;
    qDebug() << wizardInfo.ssIndex_Dtm.portName << wizardInfo.ssIndex_Dtm.portNameIndex << wizardInfo.ssIndex_Dtm.baudRate << wizardInfo.ssIndex_Dtm.dataBitsIndex << wizardInfo.ssIndex_Dtm.parityIndex << wizardInfo.ssIndex_Dtm.stopBitsIndex << wizardInfo.ssIndex_Dtm.flowControlIndex << wizardInfo.ssIndex_Dtm.encodeModeIndex << wizardInfo.ssIndex_Dtm.analyseModeIndex;

    emit wizardInfoFinish(wizardInfo);
}

/// @brief 初始化 串口combo box 扫描更新界面串口端口信息
void Wizard::updateSerialPortInfo(char who)
{
    QStringList serialNamePort;
    // 查询串口端口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        serialNamePort << info.portName();
    }

    switch (who)
    {
    case 's':
        ui->cbSerial_Std->clear();
        ui->cbSerial_Std->addItems(serialNamePort);
        break;
    case 'd':
        ui->cbSerial_Dtm->clear();
        ui->cbSerial_Dtm->addItems(serialNamePort);
        break;

    case 'a':
        ui->cbSerial_Std->clear();
        ui->cbSerial_Std->addItems(serialNamePort);
        ui->cbSerial_Dtm->clear();
        ui->cbSerial_Dtm->addItems(serialNamePort);
        break;
    }
}

/// @brief 事件过滤器
/// @param obj
/// @param event
/// @return
bool Wizard::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) // 鼠标点击事件
    {
        if (obj == ui->cbSerial_Std)
        {
            if (ui->cbSerial_Std->isEnabled())
            {
                updateSerialPortInfo('s');
            }
        }
        else if (obj == ui->cbSerial_Dtm)
        {
            if (ui->cbSerial_Dtm->isEnabled())
            {
                updateSerialPortInfo('d');
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void Wizard::on_cbSerial_Std_activated(int index)
{
    Q_UNUSED(index);
    ui->twSerial->setCurrentIndex(0);
}

void Wizard::on_cbSerial_Dtm_activated(int index)
{
    Q_UNUSED(index);
    ui->twSerial->setCurrentIndex(1);
}

void Wizard::on_cbSerial_Std_currentIndexChanged(const QString &arg1)
{
    ui->ss_Std->setSerialPortName(arg1);
}

void Wizard::on_cbSerial_Dtm_currentIndexChanged(const QString &arg1)
{
    ui->ss_Dtm->setSerialPortName(arg1);
}

void Wizard::on_cbAutoCollect_currentIndexChanged(int index)
{
    ui->twTarget->setEnabled(index);
}
