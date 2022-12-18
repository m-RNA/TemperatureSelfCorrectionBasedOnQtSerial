#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startcommunication.h"
#include "collectpanel.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>

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

    //    connect(ui->start_Std, &StartCommunication::RecvDataAnalyseFinish, ui->wave_Std, &CustomChart::addYPoint);
    //    connect(ui->start_Dtm, &StartCommunication::RecvDataAnalyseFinish, ui->wave_Dtm, &CustomChart::addYPoint);
    //    connect(ui->start_Std, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addVLine);
    //    connect(ui->start_Dtm, &StartCommunication::RecvDataAnalyseFinish, ui->calibrationChart, &CustomChart::addHLine);
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
            //            QMessageBox::critical(this, "错误", "请同时连接两个仪器");
            //            return;
        }

        // 一段时间内标准仪器波动<0.01

        // 清空实时波形时间轴变为相对时间戳
        // # 这个放在if (sampledPointNum == 0)里面才对(调试)
        if (sampledPointNum == 0)
        {
            collectTimestamp = ui->spbxSampleTime->value() * 60 * TIMESTAMP_FACTOR; // 分钟转换时间戳
            qDebug() << "collectTimestamp" << collectTimestamp;
            ui->pgsbSum->setMaximum(ui->spbxSamplePointNum->value());
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
