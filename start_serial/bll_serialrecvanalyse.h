#ifndef BLL_SERIALRECVANALYSE_H
#define BLL_SERIALRECVANALYSE_H

#include <QObject>
#include <QRunnable>

class Bll_SerialRecvAnalyse : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Bll_SerialRecvAnalyse(QObject *parent = nullptr);
    ~Bll_SerialRecvAnalyse();

    void run() override;

public slots:
    void slBll_GetRowRecvData(QByteArray rxData) { rxRowData = rxData; }

signals:
    void sgBll_AnalyseFinish(double);

private:
    QByteArray rxRowData;
};

#endif // BLL_SERIALRECVANALYSE_H
