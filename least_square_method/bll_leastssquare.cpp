#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include <cmath>
#include <QDebug>
#include <QElapsedTimer>
#include "bll_leastssquare.h"

using namespace Eigen;
typedef Matrix<DECIMAL_TYPE, Dynamic, Dynamic> MatrixX_Dec;

/*
    qt中获取容器Vector中的最大值和最小值：
    https://blog.csdn.net/Littlehero_121/article/details/100565527
*/
DECIMAL_TYPE __max__(const std::vector<DECIMAL_TYPE> &data)
{
    auto max = std::max_element(std::begin(data), std::end(data));
    return *max;
}

DECIMAL_TYPE __min__(const std::vector<DECIMAL_TYPE> &data)
{
    auto min = std::min_element(std::begin(data), std::end(data));
    return *min;
}

Bll_LeastSquareMethod::Bll_LeastSquareMethod(QObject *parent) : QObject(parent)
{
}

/*
    最小二乘法参考：
    https://blog.csdn.net/weixin_44344462/article/details/88850409

    A W = B
    AT A W = AT B
    (AT A)^(-1) AT A W = (AT A)^(-1) AT B
    W = (AT A)^(-1) AT B
*/
void Bll_LeastSquareMethod::work(size_t order,
                                 const vector<DECIMAL_TYPE> &x, const vector<DECIMAL_TYPE> &y)
{
    QElapsedTimer eTimer;
    eTimer.start();

    // N个点可以确定一个 唯一的 N-1 阶的曲线
    if (x.size() <= order)
        order = x.size() - 1;

    // 创建A矩阵
    MatrixX_Dec A(x.size(), order + 1);
    for (unsigned long long i = 0; i < x.size(); ++i) // 遍历所有点
    {
        for (int n = order, dex = 0; n >= 1; --n, ++dex) // 遍历order到1阶
        {
            A(i, dex) = pow(x.at(i), n);
        }
        A(i, order) = 1; // 最后一列为1
    }

    // 创建B矩阵
    MatrixX_Dec B(y.size(), 1);
    for (unsigned long long i = 0; i < y.size(); ++i)
    {
        B(i, 0) = y.at(i);
    }

    // 创建矩阵W
    MatrixX_Dec W;
    W = (A.transpose() * A).inverse() * A.transpose() * B;

    // 打印W结果
    qDebug() << "Factor:";
    vector<DECIMAL_TYPE> factor;
    for (size_t i = 0; i <= order; i++)
    {
        DECIMAL_TYPE temp = W(i, 0);
        factor.push_back(temp);
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "%LE", temp);
        qDebug() << globalStringBuffer;
    }
    qDebug() << "拟合耗时：" << eTimer.elapsed() << "ms";
    emit leastSquareMethodFinish(factor);

    DECIMAL_TYPE max = __max__(x);
    DECIMAL_TYPE min = __min__(x);
    DECIMAL_TYPE addRange = (max - min) / 1.0f;

    generateFitData(min - addRange, max + addRange,
                    (max - min) / (DECIMAL_TYPE)(x.size()) / 5000.0f,
                    factor);
}

void Bll_LeastSquareMethod::generateFitData(DECIMAL_TYPE left, DECIMAL_TYPE right,
                                            const DECIMAL_TYPE step, const vector<DECIMAL_TYPE> &factor)
{
    QElapsedTimer eTimer;
    eTimer.start();

    vector<DECIMAL_TYPE> x, y;

    if (left > right)
    {
        DECIMAL_TYPE temp = right;
        right = left;
        left = temp;
    }
    x.clear();
    y.clear();
    int order = factor.size(); // 阶数
    DECIMAL_TYPE temp;
    for (DECIMAL_TYPE i = left; i <= right; i += step)
    {
        x.push_back(i);
        temp = factor.at(order - 1);
        for (int j = 1; j < order; j++)
        {
            temp += factor.at(order - 1 - j) * pow(i, j);
        }
        y.push_back(temp);
    }
    qDebug() << "生成曲线数据耗时：" << eTimer.elapsed() << "ms";

    QVector<double> qv_X = QVector<double>(x.begin(), x.end());
    QVector<double> qv_Y = QVector<double>(y.begin(), y.end());

    emit generateFitDataFinish(qv_X, qv_Y);
}
