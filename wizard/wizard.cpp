#include "wizard.h"
#include "ui_wizard.h"
#include <QDate>

Wizard::Wizard(QWidget *parent) : QWizard(parent),
                                  ui(new Ui::Wizard)
{
    ui->setupUi(this);
    // 设置为当前日期
    ui->dateEdit->setDate(QDate::currentDate());
    // 去掉问号，只保留关闭
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

Wizard::~Wizard()
{
    delete ui;
}
