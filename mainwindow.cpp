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
#include "message.h"
#include <QInputDialog>
#include <QTimer>
#include <QStringList>
#include <QSettings>
using LeastSquare = MainWindow;
using Bll_CollectBtn = MainWindow;

// 初始化静态成员变量
int MainWindow::collectCounter = 0;          // 已采集点数
int MainWindow::collectIndex = 0;            // 采集序号
bool MainWindow::returnCollectIndex = false; // 是否返回序号

const int PGSB_REFRESH_MS = 500;                             // 进度条刷新间隔
const double TIMESTAMP_FACTOR = (1000.0f / PGSB_REFRESH_MS); // 时间戳因子，用于计算时间戳

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Message::setParent(this);

    timerCollect = new QTimer(this);
    timerCollect->setTimerType(Qt::PreciseTimer); // 设置为精准定时器
    timerCollect->setInterval(PGSB_REFRESH_MS);   // 每 PGSB_REFRESH_MS ms检查一次
    connect(timerCollect, &QTimer::timeout, this, &MainWindow::timerCollectTimeOut);

    setDeviceName_Dtm("待定仪器");
    setDeviceName_Std("标准仪器");
    ui->start_Std->setCbxSerialIndex(1);
    ui->start_Dtm->setCbxSerialIndex(0);
    // 读取串口设置
    ui->start_Std->loadUiSettings("_Std");
    ui->start_Dtm->loadUiSettings("_Dtm");
    loadUiSettings();

    /*** LeastSquare Begin ***/

    // 这里是根据表格的行数来设置采集点数的
    samplePointSum = ui->twAverage->rowCount();
    ui->spbxSamplePointSum->setValue(samplePointSum);
    order = ui->spbxOrder->value();

    ui->twAverage->horizontalHeader()->setVisible(true);
    ui->twAverage->verticalHeader()->setVisible(true);
    ui->twAverage->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twAverage->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    ui->twFactor->horizontalHeader()->setVisible(true);
    ui->twFactor->verticalHeader()->setVisible(true);
    ui->twFactor->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twFactor->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    ui->twRange->horizontalHeader()->setVisible(true);
    ui->twRange->verticalHeader()->setVisible(true);
    ui->twRange->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twRange->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

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
        }
    }
    connect(ui->twAverage, &QTableWidget::itemChanged, this, &LeastSquare::twAverage_itemChanged);

    connect(this, &LeastSquare::collectDataXYChanged, ui->chartFit, &FitChart::updateCollectPlot);

    // 同步两个表格的滚动条
    connect(ui->twAverage->verticalScrollBar(), &QScrollBar::valueChanged, ui->twRange->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->twRange->verticalScrollBar(), &QScrollBar::valueChanged, ui->twAverage->verticalScrollBar(), &QScrollBar::setValue);

    // 拟合阶数改变时，迟滞一定时间后更新拟合图表
    timerOrderChangeDelayUpdate = new QTimer(this);
    timerOrderChangeDelayUpdate->setInterval(200);
    timerOrderChangeDelayUpdate->setSingleShot(true);
    connect(timerOrderChangeDelayUpdate, &QTimer::timeout, [&]()
            { tryUpdateFitChart(false); });

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

    // 同步解析数据传输
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Std, &CollectPanel::slCollectData);
    connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->collectPanel_Dtm, &CollectPanel::slCollectData);
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerY);
    connect(ui->start_Dtm, &StartCommunication::sgStartAnalyseFinish, ui->chartFit, &FitChart::updateVerifyTracerX);

    connect(ui->collectPanel_Std, &CollectPanel::sgShowData, [&]()
            {ui->tabMain->setCurrentIndex(1); ui->start_Std->showTextEditRx(); });
    connect(ui->collectPanel_Dtm, &CollectPanel::sgShowData, [&]()
            {ui->tabMain->setCurrentIndex(1); ui->start_Dtm->showTextEditRx(); });

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
    connect(threadXlsx, &QThread::finished, taskXlsxData, &QObject::deleteLater); // 释放内存资源
    connect(threadXlsx, &QThread::finished, threadXlsx, &QThread::deleteLater);   // 释放内存资源
    connect(this, &MainWindow::sgXlsxStartPoint, taskXlsxData, &Bll_SaveDataToXlsx::startPoint);
    connect(this, &MainWindow::sgXlsxSaveReport, taskXlsxData, &Bll_SaveDataToXlsx::saveReport);
    connect(this, &MainWindow::sgXlsxSetAutoSave, taskXlsxData, &Bll_SaveDataToXlsx::setAutoSave);
    connect(this, &MainWindow::sgXlsxSaveInfo, taskXlsxData, &Bll_SaveDataToXlsx::saveInfo);
    threadXlsx->start();
    emit sgXlsxSetAutoSave(ui->actionAutoSave->isChecked());

    shortcutInit();
    statusBarInit();
}

