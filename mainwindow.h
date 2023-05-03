#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTableWidgetItem>
#include <QLabel>
#include "bll_leastssquare.h"
#include "bll_save_data_to_xlsx.h"
#include "bll_sound.h"
#include "bll_data_wave.h"
#include "bll_data_collect.h"
#include "wizard.h"

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

    void setDeviceName_Std(const QString &name);
    void setDeviceName_Dtm(const QString &name);

    static int getCollectIndex(void);

public slots:
    void showMessage(const QString &msg, int timeout);

private slots:
    void on_btnCollectSwitch_clicked();

    void on_btnCollectRestart_clicked();

    void on_spbxSamplePointSum_valueChanged(int arg1);

    void on_spbxOrder_valueChanged(int arg1);

    void on_btnFit_clicked();

    void on_twAverage_itemSelectionChanged();

    void twAverage_itemChanged(QTableWidgetItem *item);

    void setOrderData(const vector<DECIMAL_TYPE> &factor);

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

    void on_spbxStableTime_valueChanged(double arg1);

    void on_btnSwitchView_clicked();

    void on_btnWizard_clicked();

    void on_cbAutoCollect_activated(int index);

    void on_actionLock_triggered(bool checked);

    void on_actionLightStyle_triggered();

    void on_actionGrayStyle_triggered();

    void on_actionBlueStyle_triggered();

    void on_actionDarkStyle_triggered();

    void on_actionQuit_triggered();

signals:
    void sgXlsxStartPoint();
    void sgXlsxSaveReport();
    void sgXlsxSetAutoSave(const bool);
    void sgXlsxSaveInfo(const BaseInfo &info);

    void sgSoundPlay1(const SoundIndex &index);
    void sgSoundPlay2(const SoundIndex &index);
    void sgSoundStop();

    void sgSetDataWaveNum(const int num);
    void sgSetDataWaveRange(const double range);
    void sgSetDataWaveStableTime(const int ms);

    void sgCollectDataFinish_Std();
    void sgCollectDataFinish_Dtm();

    void collectDataXYChanged(const QVector<double> &x, const QVector<double> &y);
    void fitDataChanged(const vector<double> &x, const vector<double> &y);

    void startGenerate(DECIMAL_TYPE t_left, DECIMAL_TYPE t_right, const DECIMAL_TYPE t_step, const vector<DECIMAL_TYPE> &t_factor);
    void startLeastSquare(const int t_N, const vector<DECIMAL_TYPE> &t_x, const vector<DECIMAL_TYPE> &t_y);

private:
    Ui::MainWindow *ui;
    QTimer *timerCollect = nullptr;
    QTimer *timerAutoCollectCheck = nullptr;
    CollectBtnState btnSwitchState = CollectBtnState_Start;
    bool isCollecting = false;
    bool waitingStdStable = false;
    int collectTimeStamp = 0;  // 采集时间戳（秒 * TIMESTAMP_FACTOR)
    static int collectCounter; // 已采集点数
    static int collectIndex;   // 采集序号
    static bool returnCollectIndex;   // 是否返回序号
    int samplePointSum = 8;    // 需要采集点数
    int pgsbSingleValue = 0;   // 单点进度条值

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

    void setCollectBtnState(const CollectBtnState &state);
    void setCollectSettingLock(bool lock);

    void pictureInit(int);
    void shortcutInit();
    void statusBarInit();
    void soundInit();
    void listenDataWaveInit();
    void listenDataWaveQuit();
    void collectDataInit();
    void collectDataQuit();
    void autoCollectTimerInit();
    void autoCollectTimerQuit();
    void leastSquareTaskStart(const int order, const vector<DECIMAL_TYPE> &x, const vector<DECIMAL_TYPE> &y);

    size_t order;                // 最小二乘法多项式阶数
    vector<DECIMAL_TYPE> factor; // 系数
    QTimer *timerOrderChangeDelayUpdate = nullptr;
    vector<DECIMAL_TYPE> collectDataX, collectDataY;
    vector<DECIMAL_TYPE> fitDataX, fitDataY;
    vector<double> rangeList_Std, rangeList_Dtm;
    QRegExp rx;
    QString old_text = "";
    int soundIndex = 0;
    bool calibrateViewState = false;
    WizardInfo wizardInfo;

    // 状态栏相关
    QLabel *lbRunTime = nullptr;
    QLabel *lbLastRange = nullptr;
    QLabel *ledErrorRange = nullptr;
    QLabel *lbVerifyData = nullptr;
    QTimer *timerRunTime = nullptr;
    QTimer *timerUpdateStatusBarVerifyData = nullptr;

    // 任务类对象
    Bll_LeastSquareMethod *taskLeastSquare = nullptr;
    QThread *threadLeastSquare = nullptr;

    Bll_SaveDataToXlsx *taskXlsxData = nullptr;
    QThread *threadXlsx = nullptr;

    Bll_Sound *taskSound = nullptr;
    QThread *threadSound = nullptr;

    Bll_DataWave *taskDataWave = nullptr;
    QThread *threadDataWave = nullptr;

    Bll_DataCollect *taskDataCollect_Std = nullptr;
    Bll_DataCollect *taskDataCollect_Dtm = nullptr;
    QThread *threadDataCollect = nullptr;

    void updateRunTime();
    void updateCollectDataXY();

    void setLEDErrorRange(double range);
    void tryUpdateFitChart(bool man);
    void updateStatusBarVerifyData();
    void switchCalibrateView(int index);

    void loadUiSettings();
    void saveUiSetting();

    int themeIndex = 0;
    void setColorStyle(const int index);
    void setChartColorStyle(const int index);
    void loadStyle(const QString &qssFile);

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
