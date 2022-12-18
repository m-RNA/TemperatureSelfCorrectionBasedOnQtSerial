#include "leastsquare.h"
#include "fitchart.h"
#include "ui_leastsquare.h"
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QThread>

/*
    qt中获取容器Vector中的最大值和最小值：
    https://blog.csdn.net/Littlehero_121/article/details/100565527
*/
double max(QVector<double> &data)
{
    auto max = std::max_element(std::begin(data), std::end(data));
    return *max;
}

double min(QVector<double> &data)
{
    auto min = std::min_element(std::begin(data), std::end(data));
    return *min;
}

LeastSquare::LeastSquare(QWidget *parent) : QWidget(parent),
                                            ui(new Ui::LeastSquare)
{
    ui->setupUi(this);

    // 1. 创建任务类对象
    taskGen = new Bll_GenerateData(this);
    taskLeastSquare = new Bll_LeastSquareMethod(this);

    samplePointSum = ui->twAverage->rowCount();
    order = ui->spbxOrder->value();

    ui->twAverage->horizontalHeader()->setVisible(true);
    ui->twAverage->verticalHeader()->setVisible(true);
    ui->twAverage->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twAverage->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整
    ui->twFactor->horizontalHeader()->setVisible(true);

    ui->twFactor->verticalHeader()->setVisible(true);
    ui->twFactor->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应缩放
    ui->twFactor->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);     // 不可调整

    QStringList HorizontalHeader;
    HorizontalHeader << "X:标准平均";
    HorizontalHeader << "Y:待定平均";
    ui->twAverage->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    HorizontalHeader.clear();
    HorizontalHeader << "N-1 阶系数";
    ui->twFactor->setHorizontalHeaderLabels(HorizontalHeader); // 设置表头

    for (int i = 0; i < samplePointSum; i++) // 需要初始化表格Item
    {
        for (int j = 0; j < 2; j++)
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            ui->twAverage->setItem(i, j, temp);
            ui->twAverage->item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }

    connect(this, &LeastSquare::collectDataXYChanged, ui->chartFit, &FitChart::updateCollectPlot);

    // 2. 链接子线程
    connect(this, &LeastSquare::startGenerate, taskGen, &Bll_GenerateData::setGenerateFitData);
    connect(taskGen, &Bll_GenerateData::generateFitDataFinish, ui->chartFit, &FitChart::updateFitPlot);

    connect(this, &LeastSquare::startLeastSquare, taskLeastSquare, &Bll_LeastSquareMethod::setLeastSquareMethod);
    connect(taskLeastSquare, &Bll_LeastSquareMethod::leastSquareMethodFinish, this, &LeastSquare::setFitChartData);
}

LeastSquare::~LeastSquare()
{
    delete ui;
}

void LeastSquare::updateCollectDataXY(void)
{
    QString qsX, qsY;
    collectDataX.clear(); // 重置x容器
    collectDataY.clear(); // 重置y容器
    for (int i = 0; i < samplePointSum; i++)
    {
        qsX = ui->twAverage->item(i, 0)->text();
        qsY = ui->twAverage->item(i, 1)->text();
        if (qsX.isEmpty() || qsY.isEmpty())
            continue;
        // qDebug() << "counter" << counter;

        collectDataX << qsX.toDouble();
        collectDataY << qsY.toDouble();
        qDebug() << i << ":" << qsX << qsY;
    }
}

// 人为 true 自动 false
void LeastSquare::tryUpdateFitChart(bool man)
{
    updateCollectDataXY();
    if (collectDataX.length() < 2)
    {
        if (man) // 是人为的就要提醒一下
            QMessageBox::critical(this, "错误", "正确格式的数据\n小于两组");
        return;
    }

    // N个点可以确定一个 唯一的 N-1 阶的曲线
    order = ui->spbxOrder->text().toInt();
    if (collectDataX.length() <= order)
        order = collectDataX.length() - 1;

    // 启动子线程
    emit startLeastSquare(order, collectDataX, collectDataY);
    QThreadPool::globalInstance()->start(taskLeastSquare);
}

void LeastSquare::on_spbxSamplePointSum_valueChanged(int arg1)
{
    ui->twAverage->setRowCount(arg1);
    if (arg1 > samplePointSum)
    {
        for (int j = 0; j < 2; j++) // 需要初始化表格Item
        {
            QTableWidgetItem *temp = new QTableWidgetItem();
            int row = arg1 - 1;
            ui->twAverage->setItem(row, j, temp);
            ui->twAverage->item(row, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    samplePointSum = arg1;
    qDebug() << "samplePointSum:" << samplePointSum;
}

void LeastSquare::on_spbxOrder_valueChanged(int arg1)
{
    order = arg1;
    ui->twFactor->setRowCount(arg1 + 1);
    qDebug() << "order:" << order;

    tryUpdateFitChart(false);
}

void LeastSquare::on_btnFit_clicked()
{
    tryUpdateFitChart(true);
}

void LeastSquare::setFitChartData(QVector<double> factor)
{
    for (int i = 0; i <= order; i++)
    {
        // 通过setItem来改变条目
        QTableWidgetItem *temp = new QTableWidgetItem(QString::number(factor.at(order - i)));
        ui->twFactor->setItem(i, 0, temp);
    }
    collectDataX_Max = max(collectDataX);
    collectDataX_Min = min(collectDataX);
    double addRange = (collectDataX_Max - collectDataX_Min) / 4.0;

    // 启动子线程
    emit startGenerate(collectDataX_Min - addRange, collectDataX_Max + addRange, 0.25, factor); // fitDataX, fitDataY);
    QThreadPool::globalInstance()->start(taskGen);
}

void LeastSquare::on_twAverage_itemSelectionChanged()
{
    // 1、记录旧的单元格内容
    old_text = ui->twAverage->item(ui->twAverage->currentRow(),
                                   ui->twAverage->currentColumn())
                   ->text();
}

/*
 * 正则表达式：
 * https://blog.csdn.net/qq_41622002/article/details/107488528
 */
void LeastSquare::on_twAverage_itemChanged(QTableWidgetItem *item)
{
    // 2、匹配正负整数、正负浮点数
    QString Pattern("(-?[1-9][0-9]+)|(-?[0-9])|(-?[1-9]\\d+\\.\\d+)|(-?[0-9]\\.\\d+)"); // 正则表达式
    QRegExp reg(Pattern);

    // 3.获取修改的新的单元格内容
    QString str = item->text();

    // 完全匹配
    if (reg.exactMatch(str))
    {
        qDebug() << "匹配成功";
        int row = item->row();
        // old_text = ""; // 替换为空
        if (ui->twAverage->item(row, 0)->text().isEmpty() ||
            ui->twAverage->item(row, 1)->text().isEmpty())
            return; // 当有一格为空时，退出

        updateCollectDataXY();
        if (collectDataX.length() > 0)
            emit collectDataXYChanged(collectDataX, collectDataY);
    }
    else
    {
        qDebug() << "匹配失败";
        item->setText(old_text); // 更换之前的内容
    }
}
