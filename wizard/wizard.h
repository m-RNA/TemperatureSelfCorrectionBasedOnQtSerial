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

signals:
    void wizardInfoFinish(const WizardInfo &);

private:
    Ui::Wizard *ui;
    WizardInfo wizardInfo;

    void getInfo();
};

#endif // WIZARD_H
