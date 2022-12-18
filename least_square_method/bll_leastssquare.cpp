#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace Eigen;

#include "bll_leastssquare.h"

#include <QThread>
#include <QDebug>
#include <QElapsedTimer>

Bll_GenerateData::Bll_GenerateData(QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(false); // 关掉 放到线程池后自动析构
}

void Bll_GenerateData::run()
{
    QElapsedTimer eTimer;
    eTimer.start();
    qDebug() << "生成拟合数据线程ID：" << QThread::currentThread();

    QVector<double> x, y;

    if (left > right)
    {
        int temp = right;
        right = left;
        left = temp;
    }
    x.clear();
    y.clear();
    int order = factor.length(); // 阶数
    double temp;
    for (double i = left; i <= right; i += step)
    {
        x << i;
        temp = factor.at(order - 1);
        for (int j = 1; j < order; j++)
        {
            temp += factor.at(order - 1 - j) * pow(i, j);
        }
        y << temp;
        // qDebug() << "generate:" << i << "\t" << temp;
    }
    qDebug() << "生成拟合数据花费时间：" << eTimer.elapsed() << "ms";
    emit generateFitDataFinish(x, y);
}

Bll_LeastSquareMethod::Bll_LeastSquareMethod(QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(false); // 关掉 放到线程池后自动析构
}

/*
    最小二乘法参考：
    https://blog.csdn.net/weixin_44344462/article/details/88850409

    A W = B
    AT A W = AT B
    (AT A)^(-1) AT A W = (AT A)^(-1) AT B
    W = (AT A)^(-1) AT B
*/
// 设置是N阶拟合
void Bll_LeastSquareMethod::run()
{
    QElapsedTimer eTimer;
    eTimer.start();
    qDebug() << "生成拟合数据线程ID：" << QThread::currentThread();

    // 这里默认格式正确，就不检查了
    // QVector<double> method00(int N, QVector<double> x, QVector<double> y)
    //  防御检查
    // if (x.size() != y.size())
    // {
    //     qDebug() << "format error!";
    //     return QVector<double>();
    // }

    // N个点可以确定一个 唯一的 N-1 阶的曲线
    if (x.length() <= N)
        N = x.length() - 1;

    // 创建A矩阵
    MatrixXd A(x.size(), N + 1);
    for (int i = 0; i < x.size(); ++i) // 遍历所有点
    {
        for (int n = N, dex = 0; n >= 1; --n, ++dex) // 遍历N到1阶
        {
            A(i, dex) = pow(x.at(i), n);
        }

        A(i, N) = 1; //
    }

    // 创建B矩阵
    MatrixXd B(y.size(), 1);
    for (int i = 0; i < y.size(); ++i)
    {
        B(i, 0) = y.at(i);
    }

    // 创建矩阵W
    MatrixXd W;
    W = (A.transpose() * A).inverse() * A.transpose() * B;

    // 打印W结果
    qDebug() << "Factor:";
    QVector<double> ans;
    for (int i = 0; i <= N; i++)
    {
        double temp = W(i, 0);
        if (abs(temp) >= 1e-10)
            ans << temp;
        else
            ans << 0;
        qDebug() << temp;
    }
    qDebug() << "生成拟合数据花费时间：" << eTimer.elapsed() << "ms";
    emit leastSquareMethodFinish(ans);
}
