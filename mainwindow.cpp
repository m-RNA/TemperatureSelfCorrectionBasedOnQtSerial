#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"
#include "collectpanel.h"
#include "interactchart.h"
#include "fitchart.h"
#include "about.h"
#include "wizard.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QStringList>
using LeastSquare = MainWindow;

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
    ui->start_Std->setAnalyseMode(2);
    connect(ui->start_Std, &StartCommunication::serialStateChange, ui->collectPanel_Std, &CollectPanel::slSetState);
    connect(ui->start_Dtm, &StartCommunication::serialStateChange, ui->collectPanel_Dtm, &CollectPanel::slSetState);
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Std, &CollectPanel::slCollectData);
    connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Dtm, &CollectPanel::slCollectData);

    //    connect(ui->start_Std, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addVLine);
    //    connect(ui->start_Dtm, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addHLine);

    /*** LeastSquare Begin ***/

    // 1. 创建任务类对象
    taskGen = new Bll_GenerateData(this);
    taskLeastSquare = new Bll_LeastSquareMethod(this);

    // 这里是根据表格的行数来设置采集点数的
    samplePointSum = ui->twAverage->rowCount();
    ui->spbxSamplePointSum->setValue(samplePointSum);
    order = ui->spbxOrder->text().toInt();

    ui->twAverage->horizontalHeader()->setVisible(true);
    ui->twAverage->verticalHeader()->setVisible(true);
    ui->twAverage->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twAverage->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    ui->twFactor->horizontalHeader()->setVisible(true);
    ui->twFactor->verticalHeader()->setVisible(true);
    ui->twFactor->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twFactor->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    QStringList HorizontalHeader;
    // 插入循序即为表头显示顺序
    HorizontalHeader << "Y:标准平均";
    HorizontalHeader << "X:待定平均";
    ui->twAverage->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    HorizontalHeader.clear();
    HorizontalHeader << "N-1 阶系数";
    ui->twFactor->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    // 需要初始化表格Item
    for (int i = 0; i < samplePointSum; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            ui->twAverage->setItem(i, j, temp);
            ui->twAverage->item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    connect(ui->twAverage, &QTableWidget::itemChanged, this, &LeastSquare::twAverage_itemChanged);

    connect(this, &LeastSquare::collectDataXYChanged, ui->chartFit, &FitChart::updateCollectPlot);

    // 2. 链接子线程
    connect(this, &LeastSquare::startGenerate, taskGen, &Bll_GenerateData::setGenerateFitData);
    connect(taskGen, &Bll_GenerateData::generateFitDataFinish, ui->chartFit, &FitChart::updateFitPlot);

    connect(this, &LeastSquare::startLeastSquare, taskLeastSquare, &Bll_LeastSquareMethod::setLeastSquareMethod);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, this, &LeastSquare::setFitChartData);

    /*** LeastSquare End ***/

    // 单点进度条
    int sec = ui->spbxSampleTime->value() * 60;
    ui->pgsbSingle->setFormat(QString::asprintf("%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 3600 % 60));
    // 整体进度条
    ui->pgsbSum->setMaximum(samplePointSum);

    connect(ui->collectPanel_Std, &CollectPanel::sgCollectDataAverage, this, &MainWindow::setAverageTableItem_Std);
    connect(ui->collectPanel_Dtm, &CollectPanel::sgCollectDataAverage, this, &MainWindow::setAverageTableItem_Dtm);

    // 互连主轴和底轴矩形的x轴范围：
    connect(ui->collectPanel_Std->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
            ui->collectPanel_Dtm->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));
    connect(ui->collectPanel_Dtm->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
            ui->collectPanel_Std->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));

    /* Xlsx 文件记录保存 */
    taskXlsxData = new Bll_SaveDataToXlsx(this);
    connect(ui->collectPanel_Std, &CollectPanel::sgCollectDataGet, taskXlsxData, &Bll_SaveDataToXlsx::saveData_Std);
    connect(ui->collectPanel_Dtm, &CollectPanel::sgCollectDataGet, taskXlsxData, &Bll_SaveDataToXlsx::saveData_Dtm);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, taskXlsxData, &Bll_SaveDataToXlsx::saveFactor);

    /* 播放提示音 */
    if (ui->cbSound->currentIndex() > 0)
    {
        taskSound = new Bll_Sound(this);
        taskSound->setIndex((SoundIndex)ui->cbSound->currentIndex());
    }
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
    ui->collectPanel_Std->collectFinish();
    ui->collectPanel_Dtm->collectFinish();
    ui->collectPanel_Std->slSetState(ui->start_Std->state());
    ui->collectPanel_Dtm->slSetState(ui->start_Dtm->state());

    // 更新整体进度条
    taskXlsxData->nextPoint();
    sampledPointNum = ui->pgsbSum->value() + 1;
    ui->pgsbSum->setValue(sampledPointNum);

    if (ui->pgsbSum->maximum() > sampledPointNum) // 各个标定点是否采集完成
    {
        // 各个标定点采集未完成
        ui->btnCollect->setText("采集下点");
        ui->btnCollectStop->setEnabled(false);
        if (taskSound)
            taskSound->play1();

        QMessageBox msgBox(QMessageBox::Information, "提示", "此点采集完成\n请准备下一点采集", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);
        if (msgBox.exec() == QMessageBox::AcceptRole)
        {
            if (taskSound)
                taskSound->stop();

            // 重置单点进度
            // ui->pgsbSingle->setFormat("等待下个采集点中");
            pgsbSingleValue = 0;
            ui->pgsbSingle->setValue(0);
            ui->pgsbSingle->setFormat(collectTimestampToHhMmSs(collectTimestamp));
            // ui->pgsbSingle->setMaximum(0);
        }
    }
    else
    {
        // 各个标定点未采集完成

        // 计算拟合结果
        // ...
        tryUpdateFitChart(false);
        ui->btnCollectStop->setEnabled(false);
        if (taskSound)
            taskSound->play2();

        QMessageBox msgBox(QMessageBox::Information, "提示", "全部采集完成！\n请在右下角查看拟合结果", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);
        msgBox.exec();
        if (taskSound)
            taskSound->stop();

        // 保存报告
        taskXlsxData->saveReport();
        ui->btnCollect->setText("全部重采");
    }
}

