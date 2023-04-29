#include "bll_data_wave.h"
#include "mainwindow.h"
#include <QDebug>

vector<AutoCollectCell> Bll_DataWave::autoCollectDataList = {};

Bll_DataWave::Bll_DataWave(QObject *parent) : QObject(parent)
{
    emit sgStableState(STABLE_STATE_INIT);
    timerWatchDog = new QTimer(this);
    timerWatchDog->setInterval(5000);
    connect(timerWatchDog, &QTimer::timeout, [&]()
            {stableState = STABLE_STATE_TIMEOUT; autoEmitStableState(); });
    timerWatchDog->start();
}

void Bll_DataWave::setRange(const double r)
{
    range = r;
}

void Bll_DataWave::setCheckNum(const int num)
{
    checkNum = num;
}

void Bll_DataWave::setStableTime(const int ms)
{
    stableTime = ms;
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

    while (data.begin()->timestamp + stableTime < data.rbegin()->timestamp)
    {
        if (data.begin()->timestamp >= min.timestamp)
        {
            min = data.at(1);
            for (int i = 2; i < data.length(); ++i)
            {
                if (data.at(i).value < min.value)
                    min = data.at(i);
            }
        }
        else if (data.begin()->timestamp >= max.timestamp)
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

    if (data.length() < checkNum)
    {
        stableState = STABLE_STATE_INIT;
        goto CHECK_STABLE;
    }

    if (cell.value > max.value)
    {
        max = cell;
        goto CHECK_MIN_MAX;
    }
    else if (cell.value < min.value)
    {
        min = cell;
        goto CHECK_MIN_MAX;
    }

    if (autoCollectState && (stableState == STABLE_STATE_STABLE))
        checkAutoCollect(cell.value);

    return;

CHECK_MIN_MAX:
    if (max.value - min.value > range)
        stableState = STABLE_STATE_UNSTABLE;
    else
        stableState = STABLE_STATE_STABLE;

CHECK_STABLE:
    autoEmitStableState();
}

void Bll_DataWave::autoEmitStableState(void)
{
    if (stableState ^ lastStableState)
    {
        emit sgStableState(stableState);
        if (stableState == STABLE_STATE_STABLE)
            emit sgTurnToStable();
    }
    lastStableState = stableState;
}

void Bll_DataWave::checkAutoCollect(const double &value)
{
    // qDebug() << "checkAutoCollect" << value;
    autoCollectState = false;

    if (autoCollectDataList.size() == 0)
        return;

    for (auto &cell : autoCollectDataList)
    {
        // 跳过已经收集过的数据
        if (cell.isCollect)
            continue;

        // 检测是否在允许范围内
        if (cell.value - autoCollectRange < value && cell.value + autoCollectRange > value)
        {
            qDebug() << "checkAutoCollect" << cell.value;
            emit sgAutoCollect();
            // 自动收集数据职责已经到达
            // 若人为停止采集，则不会自动采集该点
            cell.isCollect = true;
        }
    }
}

void Bll_DataWave::setAutoCollectStart()
{
    autoCollectState = true;
}

void Bll_DataWave::setAutoCollectDataList(const vector<double> &list)
{
    autoCollectDataList.clear();
    for (auto &value : list)
    {
        autoCollectDataList.push_back({value, false});
        qDebug() << "setAutoCollectDataList" << value;
    }
    qDebug() << "setAutoCollectDataList" << autoCollectDataList.size();
}

void Bll_DataWave::setAutoCollectRange(const double &range)
{
    autoCollectRange = range;
}
