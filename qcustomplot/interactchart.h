#ifndef INTERACTCHART_H
#define INTERACTCHART_H

#include <QWidget>
#include "qcustomplot.h"
#include "charttracer.h"

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

public slots:
    void clear();

    void addYPoint(double y);

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

    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

private:
    ChartTracer *mxTracer = nullptr; // 坐标跟随鼠标.使用时创建

    int x_default = 0;
    QString deviceName = "未知仪器";
};

#endif // INTERACTCHART_H
