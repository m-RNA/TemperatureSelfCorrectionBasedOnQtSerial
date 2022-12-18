#ifndef BLL_LEASTSSQUARE_H
#define BLL_LEASTSSQUARE_H

#include <QRunnable>
#include <QObject>
#include <QVector>

// QRunnable的run是public
class Bll_GenerateData : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_GenerateData(QObject *parent = nullptr);

    void run() override;

public slots:
    void setGenerateFitData(int t_left, int t_right, double t_step,
                            QVector<double> t_factor)
    {
        left = t_left;
        right = t_right;
        step = t_step;
        factor = t_factor;
    }
signals:
    void generateFitDataFinish(QVector<double> x, QVector<double> y); // f

private:
    int left;
    int right;
    double step;
    QVector<double> factor;
};

class Bll_LeastSquareMethod : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_LeastSquareMethod(QObject *parent = nullptr);

    void run() override;

public slots:
    // QVector<double> method00

    void setLeastSquareMethod(int t_N, QVector<double> t_x, QVector<double> t_y)
    {
        N = t_N;
        x = t_x;
        y = t_y;
    }
signals:
    void leastSquareMethodFinish(QVector<double> factor); // f

private:
    int N;
    QVector<double> x;
    QVector<double> y;
};

#endif // BLL_LEASTSSQUARE_H