void MainWindow::on_btnCollect_clicked()
{
    if (samplePointSum > sampledPointNum)
    {
        // SAMPLE_UNFINISHED: // 采集未完成
        // 两个串口是否同时打开
        if (!(ui->start_Dtm->state() && ui->start_Std->state()))
        {
            QMessageBox::critical(this, "错误", "请同时连接两个仪器");
            return;
        }
        ui->spbxSamplePointSum->setEnabled(false);
        ui->spbxSampleTime->setEnabled(false);
        ui->collectPanel_Std->slSetState(2);
        // 一段时间内标准仪器波动<0.01
        // if (ui->collectPanel_Std->isStable() == false)
        // {
        //     QMessageBox::critical(this, "错误", "标准仪器波动过大，请重新采集");
        //     return;
        // }

        ui->btnCollect->setText("重采该点");
        ui->btnCollectStop->setEnabled(true);

        // # 这个放在if (sampledPointNum == 0)里面才对(调试)
        if (sampledPointNum == 0) // 第一次采集
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
        taskXlsxData->startPoint();
        ui->collectPanel_Dtm->slSetState(2);
        ui->collectPanel_Std->collectStart();
        ui->collectPanel_Dtm->collectStart();
    }
    else // 所有点采集完毕
    {
        QMessageBox msgBox(QMessageBox::Warning, "警告", "这将清除全部数据\n是否继续？", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);
        msgBox.addButton("No", QMessageBox::RejectRole);
        if (msgBox.exec() == QMessageBox::AcceptRole)
        {
            // qDebug() << "确认";
            ui->spbxSamplePointSum->setEnabled(true);
            ui->spbxSampleTime->setEnabled(true);
            sampledPointNum = 0;
            ui->pgsbSum->setValue(0);
            ui->pgsbSingle->setValue(0);
            ui->pgsbSingle->setFormat(collectTimestampToHhMmSs(collectTimestamp));

            // 需要清空表格数据
            ui->twAverage->clearContents();
            ui->twFactor->clearContents();

            // 清空拟合图形
            ui->chartFit->clear();

            // 重置表格
            taskXlsxData->resetIndex();

            ui->btnCollect->setText("开始采集");
            // goto SAMPLE_UNFINISHED;
        }
    }
}

