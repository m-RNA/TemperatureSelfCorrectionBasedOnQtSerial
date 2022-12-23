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

bool isNum(QString data)
{
    QRegExp reg("(-?[1-9][0-9]+)|(-?[0-9])|(-?[1-9]\\d+\\.\\d+)|(-?[0-9]\\.\\d+)");
    if (reg.exactMatch(data))
        return true;
    return false;
}

// void Bll_SerialRecvAnalyse::run()
void Bll_SerialRecvAnalyse::working(QByteArray rxRowData)
{
    // qDebug() << "rxRowData:" << QString(rxRowData);

    QByteArray rxFrame;
    int endIndex;

    staticTemp.append(rxRowData); // 读取串口，附在 staticTemp 之后
    while (1)
    {
        endIndex = staticTemp.indexOf("\n"); // 获取"\n"的索引

        // \r\n 和 \n 两种情况
        if (endIndex >= 0)
        {
            if (staticTemp.at(endIndex - 1) == '\r')
                rxFrame.append(staticTemp.left(endIndex - 1)); // 去除\n"
            else
                rxFrame.append(staticTemp.left(endIndex)); // 去除\n"
            staticTemp.remove(0, endIndex + 1);            // 移除"\n"与"\n"之前的内容
        }

        if (rxFrame.isEmpty())
            return;

        if (rxFrame.at(0) == '{')
        {
            int titleIndexRight;
            double data;

            titleIndexRight = rxFrame.indexOf("}");
            if (titleIndexRight >= 0)
            {
                rxRowData = rxFrame.mid(1, titleIndexRight - 1);
                qDebug() << "Title:" << QString(rxRowData);

                rxFrame.remove(0, titleIndexRight + 1); // 移除"}"与"}"之前的内容
                if (isNum(rxFrame) == true)
                {
                    data = QString(rxFrame).toDouble();
                    qDebug() << "解析" << data;

                    emit sgBll_AnalyseFinish(data);
                }
            }
        }
        rxFrame.clear();
    }
}
