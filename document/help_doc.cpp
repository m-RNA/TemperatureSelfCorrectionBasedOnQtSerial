#include "help_doc.h"
#include "ui_help_doc.h"

HelpDoc::HelpDoc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDoc)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 去掉问号，只保留关闭
}

HelpDoc::~HelpDoc()
{
    delete ui;
}
