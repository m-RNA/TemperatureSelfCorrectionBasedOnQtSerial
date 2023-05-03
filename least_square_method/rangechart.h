#ifndef RANGECHART_H
#define RANGECHART_H

#include <QObject>
#include "qcustomplot.h"
using namespace std;

class RangeChart : public QCustomPlot
{
    Q_OBJECT
public:
    RangeChart(QWidget *parent);
    ~RangeChart();

    void setData(const vector<double> &std, const vector<double> &dtm);

    void setColorStyle(const int style);

private:
    QCPBars *bar_Std = nullptr;
    QVector<double> x;

    void setAxisColor(const QColor &color, const QColor &selectedColor);
    void setColor(const QColor &background, const QColor &foreground, const QColor &selectedColor);
};

#endif // RANGECHART_H
