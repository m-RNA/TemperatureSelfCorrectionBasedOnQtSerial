#ifndef COLLECTPANEL_H
#define COLLECTPANEL_H

#include <QWidget>

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
    void slSetState(bool state);
    void setDeviceName(QString name);

private:
    Ui::CollectPanel *ui;
};

#endif // COLLECTPANEL_H
