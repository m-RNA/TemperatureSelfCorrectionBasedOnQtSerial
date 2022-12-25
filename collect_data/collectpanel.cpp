#include "collectpanel.h"
#include "ui_collectpanel.h"

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

void CollectPanel::slSetState(int state)
{
    switch (state)
    {
    case 0:
        ui->ledText->setText("离线     ");
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");
        break;

    case 1:
        ui->ledText->setText("在线     ");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);"); // 绿色
        break;

    case 2:
        ui->ledText->setText("采集中...");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(23, 111, 217);"); // 蓝色
        break;

    case 3:
        ui->ledText->setText("波动     ");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(197, 186, 10);"); // 黄色
        break;
    }
}

void CollectPanel::setDeviceName(QString name)
{
    ui->chart->graph()->setName(name);
}

// 防止溢出平均值 double在数值较小时，精度较高
double average(vector<double> data)
{
    double ans = 0;
    unsigned long long size = data.size();

    for (unsigned long long i = 0; i < size; ++i)
    {
        ans = (double)i / (double)(i + 1) * ans + data.at(i) / (i + 1);
    }
    return ans;
}

void CollectPanel::slCollectData(double data_t)
{
    ui->leastData->setText(QString::number(data_t));
    if (collectState == true)
        data.push_back(data_t);
}

void CollectPanel::collectFinish(void)
{
    ui->chart->chartRefresh(); // 最新几个数据点可能卡在软件定时器里了，更新一下

    collectStop();
    emit sgCollectDataAverage(average(data));
    data.clear();
}

void CollectPanel::slAddYPoint(double y)
{
    ui->chart->addYPointBaseOnCurrentTime(y);
}
