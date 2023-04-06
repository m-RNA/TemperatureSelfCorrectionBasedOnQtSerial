#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"
#include "collectpanel.h"
#include "interactchart.h"
#include "fitchart.h"
#include "about.h"
#include "wizard.h"
#include "BigFloat.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QStringList>
using LeastSquare = MainWindow;
using Bll_CollectBtn = MainWindow;

#define PGSB_REFRESH_MS 500
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
    // ui->start_Std->setAnalyseMode(2);
    ui->start_Std->setCbxSerialIndex(1);
    ui->start_Dtm->setCbxSerialIndex(3);

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

    /* 采集仪表盘 */
    ui->collectPanel_Dtm->setYAxisFormat("f", 0);

    // 互连主轴和底轴矩形的x轴范围：
    connect(ui->collectPanel_Std->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
            ui->collectPanel_Dtm->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));
    connect(ui->collectPanel_Dtm->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
            ui->collectPanel_Std->getXAxis(), static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::setRange));

    // 同步指示灯的变化
    connect(ui->start_Std, &StartCommunication::serialStateChange, ui->collectPanel_Std, &CollectPanel::setOnlineState);
    connect(ui->start_Dtm, &StartCommunication::serialStateChange, ui->collectPanel_Dtm, &CollectPanel::setOnlineState);
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Std, &CollectPanel::slCollectData);
    connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Dtm, &CollectPanel::slCollectData);

    // 监听数据波动
    connect(ui->start_Std, &StartCommunication::serialStateChange, [&](bool state)
            {
                if(state)
                    listenDataWaveInit();
                else
                    listenDataWaveQuit(); });

    /* Xlsx 文件记录保存 */
    taskXlsxData = new Bll_SaveDataToXlsx;
    threadXlsx = new QThread();
    taskXlsxData->moveToThread(threadXlsx);
    connect(threadXlsx, &QThread::finished, taskXlsxData, &QObject::deleteLater); // 防止内存泄漏
    connect(this, &MainWindow::sgXlsxStartPoint, taskXlsxData, &Bll_SaveDataToXlsx::startPoint);
    connect(this, &MainWindow::sgXlsxNextPoint, taskXlsxData, &Bll_SaveDataToXlsx::nextPoint);
    connect(this, &MainWindow::sgXlsxSaveReport, taskXlsxData, &Bll_SaveDataToXlsx::saveReport);
    connect(this, &MainWindow::sgXlsxSetAutoSave, taskXlsxData, &Bll_SaveDataToXlsx::setAutoSave);
    connect(this, &MainWindow::sgXlsxSaveInfo, taskXlsxData, &Bll_SaveDataToXlsx::saveInfo);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, taskXlsxData, &Bll_SaveDataToXlsx::saveFactor);
    threadXlsx->start();
    emit sgXlsxSetAutoSave(ui->actionAutoSave->isChecked());

    pictureInit();
}

MainWindow::~MainWindow()
{
    // 释放线程
    threadXlsx->quit();
    threadXlsx->wait();

    if (threadSound)
    {
        emit sgSoundStop();
        threadSound->quit();
        threadSound->wait();
    }

    listenDataWaveQuit();
    collectDataQuit();
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

void MainWindow::pictureInit()
{
    QPixmap pix("://picture/NUC_day_heart.jpg");
    // QPixmap temp(pix.size());
    // temp.fill(Qt::transparent);

    // QPainter p1(&temp);
    // p1.setCompositionMode(QPainter::CompositionMode_Source);
    // p1.drawPixmap(0, 0, pix);
    // p1.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    // p1.fillRect(temp.rect(), QColor(0, 0, 0, 255)); // 这里设置透明度
    // p1.end();
    // pix = temp;

    // ui->lbPicture->setScaledContents(true);
    // ui->lbPicture->setStyleSheet("background-color:white");
    // ui->lbPicture->setAlignment(Qt::AlignCenter);
    // pix.scaled(ui->lbPicture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lbPicture->setPixmap(pix);

    // QPalette background = ui->lbPicture->palette();
    // QImage img("://picture/NUC_day.jpg");
    // QImage fitImg = img.scaled(ui->lbPicture->width(), ui->lbPicture->height(), Qt::IgnoreAspectRatio);
    // background.setBrush(QPalette::Window, QBrush(fitImg));
    // ui->lbPicture->setPalette(background);
}

void MainWindow::listenDataWaveInit()
{
    listenDataWaveQuit();
    taskDataWave = new Bll_DataWave;
    taskDataWave->setInterval(3000);
    taskDataWave->setRange(ui->spbxWaveRange->value());
    taskDataWave->setCheckNum(ui->spbxWaveNum->value());

    threadDataWave = new QThread;
    taskDataWave->moveToThread(threadDataWave);
    connect(threadDataWave, &QThread::finished, taskDataWave, &QObject::deleteLater);
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, taskDataWave, &Bll_DataWave::addData);
    connect(taskDataWave, &Bll_DataWave::sgReceiveTimeout, ui->collectPanel_Std, &CollectPanel::setReceiveTimeout);
    connect(taskDataWave, &Bll_DataWave::sgStableState, ui->collectPanel_Std, &CollectPanel ::setStableState);
    connect(this, &MainWindow::sgSetDataWaveRange, taskDataWave, &Bll_DataWave::setRange);
    connect(this, &MainWindow::sgSetDataWaveInterval, taskDataWave, &Bll_DataWave::setInterval);
    connect(this, &MainWindow::sgSetDataWaveNum, taskDataWave, &Bll_DataWave::setCheckNum);

    // 标准仪器稳定后，开始采集数据
    connect(taskDataWave, &Bll_DataWave::sgTurnToStable, this, &MainWindow::goOnCollect);
    threadDataWave->start();

    qDebug() << "threadDataWave start";
}

