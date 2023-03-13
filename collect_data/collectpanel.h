#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include <vector>
#include "interactchart.h"
#include "bll_serialrecvanalyse.h"
using namespace std;

namespace Ui
{
    class CollectPanel;
}

class CollectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CollectPanel(QWidget *parent = nullptr);
    ~CollectPanel();
    void slSetState(int state);
    void setDeviceName(QString name);

    void collectStart(void);
    void collectStop(void);
    void collectFinish(void);

    double getRange(void);
    QCPAxis *getXAxis(void);
    void setRange(const double range);
    void setCheckState(bool check);
    bool isStable(void);

public slots:
    void slCollectData(const serialAnalyseCell &cell);

signals:
    void sgCollectDataAverage(const DECIMAL_TYPE &average);

    void sgCollectDataGet(const vector<double> &data);

private slots:

private:
    Ui::CollectPanel *ui;
    QString deviceName;
    vector<double> data;
    bool collectState = false;

    double min = 0;
    double max = 0;
    double currentRange = 0;
    double commandRange = 0.01;
    bool resetRange = true;

    bool checkState = true;
    bool stableState = false;
    QVector<double> dataWave;

    double average(void);
    void checkDataWave(const double &data);
};

#endif // COLLECTPANEL_H
