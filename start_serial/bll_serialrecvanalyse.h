#ifndef BLL_SERIALRECVANALYSE_H
#define BLL_SERIALRECVANALYSE_H

#include <QObject>
// #include <QRunnable>

typedef struct
{
    double date;  // 日期
    double value; // 数据
} serialAnalyseData;

class Bll_SerialRecvAnalyse : public QObject //, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_SerialRecvAnalyse(QObject *parent = nullptr);
    ~Bll_SerialRecvAnalyse();

    bool canIntoNum(const QString data);
    void setAnalyseMode(unsigned int type);

    // void run() override;
    void working(QByteArray);

public slots:
    // void slBll_GetRowRecvData(QByteArray rxData) { rxRowData = rxData; }

signals:
    void sgBll_AnalyseFinish(double);

private:
    // QByteArray rxRowData;
    QByteArray buffer = "";
    unsigned id = 0;

    void analyseNum();
    void analyseDaoWanTech();

    void (Bll_SerialRecvAnalyse::*analyseBuffer)() = nullptr;
};

#endif // BLL_SERIALRECVANALYSE_H
