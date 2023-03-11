#ifndef SERIALSETTING_H
#define SERIALSETTING_H

#include <QWidget>

namespace Ui
{
    class SerialSetting;
}

class SerialSetting : public QWidget
{
    Q_OBJECT

public:
    explicit SerialSetting(QWidget *parent = nullptr);
    ~SerialSetting();

private:
    Ui::SerialSetting *ui;
};

#endif // SERIALSETTING_H
