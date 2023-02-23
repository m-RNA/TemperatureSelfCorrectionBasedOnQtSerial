#ifndef INTERACTCHART_H
#define INTERACTCHART_H

#include <QWidget>
#include "qcustomplot.h"
#include <sys/timeb.h>

namespace Ui
{
    class InteractChart;
}

class InteractChart : public QCustomPlot
{
    Q_OBJECT

public:
    explicit InteractChart(QWidget *parent = nullptr);

    ~InteractChart();

    void setDeviceName(QString name)
    {
        deviceName = name;
        this->graph()->setName(QString(deviceName));
    }

    void chartRefresh(void);

    void setXAxisToTimelineState(bool state);

public slots:
    void clear();

    void addYPoint(double y);
    void addYPointBaseOnCurrentTime(double y);

protected:
    // 鼠标滚轮事件
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
    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

    void chartStart() { pauseState = false; } // 开始绘图
    void chartPause() { pauseState = true; }  // 暂停绘图

    void yAxisAutoZoomYes() { yAxisAutoZoomState = true; } // 开启Y轴自动缩放
    void yAxisAutoZoomNo() { yAxisAutoZoomState = false; } // 关闭Y轴自动缩放

private:
    QCPItemTracer *tracer = nullptr; // 坐标跟随鼠标.使用时创建

    QString deviceName = "未知仪器";
    int xDefault = -1;
    bool pauseState = false;
    bool yAxisAutoZoomState = true;
    bool timelineState = false;
    timeb t1, t2;
    int nowTime, oldTime;
};

#endif // INTERACTCHART_H
