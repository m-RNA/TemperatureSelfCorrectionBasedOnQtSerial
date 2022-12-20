#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"
#include "collectpanel.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QStringList>
#include "fitchart.h"

#define PGSB_REFRESH_MS 50
#define TIMESTAMP_FACTOR (1000.0f / PGSB_REFRESH_MS)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timerCollect = new QTimer(this);
    timerCollect->setTimerType(Qt::PreciseTimer); // 设置为精准定时器
    timerCollect->setInterval(PGSB_REFRESH_MS);   // 每 PGSB_REFRESH_MS ms检查一次
    connect(timerCollect, &QTimer::timeout, this, &MainWindow::timerCollectTimeOut);

    setDeviceName_Dtm("待定仪器");
    setDeviceName_Std("标准仪器");
    connect(ui->start_Std, &StartCommunication::serialStateChange, ui->collectPanel_Std, &CollectPanel::slSetState);
    connect(ui->start_Dtm, &StartCommunication::serialStateChange, ui->collectPanel_Dtm, &CollectPanel::slSetState);

    // connect(ui->start_Std, &Bll_SerialRecvAnalyse::sgBll_AnalyseFinish, ui->wave_Std, &CustomChart::addYPoint);
    // connect(ui->start_Dtm, &Bll_SerialRecvAnalyse::sgBll_AnalyseFinish, ui->wave_Dtm, &CustomChart::addYPoint);

    //    connect(ui->start_Std, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addVLine);
    //    connect(ui->start_Dtm, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addHLine);

    // -----
    // 1. 创建任务类对象
    taskGen = new Bll_GenerateData(this);
    taskLeastSquare = new Bll_LeastSquareMethod(this);

    samplePointSum = ui->spbxSamplePointSum->value();
    order = ui->spbxOrder->value();

    ui->twAverage->horizontalHeader()->setVisible(true);
    ui->twAverage->verticalHeader()->setVisible(true);
    ui->twAverage->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twAverage->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    ui->twFactor->horizontalHeader()->setVisible(true);
    ui->twFactor->verticalHeader()->setVisible(true);
    ui->twFactor->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twFactor->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    QStringList HorizontalHeader;
    HorizontalHeader << "X:标准平均";
    HorizontalHeader << "Y:待定平均";
    ui->twAverage->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    HorizontalHeader.clear();
    HorizontalHeader << "N-1 阶系数";
    ui->twFactor->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    for (int i = 0; i < samplePointSum; i++) // 需要初始化表格Item
    {
        for (int j = 0; j < 2; j++)
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            ui->twAverage->setItem(i, j, temp);
            ui->twAverage->item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }

    connect(this, &MainWindow::collectDataXYChanged, ui->chartFit, &FitChart::updateCollectPlot);

    // 2. 链接子线程
    connect(this, &MainWindow::startGenerate, taskGen, &Bll_GenerateData::setGenerateFitData);
    connect(taskGen, &Bll_GenerateData::generateFitDataFinish, ui->chartFit, &FitChart::updateFitPlot);

    connect(this, &MainWindow::startLeastSquare, taskLeastSquare, &Bll_LeastSquareMethod::setLeastSquareMethod);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, this, &MainWindow::setFitChartData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDeviceName_Std(QString name)
{
    ui->start_Std->setDeviceName(name);
    ui->collectPanel_Std->setDeviceName(name);
}

void MainWindow::setDeviceName_Dtm(QString name)
{
    ui->start_Dtm->setDeviceName(name);
    ui->collectPanel_Dtm->setDeviceName(name);
}

