#include "wizard.h"
#include "ui_wizard.h"
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

    // 设置界面
    loadUiSettings();
    ui->ss_Std->setSettingIndex(settings_Std);
    ui->ss_Dtm->setSettingIndex(settings_Dtm);

    // 从串口设置中获取串口名，设置到Tab标签名上
    setTabName_Std(settings_Std.portName);
    setTabName_Dtm(settings_Dtm.portName);

    // 实时改变Tab标签名
    connect(ui->ss_Std, &SerialSetting::serialPortChanged, this, &Wizard::setTabName_Std);
    connect(ui->ss_Dtm, &SerialSetting::serialPortChanged, this, &Wizard::setTabName_Dtm);

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
    settings.endGroup();
}

void Wizard::setTabName_Std(const QString &portName)
{
    QString s = "标准仪器";
    s += "[";
    s += portName;
    s += "]";
    qDebug() << s;
    ui->tabWidget->setTabText(0, s);
}

void Wizard::setTabName_Dtm(const QString &portName)
{
    QString s = "待定仪器";
    s += "[";
    s += portName;
    s += "]";
    qDebug() << s;
    ui->tabWidget->setTabText(1, s);
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

    qDebug() << wizardInfo.ssIndex_Std.portName << wizardInfo.ssIndex_Std.portNameIndex << wizardInfo.ssIndex_Std.baudRate << wizardInfo.ssIndex_Std.dataBitsIndex << wizardInfo.ssIndex_Std.parityIndex << wizardInfo.ssIndex_Std.stopBitsIndex << wizardInfo.ssIndex_Std.flowControlIndex << wizardInfo.ssIndex_Std.encodeModeIndex << wizardInfo.ssIndex_Std.analyseModeIndex;
    qDebug() << wizardInfo.ssIndex_Dtm.portName << wizardInfo.ssIndex_Dtm.portNameIndex << wizardInfo.ssIndex_Dtm.baudRate << wizardInfo.ssIndex_Dtm.dataBitsIndex << wizardInfo.ssIndex_Dtm.parityIndex << wizardInfo.ssIndex_Dtm.stopBitsIndex << wizardInfo.ssIndex_Dtm.flowControlIndex << wizardInfo.ssIndex_Dtm.encodeModeIndex << wizardInfo.ssIndex_Dtm.analyseModeIndex;

    emit wizardInfoFinish(wizardInfo);
}
