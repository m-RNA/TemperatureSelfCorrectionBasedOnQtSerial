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
    onlineState = state;
    setLEDState(onlineState);
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

    // 如果是刚刚打开采集状态，就要重置最大值和最小值,并清空数据
    if (resetRange == true)
    {
        min = cell.value;
        max = cell.value;
        resetRange = false;
    }

    // 如果是采集状态，将数据添加到data中
    if (collectState == true)
    {
        // 采集数据
        data.push_back(cell.value);

        // 计算采集过程中的极差
        if (cell.value > max)
        {
            max = cell.value;
            currentRange = max - min;
            ui->lbRange->setText(QString::number(currentRange));
        }
        else if (cell.value < min)
        {
            min = cell.value;
            currentRange = max - min;
            ui->lbRange->setText(QString::number(currentRange));
        }
    }
}

void CollectPanel::collectStart(void)
{
    resetRange = true;
    collectState = true;
    stableState = false; // 设置为不稳定，防止采集完成后，波动检查还没完成，就开始采集下一组数据
    data.clear();
    setLEDState(2);

    // 关闭线程，应该可以早点关闭的
    if (threadAverage)
    {
        threadAverage->quit();
        threadAverage->wait();
        threadAverage = nullptr;
    }
}
void CollectPanel::collectStop(void)
{
    collectState = false;
    ui->lbRange->setText("NULL");
}
void CollectPanel::collectFinish(void)
{
    // 最新几个数据点可能卡在软件定时器里了，更新一下
    ui->chart->chartRefresh();

    // 启动计算平均值线程
    // 局部线程的创建的创建
    if (threadAverage)
    {
        threadAverage->quit();
        threadAverage->wait();
    }
    taskAverage = new Bll_Average;
    threadAverage = new QThread;
    taskAverage->moveToThread(threadAverage);
    connect(threadAverage, &QThread::finished, [=]()
            {taskAverage->deleteLater(); qDebug() << "threadAverage finished"; });
    connect(this, &CollectPanel::sgCollectDataGet, taskAverage, &Bll_Average::slAverage);
    connect(taskAverage, &Bll_Average::sgAverage, [=](const string &str)
            { emit sgCollectDataAverage(str); });
    threadAverage->start();

    // 将数据发送给主线程中的xlsx表格保存数据, 同时用于计算平均值
    emit sgCollectDataGet(data);

    collectStop();
}
void CollectPanel::collectRestart(void)
{
    collectStart();
    ui->lbRange->setText("NULL");
}

double CollectPanel::getRange(void)
{
    return currentRange;
}

QCPAxis *CollectPanel::getXAxis(void)
{
    return ui->chart->xAxis;
}

void CollectPanel::setReceiveTimeout(void)
{
    stableState = false;
    setLEDState(5);
}

bool CollectPanel::isStable(void)
{
    return stableState;
}

Bll_Average::Bll_Average(QObject *parent) : QObject(parent)
{
}

void Bll_Average::slAverage(const vector<double> &data)
{
    qDebug() << "Bll_Average::slAverage" << QThread::currentThread();

    // BigFloat计算平均值，模拟笔算，精度更高
    BigFloat sum("0");
    size_t size = data.size();

    for (size_t i = 0; i < size; ++i)
    {
        sum += data[i];
    }

    emit sgAverage((sum / (double)size).toString(10));
}
