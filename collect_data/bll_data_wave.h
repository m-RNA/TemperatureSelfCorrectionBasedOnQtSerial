#pragma once

#include <QWidget>
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

signals:
    void sgTurnToStable();
    void sgStableState(bool);
    void sgReceiveNull();

private:
    int interval = 0;
    double range = 0;
    bool stableState = false;
    bool lastStableState = false;
    QTimer *timerWatchDog = nullptr;

    SerialAnalyseCell min;
    SerialAnalyseCell max;
    QVector<SerialAnalyseCell> data;
};
