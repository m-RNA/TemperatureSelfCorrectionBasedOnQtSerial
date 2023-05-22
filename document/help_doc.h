#ifndef HELP_DOC_H
#define HELP_DOC_H

#include <QDialog>

namespace Ui {
class HelpDoc;
}

class HelpDoc : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDoc(QWidget *parent = nullptr);
    ~HelpDoc();

private:
    Ui::HelpDoc *ui;
};

#endif // HELP_DOC_H
