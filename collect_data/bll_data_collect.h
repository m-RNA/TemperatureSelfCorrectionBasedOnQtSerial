#pragma once

#include <QObject>
#include <vector>
#include <string>
#include "bll_serialrecvanalyse.h"
using namespace std;

class Bll_DataCollect : public QObject
{
    Q_OBJECT
public:
    explicit Bll_DataCollect(QObject *parent = nullptr);

public slots:
    void slAddData(const SerialAnalyseCell &cell); // 添加数据
    void slCollectFinish(void);                    // 完成采集

signals:
    void sgAverage(const string &average);      // 发送平均值
    void sgRange(const double &range);          // 发送极差
    void sgCollectData(const vector<double> &); // 发送一组数据
    void sgTaskFinish(void);                    // 任务完成

private:
    vector<double> data;
    double max;
    double min;

    string average(void);
};
