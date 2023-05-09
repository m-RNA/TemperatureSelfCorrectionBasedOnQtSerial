#include "rangechart.h"
#include <QDebug>

RangeChart::RangeChart(QWidget *parent) : QCustomPlot(parent)
{
    setOpenGl(true);
    qDebug() << "RangeChart    OpenGL=" << openGl();
    legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    legend->setFont(legendFont);
    legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    bar_Std = new QCPBars(xAxis, yAxis);
    bar_Std->setWidth(0.5);
    bar_Std->setPen(Qt::NoPen);
    bar_Std->setBrush(QColor(10, 140, 70, 160));
    bar_Std->setName("标准");

    // setup for graph 1: key axis bottom, value axis left (those are the default axes)
    // will contain bottom maxwell-like function with error bars
    addGraph(xAxis, yAxis2);
    graph(0)->setPen(QPen(Qt::gray, 2.5));
    graph(0)->setLineStyle(QCPGraph::lsLine);
    graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::gray, Qt::white, 7));
    graph(0)->setName("待定");

    // activate top and right axes, which are invisible by default:
    yAxis2->setVisible(true);

    // set labels:
    xAxis->setLabel("采集序号");
    yAxis->setLabel("标准极差");
    yAxis2->setLabel("待定极差");

    // make ticks on bottom axis go outward:
    // xAxis->setTickLength(0, 5);
    // xAxis->setSubTickLength(0, 3);
    // make ticks on right axis go inward and outward:
    yAxis2->setTickLength(3, 3);
    yAxis2->setSubTickLength(1, 1);

    // 结尾带箭头
    yAxis->setUpperEnding(QCPLineEnding::esLineArrow);
    yAxis2->setUpperEnding(QCPLineEnding::esLineArrow);
}

RangeChart::~RangeChart()
{
    bar_Std->deleteLater();
}

void RangeChart::setData(const vector<double> &r_Std, const vector<double> &r_Dtm)
{
    int size = r_Std.size() < r_Dtm.size() ? r_Dtm.size() : r_Std.size();
    if (x.size() < size)
    {
        x.resize(size);
        for (int i = 0; i < size; ++i)
        {
            x[i] = i + 1;
        }
    }
    else if (x.size() > size)
    {
        x.resize(size);
    }
    QVector<double> q_Std = QVector<double>(r_Std.begin(), r_Std.end());
    QVector<double> q_Dtm = QVector<double>(r_Dtm.begin(), r_Dtm.end());

    bar_Std->setData(x, q_Std);
    graph(0)->setData(x, q_Dtm);

    // 设置坐标轴范围
    yAxis->setRange(0, *max_element(r_Std.begin(), r_Std.end()) * 1.3);
    yAxis2->setRange(0, *max_element(r_Dtm.begin(), r_Dtm.end()) * 1.3);
    xAxis->setRange(0, size + 1);

    replot();
}

void RangeChart::setAxisColor(const QColor &color, const QColor &selectedColor = Qt::blue)
{
    Q_UNUSED(selectedColor);
    xAxis->setBasePen(QPen(color));
    yAxis->setBasePen(QPen(color));
    xAxis->setTickPen(QPen(color));
    yAxis->setTickPen(QPen(color));
    xAxis->setSubTickPen(QPen(color));
    yAxis->setSubTickPen(QPen(color));
    xAxis->setLabelColor(color);
    yAxis->setLabelColor(color);
    xAxis->setTickLabelColor(color);
    yAxis->setTickLabelColor(color);

    xAxis2->setBasePen(QPen(color));
    yAxis2->setBasePen(QPen(color));
    xAxis2->setTickPen(QPen(color));
    yAxis2->setTickPen(QPen(color));
    xAxis2->setSubTickPen(QPen(color));
    yAxis2->setSubTickPen(QPen(color));
    xAxis2->setLabelColor(color);
    yAxis2->setLabelColor(color);
    xAxis2->setTickLabelColor(color);
    yAxis2->setTickLabelColor(color);

    // xAxis->setSelectedBasePen(QPen(selectedColor, 2));
    // yAxis->setSelectedBasePen(QPen(selectedColor, 2));
    // xAxis->setSelectedTickPen(QPen(selectedColor, 2));
    // yAxis->setSelectedTickPen(QPen(selectedColor, 2));
    // xAxis->setSelectedSubTickPen(QPen(selectedColor, 2));
    // yAxis->setSelectedSubTickPen(QPen(selectedColor, 2));
    // xAxis->setSelectedLabelColor(selectedColor);
    // yAxis->setSelectedLabelColor(selectedColor);
    // xAxis->setSelectedTickLabelColor(selectedColor);
    // yAxis->setSelectedTickLabelColor(selectedColor);

    // xAxis2->setSelectedBasePen(QPen(selectedColor, 2));
    // yAxis2->setSelectedBasePen(QPen(selectedColor, 2));
    // xAxis2->setSelectedTickPen(QPen(selectedColor, 2));
    // yAxis2->setSelectedTickPen(QPen(selectedColor, 2));
    // xAxis2->setSelectedSubTickPen(QPen(selectedColor, 2));
    // yAxis2->setSelectedSubTickPen(QPen(selectedColor, 2));
    // xAxis2->setSelectedLabelColor(selectedColor);
    // yAxis2->setSelectedLabelColor(selectedColor);
    // xAxis2->setSelectedTickLabelColor(selectedColor);
    // yAxis2->setSelectedTickLabelColor(selectedColor);
}

void RangeChart::setColor(const QColor &background, const QColor &foreground, const QColor &selectedColor = Qt::blue)
{
    setBackground(QBrush(background));
    legend->setBrush(QBrush(background));

    setAxisColor(foreground, selectedColor);
    legend->setTextColor(foreground);
    legend->setBorderPen(QPen(foreground));

    // legend->setSelectedIconBorderPen(QPen(selectedColor, 2));
    // legend->setSelectedTextColor(selectedColor);
    // legend->setSelectedBrush(QBrush(selectedColor));

    // selectedDec0->setPen(QPen(selectedColor, 2));
    // selectedDec1->setPen(QPen(selectedColor, 2));
}

void RangeChart::setColorStyle(const int style)
{
    if (style < 0 || style > 3)
        return;

    switch (style)
    {
    case 0: // 白色
        setColor(QColor("#FFFFFF"), Qt::black);
        break;

    case 1: // 灰色
        setColor(QColor("#FFFFFF"), QColor("#57595B"));
        break;

    case 2: // 清凉
        setColor(QColor("#EAF7FF"), QColor("#386487"));
        break;

    case 3: // 深色
        setColor(QColor("#444444"), QColor("#DCDCDC"), QColor("#DCDC00"));
        break;

    default:
        setColor(QColor("#FFFFFF"), Qt::black);
        break;
    }

    /* RangeChart 独有*/
    // if (style == 3) // 深色
    // {
    //     graph(0)->setPen(QPen(QColor("#2ECC71"), 2.5, Qt::PenStyle::DotLine));
    // }
    // else // 浅色
    // {
    //     graph(0)->setPen(QPen(Qt::darkGreen, 2.5, Qt::PenStyle::DotLine));
    // }

    replot();
}