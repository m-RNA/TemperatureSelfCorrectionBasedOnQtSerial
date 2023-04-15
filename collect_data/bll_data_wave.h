#pragma once

#include <QObject>
#include <QTimer>
#include "bll_serialrecvanalyse.h"
using namespace std;

typedef struct
{
    double value;
    bool isCollect;
} AutoCollectCell;

class Bll_DataWave : public QObject
{
    Q_OBJECT
public:
    explicit Bll_DataWave(QObject *parent = nullptr);

public slots:
    void addData(const SerialAnalyseCell &cell);
    void setRange(const double r);
    void setInterval(const int ms);
    void setCheckNum(const int num);

    void setAutoCollectStart();
    static void setAutoCollectDataList(const vector<double> &list);
    void setAutoCollectRange(const double &range);

signals:
    void sgTurnToStable();
    void sgStableState(const char state);
    void sgReceiveTimeout();

    void sgAutoCollect();

private:
    int interval = 0;
    int checkNum = 10;
    double range = 0;
    char stableState = false;
    char lastStableState = false;
    QTimer *timerWatchDog = nullptr;
    SerialAnalyseCell min;
    SerialAnalyseCell max;
    QVector<SerialAnalyseCell> data;
    void autoEmitStableState(void);

    bool autoCollectState = false;
    double autoCollectRange = 1;
    void checkAutoCollect(const double &value);
    static vector<AutoCollectCell> autoCollectDataList;
};