MainWindow::~MainWindow()
{
    saveUiSetting();

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

void MainWindow::loadUiSettings()
{
    QSettings setting(CONFIG_FILE_NAME, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("GB18030"));

    // 读取窗口大小和位置
    if (QFile::exists(CONFIG_FILE_NAME))
    {
        setting.beginGroup("MainWindow");
        resize(setting.value("Size").toSize());
        move(setting.value("Pos", QPoint(0, 0)).toPoint());
        setting.endGroup();
    }

    // 读取启动时是否显示欢迎界面
    setting.beginGroup("UI");
    bool showWelcome = setting.value("ShowWelcome", true).toBool();
    ui->ckbxWelcome->setChecked(showWelcome);
    if (showWelcome)
        ui->tabMain->setCurrentIndex(0);
    else
        ui->tabMain->setCurrentIndex(1);

    // 读取主题
    int themeTemp = setting.value("Theme", 0).toInt();

    switch (themeTemp)
    {
    case 0:
        pictureInit(rand() % 2); // 白天照片
        // on_actionLightStyle_triggered();
        break;
    case 1:
        on_actionGrayStyle_triggered();
        break;
    case 2:
        on_actionBlueStyle_triggered();
        break;
    case 3:
        on_actionDarkStyle_triggered();
        break;
    }
    setting.endGroup();

    // 读取采集设置
    setting.beginGroup("Collect");
    ui->spbxSamplePointSum->setValue(setting.value("SamplePointSum", 8).toInt());
    ui->spbxSampleTime->setValue(setting.value("SampleTime", 0.05).toDouble());
    bool autoCollect = setting.value("AutoCollect", true).toBool();
    ui->cbAutoCollect->setCurrentIndex(autoCollect);
    // ui->twTarget->setEnable(autoCollect);
    // 读取autoList
    if (QFile::exists(CONFIG_FILE_NAME))
    {
        int autoListSize = setting.beginReadArray("AutoCollectDataList");
        if (autoListSize == 0)
            goto AUTO_LIST_DEFAULT;
        for (int i = 0; i < autoListSize; ++i)
        {
            setting.setArrayIndex(i);
            wizardInfo.collectSetting.autoList.push_back(setting.value("Data").toDouble());
        }
        setting.endArray();
    }
    else
    {
        // 给autoLis赋默认值 {35, 30, 25, 20, 15, 10, 5, 0.15}
    AUTO_LIST_DEFAULT:
        for (int i = 0; i < 7; ++i)
        {
            wizardInfo.collectSetting.autoList.push_back(35 - i * 5);
        }
        wizardInfo.collectSetting.autoList.push_back(0.15);
    }
    setting.endGroup();

    // 读取波动监测设置
    setting.beginGroup("WaveCheck");
    ui->spbxWaveNum->setValue(setting.value("WaveNum", 10).toInt());
    ui->spbxWaveRange->setValue(setting.value("WaveRange", 0.01).toDouble());
    ui->spbxStableTime->setValue(setting.value("StableTime", 0.05).toDouble());

    setting.endGroup();

    // 读取声音设置
    setting.beginGroup("Sound");
    ui->cbSound->setCurrentIndex(setting.value("Sound", 1).toInt());
    setting.endGroup();

    // 读取最小二乘法设置
    setting.beginGroup("LeastSquareMethod");
    ui->spbxOrder->setValue(setting.value("Order", 3).toInt());
    setting.endGroup();

    // 读取表格自动保存设置
    setting.beginGroup("Xlsx");
    bool autoSave = setting.value("AutoSave", true).toBool();
    ui->actionAutoSave->setChecked(autoSave);
    emit sgXlsxSetAutoSave(autoSave);
    setting.endGroup();

    // 读取向导基本信息
    setting.beginGroup("WizardBaseInfo");
    wizardInfo.baseInfo.place = setting.value("Place", "中北大学工程训练中心").toString();
    wizardInfo.baseInfo.temp = setting.value("Temp", 21).toInt();
    wizardInfo.baseInfo.rh = setting.value("RH", 52).toInt();
    wizardInfo.baseInfo.operatorName = setting.value("Operator", "老陈皮").toString();
    wizardInfo.baseInfo.id_Std = setting.value("ID_Std", "道万").toString();
    wizardInfo.baseInfo.id_Dtm = setting.value("ID_Dtm", "待定").toString();
    setting.endGroup();
}

void MainWindow::saveUiSetting()
{
    QSettings setting(CONFIG_FILE_NAME, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("GB18030"));

    // 保存窗口大小和位置
    setting.beginGroup("MainWindow");
    setting.setValue("Size", size());
    setting.setValue("Pos", pos());
    setting.endGroup();

    // 保存启动时是否显示欢迎界面
    setting.beginGroup("UI");
    setting.setValue("ShowWelcome", ui->ckbxWelcome->isChecked());
    setting.setValue("Theme", themeIndex);
    setting.endGroup();

    // 保存采集设置
    setting.beginGroup("Collect");
    setting.setValue("SamplePointSum", ui->spbxSamplePointSum->value());
    setting.setValue("SampleTime", ui->spbxSampleTime->value());
    setting.setValue("AutoCollect", ui->cbAutoCollect->currentIndex());
    // 保存autoList
    setting.beginWriteArray("AutoCollectDataList");
    for (size_t i = 0; i < wizardInfo.collectSetting.autoList.size(); ++i)
    {
        setting.setArrayIndex(i);
        setting.setValue("Data", wizardInfo.collectSetting.autoList[i]);
    }
    setting.endArray();
    setting.endGroup();

    // 保存波动监测设置
    setting.beginGroup("WaveCheck");
    setting.setValue("WaveNum", ui->spbxWaveNum->value());
    setting.setValue("WaveRange", ui->spbxWaveRange->value());
    setting.setValue("StableTime", ui->spbxStableTime->value());
    setting.endGroup();

    // 保存声音设置
    setting.beginGroup("Sound");
    setting.setValue("Sound", ui->cbSound->currentIndex());
    setting.endGroup();

    // 保存最小二乘法设置
    setting.beginGroup("LeastSquareMethod");
    setting.setValue("Order", ui->spbxOrder->value());
    setting.endGroup();

    // 保存表格自动保存设置
    setting.beginGroup("Xlsx");
    setting.setValue("AutoSave", ui->actionAutoSave->isChecked());
    setting.endGroup();

    // 保存向导基本信息
    setting.beginGroup("WizardBaseInfo");
    setting.setValue("Place", wizardInfo.baseInfo.place);
    setting.setValue("Temp", wizardInfo.baseInfo.temp);
    setting.setValue("RH", wizardInfo.baseInfo.rh);
    setting.setValue("Operator", wizardInfo.baseInfo.operatorName);
    setting.setValue("ID_Std", wizardInfo.baseInfo.id_Std);
    setting.setValue("ID_Dtm", wizardInfo.baseInfo.id_Dtm);
    setting.endGroup();
}

void MainWindow::setDeviceName_Std(const QString &name)
{
    ui->start_Std->setDeviceName(name);
    ui->collectPanel_Std->setDeviceName(name);
}

void MainWindow::setDeviceName_Dtm(const QString &name)
{
    ui->start_Dtm->setDeviceName(name);
    ui->collectPanel_Dtm->setDeviceName(name);
}

int MainWindow::getCollectIndex(void)
{
    if (returnCollectIndex)
        return collectIndex;
    return collectCounter;
}

void MainWindow::pictureInit(int index)
{
    QPixmap pix;
    switch (index)
    {
    case 0:
        pix.load("://picture/NUC_day.jpg");
        break;

    case 1:
        pix.load("://picture/NUC_day_heart.jpg");
        break;

    case 2:
        pix.load("://picture/NUC_night.png");
        break;

    case 3:
        pix.load("://picture/NUC_night_library.png");
        break;

    default:
        pix.load("://picture/NUC_day.jpg");
    }

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
    ui->collectPanel_Std->setStableState(STABLE_STATE_INIT);
    ui->collectPanel_Dtm->setOnlineState(ui->start_Dtm->state());

    listenDataWaveQuit();
    taskDataWave = new Bll_DataWave;
    taskDataWave->setRange(ui->spbxWaveRange->value());
    taskDataWave->setCheckNum(ui->spbxWaveNum->value());
    taskDataWave->setStableTime(ui->spbxStableTime->value() * 60000);

    threadDataWave = new QThread;
    taskDataWave->moveToThread(threadDataWave);
    connect(threadDataWave, &QThread::finished, taskDataWave, &QObject::deleteLater);
    connect(threadDataWave, &QThread::finished, threadDataWave, &QThread::deleteLater);
    connect(ui->start_Std, &StartCommunication::sgStartAnalyseFinish, taskDataWave, &Bll_DataWave::addData);
    connect(taskDataWave, &Bll_DataWave::sgStableState, ui->collectPanel_Std, &CollectPanel ::setStableState);
    connect(this, &MainWindow::sgSetDataWaveRange, taskDataWave, &Bll_DataWave::setRange);
    connect(this, &MainWindow::sgSetDataWaveNum, taskDataWave, &Bll_DataWave::setCheckNum);
    connect(this, &MainWindow::sgSetDataWaveStableTime, taskDataWave, &Bll_DataWave::setStableTime);

    // 标准仪器稳定后，开始采集数据
    connect(taskDataWave, &Bll_DataWave::sgTurnToStable, this, &MainWindow::goOnCollect);
    threadDataWave->start();

    qDebug() << "threadDataWave start";

    if (ui->cbAutoCollect->currentIndex() == 1)
    {
        autoCollectTimerInit();
    }
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

    autoCollectTimerQuit();
}

void MainWindow::collectDataInit()
{
    taskDataCollect_Std = new Bll_DataCollect;
    taskDataCollect_Dtm = new Bll_DataCollect;
    threadDataCollect = new QThread;
    taskDataCollect_Std->moveToThread(threadDataCollect);
    taskDataCollect_Dtm->moveToThread(threadDataCollect);
    connect(threadDataCollect, &QThread::finished, [&]()
            {taskDataCollect_Std->deleteLater(); taskDataCollect_Dtm->deleteLater(); threadDataCollect->deleteLater(); });

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

void MainWindow::autoCollectTimerInit()
{
    autoCollectTimerQuit();
    timerAutoCollectCheck = new QTimer;
    timerAutoCollectCheck->setInterval(1500);
    connect(timerAutoCollectCheck, &QTimer::timeout, taskDataWave, &Bll_DataWave::setAutoCollectStart);
    connect(taskDataWave, &Bll_DataWave::sgAutoCollect, this, [&]()
            {
        if ((btnSwitchState == CollectBtnState_Next) || (btnSwitchState == CollectBtnState_Start))
        {
            on_btnCollectSwitch_clicked();
        }
        else if (btnSwitchState == CollectBtnState_End)
        {
            // 只提示一次
            static bool isShow = false;
            if (!isShow)
            {
                isShow = true;
                QMessageBox::information(this, "已全部采集完成", "怀疑有重复采集数据，若有，\n可将最后的数据粘贴到该处\n然后点击重采本点即可");
            }} });
    // Message::information("已全部采集完成\n怀疑表格里有重复采集数据\n若有重复数据,可以将最后一点的数据剪切到该处\n然后点击重采本点即可", 5000); });
    timerAutoCollectCheck->start();
    qDebug() << "AutoCollectCheck start";
}

void MainWindow::autoCollectTimerQuit()
{
    if (timerAutoCollectCheck)
    {
        timerAutoCollectCheck->stop();
        timerAutoCollectCheck->deleteLater();
        timerAutoCollectCheck = nullptr;
        qDebug() << "AutoCollectCheck quit";
    }
}

void MainWindow::soundInit()
{
    if (threadSound)
    {
        emit sgSoundStop();
        threadSound->quit();
        threadSound->wait();
    }
    taskSound = new Bll_Sound;
    threadSound = new QThread;
    taskSound->moveToThread(threadSound);
    connect(threadSound, &QThread::finished, taskSound, &QObject::deleteLater);
    connect(threadSound, &QThread::finished, threadSound, &QThread::deleteLater);
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

void Bll_CollectBtn::on_btnCollectSwitch_clicked()
{
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
            // QMessageBox::critical(this, "错误", "请同时连接两个仪器");
            Message::error("请同时连接两个仪器");
            return;
        }
        setCollectSettingLock(true);

        // 最终的完成采集
        if (btnSwitchState == CollectBtnState_End)
        {
            ui->btnCollectSwitch->setEnabled(false);
            ui->btnCollectRestart->setEnabled(false);

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
            setCollectSettingLock(false);
        }
    }
}

void Bll_CollectBtn::setCollectSettingLock(bool lock)
{
    bool state = !lock;
    ui->spbxSamplePointSum->setEnabled(state);
    ui->spbxSampleTime->setEnabled(state);
    ui->spbxWaveNum->setEnabled(state);
    ui->spbxWaveRange->setEnabled(state);
    ui->spbxStableTime->setEnabled(state);
    if (lock)
        ui->actionLock->setChecked(true);
    else
        ui->actionLock->setChecked(false);
}

void Bll_CollectBtn::on_btnCollectRestart_clicked()
{
    // 两个串口是否同时打开
    if (!(ui->start_Dtm->state() && ui->start_Std->state()))
    {
        // QMessageBox::critical(this, "错误", "请同时连接两个仪器");
        Message::error("请同时连接两个仪器");
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
    ui->pgsbSum->setValue(collectCounter + 1);

    double range_Std = ui->collectPanel_Std->getRange();
    double range_Dtm = ui->collectPanel_Dtm->getRange();
    QString strRange_Std = QString::number(range_Std);
    QString strRange_Dtm = QString::number(range_Dtm);
    rangeList_Std.resize(collectCounter + 1);
    rangeList_Dtm.resize(collectCounter + 1);
    rangeList_Std[getCollectIndex()] = range_Std;
    rangeList_Dtm[getCollectIndex()] = range_Dtm;
    // 更新极差图表
    ui->chartRange->setData(rangeList_Std, rangeList_Dtm);
    // 填入极差表格
    if (ui->twRange->item(getCollectIndex(), 0) == nullptr)
    {
        ui->twRange->setItem(getCollectIndex(), 0, new QTableWidgetItem(strRange_Std));
        ui->twRange->item(getCollectIndex(), 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    else
        ui->twRange->item(getCollectIndex(), 0)->setText(strRange_Std);
    if (ui->twRange->item(getCollectIndex(), 1) == nullptr)
    {
        ui->twRange->setItem(getCollectIndex(), 1, new QTableWidgetItem(strRange_Dtm));
        ui->twRange->item(getCollectIndex(), 1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    else
        ui->twRange->item(getCollectIndex(), 1)->setText(strRange_Dtm);

    QString msg = "标准仪器极差：" + strRange_Std + "\n" +
                  "待定仪器极差：" + strRange_Dtm + "\n";

    if (collectCounter < samplePointSum - 1)
    {
        setCollectBtnState(CollectBtnState_Next);

        if (ui->cbSound->currentIndex() > 0) // 开启语音提醒
        {
            soundInit();
            emit sgSoundPlay1((SoundIndex)ui->cbSound->currentIndex());
        }

        msg += "请准备下一点采集";
        // 如果是自动采集，就不弹出消息框
        // if (ui->cbAutoCollect->currentIndex() == 1)
        //     Message::information("此点采集完成\n" + msg, 5000);
        // else
        QMessageBox::information(this, "此点采集完成", msg);

        // 重置单点进度
        pgsbSingleReset();
    }
    else // 全部采集完成
    {
        setCollectBtnState(CollectBtnState_End);

        if (ui->cbSound->currentIndex() > 0) // 开启语音提醒
        {
            soundInit();
            emit sgSoundPlay2((SoundIndex)ui->cbSound->currentIndex());
        }
        on_btnSaveReport_clicked(); // 保存报告

        msg += "请在右下角查看拟合结果";
        // 如果是自动采集，就不弹出消息框
        // if (ui->cbAutoCollect->currentIndex() == 1)
        //     Message::information("全部采集完成\n" + msg, 5000);
        // else
        QMessageBox::information(this, "全部采集完成", msg);

        tryUpdateFitChart(false); // 更新拟合图表
    }

    // 关闭声音
    if (ui->cbSound->currentIndex() > 0)
    {
        emit sgSoundStop();
        threadSound->quit();
        threadSound->wait(); // 这里需要吗？
        threadSound = nullptr;
    }

    // 更新状态栏
    lbLastRange->setText("上次极差 标准：" + strRange_Std + " 待定：" + strRange_Dtm);
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

    qDebug() << "开始采集" << collectCounter << getCollectIndex();
}

void Bll_CollectBtn::stopCollect()
{
    collectDataQuit();
    ui->collectPanel_Std->collectStop();
    ui->collectPanel_Dtm->collectStop();

    listenDataWaveInit();
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
    qDebug() << "本点采集结束" << collectCounter << getCollectIndex();
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
    returnCollectIndex = false;
    ui->statusbar->showMessage("Alt+O 或 Alt+P ：切换采集序号，可重新采集之前数据", 15000);

    collectCounter++;

    qDebug() << "采集下点" << collectCounter << getCollectIndex();
}

// 更新单点进度条显示时间
void MainWindow::on_spbxSampleTime_valueChanged(double arg1)
{
    int sec = arg1 * 60;
    ui->pgsbSingle->setFormat(QString::asprintf("%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 3600 % 60));
}

void LeastSquare::leastSquareTaskStart(const int order, const vector<DECIMAL_TYPE> &x, const vector<DECIMAL_TYPE> &y)
{
    if (threadLeastSquare != nullptr)
    {
        threadLeastSquare->quit();
        threadLeastSquare->wait();
    }
    taskLeastSquare = new Bll_LeastSquareMethod;
    threadLeastSquare = new QThread;
    taskLeastSquare->moveToThread(threadLeastSquare);
    connect(threadLeastSquare, &QThread::finished, taskLeastSquare, &Bll_LeastSquareMethod::deleteLater); // 任务结束后自动删除
    connect(threadLeastSquare, &QThread::finished, threadLeastSquare, &QThread::deleteLater);             // 线程结束后自动删除
    connect(this, &LeastSquare::startLeastSquare, taskLeastSquare, &Bll_LeastSquareMethod::work);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, this, &LeastSquare::setOrderData);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, taskXlsxData, &Bll_SaveDataToXlsx::saveFactor);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::generateFitDataFinish, this, [&](const QVector<double> &x, const QVector<double> &y)
            {
                ui->chartFit->updateFitPlot(x, y);
                threadLeastSquare->quit();
                threadLeastSquare->wait();
                threadLeastSquare = nullptr; });

    threadLeastSquare->start();
    emit startLeastSquare(order, x, y);
}

// 采集点数改变
void LeastSquare::on_spbxSamplePointSum_valueChanged(int arg1)
{
    if (arg1 > samplePointSum)
    {
        ui->twRange->setRowCount(arg1);   // 设置极差表格行数
        ui->twAverage->setRowCount(arg1); // 设置平均值表格行数
        // 需要初始化表格Item
        for (int i = samplePointSum; i < arg1; i++)
        {
            // 平均数表格
            QTableWidgetItem *itemY = new QTableWidgetItem;
            QTableWidgetItem *itemX = new QTableWidgetItem;
            ui->twAverage->setItem(i, 0, itemY);
            ui->twAverage->setItem(i, 1, itemX);
        }
    }
    else if (arg1 < samplePointSum)
    {
        // 判断是否有数据
        for (int i = samplePointSum - 1; i >= arg1; i--)
        {
            if (ui->twAverage->item(i, 0)->text() != "")
            {
                QMessageBox::warning(this, "警告", "请先删除多余平均数！");
                ui->spbxSamplePointSum->setValue(samplePointSum);
                switchCalibrateView(1);
                return;
            }
        }
        ui->twRange->setRowCount(arg1);   // 设置极差表格行数
        ui->twAverage->setRowCount(arg1); // 设置平均值表格行数
    }

    // 设置采集点数
    samplePointSum = arg1;
    // 设置整体进度条最大值
    ui->pgsbSum->setMaximum(arg1);

    // 设置采集按钮状态
    if (collectCounter + 1 > samplePointSum)
    {
        collectCounter = samplePointSum - 1;
        if (btnSwitchState == CollectBtnState_Next)
            setCollectBtnState(CollectBtnState_End);
        else if (btnSwitchState == CollectBtnState_Start)
        {
            setCollectBtnState(CollectBtnState_End);
            ui->btnCollectRestart->setEnabled(true);
        }
    }
    else if (collectCounter + 1 < samplePointSum)
    {
        if (btnSwitchState == CollectBtnState_End)
        {
            setCollectBtnState(CollectBtnState_Next);
            ui->btnCollectSwitch->setEnabled(true);
            ui->btnCollectRestart->setEnabled(true);
        }
    }
}

void LeastSquare::updateCollectDataXY(void)
{
    DECIMAL_TYPE temp;
    collectDataX.clear(); // 重置x容器
    collectDataY.clear(); // 重置y容器
    // qDebug() << "LeastSquare::updateCollectDataXY ";
    for (int i = 0; i < samplePointSum; i++)
    {
        QString qsX, qsY;
        qsY = ui->twAverage->item(i, 0)->text();
        qsX = ui->twAverage->item(i, 1)->text();
        if (qsX.isEmpty() || qsY.isEmpty())
            continue;
        // qDebug() << "counter" << counter;

        // qDebug() << i;
        temp = stold(qsX.toStdString());
        collectDataX.push_back(temp);
        // snprintf(globalStringBuffer, sizeof(globalStringBuffer), "toLoDouble = %.20LE", temp);
        // qDebug() << globalStringBuffer;

        temp = stold(qsY.toStdString());
        collectDataY.push_back(temp);
        // snprintf(globalStringBuffer, sizeof(globalStringBuffer), "toLoDouble = %.20LE", temp);
        // qDebug() << globalStringBuffer;
    }
}

// 人为 true 自动 false
void LeastSquare::tryUpdateFitChart(bool man)
{
    updateCollectDataXY();
    if (collectDataX.size() < 2)
    {
        if (man) // 是人为的就要提醒一下
            Message::error("正确格式的数据小于两组");
        // QMessageBox::critical(this, "错误", "正确格式的数据\n小于两组");
        return;
    }

    // N个点可以确定一个 唯一的 N-1 阶的曲线
    order = ui->spbxOrder->value();
    if (collectDataX.size() <= order)
        order = collectDataX.size() - 1;

    // 启动子线程
    leastSquareTaskStart(order, collectDataX, collectDataY);

    ui->statusbar->showMessage("差值指示灯：<+-0.05 绿色 <+-0.2 黄色", 10000);
}

void LeastSquare::on_spbxOrder_valueChanged(int arg1)
{
    order = arg1;
    ui->twFactor->setRowCount(arg1 + 1);

    // 重置更新拟合图表定时器
    if (timerOrderChangeDelayUpdate) // 初始化完成后才会有定时器
        timerOrderChangeDelayUpdate->start();
}

void LeastSquare::on_btnFit_clicked()
{
    timerOrderChangeDelayUpdate->stop();
    tryUpdateFitChart(true);
}

void LeastSquare::setOrderData(const vector<DECIMAL_TYPE> &factorList)
{
    factor = factorList;

    for (size_t i = 0; i < factorList.size(); i++)
    {
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "%.8LE", factorList[factorList.size() - i - 1]);
        QTableWidgetItem *temp = new QTableWidgetItem(globalStringBuffer); // QString::fromStdString(globalStringBuffer));
        ui->twFactor->setItem(i, 0, temp);
        ui->twFactor->item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    // 清除残余数据
    for (int i = factorList.size(); i < ui->twFactor->rowCount(); i++)
    {
        if (ui->twFactor->item(i, 0) != nullptr)
            ui->twFactor->item(i, 0)->setText("");
    }
}

// 单元格选中变化槽函数
void LeastSquare::on_twAverage_itemSelectionChanged()
{
    // 1、记录旧的单元格内容
    old_text = ui->twAverage->item(ui->twAverage->currentRow(),
                                   ui->twAverage->currentColumn())
                   ->text();
}

/* 正则表达式：
 * https://blog.csdn.net/qq_41622002/article/details/107488528
 */
// 单元格变化
void LeastSquare::twAverage_itemChanged(QTableWidgetItem *item)
{
    // 匹配正负整数、正负浮点数
    const QString Pattern("(-?[1-9][0-9]+)|(-?[0-9])|(-?[1-9]\\d+\\.\\d+)|(-?[0-9]\\.\\d+)"); // 正则表达式
    QRegExp reg(Pattern);

    // 获取修改的新的单元格内容
    QString str = item->text();
    if (str == "")
    {
        if (old_text == "")
            return;

        goto GO_ON;
    }

    // 去除字符串结尾的空格、回车、换行
    while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '\n' || str[str.length() - 1] == '\r')
    {
        str = str.left(str.length() - 1);
        if (str == "")
            break;
    }

    // 完全匹配
    if (reg.exactMatch(str))
    {
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
    if (ui->twAverage->item(getCollectIndex(), 0) == nullptr)
    {
        QTableWidgetItem *data_Std = new QTableWidgetItem(QString::fromStdString(data));
        ui->twAverage->setItem(getCollectIndex(), 0, data_Std);
    }
    else
    {
        ui->twAverage->item(getCollectIndex(), 0)->setText(QString::fromStdString(data));
    }
}

void MainWindow::setAverageTableItem_Dtm(const string &data)
{
    if (ui->twAverage->item(getCollectIndex(), 1) == nullptr)
    {
        QTableWidgetItem *data_Dtm = new QTableWidgetItem(QString::fromStdString(data));
        ui->twAverage->setItem(getCollectIndex(), 1, data_Dtm);
    }
    else
    {
        ui->twAverage->item(getCollectIndex(), 1)->setText(QString::fromStdString(data));
    }
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
    Message::success("Xlsx保存成功");
}

void MainWindow::on_actionSaveReport_triggered()
{
    on_btnSaveReport_clicked();
}

void MainWindow::on_actionAutoSave_triggered(bool checked)
{
    emit sgXlsxSetAutoSave(checked);
}

// 向导相关
void MainWindow::on_actionWizard_triggered()
{
    ui->start_Std->getSettingIndex(wizardInfo.ssIndex_Std);
    ui->start_Dtm->getSettingIndex(wizardInfo.ssIndex_Dtm);
    wizardInfo.collectSetting.num = ui->spbxSamplePointSum->value();
    wizardInfo.collectSetting.time = ui->spbxSampleTime->value();
    wizardInfo.collectSetting.isAuto = ui->cbAutoCollect->currentIndex();
    wizardInfo.checkWaveSetting.range = ui->spbxWaveRange->value();
    wizardInfo.checkWaveSetting.num = ui->spbxWaveNum->value();
    wizardInfo.checkWaveSetting.stableTime = ui->spbxStableTime->value();

    Wizard wizard(&wizardInfo, this);
    connect(&wizard, &Wizard::wizardInfoFinish, [&]()
            {
                qDebug() << "向导结束";
                emit sgXlsxSaveInfo(wizardInfo.baseInfo);

                ui->btnWizard->setEnabled(false);
                ui->btnWizardEdit->setEnabled(true);
                ui->actionWizard->setEnabled(false);

                ui->start_Std->setSerialSettingIndex(wizardInfo.ssIndex_Std);
                ui->start_Dtm->setSerialSettingIndex(wizardInfo.ssIndex_Dtm);
                if (ui->start_Std->state() == true)
                    ui->start_Std->on_btnSerialSwitch_clicked();
                if (ui->start_Dtm->state() == true)
                    ui->start_Dtm->on_btnSerialSwitch_clicked();
                setDeviceName_Std(wizardInfo.baseInfo.id_Std);
                setDeviceName_Dtm(wizardInfo.baseInfo.id_Dtm);

                ui->tabMain->setCurrentIndex(2);
                ui->spbxSamplePointSum->setValue(wizardInfo.collectSetting.num);
                ui->spbxSampleTime->setValue(wizardInfo.collectSetting.time);
                bool isAutoCollect = wizardInfo.collectSetting.isAuto;
                ui->cbAutoCollect->setCurrentIndex(isAutoCollect);
                if (isAutoCollect)
                {
                    qDebug() << "自动采集";
                    Bll_DataWave::setAutoCollectDataList(wizardInfo.collectSetting.autoList);
                }

                ui->spbxWaveRange->setValue(wizardInfo.checkWaveSetting.range);
                ui->spbxWaveNum->setValue(wizardInfo.checkWaveSetting.num);
                ui->spbxStableTime->setValue(wizardInfo.checkWaveSetting.stableTime);

                ui->start_Std->on_btnSerialSwitch_clicked();
                ui->start_Dtm->on_btnSerialSwitch_clicked(); });
    wizard.exec();
}

void MainWindow::on_btnWizard_clicked()
{
    on_actionWizard_triggered();
}

// 波动检测相关
void MainWindow::on_spbxWaveNum_valueChanged(int arg1)
{
    emit sgSetDataWaveNum(arg1);
}

void MainWindow::on_spbxWaveRange_valueChanged(double arg1)
{
    emit sgSetDataWaveRange(arg1);
}

void MainWindow::on_spbxStableTime_valueChanged(double arg1)
{
    emit sgSetDataWaveStableTime(arg1 * 60000);
}

// 切换视图
void MainWindow::on_btnSwitchView_clicked()
{
    calibrateViewState = !calibrateViewState;
    switchCalibrateView(calibrateViewState);
}

void MainWindow::switchCalibrateView(int index)
{
    if (index > 1 || index < 0)
        return;

    ui->swFitAverage->setCurrentIndex(index);
    ui->swRange->setCurrentIndex(index);

    if (index == 1)
        ui->btnSwitchView->setText("绘图视图");
    else
        ui->btnSwitchView->setText("数据视图");
}

// 自动采集开关
void MainWindow::on_cbAutoCollect_activated(int index)
{
    if (index == 1)
        autoCollectTimerInit();
    else
        autoCollectTimerQuit();
}

// 锁定解锁校准面板
void MainWindow::on_actionLock_triggered(bool checked)
{
    setCollectSettingLock(checked);
}

/* QSS主题来自开源项目：https://github.com/feiyangqingyun/QWidgetDemo
 * QString cssFileList[3] = {":/qss/flatgray.css", ":/qss/lightblue.css", ":/qss/blacksoft.css"};
 */
using ColorStyle = MainWindow;

void ColorStyle::loadStyle(const QString &qssFile)
{
    // 开启计时
    QElapsedTimer time;
    time.start();

    // 加载样式表
    QString qss;
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly))
    {
        // 用QTextStream读取样式文件不用区分文件编码 带bom也行
        QTextStream in(&file);
        // in.setCodec("utf-8");
        while (!in.atEnd())
        {
            qss += in.readLine() + "\n";
        }
        file.close();

        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(paletteColor));
        // 用时主要在下面这句
        qApp->setStyleSheet(qss);
    }

    qDebug() << "切换主题用时:" << time.elapsed();
}

void ColorStyle::setChartColorStyle(const int index)
{
    if (index == 0)
        return;
    ui->chartFit->setColorStyle(index);
    ui->chartRange->setColorStyle(index);
    ui->collectPanel_Std->setChartColorStyle(index);
    ui->collectPanel_Dtm->setChartColorStyle(index);
}

void ColorStyle::setColorStyle(const int index)
{
    if (index == themeIndex && index == 0)
        return;

    switch (index)
    {
    case 1:
        loadStyle(":/qss/flatgray.css");
        break;

    case 2:
        loadStyle(":/qss/lightblue.css");
        break;

    case 3:
        loadStyle(":/qss/blacksoft.css");
        break;
    }
    setChartColorStyle(index);
}

void ColorStyle::on_actionLightStyle_triggered()
{
    ui->actionLightStyle->setChecked(true);
    if (themeIndex == 0)
        return;
    themeIndex = 0;
    ui->actionBlueStyle->setChecked(false);
    ui->actionGrayStyle->setChecked(false);
    ui->actionDarkStyle->setChecked(false);

    // Message::information("重启软件后生效");
    QMessageBox::information(this, "提示", "重启软件后生效");
}

void ColorStyle::on_actionGrayStyle_triggered()
{
    ui->actionGrayStyle->setChecked(true);
    if (themeIndex == 1)
        return;
    themeIndex = 1;
    ui->actionLightStyle->setChecked(false);
    ui->actionBlueStyle->setChecked(false);
    ui->actionDarkStyle->setChecked(false);

    setColorStyle(themeIndex);
    pictureInit(rand() % 2); // 白天照片
}

void ColorStyle::on_actionBlueStyle_triggered()
{
    ui->actionBlueStyle->setChecked(true);
    if (themeIndex == 2)
        return;
    themeIndex = 2;
    ui->actionLightStyle->setChecked(false);
    ui->actionGrayStyle->setChecked(false);
    ui->actionDarkStyle->setChecked(false);

    setColorStyle(themeIndex);
    pictureInit(rand() % 2); // 白天照片
}

void ColorStyle::on_actionDarkStyle_triggered()
{
    ui->actionDarkStyle->setChecked(true);
    if (themeIndex == 3)
        return;
    themeIndex = 3;
    ui->actionLightStyle->setChecked(false);
    ui->actionBlueStyle->setChecked(false);
    ui->actionGrayStyle->setChecked(false);

    setColorStyle(themeIndex);
    pictureInit(2); // 黑夜照片
}

void MainWindow::on_actionQuit_triggered()
{
    close(); // 关闭MainWindow
}

// 退出前保存数据
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("警告");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "退出");
    msgBox.setButtonText(QMessageBox::No, "取消");

    if (isCollecting) // 检查是否正在采集数据
    {
        msgBox.setText("正在采集数据\n是否退出程序？");
        if (msgBox.exec() == QMessageBox::No)
        {
            event->ignore();
            return;
        }

        // 直接完成采集
        timerCollect->stop();
        isCollecting = false;
        finishCollect();
    }
    else if ((ui->pgsbSum->value() > 0) && (collectCounter + 1) < samplePointSum) // 检查校准任务是否开始或正在进行
    {
        msgBox.setText("校准任务未完成\n是否退出程序？");
        if (msgBox.exec() == QMessageBox::No)
        {
            event->ignore();
            return;
        }
    }

    if (collectCounter > 0)
        emit sgXlsxSaveReport(); // 保存数据
    event->accept();
}

