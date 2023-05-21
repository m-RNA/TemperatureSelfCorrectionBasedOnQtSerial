#include "othersetting.h"
#include "ui_othersetting.h"

OtherSetting::OtherSetting(QWidget *parent) : QDialog(parent),
                                              ui(new Ui::OtherSetting)
{
    ui->setupUi(this);
    // 去掉问号，只保留关闭
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // 设置默认窗口大小
    this->resize(352, 117);
}

OtherSetting::~OtherSetting()
{
    delete ui;
}

void OtherSetting::on_spbxYellow_valueChanged(double arg1)
{
    ui->spbxRed->setMinimum(arg1);
}

void OtherSetting::on_spbxRed_valueChanged(double arg1)
{
    ui->spbxYellow->setMaximum(arg1);
}

void OtherSetting::setOtherSetting(const OtherSettingData &data)
{
    ui->cbSound->setCurrentIndex(data.soundIndex);
    ui->spbxAutoRange->setValue(data.autoRange);
    ui->spbxYellow->setValue(data.yellowRange);
    ui->spbxRed->setValue(data.redRange);
}

void OtherSetting::getOtherSetting(OtherSettingData &data)
{
    data.soundIndex = ui->cbSound->currentIndex();
    data.autoRange = ui->spbxAutoRange->value();
    data.yellowRange = ui->spbxYellow->value();
    data.redRange = ui->spbxRed->value();
}
