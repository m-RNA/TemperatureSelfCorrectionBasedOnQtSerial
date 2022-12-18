#ifndef FITCHART_H
#define FITCHART_H

#include <QWidget>
#include "qcustomplot.h"
#include "charttracer.h"

namespace Ui
{
    class FitChart;
}

class FitChart : public QCustomPlot
{
    Q_OBJECT

public:
    explicit FitChart(QWidget *parent = nullptr);
    ~FitChart();

public slots:
    void updateCollectPlot(QVector<double> x, QVector<double> y); // 更新散点图
    void updateFitPlot(QVector<double> x, QVector<double> y);     // 更新折线图

    void addVLine(double x);
    void addHLine(double y);

    void clear();

protected:
    // 滚轮
    void wheelEvent(QWheelEvent *ev);
    // 鼠标点击事件
    void mousePressEvent(QMouseEvent *ev);
    // 鼠标移动事件
    void mouseMoveEvent(QMouseEvent *ev);

private slots:
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void axisXYDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();

    void moveLegend();
    void findGraph();
    void hideCollectPlot();
    void showCollectPlot();
    void hideFitPlot();
    void showFitPlot();

    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

private:
    ChartTracer *mxTracer = nullptr; // 坐标跟随鼠标.使用时创建

    int x_default = 0;
};

#endif // CUSTOMCHART_H