void MainWindow::listenDataWaveQuit()
{
    if (threadDataWave)
    {
        threadDataWave->quit();
        threadDataWave->wait();
        threadDataWave = nullptr;
        qDebug() << "threadDataWave quit";
    }
}

void MainWindow::collectDataInit()
{
    taskDataCollect_Std = new Bll_DataCollect;
    taskDataCollect_Dtm = new Bll_DataCollect;
    threadDataCollect = new QThread;
    taskDataCollect_Std->moveToThread(threadDataCollect);
    taskDataCollect_Dtm->moveToThread(threadDataCollect);
    connect(threadDataCollect, &QThread::finished, [&]()
            {taskDataCollect_Std->deleteLater(); taskDataCollect_Dtm->deleteLater(); });

    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, taskDataCollect_Std, &Bll_DataCollect::slAddData);
    connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, taskDataCollect_Dtm, &Bll_DataCollect::slAddData);
    connect(taskDataCollect_Std, &Bll_DataCollect::sgRange, ui->collectPanel_Std, &CollectPanel::slSetRange);
    connect(taskDataCollect_Dtm, &Bll_DataCollect::sgRange, ui->collectPanel_Dtm, &CollectPanel::slSetRange);
    connect(this, &MainWindow::sgCollectDataFinish_Std, taskDataCollect_Std, &Bll_DataCollect::slCollectFinish);
    connect(this, &MainWindow::sgCollectDataFinish_Dtm, taskDataCollect_Dtm, &Bll_DataCollect::slCollectFinish);
    connect(taskDataCollect_Std, &Bll_DataCollect::sgAverage, this, &MainWindow::setAverageTableItem_Std);
    connect(taskDataCollect_Dtm, &Bll_DataCollect::sgAverage, this, &MainWindow::setAverageTableItem_Dtm);

    connect(taskDataCollect_Std, &Bll_DataCollect::sgCollectData, taskXlsxData, &Bll_SaveDataToXlsx::saveData_Std);
    connect(taskDataCollect_Dtm, &Bll_DataCollect::sgCollectData, taskXlsxData, &Bll_SaveDataToXlsx::saveData_Dtm);

    threadDataCollect->start();
    qDebug() << "threadDataCollect start";
}

void MainWindow::collectDataQuit()
{
    if (threadDataCollect)
    {
        threadDataCollect->quit();
        threadDataCollect->wait();
        threadDataCollect = nullptr;
        qDebug() << "threadDataCollect quit";
    }
}

void MainWindow::soundInit()
{
    taskSound = new Bll_Sound;
    threadSound = new QThread;
    taskSound->moveToThread(threadSound);
    connect(threadSound, &QThread::finished, taskSound, &QObject::deleteLater); // 防止内存泄漏
    connect(this, &MainWindow::sgSoundPlay1, taskSound, &Bll_Sound::play1);
    connect(this, &MainWindow::sgSoundPlay2, taskSound, &Bll_Sound::play2);
    connect(this, &MainWindow::sgSoundStop, taskSound, &Bll_Sound::stop);
    threadSound->start();
}

