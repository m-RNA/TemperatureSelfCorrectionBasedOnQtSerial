#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTableWidgetItem>
#include "bll_leastssquare.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnCollect_clicked();

    void on_spbxSamplePointSum_valueChanged(int arg1);

    void on_spbxOrder_valueChanged(int arg1);

    void on_btnFit_clicked();

    void on_twAverage_itemSelectionChanged();

    void twAverage_itemChanged(QTableWidgetItem *item);

    void setFitChartData(vector<double> factor);

    void setAverageTableItem_Std(double);

    void setAverageTableItem_Dtm(double);

    void on_actionAbout_triggered();

signals:
    void collectDataXYChanged(QVector<double> x, QVector<double> y);
    void fitDataChanged(vector<double> x, vector<double> y);

    void startGenerate(int t_left, int t_right, double t_step, vector<double> t_factor);
    void startLeastSquare(int t_N, vector<double> t_x, vector<double> t_y);

private:
    Ui::MainWindow *ui;
    QTimer *timerCollect;
    int collectTimestamp;    // 采集时间戳（秒 * TIMESTAMP_FACTOR)
    int sampledPointNum = 0; // 已采集点数
    int sampledPointSum = 8; // 需要采集点数
    int pgsbSingleValue = 0; //

    void timerCollectTimeOut();
    QString collectTimestampToHhMmSs(int timestamp);
    void setDeviceName_Std(QString name);
    void setDeviceName_Dtm(QString name);

    unsigned long long order; // 最小二乘法多项式阶数
    int samplePointSum;       // 标定点数
    vector<double> collectDataX, collectDataY;
    double collectDataX_Max, collectDataX_Min;
    vector<double> fitDataX, fitDataY;
    QRegExp rx;
    QString old_text = "";

    // 任务类对象
    Bll_GenerateData *taskGen;
    Bll_LeastSquareMethod *taskLeastSquare;

    void updateCollectDataXY(void);
    void tryUpdateFitChart(bool man);
};
#endif // MAINWINDOW_H