// 停止采集
void MainWindow::on_btnCollectStop_clicked()
{
    ui->btnCollectStop->setEnabled(false);
    if (timerCollect->isActive())
        timerCollect->stop();

    // 重置单点进度
    pgsbSingleValue = 0;
    ui->pgsbSingle->setValue(0);
    ui->pgsbSingle->setFormat(collectTimestampToHhMmSs(collectTimestamp));

    ui->collectPanel_Std->collectStop();
    ui->collectPanel_Dtm->collectStop();
    ui->collectPanel_Std->slSetState(ui->start_Std->state());
    ui->collectPanel_Dtm->slSetState(ui->start_Dtm->state());
}

// 更新单点进度条显示时间
void MainWindow::on_spbxSampleTime_valueChanged(double arg1)
{
    int sec = arg1 * 60;
    ui->pgsbSingle->setFormat(QString::asprintf("%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 3600 % 60));
}

void LeastSquare::on_spbxSamplePointSum_valueChanged(int arg1)
{
    // 设置整体进度条最大值
    ui->pgsbSum->setMaximum(arg1);

    // 设置平均值表格行数
    ui->twAverage->setRowCount(arg1);
    if (arg1 > samplePointSum)
    {
        // 需要初始化表格Item
        for (int j = 0; j < 2; j++)
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            int row = arg1 - 1;
            ui->twAverage->setItem(row, j, temp);
            ui->twAverage->item(row, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }

    // 设置采集点数
    samplePointSum = arg1;
    qDebug() << "samplePointSum:" << samplePointSum;
}

/*
    qt中获取容器Vector中的最大值和最小值：
    https://blog.csdn.net/Littlehero_121/article/details/100565527
*/
DECIMAL_TYPE max(vector<DECIMAL_TYPE> &data)
{
    auto max = std::max_element(std::begin(data), std::end(data));
    return *max;
}

DECIMAL_TYPE min(vector<DECIMAL_TYPE> &data)
{
    auto min = std::min_element(std::begin(data), std::end(data));
    return *min;
}

DECIMAL_TYPE atoDec(const char *str)
{
    bool negativeFlag = false;
    if (str[0] == '-')
    {
        negativeFlag = true;
        str++;
    }
    else if (str[0] == '+')
    {
        str++;
    }

    unsigned long long allNum = 0;
    unsigned long long count = 1;
    bool dotFlag = false;
    while (*str)
    {
        if (*str == '.')
        {
            dotFlag = true;
            str++;
            continue;
        }
        if (dotFlag == true)
            count *= 10;

        allNum = allNum * 10 + (*str - '0');
        str++;
    }
    return negativeFlag ? -((DECIMAL_TYPE)allNum / count) : ((DECIMAL_TYPE)allNum / count);
}

void LeastSquare::updateCollectDataXY(void)
{
    DECIMAL_TYPE temp;
    collectDataX.clear(); // 重置x容器
    collectDataY.clear(); // 重置y容器
    for (int i = 0; i < samplePointSum; i++)
    {
        QString qsX, qsY;
        qsY = ui->twAverage->item(i, 0)->text();
        qsX = ui->twAverage->item(i, 1)->text();
        if (qsX.isEmpty() || qsY.isEmpty())
            continue;
        // qDebug() << "counter" << counter;

        temp = atoDec(qsX.toStdString().c_str());
        collectDataX.push_back(temp);
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "LeastSquare::updateCollectDataXY   atoDec x = %.20LE\n", temp);
        qDebug() << globalStringBuffer;
        // printf("LeastSquare::updateCollectDataXY toDouble x = %.20e\n", qsX.toDouble());

        temp = atoDec(qsY.toStdString().c_str());
        collectDataY.push_back(temp);
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "LeastSquare::updateCollectDataXY   atoDec y = %.20LE\n", temp);
        qDebug() << globalStringBuffer;
        // printf("LeastSquare::updateCollectDataXY toDouble y = %.20e\n", qsY.toDouble());

        qDebug() << i << ":" << qsX << qsY;
    }
}

// 人为 true 自动 false
void LeastSquare::tryUpdateFitChart(bool man)
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

void LeastSquare::on_spbxOrder_valueChanged(int arg1)
{
    order = arg1;
    ui->twFactor->setRowCount(arg1 + 1);
    qDebug() << "order:" << order;

    // tryUpdateFitChart(false);
}

void LeastSquare::on_btnFit_clicked()
{
    tryUpdateFitChart(true);
}

void LeastSquare::setFitChartData(const vector<DECIMAL_TYPE> &factor)
{
    for (unsigned long long i = 0; i <= order; i++)
    {
        // 通过setItem来改变条目
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "%.8LE", factor.at(order - i));
        // printf("LeastSquare::setFitChartData %s\r\n", buffer);
        QTableWidgetItem *temp = new QTableWidgetItem(globalStringBuffer); // QString::fromStdString(globalStringBuffer));
        ui->twFactor->setItem(i, 0, temp);
        ui->twFactor->item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    collectDataX_Max = max(collectDataX);
    collectDataX_Min = min(collectDataX);
    DECIMAL_TYPE addRange = (collectDataX_Max - collectDataX_Min) / 1.0f;

    // 启动子线程 生成曲线数据
    emit startGenerate(collectDataX_Min - addRange, collectDataX_Max + addRange,
                       (collectDataX_Max - collectDataX_Min) / (DECIMAL_TYPE)samplePointSum / 5000.0f,
                       factor); // fitDataX, fitDataY);
    QThreadPool::globalInstance()->start(taskGen);
}

void LeastSquare::on_twAverage_itemSelectionChanged()
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
void LeastSquare::twAverage_itemChanged(QTableWidgetItem *item)
{
    // 匹配正负整数、正负浮点数
    const QString Pattern("(-?[1-9][0-9]+)|(-?[0-9])|(-?[1-9]\\d+\\.\\d+)|(-?[0-9]\\.\\d+)"); // 正则表达式
    QRegExp reg(Pattern);

    // 获取修改的新的单元格内容
    QString str = item->text();
    if (str == "")
    {
        qDebug() << "空字符";
        if (old_text == "")
            return;

        goto GO_ON;
    }

    // 去除字符串结尾的空格、回车、换行
    while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '\n' || str[str.length() - 1] == '\r')
    {
        str = str.left(str.length() - 1);
        qDebug() << "去除空格";
        if (str == "")
            break;
    }

    // 完全匹配
    if (reg.exactMatch(str))
    {
        qDebug() << "正则匹配成功";
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // qDebug() << ui->twAverage->currentRow(); // 没选择单元格 值为-1
        // qDebug() << ui->twAverage->currentColumn(); // 没选择单元格 值为-1

        // 下面这一条代码导致程序奔溃：没选中单元格时，开始采集完成后就奔溃。
        // ui->twAverage->item(ui->twAverage->currentRow(), ui->twAverage->currentColumn())->setText(str);
        item->setText(str);

    GO_ON:
        updateCollectDataXY();

        // std::vector 转换为 QVector
        QVector<double> qv_X = QVector<double>(collectDataX.begin(), collectDataX.end());
        QVector<double> qv_Y = QVector<double>(collectDataY.begin(), collectDataY.end());

        emit collectDataXYChanged(qv_X, qv_Y);
    }
    else
    {
        qDebug() << "正则匹配失败";
        item->setText(old_text); // 更换之前的内容
    }
}

