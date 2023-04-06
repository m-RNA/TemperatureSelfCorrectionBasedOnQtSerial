#include "bll_data_collect.h"
#include "BigFloat.h"

Bll_DataCollect::Bll_DataCollect(QObject *parent) : QObject(parent)
{
}

void Bll_DataCollect::slAddData(const SerialAnalyseCell &cell)
{
    data.push_back(cell.value);

    if (data.size() == 1)
    {
        max = cell.value;
        min = cell.value;
        return;
    }

    if (cell.value > max)
    {
        max = cell.value;
        emit sgRange(max - min);
    }
    else if (cell.value < min)
    {
        min = cell.value;
        emit sgRange(max - min);
    }
}

void Bll_DataCollect::slCollectFinish(void)
{
    emit sgCollectData(data);
    emit sgAverage(average());
    emit sgTaskFinish();
}

string Bll_DataCollect::average(void)
{
    // BigFloat计算平均值，模拟笔算，精度更高
    BigFloat sum("0");
    for (auto &d : data)
    {
        sum += BigFloat(d);
    }
    return (sum / (double)data.size()).toString(10);
}
