#include "serialsetting.h"
#include "ui_serialsetting.h"

SerialSetting::SerialSetting(QWidget *parent) : QWidget(parent),
                                                ui(new Ui::SerialSetting)
{
    ui->setupUi(this);
}

SerialSetting::~SerialSetting()
{
    delete ui;
}
