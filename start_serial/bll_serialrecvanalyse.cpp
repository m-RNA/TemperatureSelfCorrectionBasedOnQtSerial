#include "bll_serialrecvanalyse.h"
#include <QThread>
#include <QDebug>

static QByteArray staticTemp; // 静态中间变量

Bll_SerialRecvAnalyse::Bll_SerialRecvAnalyse(QObject *parent) : QObject(parent) // , QRunnable()
{
}

Bll_SerialRecvAnalyse::~Bll_SerialRecvAnalyse()
{
    qDebug() << "Bll_SerialRecvAnalyse Destroyed";
}

// void Bll_SerialRecvAnalyse::run()
void Bll_SerialRecvAnalyse::working(QByteArray rxRowData)
{
    // qDebug() << "分析线程ID：" << QThread::currentThread();

    QByteArray rxFrame;
    int startIndex = -1;

    staticTemp.append(rxRowData); // 读取串口，附在 staticTemp 之后

    // BUG显然 待优化？
    startIndex = staticTemp.lastIndexOf("\n"); // 获取"\n"的索引

    if (startIndex >= 0)
    {
        rxFrame.append(staticTemp.left(startIndex - 1)); // 去除"\r\n"
        staticTemp.remove(0, startIndex + 1);            // 移除"\n"与"\n"之前的内容
    }

    if (rxFrame.isEmpty())
        return;

    // qDebug() << "RxFrame:" << rxFrame;
    // 解析数据
    // {text}23.3
    // printf("temp=%f\r\n",rtd);
    int title_index_left;
    title_index_left = rxFrame.indexOf("}");
    rxRowData = rxFrame.replace("{", "").left(title_index_left - 1);
    // qDebug() << "Title:" << QString(rxRowData);
    int titleLength = rxRowData.length();
    double data;
    data = QString(rxFrame.right(rxFrame.length() - 1 - titleLength)).toDouble();
    qDebug() << data;

    emit sgBll_AnalyseFinish(data);
}
