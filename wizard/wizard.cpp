#include "wizard.h"
#include "ui_wizard.h"
#include <QDate>
#include <QDebug>

Wizard::Wizard(QWidget *parent) : QWizard(parent),
                                  ui(new Ui::Wizard)
{
    ui->setupUi(this);
    // 设置为当前日期
    ui->dateEdit->setDate(QDate::currentDate());
    // 去掉问号，只保留关闭
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 设置串口设置的默认值
    ui->ss_Std->setSerialIndex(1);
    ui->ss_Dtm->setSerialIndex(0);
    ui->ss_Std->setAnalyseIndex(2);
    ui->ss_Dtm->setAnalyseIndex(1);

    // 从串口设置中获取串口名，设置到Tab标签名上
    ui->ss_Std->getSettings(wizardInfo.sSetting_Std);
    ui->ss_Dtm->getSettings(wizardInfo.sSetting_Dtm);
    setTabName_Std(wizardInfo.sSetting_Std.portName);
    setTabName_Dtm(wizardInfo.sSetting_Dtm.portName);

    // 实时改变Tab标签名
    connect(ui->ss_Std, &SerialSetting::serialPortChanged, this, &Wizard::setTabName_Std);
    connect(ui->ss_Dtm, &SerialSetting::serialPortChanged, this, &Wizard::setTabName_Dtm);

    // 当向导关闭时，从向导界面获取配置信息
    connect(this, &Wizard::accepted, this, &Wizard::getInfo);
}

Wizard::~Wizard()
{
    delete ui;
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

    ui->ss_Std->getSettings(wizardInfo.sSetting_Std);
    ui->ss_Dtm->getSettings(wizardInfo.sSetting_Dtm);
    qDebug() << wizardInfo.sSetting_Std.portName << wizardInfo.sSetting_Std.baudRate << wizardInfo.sSetting_Std.parity << wizardInfo.sSetting_Std.dataBits << wizardInfo.sSetting_Std.stopBits << wizardInfo.sSetting_Std.flowControl << wizardInfo.sSetting_Std.encodeMode << wizardInfo.sSetting_Std.analyseMode;
    qDebug() << wizardInfo.sSetting_Dtm.portName << wizardInfo.sSetting_Dtm.baudRate << wizardInfo.sSetting_Dtm.parity << wizardInfo.sSetting_Dtm.dataBits << wizardInfo.sSetting_Dtm.stopBits << wizardInfo.sSetting_Dtm.flowControl << wizardInfo.sSetting_Dtm.encodeMode << wizardInfo.sSetting_Dtm.analyseMode;

    emit wizardInfoFinish(wizardInfo);
}
