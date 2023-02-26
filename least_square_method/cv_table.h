#ifndef CV_TABLE_H
#define CV_TABLE_H

#include <QWidget>
#include <QTableWidget>

class CVTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit CVTable(QWidget *parent = nullptr);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    void cutSelectedCells();
    void copySelectedCells();
    void pasteSelectedCells();
    void removeSelectedCells();
};

#endif // CV_TABLE_H
