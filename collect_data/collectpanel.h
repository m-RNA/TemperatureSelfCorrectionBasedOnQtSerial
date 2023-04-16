#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include "interactchart.h"
#include "bll_serialrecvanalyse.h"
#include "bll_data_wave.h"
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

    void setDeviceName(QString name);
    void setYAxisFormat(const QString &format, const int precision);

    void setOnlineState(bool state);
    void setStableState(const StableStateEnum state);

    bool isStable(void);
    double getRange(void);
    QCPAxis *getXAxis(void);

    void collectStart(void);
    void collectStop(void);
    void collectFinish(void);
    void collectRestart(void);

public slots:
    void slCollectData(const SerialAnalyseCell &cell);

    void slSetRange(const double &range);

signals:
    void sgShowData();

private slots:
    void on_btnShowData_clicked();

private:
    Ui::CollectPanel *ui;
    QString deviceName;
    double range = 0;
    StableStateEnum stableState = STABLE_STATE_INIT;

    void setLEDState(int state);
};

#endif // COLLECTPANEL_H
