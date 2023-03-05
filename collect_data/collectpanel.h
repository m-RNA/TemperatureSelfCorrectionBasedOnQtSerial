#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include <vector>
#include "interactchart.h"
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

    QCPAxis *getXAxis(void);

public slots:
    void slCollectData(double);

signals:
    void sgCollectDataAverage(DECIMAL_TYPE average);

private slots:

private:
    Ui::CollectPanel *ui;
    vector<double> data;
    bool collectState = false;
};

#endif // COLLECTPANEL_H
