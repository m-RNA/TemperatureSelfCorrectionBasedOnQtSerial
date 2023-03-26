#ifndef FITCHART_H
#define FITCHART_H

#include <QWidget>
#include "qcustomplot.h"
#include "bll_serialrecvanalyse.h"

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

    void setVerifyTracerVisible(const bool visible); // 设置是否有验证游标
    void setColorStyle(const int style);

public slots:
    void updateCollectPlot(const QVector<double> &x, const QVector<double> &y); // 更新散点图
    void updateFitPlot(const QVector<double> &x, const QVector<double> &y);     // 更新折线图

    void updateVerifyTracerX(const serialAnalyseCell &x); // 更新验证游标的x坐标
    void updateVerifyTracerY(const serialAnalyseCell &y); // 更新验证游标的y坐标

    void clear();

protected:
    // 鼠标滚轮事件
    virtual void wheelEvent(QWheelEvent *ev);
    // 鼠标点击事件
    virtual void mousePressEvent(QMouseEvent *ev);
    // 鼠标移动事件
    virtual void mouseMoveEvent(QMouseEvent *ev);

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
    QCPItemTracer *tracer = nullptr;       // 坐标跟随鼠标
    QCPItemTracer *verifyTracer = nullptr; // 坐标跟随采集数据
    double xVerify = 0;
    double yVerify = 0;

    int x_default = 0;

    void setAxisColor(const QColor &color);
    void setColor(const QColor &background, const QColor &foreground);
};

#endif // CUSTOMCHART_H
