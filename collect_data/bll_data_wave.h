#pragma once

#include <QObject>
#include <QTimer>
#include "bll_serialrecvanalyse.h"

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

signals:
    void sgTurnToStable();
    void sgStableState(const char state);
    void sgReceiveTimeout();

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
    void autoSendStableState(void);
};