/** 快捷键
 *  Alt + 1：切换到欢迎界面
 *  Alt + 2：切换到调试界面
 *  Alt + 3：切换到校准界面
 *  Alt + 4：切换校准视图
 *  Alt + S：开关仪器标准串口 Standard
 *  Alt + D：开关待定仪器串口 Determine
 *  Alt + C：开关采集 Collect
 *  Alt + R：重新采集 ReCollect
 *  Alt + F：拟合数据 Fit
 *  Alt + O: 采集序号 -1
 *  Alt + P: 采集序号 +1
 * Ctrl + M: 关闭语音提醒
 */
void MainWindow::shortcutInit()
{
    // 关闭语音提醒
    QShortcut *shortcut = nullptr;

    // 切换到欢迎界面
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { ui->tabMain->setCurrentIndex(0); });

    // 切换到调试界面
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_2), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { ui->tabMain->setCurrentIndex(1); });

    // 切换到校准界面
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_3), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { ui->tabMain->setCurrentIndex(2); });

    // 切换校准界面的视图
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_4), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { calibrateViewState = !calibrateViewState;
                switchCalibrateView(calibrateViewState); });

    // 开关仪器标准串口 Standard
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_S), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { ui->start_Std->on_btnSerialSwitch_clicked(); });

    // 开关待定仪器串口 Determine
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_D), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { ui->start_Dtm->on_btnSerialSwitch_clicked(); });

    // 开关采集 Collect
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_C), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnCollectSwitch_clicked);

    // 重新采集 ReCollect
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_R), this);
    connect(shortcut, &QShortcut::activated, [&]()
            { if (ui->btnCollectRestart->isEnabled())
                on_btnCollectRestart_clicked(); });

    // 拟合数据 Fit
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnFit_clicked);

    // 采集序号 -1
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_O), this);
    connect(
        shortcut, &QShortcut::activated, [&]()
        {
        if (collectCounter == 0)
            return;

        collectIndex--;
        collectIndex %= (collectCounter + 1);
        if(collectIndex < 0)
            collectIndex += collectCounter + 1;
        returnCollectIndex = true;

        setCollectBtnState(CollectBtnState_Start);
        ui->btnCollectSwitch->setEnabled(true);
        ui->btnCollectRestart->setEnabled(false);
        ui->statusbar->showMessage("切换采集序号为：" + QString::number(collectIndex + 1)); });

    // 采集序号 +1
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_P), this);
    connect(
        shortcut, &QShortcut::activated, [&]()
        {
        if (collectCounter == 0)
            return;

        collectIndex++;
        collectIndex %= (collectCounter + 1);
        returnCollectIndex = true;

        setCollectBtnState(CollectBtnState_Start);
        ui->btnCollectSwitch->setEnabled(true);
        ui->btnCollectRestart->setEnabled(false);
        ui->statusbar->showMessage("切换采集序号为：" + QString::number(collectIndex + 1)); });

    // 关闭语音提醒
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this);
    connect(shortcut, &QShortcut::activated, [&]()
            {
                if (ui->cbSound->currentIndex() != 0)
                {
                    ui->cbSound->setCurrentIndex(0);
                }
                Message::information("已关闭语音提醒"); });
}

