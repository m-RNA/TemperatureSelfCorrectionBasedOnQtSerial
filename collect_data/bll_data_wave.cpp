#include "bll_data_wave.h"
#include <QDebug>

Bll_DataWave::Bll_DataWave(QObject *parent) : QObject(parent)
{
    timerWatchDog = new QTimer(this);
    timerWatchDog->setInterval(5000);
    connect(timerWatchDog, &QTimer::timeout, [=]()
            { emit sgReceiveNull(); });
    timerWatchDog->start();
}

void Bll_DataWave::setRange(const double r)
{
    range = r;
}

void Bll_DataWave::setInterval(const int ms)
{
    interval = ms;
}

// 一定时间范围的波动范围检测
void Bll_DataWave::addData(const SerialAnalyseCell &cell)
{
    timerWatchDog->start();
    data.push_back(cell);

    // 第一个数据
    if (data.length() == 1)
    {
    FIRST:
        max = cell;
        min = cell;
        return;
    }

    while (data.begin()->moment + interval < data.rbegin()->moment)
    {
        if (data.begin()->moment >= min.moment)
        {
            min = data.at(1);
            for (int i = 2; i < data.length(); ++i)
            {
                if (data.at(i).value < min.value)
                    min = data.at(i);
            }
        }
        else if (data.begin()->moment >= max.moment)
        {
            max = data.at(1);
            for (int i = 2; i < data.length(); ++i)
            {
                if (data.at(i).value > max.value)
                    max = data.at(i);
            }
        }
        data.pop_front();
    }

    if (data.length() == 1)
        goto FIRST;

    if (cell.value > max.value)
    {
        max = cell;
        goto CHECK_STABLE;
    }
    else if (cell.value < min.value)
    {
        min = cell;
        goto CHECK_STABLE;
    }

    return;

CHECK_STABLE:
    if (max.value - min.value > range)
        stableState = false;
    else
        stableState = true;

    if (stableState ^ lastStableState)
    {
        if (stableState)
            emit sgTurnToStable();
        emit sgStableState(stableState);
    }
    lastStableState = stableState;
}
