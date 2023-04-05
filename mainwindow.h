#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTableWidgetItem>
#include "bll_leastssquare.h"
#include "bll_save_data_to_xlsx.h"
#include "bll_sound.h"
#include "bll_data_wave.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef enum
    {
        CollectBtnState_Start = 0,
        CollectBtnState_Wait,
        CollectBtnState_Stop,
        CollectBtnState_Next,
        CollectBtnState_End,
    } CollectBtnState;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnCollectSwitch_clicked();

    void on_btnCollectRestart_clicked();

    void on_spbxSamplePointSum_valueChanged(int arg1);

    void on_spbxOrder_valueChanged(int arg1);

    void on_btnFit_clicked();

    void on_twAverage_itemSelectionChanged();

    void twAverage_itemChanged(QTableWidgetItem *item);

    void setFitChartData(const vector<DECIMAL_TYPE> &factor);

    void setAverageTableItem_Std(const string &average);

    void setAverageTableItem_Dtm(const string &average);

    void on_actionAbout_triggered();

    void on_spbxSampleTime_valueChanged(double arg1);

    void on_cbSound_currentIndexChanged(int index);

    void on_btnSaveReport_clicked();

    void on_actionSaveReport_triggered();

    void on_actionAutoSave_triggered(bool checked);

    void on_actionWizard_triggered();

    void on_spbxWaveNum_valueChanged(int arg1);

    void on_spbxWaveRange_valueChanged(double arg1);

    void on_btnVerify_clicked();

    void on_btnWizard_clicked();

signals:
    void sgXlsxStartPoint();
    void sgXlsxNextPoint();
    void sgXlsxSaveReport();
    void sgXlsxSetAutoSave(const bool);
    void sgXlsxSaveInfo(const QStringList &info);

    void sgSoundPlay1(const SoundIndex &index);
    void sgSoundPlay2(const SoundIndex &index);
    void sgSoundStop();

    void sgSetDataWaveNum(const int num);
    void sgSetDataWaveRange(const double range);
    void sgSetDataWaveInterval(const int ms);
    void collectDataXYChanged(const QVector<double> &x, const QVector<double> &y);
    void fitDataChanged(const vector<double> &x, const vector<double> &y);

    void startGenerate(const DECIMAL_TYPE &t_left, const DECIMAL_TYPE &t_right, const DECIMAL_TYPE &t_step, const vector<DECIMAL_TYPE> &t_factor);
    void startLeastSquare(const int &t_N, const vector<DECIMAL_TYPE> &t_x, const vector<DECIMAL_TYPE> &t_y);

private:
    Ui::MainWindow *ui;
    QTimer *timerCollect = nullptr;
    CollectBtnState btnSwitchState = CollectBtnState_Start;
    bool isCollecting = false;
    bool waitingStdStable = false;
    int collectTimeStamp = 0; // 采集时间戳（秒 * TIMESTAMP_FACTOR)
    int collectCounter = 0;   // 已采集点数
    int samplePointSum = 8;   // 需要采集点数
    int pgsbSingleValue = 0;  // 单点进度条值

    void pgsbSingleInit();
    void pgsbSingleReset();
    QString collectTimeStampToHhMmSs(int timestamp);
    void timerCollectTimeOut();
    void startCollect();
    void stopCollect();
    void finishCollect();
    void resetCollect();
    void nextCollect();
    void goOnCollect();

    void setDeviceName_Std(QString name);
    void setDeviceName_Dtm(QString name);
    void setCollectBtnState(const CollectBtnState &state);

    void pictureInit();
    void soundInit();
    void listenDataWaveInit();
    void listenDataWaveQuit();

    unsigned long long order; // 最小二乘法多项式阶数
    vector<DECIMAL_TYPE> collectDataX, collectDataY;
    DECIMAL_TYPE collectDataX_Max, collectDataX_Min;
    vector<DECIMAL_TYPE> fitDataX, fitDataY;
    QRegExp rx;
    QString old_text = "";
    int soundIndex = 0;
    bool verifyState = false;

    // 任务类对象
    Bll_GenerateData *taskGen = nullptr;
    Bll_LeastSquareMethod *taskLeastSquare = nullptr;
    Bll_SaveDataToXlsx *taskXlsxData = nullptr;
    QThread *threadXlsx = nullptr;
    Bll_Sound *taskSound = nullptr;
    QThread *threadSound = nullptr;
    Bll_DataWave *taskDataWave = nullptr;
    QThread *threadDataWave = nullptr;

    void updateCollectDataXY(void);
    void tryUpdateFitChart(bool man);
};
#endif // MAINWINDOW_H