// 状态栏初始化
void MainWindow::statusBarInit()
{
    QTime time = QTime::currentTime();
    QString strHello = "";

    // 正常问候语
    QStringList strListNormal = {
        "欢迎使用温度传感器校准上位机！",
        "随时可开始校准任务!",
        "完美的校准任务从一个友好的欢迎开始~",
    };
    // 亲切问候语
    QStringList strListFriendly = {
        "一起愉快地开始校准之旅吧！",
        "好久不见，可想你了，哦不，是温度传感器！",
        "哟吼，小伙子/姑娘，准备干啥呢？还是先校准吧！",
        "神秘的科研人员，来看看我们有哪些新奇且坑爹的数据吧！",
        "Welcome to the sensor calibration wonderland！今天喜提最优数据怎么样？",
    };

    // 早晨问候语更亲切，因为其他时间段加入这种问候语感觉可能有反效果
    // 也不知道是不是我多虑了  (￣o￣) . z Z  这个就算彩蛋吧
    if (time.hour() > 7 && time.hour() < 9)
        strListNormal.append(strListFriendly);

    // 根据时间问候用户
    if (time.hour() < 4)
        strHello = "深夜好！";
    else if (time.hour() < 6)
        strHello = "凌晨好！";
    else if (time.hour() < 9)
        strHello = "早安！";
    else if (time.hour() < 12)
        strHello = "上午好！";
    else if (time.hour() < 14)
        strHello = "中午好！";
    else if (time.hour() < 17)
        strHello = "下午好！";
    else if (time.hour() < 19)
        strHello = "傍晚好！";
    else
        strHello = "晚上好！";

    ui->statusbar->showMessage(strHello + strListNormal[rand() % strListNormal.size()], 30000);

    lbVerifyData = new QLabel(this);
    ui->statusbar->addPermanentWidget(lbVerifyData);
    lbVerifyData->setText("拟合：NULL 实际：NULL 差值：NULL");
    timerUpdateStatusBarVerifyData = new QTimer(this);
    timerUpdateStatusBarVerifyData->start(500);
    connect(timerUpdateStatusBarVerifyData, &QTimer::timeout, this, &MainWindow::updateStatusBarVerifyData);

    ledErrorRange = new QLabel(this);
    ledErrorRange->setFixedSize(16, 16); // 设置大小
    ui->statusbar->addPermanentWidget(ledErrorRange);

    // 用竖线分隔
    QFrame *line;
    line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    ui->statusbar->addPermanentWidget(line);

    lbLastRange = new QLabel(this);
    ui->statusbar->addPermanentWidget(lbLastRange);
    lbLastRange->setText("上次极差 标准：NULL 待定：NULL");

    line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    ui->statusbar->addPermanentWidget(line);

    lbRunTime = new QLabel(this);
    ui->statusbar->addPermanentWidget(lbRunTime);
    lbRunTime->setText("运行时间：0:00:00");
    timerRunTime = new QTimer(this);
    timerRunTime->start(1000);
    connect(timerRunTime, &QTimer::timeout, this, &MainWindow::updateRunTime);

    connect(ui->chartFit, &FitChart::sgShowMessage, this, &MainWindow::showMessage);
}

