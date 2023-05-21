#ifndef OTHERSETTING_H
#define OTHERSETTING_H

#include <QDialog>

typedef struct
{
    int soundIndex;
    double autoRange;
    double yellowRange;
    double redRange;
} OtherSettingData;

namespace Ui
{
    class OtherSetting;
}

class OtherSetting : public QDialog
{
    Q_OBJECT

public:
    explicit OtherSetting(QWidget *parent = nullptr);
    ~OtherSetting();

    void setOtherSetting(const OtherSettingData &data);

    void getOtherSetting(OtherSettingData &data);

private slots:
    void on_spbxYellow_valueChanged(double arg1);

    void on_spbxRed_valueChanged(double arg1);

private:
    Ui::OtherSetting *ui;
};

#endif // OTHERSETTING_H
