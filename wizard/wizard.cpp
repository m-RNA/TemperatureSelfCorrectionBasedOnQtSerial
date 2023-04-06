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
    ui->ss_Dtm->setSerialIndex(3);
    ui->ss_Std->setAnalyseIndex(1);
    ui->ss_Dtm->setAnalyseIndex(1);

    // 从串口设置中获取串口名，设置到Tab标签名上
    ui->ss_Std->getSettingIndex(wizardInfo.ssIndex_Std);
    ui->ss_Dtm->getSettingIndex(wizardInfo.ssIndex_Dtm);
    setTabName_Std(wizardInfo.ssIndex_Std.portName);
    setTabName_Dtm(wizardInfo.ssIndex_Dtm.portName);

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

    ui->ss_Std->getSettingIndex(wizardInfo.ssIndex_Std);
    ui->ss_Dtm->getSettingIndex(wizardInfo.ssIndex_Dtm);

    qDebug() << wizardInfo.ssIndex_Std.portName << wizardInfo.ssIndex_Std.portNameIndex << wizardInfo.ssIndex_Std.baudRate << wizardInfo.ssIndex_Std.dataBitsIndex << wizardInfo.ssIndex_Std.parityIndex << wizardInfo.ssIndex_Std.stopBitsIndex << wizardInfo.ssIndex_Std.flowControlIndex << wizardInfo.ssIndex_Std.encodeModeIndex << wizardInfo.ssIndex_Std.analyseModeIndex;
    qDebug() << wizardInfo.ssIndex_Dtm.portName << wizardInfo.ssIndex_Dtm.portNameIndex << wizardInfo.ssIndex_Dtm.baudRate << wizardInfo.ssIndex_Dtm.dataBitsIndex << wizardInfo.ssIndex_Dtm.parityIndex << wizardInfo.ssIndex_Dtm.stopBitsIndex << wizardInfo.ssIndex_Dtm.flowControlIndex << wizardInfo.ssIndex_Dtm.encodeModeIndex << wizardInfo.ssIndex_Dtm.analyseModeIndex;

    emit wizardInfoFinish(wizardInfo);
}