QString MainWindow::collectTimestampToHhMmSs(int timestamp)
{
    int sec = timestamp / TIMESTAMP_FACTOR;
    return QString::asprintf("%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 3600 % 60);
}

void MainWindow::timerCollectTimeOut()
{
    pgsbSingleValue++; // 进来进度条++
    ui->pgsbSingle->setValue(pgsbSingleValue);
    ui->pgsbSingle->setFormat(collectTimestampToHhMmSs(collectTimestamp - pgsbSingleValue));

    if (collectTimestamp > pgsbSingleValue) // 时间是否到了
        return;                             // 没到退出

    // 时间到了
    timerCollect->stop();
    ui->pgsbSingle->setFormat("00:00:00");

    // emit

    // 计算平均值

    // 打点填表

    // 更新整体进度条
    sampledPointNum = ui->pgsbSum->value() + 1;
    ui->pgsbSum->setValue(sampledPointNum);

    if (ui->pgsbSum->maximum() > sampledPointNum) // 各个标定点是否采集完成
    {
        // 各个标定点采集未完成
        QMessageBox msgBox(QMessageBox::Information, "提示", "此点采集完成\n请准备下一点采集", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);
        if (msgBox.exec() == QMessageBox::AcceptRole)
        {
            // 重置单点进度
            ui->pgsbSingle->setFormat("等待下个采集点中");
            pgsbSingleValue = 0;
            ui->pgsbSingle->setValue(0);
            ui->pgsbSingle->setMaximum(0);
        }
    }
    else
    {
        QMessageBox::information(this, "提示", "全部采集完成！\n请在右下角查看拟合结果");

        // 各个标定点未采集完成
        // 计算拟合结果
        // ...
    }
}

void MainWindow::on_btnCollect_clicked()
{
    if (sampledPointSum > sampledPointNum)
    {
    SAMPLE_UNFINISHED: // 采集未完成
        // 两个串口是否同时打开
        if (!(ui->start_Dtm->state() && ui->start_Std->state()))
        {
            QMessageBox::critical(this, "错误", "请同时连接两个仪器");
            // return;
        }

        // 一段时间内标准仪器波动<0.01

        // 清空实时波形时间轴变为相对时间戳
        // # 这个放在if (sampledPointNum == 0)里面才对(调试)
        if (sampledPointNum == 0)
        {
            collectTimestamp = ui->spbxSampleTime->value() * 60 * TIMESTAMP_FACTOR; // 分钟转换时间戳
            qDebug() << "collectTimestamp" << collectTimestamp;
            ui->pgsbSum->setMaximum(ui->spbxSamplePointSum->value());
            ui->pgsbSum->setValue(0);
        }
        pgsbSingleValue = 0;
        ui->pgsbSingle->setValue(0);
        ui->pgsbSingle->setMaximum(collectTimestamp);
        ui->pgsbSingle->setFormat(collectTimestampToHhMmSs(collectTimestamp));
        timerCollect->start();
    }
    else // 采集完毕
    {
        QMessageBox msgBox(QMessageBox::Warning, "警告", "这将清除这次拟合数据\n是否继续？", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);
        msgBox.addButton("No", QMessageBox::RejectRole);
        if (msgBox.exec() == QMessageBox::AcceptRole)
        {
            qDebug() << "确认";
            sampledPointNum = 0;
            goto SAMPLE_UNFINISHED;
        }
    }
}

/*
    qt中获取容器Vector中的最大值和最小值：
    https://blog.csdn.net/Littlehero_121/article/details/100565527
*/
double max(vector<double> &data)
{
    auto max = std::max_element(std::begin(data), std::end(data));
    return *max;
}

double min(vector<double> &data)
{
    auto min = std::min_element(std::begin(data), std::end(data));
    return *min;
}

void MainWindow::updateCollectDataXY(void)
{
    QString qsX, qsY;
    collectDataX.clear(); // 重置x容器
    collectDataY.clear(); // 重置y容器
    for (int i = 0; i < samplePointSum; i++)
    {
        qsX = ui->twAverage->item(i, 0)->text();
        qsY = ui->twAverage->item(i, 1)->text();
        if (qsX.isEmpty() || qsY.isEmpty())
            continue;
        // qDebug() << "counter" << counter;

        collectDataX.push_back(qsX.toDouble());
        collectDataY.push_back(qsY.toDouble());
        qDebug() << i << ":" << qsX << qsY;
    }
}

// 人为 true 自动 false
void MainWindow::tryUpdateFitChart(bool man)
{
    updateCollectDataXY();
    if (collectDataX.size() < 2)
    {
        if (man) // 是人为的就要提醒一下
            QMessageBox::critical(this, "错误", "正确格式的数据\n小于两组");
        return;
    }

    // N个点可以确定一个 唯一的 N-1 阶的曲线
    order = ui->spbxOrder->text().toInt();
    if (collectDataX.size() <= order)
        order = collectDataX.size() - 1;

    // 启动子线程
    emit startLeastSquare(order, collectDataX, collectDataY);
    QThreadPool::globalInstance()->start(taskLeastSquare);
}

void MainWindow::on_spbxSamplePointSum_valueChanged(int arg1)
{
    ui->twAverage->setRowCount(arg1);
    if (arg1 > samplePointSum)
    {
        for (int j = 0; j < 2; j++) // 需要初始化表格Item
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            int row = arg1 - 1;
            ui->twAverage->setItem(row, j, temp);
            ui->twAverage->item(row, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    samplePointSum = arg1;
    qDebug() << "samplePointSum:" << samplePointSum;
}

void MainWindow::on_spbxOrder_valueChanged(int arg1)
{
    order = arg1;
    ui->twFactor->setRowCount(arg1 + 1);
    qDebug() << "order:" << order;

    tryUpdateFitChart(false);
}

void MainWindow::on_btnFit_clicked()
{
    tryUpdateFitChart(true);
}

void MainWindow::setFitChartData(vector<double> factor)
{
    for (unsigned long long i = 0; i <= order; i++)
    {
        // 通过setItem来改变条目
        QTableWidgetItem *temp = new QTableWidgetItem(QString::number(factor.at(order - i)));
        ui->twFactor->setItem(i, 0, temp);
    }
    collectDataX_Max = max(collectDataX);
    collectDataX_Min = min(collectDataX);
    double addRange = (collectDataX_Max - collectDataX_Min) / 4.0;

    // 启动子线程
    emit startGenerate(collectDataX_Min - addRange, collectDataX_Max + addRange, 0.25, factor); // fitDataX, fitDataY);
    QThreadPool::globalInstance()->start(taskGen);
}

void MainWindow::on_twAverage_itemSelectionChanged()
{
    // 1、记录旧的单元格内容
    old_text = ui->twAverage->item(ui->twAverage->currentRow(),
                                   ui->twAverage->currentColumn())
                   ->text();
}

/*
 * 正则表达式：
 * https://blog.csdn.net/qq_41622002/article/details/107488528
 */
void MainWindow::on_twAverage_itemChanged(QTableWidgetItem *item)
{
    // 2、匹配正负整数、正负浮点数
    QString Pattern("(-?[1-9][0-9]+)|(-?[0-9])|(-?[1-9]\\d+\\.\\d+)|(-?[0-9]\\.\\d+)"); // 正则表达式
    QRegExp reg(Pattern);

    // 3.获取修改的新的单元格内容
    QString str = item->text();

    // 完全匹配
    if (reg.exactMatch(str))
    {
        qDebug() << "匹配成功";
        int row = item->row();
        // old_text = ""; // 替换为空
        if (ui->twAverage->item(row, 0)->text().isEmpty() ||
            ui->twAverage->item(row, 1)->text().isEmpty())
            return; // 当有一格为空时，退出

        updateCollectDataXY();
        if (collectDataX.size() > 0)
        {
            QVector<double> qv_X = QVector<double>(collectDataX.begin(), collectDataX.end());
            QVector<double> qv_Y = QVector<double>(collectDataY.begin(), collectDataY.end());

            emit collectDataXYChanged(qv_X, qv_Y);
        }
    }
    else
    {
        qDebug() << "匹配失败";
        item->setText(old_text); // 更换之前的内容
    }
}
