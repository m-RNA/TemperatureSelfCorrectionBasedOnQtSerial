#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H
#include "config.h"

#include <QWidget>
#include <vector>
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

    void collectStart(void) { collectState = true; }
    void collectStop(void) { collectState = false; }
    void collectFinish(void);

public slots:
    void slCollectData(double);
    void slAddYPoint(double);

signals:
    void sgCollectDataAverage(DECIMAL_TYPE average);

private slots:

private:
    Ui::CollectPanel *ui;
    vector<double> data;
    bool collectState = false;
};

#endif // COLLECTPANEL_H