void MainWindow::on_actionAbout_triggered()
{
    About about(this);
    about.exec();
}

void MainWindow::setAverageTableItem_Std(const DECIMAL_TYPE &data)
{
    QTableWidgetItem *data_Std = new QTableWidgetItem(QString::fromStdString(std::to_string(data)));
    ui->twAverage->setItem(sampledPointNum, 0, data_Std);
}

void MainWindow::setAverageTableItem_Dtm(const DECIMAL_TYPE &data)
{
    QTableWidgetItem *data_Dtm = new QTableWidgetItem(QString::fromStdString(std::to_string(data)));
    ui->twAverage->setItem(sampledPointNum, 1, data_Dtm);
}

void MainWindow::on_cbSound_currentIndexChanged(int index)
{
    if (taskSound)
    {
        taskSound->stop();
        taskSound->deleteLater();
    }

    if (index > 0)
    {
        taskSound = new Bll_Sound(this);
        taskSound->setIndex((SoundIndex)index);
    }
    else
        taskSound = nullptr;
}

void MainWindow::on_btnSaveReport_clicked()
{
    taskXlsxData->saveReport();
}

void MainWindow::on_actionWizard_triggered()
{
    Wizard wizard(this);
    connect(&wizard, &Wizard::wizardInfoFinish, this, [&](const WizardInfo &info)
            {
                qDebug() << "向导结束";
                taskXlsxData->saveInfo(info.baseInfo);
                ui->start_Std->setSerialSettingIndex(info.ssIndex_Std);
                ui->start_Dtm->setSerialSettingIndex(info.ssIndex_Dtm); });
    wizard.exec();
}
