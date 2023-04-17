#include "wizard.h"
#include "ui_wizard.h"
#include <QSerialPortInfo>
#include <QDate>
#include <QDebug>
#include "mainwindow.h"

Wizard::Wizard(WizardInfo *info, QWidget *parent) : QWizard(parent),
                                                    ui(new Ui::Wizard)
{
    ui->setupUi(this);
    // 去掉问号，只保留关闭
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // 更新串口信息
    updateSerialPortInfo('a');

    wizardInfo = info;
    // 设置界面
    ui->lePlace->setText(wizardInfo->baseInfo.place);
    ui->spbxTemp->setValue(wizardInfo->baseInfo.temp);
    ui->spbxRH->setValue(wizardInfo->baseInfo.rh);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->leOperator->setText(wizardInfo->baseInfo.operatorName);
    ui->leID_Std->setText(wizardInfo->baseInfo.id_Std);
    ui->leID_Dtm->setText(wizardInfo->baseInfo.id_Dtm);
    ui->spbxSamplePointSum->setValue(wizardInfo->collectSetting.num);
    ui->spbxSampleTime->setValue(wizardInfo->collectSetting.time);
    ui->cbAutoCollect->setCurrentIndex(wizardInfo->collectSetting.isAuto);
    ui->spbxWaveRange->setValue(wizardInfo->checkWaveSetting.range);
    ui->spbxWaveNum->setValue(wizardInfo->checkWaveSetting.num);
    ui->spbxStableTime->setValue(wizardInfo->checkWaveSetting.stableTime);
    ui->cbSerial_Std->setCurrentIndex(wizardInfo->ssIndex_Std.portNameIndex);
    ui->cbSerial_Dtm->setCurrentIndex(wizardInfo->ssIndex_Dtm.portNameIndex);
    ui->ss_Std->setSettingIndex(wizardInfo->ssIndex_Std);
    ui->ss_Dtm->setSettingIndex(wizardInfo->ssIndex_Dtm);
    ui->ss_Std->setSerialPortName(wizardInfo->ssIndex_Std.portName);
    ui->ss_Dtm->setSerialPortName(wizardInfo->ssIndex_Dtm.portName);

    ui->twTarget->setRowCount(wizardInfo->collectSetting.num);
    // 在twTarget 填入autoList的数据
    int size = (int)wizardInfo->collectSetting.autoList.size();
    size = size > wizardInfo->collectSetting.num ? wizardInfo->collectSetting.num : size;
    for (int i = 0; i < size; i++)
    {
        ui->twTarget->setItem(i, 0, new QTableWidgetItem(QString::number(wizardInfo->collectSetting.autoList[i])));
    }
    // 补上空行
    for (int i = size; i < wizardInfo->collectSetting.num; i++)
    {
        ui->twTarget->setItem(i, 0, new QTableWidgetItem(""));
    }
    ui->twTarget->setEnabled(wizardInfo->collectSetting.isAuto);

    // 当向导完成时，从向导界面获取配置信息
    connect(this, &Wizard::accepted, this, &Wizard::getInfo);
}

Wizard::~Wizard()
{
    delete ui;
}

void Wizard::getInfo()
{
    wizardInfo->baseInfo.place = ui->lePlace->text();
    wizardInfo->baseInfo.temp = ui->spbxTemp->value();
    wizardInfo->baseInfo.rh = ui->spbxRH->value();
    wizardInfo->baseInfo.date = ui->dateEdit->date().toString("yyyy/MM/dd");
    wizardInfo->baseInfo.operatorName = ui->leOperator->text();
    wizardInfo->baseInfo.id_Std = ui->leID_Std->text();
    wizardInfo->baseInfo.id_Dtm = ui->leID_Dtm->text();

    ui->ss_Std->getSettingIndex(wizardInfo->ssIndex_Std);
    ui->ss_Dtm->getSettingIndex(wizardInfo->ssIndex_Dtm);
    wizardInfo->ssIndex_Std.portNameIndex = ui->cbSerial_Std->currentIndex();
    wizardInfo->ssIndex_Dtm.portNameIndex = ui->cbSerial_Dtm->currentIndex();
    qDebug() << wizardInfo->ssIndex_Std.portName << wizardInfo->ssIndex_Std.portNameIndex;
    qDebug() << wizardInfo->ssIndex_Dtm.portName << wizardInfo->ssIndex_Dtm.portNameIndex;

    int samplePointSum = ui->spbxSamplePointSum->value();
    wizardInfo->collectSetting.num = samplePointSum;
    wizardInfo->collectSetting.time = ui->spbxSampleTime->value();
    wizardInfo->collectSetting.isAuto = ui->cbAutoCollect->currentIndex();
    if (wizardInfo->collectSetting.isAuto)
    {
        wizardInfo->collectSetting.autoList.clear();
        // 将twTarget中的数据保存到collectSetting中
        for (int i = 0; i < samplePointSum; i++)
        {
            QString temp;
            temp = ui->twTarget->item(i, 0)->text();
            if (temp.isEmpty())
                continue;
            qDebug() << temp;
            wizardInfo->collectSetting.autoList.push_back(temp.toDouble());
        }
    }

    wizardInfo->checkWaveSetting.range = ui->spbxWaveRange->value();
    wizardInfo->checkWaveSetting.num = ui->spbxWaveNum->value();
    wizardInfo->checkWaveSetting.stableTime = ui->spbxStableTime->value();

    emit wizardInfoFinish();
}

/// @brief 初始化 串口combo box 扫描更新界面串口端口信息
void Wizard::updateSerialPortInfo(char who)
{
    QStringList serialNamePort;
    // 查询串口端口信息
    foreach (const QSerialPortInfo &spInfo, QSerialPortInfo::availablePorts())
    {
        serialNamePort << spInfo.portName();
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

void Wizard::on_spbxSamplePointSum_valueChanged(int arg1)
{
    ui->twTarget->setRowCount(arg1);
    // 补上空行
    for (int i = 0; i < arg1; i++)
    {
        if (ui->twTarget->item(i, 0) == nullptr)
        {
            ui->twTarget->setItem(i, 0, new QTableWidgetItem(""));
        }
    }
}
