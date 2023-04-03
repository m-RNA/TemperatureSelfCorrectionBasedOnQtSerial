#ifndef BLL_SERIALRECVANALYSE_H
#define BLL_SERIALRECVANALYSE_H

#include <QObject>
// #include <QRunnable>

typedef struct
{
    int moment;   // 时刻
    double value; // 数据
} SerialAnalyseCell;

class Bll_SerialRecvAnalyse : public QObject //, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_SerialRecvAnalyse(QObject *parent = nullptr);
    ~Bll_SerialRecvAnalyse();

    bool canIntoNum(const QString data);
    void setAnalyseMode(int type);

    // void run() override;
    void working(QByteArray);

public slots:
    // void slBll_GetRowRecvData(QByteArray rxData) { rxRowData = rxData; }

signals:
    void sgBll_AnalyseFinish(const SerialAnalyseCell &);

private:
    // QByteArray rxRowData;
    QByteArray buffer = "";
    unsigned int id = 0;

    void analyseNum();
    void analyseDaoWanTech();

    void (Bll_SerialRecvAnalyse::*analyseBuffer)() = nullptr;
};

#endif // BLL_SERIALRECVANALYSE_H
