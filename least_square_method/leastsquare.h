#ifndef LEASTSQUARE_H
#define LEASTSQUARE_H

#include <QWidget>
#include <QTableWidgetItem>
#include "bll_leastssquare.h"

namespace Ui
{
    class LeastSquare;
}

class LeastSquare : public QWidget
{
    Q_OBJECT

public:
    explicit LeastSquare(QWidget *parent = nullptr);
    ~LeastSquare();

public slots:

private slots:
    void on_spbxSamplePointSum_valueChanged(int arg1);

    void on_spbxOrder_valueChanged(int arg1);

    void on_btnFit_clicked();

    void on_twAverage_itemSelectionChanged();

    void on_twAverage_itemChanged(QTableWidgetItem *item);

    void setFitChartData(QVector<double> factor);

signals:
    void collectDataXYChanged(QVector<double> x, QVector<double> y);
    void fitDataChanged(QVector<double> x, QVector<double> y);

    void startGenerate(int t_left, int t_right, double t_step, QVector<double> t_factor);
    void startLeastSquare(int t_N, QVector<double> t_x, QVector<double> t_y);

private:
    Ui::LeastSquare *ui;
    int order;          // 最小二乘法多项式阶数
    int samplePointSum; // 标定点数
    QVector<double> collectDataX, collectDataY;
    double collectDataX_Max, collectDataX_Min;
    QVector<double> fitDataX, fitDataY;
    QRegExp rx;
    QString old_text = "";

    // 任务类对象
    Bll_GenerateData *taskGen;
    Bll_LeastSquareMethod *taskLeastSquare;

    void updateCollectDataXY(void);
    void tryUpdateFitChart(bool man);
};

#endif // LEASTSQUARE_H
