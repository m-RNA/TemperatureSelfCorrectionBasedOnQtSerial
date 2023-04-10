#ifndef BLL_LEASTSSQUARE_H
#define BLL_LEASTSSQUARE_H
#include "config.h"

#include <QObject>
#include <QVector>
#include <vector>
using namespace std;

class Bll_LeastSquareMethod : public QObject
{
    Q_OBJECT
public:
    explicit Bll_LeastSquareMethod(QObject *parent = nullptr);

public slots:
    void work(size_t order, const vector<DECIMAL_TYPE> &x, const vector<DECIMAL_TYPE> &y);

signals:
    void leastSquareMethodFinish(const vector<DECIMAL_TYPE> &factor);

    void generateFitDataFinish(const QVector<double> &x, const QVector<double> &y);

private:
    void generateFitData(DECIMAL_TYPE left, DECIMAL_TYPE right,
                         const DECIMAL_TYPE step, const vector<DECIMAL_TYPE> &factor);
};

#endif
