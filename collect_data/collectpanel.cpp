#include "collectpanel.h"
#include "ui_collectpanel.h"
#include "BigFloat.h"

CollectPanel::CollectPanel(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::CollectPanel)
{
    ui->setupUi(this);
    ui->chart->setXAxisToTimelineState(true);
}

CollectPanel::~CollectPanel()
{
    delete ui;
}

void CollectPanel::setOnlineState(bool state)
{
    setLEDState(state);
    stableState = 0;
}

void CollectPanel::setStableState(const char state)
{
    stableState = state;
    setLEDState(3 + state);
}

void CollectPanel::setLEDState(int state)
{
    switch (state)
    {
    case 0:
        ui->ledText->setText("离线    ");
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");
        break;

    case 1:
        ui->ledText->setText("在线    ");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);"); // 绿色
        break;

    case 2:
        ui->ledText->setText("采集中  ");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(23, 111, 217);"); // 蓝色
        break;

    case 3:
        ui->ledText->setText("波动超阈");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(255, 176, 5);"); // 黄色
        break;

    case 4:
        ui->ledText->setText("波动稳定");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);"); // 绿色
        break;

    case 5:
        ui->ledText->setText("解析超时");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(255, 176, 5);"); // 黄色
        break;
    }
}

void CollectPanel::setDeviceName(QString name)
{
    deviceName = name;
    ui->chart->graph()->setName(name);
}

void CollectPanel::setYAxisFormat(const QString &format, const int precision)
{
    ui->chart->yAxis->setNumberFormat(format);
    ui->chart->yAxis->setNumberPrecision(precision);
}

void CollectPanel::slCollectData(const SerialAnalyseCell &cell)
{
    // 更新最后示数
    ui->leastData->setText(QString::number(cell.value));

    // 将Y轴数据添加到曲线图上
    ui->chart->addYPointBaseOnTime(cell);
}

void CollectPanel::collectStart(void)
{
    setLEDState(2);
}

void CollectPanel::collectStop(void)
{
    ui->lbRange->setText("NULL");
}
void CollectPanel::collectFinish(void)
{
    // 最新几个数据点可能卡在软件定时器里了，更新一下
    ui->chart->chartRefresh();
    collectStop();
}
void CollectPanel::collectRestart(void)
{
    collectStart();
    ui->lbRange->setText("0");
}

double CollectPanel::getRange(void)
{
    return range;
}

QCPAxis *CollectPanel::getXAxis(void)
{
    return ui->chart->xAxis;
}

void CollectPanel::setReceiveTimeout(void)
{
    stableState = 0;
    setLEDState(5);
}

void CollectPanel::slSetRange(const double &range)
{
    this->range = range;
    ui->lbRange->setText(QString::number(range));
}

bool CollectPanel::isStable(void)
{
    if (stableState != 1)
        return false;
    return true;
}

void CollectPanel::on_btnShowData_clicked()
{
    emit sgShowData();
}
