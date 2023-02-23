#ifndef BLL_LEASTSSQUARE_H
#define BLL_LEASTSSQUARE_H
#include "config.h"

#include <QRunnable>
#include <QObject>
#include <QVector>
#include <vector>
using namespace std;

// QRunnable的run是public
class Bll_GenerateData : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_GenerateData(QObject *parent = nullptr);

    void run() override;

public slots:
    void setGenerateFitData(int t_left, int t_right, DECIMAL_TYPE t_step,
                            vector<DECIMAL_TYPE> t_factor)
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
    DECIMAL_TYPE step;
    vector<DECIMAL_TYPE> factor;
};

class Bll_LeastSquareMethod : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_LeastSquareMethod(QObject *parent = nullptr);

    void run() override;

public slots:
    void setLeastSquareMethod(int t_N, vector<DECIMAL_TYPE> t_x, vector<DECIMAL_TYPE> t_y)
    {
        N = t_N;
        x = t_x;
        y = t_y;
    }
signals:
    void leastSquareMethodFinish(vector<DECIMAL_TYPE> factor); // f

private:
    unsigned long long N;
    vector<DECIMAL_TYPE> x;
    vector<DECIMAL_TYPE> y;
};

#endif // BLL_LEASTSSQUARE_H
