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
        ui->ledText->setText("数据波动");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(197, 186, 10);"); // 黄色
        break;
    }
}

void CollectPanel::setDeviceName(QString name)
{
    ui->chart->graph()->setName(name);
}

// 防止溢出平均值 double在数值较小时，精度较高
double CollectPanel::average(void)
{
    double ans = 0;
    unsigned long long size = data.size();

    for (unsigned long long i = 0; i < size; ++i)
    {
        ans = (double)i / (double)(i + 1) * ans + data.at(i) / (i + 1);
    }
    emit sgCollectDataGet(data);
    return ans;
}

void CollectPanel::slCollectData(const serialAnalyseCell &cell)
{
    // 更新最后示数
    ui->leastData->setText(QString::number(cell.value));

    // 将Y轴数据添加到曲线图上
    ui->chart->addYPointBaseOnTime(cell);

    // 如果是刚刚打开采集状态，就要重置最大值和最小值
    if (resetRange == true)
    {
        min = cell.value;
        max = cell.value;
        resetRange = false;
    }

    // 如果是采集状态，将数据添加到data中
    if (collectState == true)
    {
        data.push_back(cell.value);
        if (cell.value > max)
        {
            max = cell.value;
            range = max - min;
            ui->lbRange->setText(QString::number(range));
        }
        if (cell.value < min)
        {
            min = cell.value;
            range = max - min;
            ui->lbRange->setText(QString::number(range));
        }
    }
}

void CollectPanel::collectStart(void)
{
    resetRange = true;
    collectState = true;
    data.clear();
}
void CollectPanel::collectStop(void)
{
    collectState = false;
    ui->lbRange->setText("NULL");
}
void CollectPanel::collectFinish(void)
{
    ui->chart->chartRefresh(); // 最新几个数据点可能卡在软件定时器里了，更新一下
    emit sgCollectDataAverage(average());
    collectStop();
}

double CollectPanel::getRange(void)
{
    return range;
}

QCPAxis *CollectPanel::getXAxis(void)
{
    return ui->chart->xAxis;
}

// 检查输入数据波动是否超过阈值
bool checkDataFluctuation(bool reset = false, const double &data = 0)
{
    static double maxData = 0;
    static double minData = 0;
    static unsigned count = 0;
    bool stateFlag = false;
    static bool lastStateFlag = stateFlag;

    // 重置变量
    if (reset == true)
    {
        maxData = data;
        minData = data;
        count = 0;
        return true;
    }

    // 记录最大值和最小值
    if (data > maxData)
        maxData = data;
    if (data < minData)
        minData = data;

    // 当数据量大于10时，检查最大值和最小值的差值是否超过阈值
    if (count > 10)
    {
        stateFlag = (maxData - minData) > 0.5 ? false : true;
    }

    // 检查状态是否发生变化 有变化就打印
    if (stateFlag ^ lastStateFlag)
    {
        qDebug() << "maxData:" << maxData;
        qDebug() << "minData:" << minData;

        if (stateFlag == false)
            qDebug() << "数据波动超过阈值";
        else
            qDebug() << "数据波动正常";
    }

    lastStateFlag = stateFlag;
    count++;
    return true;
}