QString MainWindow::collectTimeStampToHhMmSs(int timestamp)
{
    int sec = timestamp / TIMESTAMP_FACTOR;
    return QString::asprintf("%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 3600 % 60);
}

void Bll_CollectBtn::setCollectBtnState(const CollectBtnState &state)
{
    btnSwitchState = state;
    switch (state)
    {
    case CollectBtnState_Start:
        ui->btnCollectSwitch->setText("开始采集");
        ui->btnCollectSwitch->setIcon(QIcon("://icon/collect.ico"));
        break;
    case CollectBtnState_Wait:
        ui->btnCollectSwitch->setText("取消候稳");
        ui->btnCollectSwitch->setIcon(QIcon("://icon/collectwait.ico"));
        break;
    case CollectBtnState_Stop:
        ui->btnCollectSwitch->setText("停止采集");
        ui->btnCollectSwitch->setIcon(QIcon("://icon/collectstop.ico"));
        break;
    case CollectBtnState_Next:
        ui->btnCollectSwitch->setText("采集下点");
        ui->btnCollectSwitch->setIcon(QIcon("://icon/collect.ico"));
        break;
    case CollectBtnState_End:
        ui->btnCollectSwitch->setText("完成采集");
        ui->btnCollectSwitch->setIcon(QIcon("://icon/yes.ico"));
        break;
    }
}

// 初始化单点进度条
void MainWindow::pgsbSingleInit()
{
    pgsbSingleReset();
    ui->pgsbSingle->setMaximum(collectTimeStamp);
}

// 重置单点进度条
void MainWindow::pgsbSingleReset()
{
    pgsbSingleValue = 0;
    ui->pgsbSingle->setValue(0);
    ui->pgsbSingle->setFormat(collectTimeStampToHhMmSs(collectTimeStamp));
}

void Bll_CollectBtn::on_btnCollectSwitch_clicked()
{
    qDebug() << "切换采集状态" << isCollecting;
    if (isCollecting == false) // 开始采集
    {
        // 取消等待标准仪器稳定
        if (btnSwitchState == CollectBtnState_Wait)
        {
            waitingStdStable = false;
            pgsbSingleInit();
            setCollectBtnState(CollectBtnState_Start);
            goto JUDGE_FIRST_COLLECT;
        }

        // 两个串口是否同时打开
        if (!(ui->start_Dtm->state() && ui->start_Std->state()))
        {
            QMessageBox::critical(this, "错误", "请同时连接两个仪器");
            return;
        }
        ui->spbxSamplePointSum->setEnabled(false);
        ui->spbxSampleTime->setEnabled(false);
        ui->spbxWaveNum->setEnabled(false);
        ui->spbxWaveRange->setEnabled(false);

        // 最终的完成采集
        if (btnSwitchState == CollectBtnState_End)
        {
            ui->btnCollectSwitch->setEnabled(false);
            ui->btnCollectRestart->setEnabled(false);

            // 保存报告
            // taskXlsxData->saveReport();
            emit sgXlsxSaveReport();

            return;
        }

        if (btnSwitchState == CollectBtnState_Next)
            nextCollect();

        if (collectCounter == 0) // 第一次采集
        {
            collectTimeStamp = ui->spbxSampleTime->value() * 60 * TIMESTAMP_FACTOR; // 分钟转换时间戳
            qDebug() << "collectTimeStamp" << collectTimeStamp;
            ui->pgsbSum->setMaximum(ui->spbxSamplePointSum->value());
            ui->pgsbSum->setValue(0);
        }

        // 检查数据波动是否超过阈值
        waitingStdStable = true;
        if (ui->collectPanel_Std->isStable() == false)
        {
            ui->pgsbSingle->setMaximum(0);
            if (btnSwitchState == CollectBtnState_Next)
                ui->btnCollectRestart->setEnabled(false);
            setCollectBtnState(CollectBtnState_Wait);
            return;
        }
        goOnCollect();
    }
    else // 停止采集
    {
        timerCollect->stop();
        isCollecting = false;

        // 重置单点进度条
        pgsbSingleReset();

        stopCollect();

        setCollectBtnState(CollectBtnState_Start);
        ui->btnCollectRestart->setEnabled(false);

        // 如果一点都没采集，就可以重新设置采集参数
    JUDGE_FIRST_COLLECT:
        if (collectCounter == 0)
        {
            ui->spbxSamplePointSum->setEnabled(true);
            ui->spbxSampleTime->setEnabled(true);
            ui->spbxWaveNum->setEnabled(true);
            ui->spbxWaveRange->setEnabled(true);
        }
    }
}

void Bll_CollectBtn::on_btnCollectRestart_clicked()
{
    // 两个串口是否同时打开
    if (!(ui->start_Dtm->state() && ui->start_Std->state()))
    {
        QMessageBox::critical(this, "错误", "请同时连接两个仪器");
        return;
    }
    isCollecting = true;
    ui->pgsbSum->setValue(collectCounter);

    // 这里是重新采集本点
    if (btnSwitchState == CollectBtnState_Next)
    {
        // 检查数据波动是否超过阈值
        if (ui->collectPanel_Std->isStable() == false)
        {
            waitingStdStable = true;
            ui->pgsbSingle->setMaximum(0);
            ui->btnCollectRestart->setEnabled(false);
            setCollectBtnState(CollectBtnState_Wait);
            return;
        }
    }

    setCollectBtnState(CollectBtnState_Stop);
    pgsbSingleReset();
    resetCollect();

    timerCollect->start(); // 开始倒计时
}

void Bll_CollectBtn::timerCollectTimeOut()
{
    pgsbSingleValue++; // 进来进度条++
    ui->pgsbSingle->setValue(pgsbSingleValue);
    ui->pgsbSingle->setFormat(collectTimeStampToHhMmSs(collectTimeStamp - pgsbSingleValue));

    if (collectTimeStamp > pgsbSingleValue) // 时间是否到了
        return;                             // 没到退出

    timerCollect->stop();
    isCollecting = false;
    finishCollect();

    ui->btnCollectRestart->setEnabled(true);
    if (collectCounter < samplePointSum - 1)
    {
        ui->pgsbSum->setValue(collectCounter + 1);
        setCollectBtnState(CollectBtnState_Next);

        QString msg = "此点采集完成\n" + (QString) "标准仪器极差：" + QString::number(ui->collectPanel_Std->getRange()) + "\n" + "待测仪器极差：" + QString::number(ui->collectPanel_Dtm->getRange()) + "\n" + "请准备下一点采集";
        QMessageBox msgBox(QMessageBox::Information, "提示", msg, 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);

        if (ui->cbSound->currentIndex() > 0)
        {
            soundInit();
            emit sgSoundPlay1((SoundIndex)ui->cbSound->currentIndex());
        }

        msgBox.exec();

        if (ui->cbSound->currentIndex() > 0)
        {
            emit sgSoundStop();
            threadSound->quit();
            threadSound->wait(); // 这里需要吗？
            threadSound = nullptr;
        }

        // 重置单点进度
        pgsbSingleReset();
    }
    else // 全部采集完成
    {
        ui->pgsbSum->setValue(collectCounter + 1);
        setCollectBtnState(CollectBtnState_End);

        tryUpdateFitChart(false);

        QMessageBox msgBox(QMessageBox::Information, "提示", "全部采集完成！\n请在右下角查看拟合结果", 0, this);
        msgBox.addButton("Yes", QMessageBox::AcceptRole);

        if (ui->cbSound->currentIndex() > 0)
        {
            soundInit();
            emit sgSoundPlay2((SoundIndex)ui->cbSound->currentIndex());
        }

        msgBox.exec();

        if (ui->cbSound->currentIndex() > 0)
        {
            emit sgSoundStop();
            threadSound->quit();
            threadSound->wait(); // 这里需要吗？
            threadSound = nullptr;
        }

        qDebug() << "全部采集完成啦~";
    }
}

// 接着开始采集
void Bll_CollectBtn::goOnCollect()
{
    if (waitingStdStable)
    {
        waitingStdStable = false;
        pgsbSingleInit();
        setCollectBtnState(CollectBtnState_Stop);
        ui->btnCollectRestart->setEnabled(true);

        isCollecting = true;
        timerCollect->start();
        startCollect();
    }
}

void Bll_CollectBtn::startCollect()
{
    listenDataWaveQuit();

    emit sgXlsxStartPoint();

    collectDataInit();
    ui->collectPanel_Std->collectStart();
    ui->collectPanel_Dtm->collectStart();

    qDebug() << "开始采集" << collectCounter;
}

void Bll_CollectBtn::stopCollect()
{
    collectDataQuit();
    ui->collectPanel_Std->collectStop();
    ui->collectPanel_Dtm->collectStop();

    listenDataWaveInit();

    ui->collectPanel_Std->setOnlineState(ui->start_Std->state());
    ui->collectPanel_Dtm->setOnlineState(ui->start_Dtm->state());

    qDebug() << "停止采集";
}

void Bll_CollectBtn::finishCollect()
{
    emit sgCollectDataFinish_Std();
    emit sgCollectDataFinish_Dtm();
    ui->collectPanel_Std->collectFinish();
    ui->collectPanel_Dtm->collectFinish();
    collectDataQuit();

    listenDataWaveInit();

    ui->collectPanel_Std->setOnlineState(ui->start_Std->state());
    ui->collectPanel_Dtm->setOnlineState(ui->start_Dtm->state());

    qDebug() << "本点采集结束";
}

void Bll_CollectBtn::resetCollect()
{
    listenDataWaveQuit();
    collectDataQuit();
    collectDataInit();

    emit sgXlsxStartPoint();

    ui->collectPanel_Std->collectRestart();
    ui->collectPanel_Dtm->collectRestart();

    qDebug() << "重采本点" << collectCounter;
}

void Bll_CollectBtn::nextCollect()
{
    collectCounter++;
    emit sgXlsxNextPoint();

    qDebug() << "采集下点" << collectCounter;
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

        temp = BigFloat::toLongDouble(qsX.toStdString());
        qDebug() << "\nLeastSquare::updateCollectDataXY ";
        qDebug() << "BigFloat x =" << BigFloat(qsX.toStdString());
        collectDataX.push_back(temp);
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "toLoDouble = %.20LE", temp);
        qDebug() << globalStringBuffer;

        temp = BigFloat::toLongDouble(qsY.toStdString());
        qDebug() << "BigFloat y =" << BigFloat(qsY.toStdString());
        collectDataY.push_back(temp);
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "toLoDouble = %.20LE", temp);
        qDebug() << globalStringBuffer;

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

