#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include "serialsetting.h"

typedef struct
{
    QStringList baseInfo;
    Ui_SerialSettingIndex ssIndex_Std;
    Ui_SerialSettingIndex ssIndex_Dtm;
} WizardInfo;

namespace Ui
{
    class Wizard;
}

class Wizard : public QWizard
{
    Q_OBJECT

public:
    explicit Wizard(const Ui_SerialSettingIndex &settings_Std, const Ui_SerialSettingIndex &settings_Dtm, QWidget *parent = nullptr);
    ~Wizard();
    void loadUiSettings();
    void saveUiSettings();

    void setTabName_Std(const QString &portName);
    void setTabName_Dtm(const QString &portName);

protected slots:
    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void wizardInfoFinish(const WizardInfo &);

private slots:
    void on_cbSerial_Std_activated(int index);

    void on_cbSerial_Dtm_activated(int index);

    void on_cbSerial_Std_currentIndexChanged(const QString &arg1);

    void on_cbSerial_Dtm_currentIndexChanged(const QString &arg1);

    void on_cbAutoCollect_currentIndexChanged(int index);

private:
    Ui::Wizard *ui;
    WizardInfo wizardInfo;

    void getInfo();
    void updateSerialPortInfo(char who);
};

#endif // WIZARD_H