void MainWindow::updateRunTime()
{
    static time_t startTime = 0;
    if (startTime == 0)
    {
        if (ui->start_Std->state() || ui->start_Dtm->state())
        {
            startTime = time(nullptr);
        }
        return;
    }

    time_t now = time(nullptr);
    int runTime = now - startTime;
    int hour = runTime / 3600;
    int min = (runTime % 3600) / 60;
    int sec = runTime % 60;
    snprintf(globalStringBuffer, sizeof(globalStringBuffer), "运行时间：%d:%02d:%02d", hour, min, sec);
    lbRunTime->setText(globalStringBuffer);
}

void MainWindow::showMessage(const QString &msg, int timeout)
{
    ui->statusbar->showMessage(msg, timeout);
}

// 更新状态栏的拟合值、实际值、差值、差值指示灯
void MainWindow::updateStatusBarVerifyData()
{
    if (factor.size() < 2)
        return;

    // 计算拟合值
    double fitValue = factor.at(factor.size() - 1);
    double xVerify = ui->chartFit->getVerifyTracerX();
    double yVerify = ui->chartFit->getVerifyTracerY();
    for (size_t j = 1; j < factor.size(); j++)
    {
        fitValue += factor.at(factor.size() - 1 - j) * pow(xVerify, j);
    }
    // 显示拟合值、实际值、差值, 6位小数
    double range = yVerify - fitValue;
    QString str;
    if (range < 0)
        str = QString("拟合：%1 实际：%2 差值：%3").arg(fitValue, 0, 'f', 6).arg(yVerify, 0, 'f', 6).arg(range, 0, 'f', 6);
    else
        str = QString("拟合：%1 实际：%2 差值：+%3").arg(fitValue, 0, 'f', 6).arg(yVerify, 0, 'f', 6).arg(range, 0, 'f', 6);
    lbVerifyData->setText(str);
    setLEDErrorRange(range);
}

void MainWindow::setLEDErrorRange(double range)
{
    if (range < 0.05 && range > -0.05)
        ledErrorRange->setStyleSheet("border-radius:6px;background-color: rgb(46, 204, 113);"); // 绿色
    else if (range < 0.2 && range > -0.2)
        ledErrorRange->setStyleSheet("border-radius:6px;background-color: rgb(255, 176, 5);"); // 黄色
    else
        ledErrorRange->setStyleSheet("border-radius:6px;background-color: red;"); // 红色
}