void MainWindow::setAverageTableItem_Std(const string &data)
{
    QTableWidgetItem *data_Std = new QTableWidgetItem(QString::fromStdString(data));
    ui->twAverage->setItem(collectCounter, 0, data_Std);
}

void MainWindow::setAverageTableItem_Dtm(const string &data)
{
    QTableWidgetItem *data_Dtm = new QTableWidgetItem(QString::fromStdString(data));
    ui->twAverage->setItem(collectCounter, 1, data_Dtm);
}

void MainWindow::on_cbSound_currentIndexChanged(int index)
{
    if (index <= 0)
    {
        if (threadSound)
        {
            emit sgSoundStop();
            threadSound->quit();
            threadSound->wait();
            threadSound = nullptr;
        }
    }
}

void MainWindow::on_btnSaveReport_clicked()
{
    emit sgXlsxSaveReport();
}

void MainWindow::on_actionSaveReport_triggered()
{
    emit sgXlsxSaveReport();
}

void MainWindow::on_actionAutoSave_triggered(bool checked)
{
    emit sgXlsxSetAutoSave(checked);
}

void MainWindow::on_actionWizard_triggered()
{
    Wizard wizard(this);
    connect(&wizard, &Wizard::wizardInfoFinish,[&](const WizardInfo &info)
            {
                qDebug() << "向导结束";
                if(ui->start_Std->state() == true)
                    ui->start_Std->on_btnSerialSwitch_clicked();
                if(ui->start_Dtm->state() == true)
                    ui->start_Dtm->on_btnSerialSwitch_clicked(); 
                emit sgXlsxSaveInfo(info.baseInfo);

                ui->start_Std->setSerialSettingIndex(info.ssIndex_Std);
                ui->start_Dtm->setSerialSettingIndex(info.ssIndex_Dtm); 
                ui->start_Std->on_btnSerialSwitch_clicked();
                ui->start_Dtm->on_btnSerialSwitch_clicked();
                ui->tabMain->setCurrentIndex(2); });
    wizard.exec();
}

void MainWindow::on_spbxWaveNum_valueChanged(int arg1)
{
    emit sgSetDataWaveNum(arg1);
}

void MainWindow::on_spbxWaveRange_valueChanged(double arg1)
{
    emit sgSetDataWaveRange(arg1);
}

void MainWindow::on_btnVerify_clicked()
{
    verifyState = !verifyState;
    if (verifyState)
    {
        connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerY);
        connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerX);
        ui->btnVerify->setText("关闭检验");
        ui->chartFit->setVerifyTracerVisible(true);
    }
    else
    {
        disconnect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerY);
        disconnect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerX);
        ui->btnVerify->setText("开启检验");
        ui->chartFit->setVerifyTracerVisible(false);
    }
}

void MainWindow::on_btnWizard_clicked()
{
    on_actionWizard_triggered();
}
