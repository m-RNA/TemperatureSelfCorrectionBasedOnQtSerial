#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include <vector>
#include <string>
#include "interactchart.h"
#include "bll_serialrecvanalyse.h"
using namespace std;

namespace Ui
{
    class CollectPanel;
}

class Bll_Average : public QObject
{
    Q_OBJECT
public:
    explicit Bll_Average(QObject *parent = nullptr);

public slots:
    void slAverage(const vector<double> &data);

signals:
    void sgAverage(const string &average);
};

class CollectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CollectPanel(QWidget *parent = nullptr);
    ~CollectPanel();

    void setDeviceName(QString name);
    void setOnlineState(bool state);
    void setCheckWaveState(bool check);
    void setCheckWaveNum(int num);
    void setCheckWaveRange(const double range);

    bool isStable(void);
    double getRange(void);
    QCPAxis *getXAxis(void);

    void collectStart(void);
    void collectStop(void);
    void collectFinish(void);
    void collectRestart(void);

public slots:
    void slCollectData(const serialAnalyseCell &cell);

signals:
    void sgCollectDataAverage(const string &average);

    void sgCollectDataGet(const vector<double> &data);

    void sgTurnToStable();

    void sgDataAverage(const vector<double> &data);

private slots:

private:
    Ui::CollectPanel *ui;
    QString deviceName;
    vector<double> data;
    bool collectState = false;
    int uiLedState = 0;

    double min = 0;
    double max = 0;
    double currentRange = 0;
    double commandRange = 0.01;
    double checkWaveNum = 10;
    bool resetRange = true;

    bool onlineState = false;
    bool checkWaveState = false;
    bool stableState = false;
    bool laseStableState = false;
    QVector<double> dataWave;
    Bll_Average *taskAverage = nullptr;
    QThread *threadAverage = nullptr;

    void setState(int state);
    void checkDataWave(const double &data);
};



#endif // COLLECTPANEL_H
