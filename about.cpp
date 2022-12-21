#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() &~Qt::WindowContextHelpButtonHint); // 去掉问号，只保留关闭
}

About::~About()
{
    delete ui;
}
