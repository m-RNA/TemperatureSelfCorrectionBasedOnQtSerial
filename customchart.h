#ifndef CUSTOMCHART_H
#define CUSTOMCHART_H

#include <QWidget>
#include "qcustomplot.h"

namespace Ui
{
    class CustomChart;
}

class CustomChart : public QWidget
{
    Q_OBJECT

public:
    explicit CustomChart(QWidget *parent = nullptr);
    ~CustomChart();

public slots:
    void addYPoint(double y);
    void addPoint(double x, double y);

    void addVLine(double x);
    void addHLine(double y);

    void clear();

private slots:
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void axisXYDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();

    void findGraph();

    void removeSelectedGraph();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

private:
    Ui::CustomChart *ui;
    unsigned int x_default = 0;
    double xRange = 80;
};

#endif // CUSTOMCHART_H
