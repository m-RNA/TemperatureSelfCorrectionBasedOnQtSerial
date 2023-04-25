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

    void setVerifyTracerVisible(const bool visible); // 设置是否有验证游标

public slots:
    void updateCollectPlot(const QVector<double> &x, const QVector<double> &y); // 更新散点图
    void updateFitPlot(const QVector<double> &x, const QVector<double> &y);     // 更新折线图

    void updateVerifyTracerX(const SerialAnalyseCell &x); // 更新验证游标的x坐标
    void updateVerifyTracerY(const SerialAnalyseCell &y); // 更新验证游标的y坐标

    void clear();

protected:
    // 鼠标滚轮事件
    virtual void wheelEvent(QWheelEvent *ev);
    // 鼠标点击事件
    virtual void mousePressEvent(QMouseEvent *ev);
    // 鼠标释放事件
    virtual void mouseReleaseEvent(QMouseEvent *ev);
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
    void updateVerifyTracer();

    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

private:
    QCPItemTracer *tracer = nullptr;       // 坐标
    QCPItemTracer *verifyTracer = nullptr; // 坐标跟随采集数据
    QTimer *timerVerify = nullptr;         // 定时器
    qint64 nowTime, oldTime;               // 时间戳
    double xVerify = 0;
    double yVerify = 0;
    double xRangeHalf = 0;   // x轴范围的 1/2
    double yRangeHalf = 0;   // y轴范围的 1/2
    double xRangeOfOne4 = 0; // x轴范围的 1/4
    double yRangeOfOne4 = 0; // y轴范围的 1/4
    double xRangeOfOne8 = 0; // x轴范围的 1/8
    double yRangeOfOne8 = 0; // y轴范围的 1/8

    bool mousePressFlag = 0;   // 鼠标点击标志位
    bool mouseReleaseFlag = 0; // 鼠标点击标志位

    static qint64 CHART_REFRESH_TIME_MS; // 图表刷新时间间隔

    void setVerifyTracerToCenter();
};

#endif // CUSTOMCHART_H
