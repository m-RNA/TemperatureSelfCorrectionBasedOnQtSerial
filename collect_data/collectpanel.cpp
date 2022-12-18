#include "collectpanel.h"
#include "ui_collectpanel.h"

CollectPanel::CollectPanel(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::CollectPanel)
{
    ui->setupUi(this);
}

CollectPanel::~CollectPanel()
{
    delete ui;
}

void CollectPanel::slSetState(bool state)
{
    if (state)
    {
        ui->ledText->setText("在线");
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
    }
    else
    {
        ui->ledText->setText("离线");
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");
    }
}

void CollectPanel::setDeviceName(QString name)
{
    ui->chart->graph()->setName(name);
}
