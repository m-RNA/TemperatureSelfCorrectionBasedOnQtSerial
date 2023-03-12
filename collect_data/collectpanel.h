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

public slots:
    void slCollectData(const serialAnalyseCell &cell);

signals:
    void sgCollectDataAverage(const DECIMAL_TYPE &average);

    void sgCollectDataGet(const vector<double> &data);

private slots:

private:
    Ui::CollectPanel *ui;
    vector<double> data;
    bool collectState = false;

    double min = 0;
    double max = 0;
    double range = 0;
    bool resetRange = true;

    double average(void);
};

#endif // COLLECTPANEL_H
