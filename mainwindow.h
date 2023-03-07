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

    void on_btnCollectStop_clicked();

    void on_spbxSamplePointSum_valueChanged(int arg1);

    void on_spbxOrder_valueChanged(int arg1);

    void on_btnFit_clicked();

    void on_twAverage_itemSelectionChanged();

    void twAverage_itemChanged(QTableWidgetItem *item);

    void setFitChartData(const vector<DECIMAL_TYPE> &factor);

    void setAverageTableItem_Std(const DECIMAL_TYPE &average);

    void setAverageTableItem_Dtm(const DECIMAL_TYPE &average);

    void on_actionAbout_triggered();

    void on_spbxSampleTime_valueChanged(double arg1);

signals:
    void collectDataXYChanged(const QVector<double> &x, const QVector<double> &y);
    void fitDataChanged(const vector<double> &x, const vector<double> &y);

    void startGenerate(const DECIMAL_TYPE &t_left, const DECIMAL_TYPE &t_right, const DECIMAL_TYPE &t_step, const vector<DECIMAL_TYPE> &t_factor);
    void startLeastSquare(const int &t_N, const vector<DECIMAL_TYPE> &t_x, const vector<DECIMAL_TYPE> &t_y);

private:
    Ui::MainWindow *ui;
    QTimer *timerCollect;
    int collectTimestamp;    // 采集时间戳（秒 * TIMESTAMP_FACTOR)
    int sampledPointNum = 0; // 已采集点数
    int samplePointSum = 8;  // 需要采集点数
    int pgsbSingleValue = 0; //

    void timerCollectTimeOut();
    QString collectTimestampToHhMmSs(int timestamp);
    void setDeviceName_Std(QString name);
    void setDeviceName_Dtm(QString name);

    unsigned long long order; // 最小二乘法多项式阶数
    vector<DECIMAL_TYPE> collectDataX, collectDataY;
    DECIMAL_TYPE collectDataX_Max, collectDataX_Min;
    vector<DECIMAL_TYPE> fitDataX, fitDataY;
    QRegExp rx;
    QString old_text = "";

    // 任务类对象
    Bll_GenerateData *taskGen;
    Bll_LeastSquareMethod *taskLeastSquare;

    void updateCollectDataXY(void);
    void tryUpdateFitChart(bool man);
};
#endif // MAINWINDOW_H
