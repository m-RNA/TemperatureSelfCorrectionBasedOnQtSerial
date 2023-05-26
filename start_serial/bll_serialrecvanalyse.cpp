#include "bll_serialrecvanalyse.h"
#include <QThread>
#include <QDebug>
#include <QDateTime>

// 全局静态变量，用于生成 id
static unsigned globalId = 0;

Bll_SerialRecvAnalyse::Bll_SerialRecvAnalyse(QObject *parent) : QObject(parent)
{
    globalId++;
    id = globalId;
}

Bll_SerialRecvAnalyse::~Bll_SerialRecvAnalyse()
{
    qDebug() << "Bll_SerialRecvAnalyse Destroyed";
}

// 在串口数据到达时，将数据添加到缓冲区
void Bll_SerialRecvAnalyse::working(QByteArray rxRowData)
{
    // 防御性编程，防止解码函数为空
    if (analyseBuffer == nullptr)
        return;

    // 将发来的数据附在 buffer 之后
    buffer.append(rxRowData);

    // 解析数据
    (this->*analyseBuffer)();
}

// 设置解码模式 （通过函数指针实现）
// 1：数字，2：道万科技
void Bll_SerialRecvAnalyse::setAnalyseMode(int type)
{
    switch (type)
    {
    case 1:
        analyseBuffer = &Bll_SerialRecvAnalyse::analyseNum;
        break;

    case 2:
        analyseBuffer = &Bll_SerialRecvAnalyse::analyseDaoWanTech;
        break;

    default:
        analyseBuffer = nullptr;
        break;
    }
}

// 判断是否能转化为数字
bool Bll_SerialRecvAnalyse::canIntoNum(const QString data)
{
    QRegExp reg("^[-+]?([0-9]+\\.?[0-9]*|\\.[0-9]+)(\\s|\r?\n|\r)*$");
    return reg.exactMatch(data);
}

// 数据格式
// 12.3456
void Bll_SerialRecvAnalyse::analyseNum()
{
    SerialAnalyseCell cell;

    while (1)
    {
        // 读取串口，附在 buffer 之后
        int endIndex = buffer.indexOf("\n");

        // 没有完整的一帧数据，直接返回
        if (endIndex <= 0)
            return;

        QByteArray rxFrame = buffer.left(endIndex);
        if (canIntoNum(rxFrame))
        {
            cell.value = rxFrame.toDouble();
            cell.timestamp = QDateTime::currentMSecsSinceEpoch(); // 记录此刻时间
            // qDebug() << id << ":" << cell.timestamp << cell.value;

            emit sgBll_AnalyseFinish(cell);
        }

        buffer.remove(0, endIndex + 1);
    }
}

// 数据格式
// $T=019.1263 ;
const char *daoWanTechHead = "$T=";
const char *daoWanTechTail = " ;\r\n";
void Bll_SerialRecvAnalyse::analyseDaoWanTech()
{
    SerialAnalyseCell cell;
    // qDebug() << id << "buffer:" << buffer;

    while (1)
    {
        int index = buffer.indexOf('$');
        if (index == -1)
        {
            // 数据异常，清空缓冲区
            buffer.clear();
            return;
        }
        // 帧头不完整，直接返回
        if ((index + (int)sizeof(daoWanTechHead) - 2) >= buffer.size())
            return;
        if (buffer.at(index + 1) != 'T' || buffer.at(index + 2) != '=')
        {
            // 数据异常，清空缓冲区
            buffer.clear();
            return;
        }

        /* 找到了数据包起始标志 */

        int endIndex = buffer.indexOf('\n', index + (int)sizeof(daoWanTechHead) - 1);

        // 没有完整的一帧数据，直接返回
        if (endIndex == -1)
            return;

        if (buffer.at(endIndex - 1) != '\r' || buffer.at(endIndex - 2) != ';' || buffer.at(endIndex - 3) != ' ')
        {
            // 数据异常，清空缓冲区
            buffer.clear();
            return;
        }

        /* 找到了数据包结束标志 */

        qDebug() << id << "row:" << buffer.mid(index, endIndex);

        QByteArray rxFrame = buffer.mid(index + 3, endIndex - index - 6);
        qDebug() << id << "rxFrame:" << rxFrame;

        // 处理解析出来的数据
        if (canIntoNum(rxFrame))
        {
            cell.value = rxFrame.toDouble();
            cell.timestamp = QDateTime::currentMSecsSinceEpoch(); // 记录此刻时间
            // qDebug() << id << ":" << cell.timestamp << cell.value;

            emit sgBll_AnalyseFinish(cell);
        }
        // 从缓冲区中移除已经解析的数据包
        buffer.remove(0, endIndex + 1);
    }
}
