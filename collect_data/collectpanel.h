#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include <vector>
#include <string>
#include "interactchart.h"
#include "bll_serialrecvanalyse.h"
using namespace std;

namespace Ui
{
    class CollectPanel;
}

class Bll_Average : public QObject
{
    Q_OBJECT
public:
    explicit Bll_Average(QObject *parent = nullptr);

public slots:
    void slAverage(const vector<double> &data);

signals:
    void sgAverage(const string &average);
};

class CollectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CollectPanel(QWidget *parent = nullptr);
    ~CollectPanel();

    void setDeviceName(QString name);
    void setYAxisFormat(const QString &format, const int precision);

    void setOnlineState(bool state);
    void setStableState(const char state);
    void setReceiveTimeout(void);

    bool isStable(void);
    double getRange(void);
    QCPAxis *getXAxis(void);

    void collectStart(void);
    void collectStop(void);
    void collectFinish(void);
    void collectRestart(void);

public slots:
    void slCollectData(const SerialAnalyseCell &cell);

signals:
    void sgCollectDataAverage(const string &average);

    void sgCollectDataGet(const vector<double> &data);

private:
    Ui::CollectPanel *ui;
    QString deviceName;
    vector<double> data;
    bool collectState = false;

    double min = 0;
    double max = 0;
    double currentRange = 0;
    bool resetRange = true;

    bool onlineState = false;
    char stableState = 0;
    Bll_Average *taskAverage = nullptr;
    QThread *threadAverage = nullptr;

    void setLEDState(int state);
};

#endif // COLLECTPANEL_H
