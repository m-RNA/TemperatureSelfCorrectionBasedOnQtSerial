#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include "serialsetting.h"

typedef struct
{
    QString place;
    int temp;
    int rh;
    QString date;
    QString operatorName;
    QString id_Std;
    QString id_Dtm;
} BaseInfo;

typedef struct
{
    int num;
    double time;
    bool isAuto;
    std::vector<double> autoList;
} CollectSetting;

typedef struct
{
    double range;
    int num;
    double stableTime;
} CheckWaveSetting;

typedef struct
{
    BaseInfo baseInfo;
    Ui_SerialSettingIndex ssIndex_Std;
    Ui_SerialSettingIndex ssIndex_Dtm;
    CollectSetting collectSetting;
    CheckWaveSetting checkWaveSetting;
} WizardInfo;

namespace Ui
{
    class Wizard;
}

class Wizard : public QWizard
{
    Q_OBJECT

public:
    explicit Wizard(WizardInfo *info, QWidget *parent = nullptr);
    ~Wizard();

protected slots:
    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void wizardInfoFinish();

private slots:
    void on_cbSerial_Std_activated(int index);

    void on_cbSerial_Dtm_activated(int index);

    void on_cbSerial_Std_currentIndexChanged(const QString &arg1);

    void on_cbSerial_Dtm_currentIndexChanged(const QString &arg1);

    void on_cbAutoCollect_currentIndexChanged(int index);

private:
    Ui::Wizard *ui;
    WizardInfo *wizardInfo;

    void getInfo();
    void updateSerialPortInfo(char who);
};

#endif // WIZARD_H
