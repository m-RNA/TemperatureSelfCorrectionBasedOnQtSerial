#include "collectpanel.h"
#include "ui_collectpanel.h"

CollectPanel::CollectPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CollectPanel)
{
    ui->setupUi(this);
}

CollectPanel::~CollectPanel()
{
    delete ui;
}
