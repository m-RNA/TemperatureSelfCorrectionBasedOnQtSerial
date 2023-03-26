#include "cv_table.h"
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>

CVTable::CVTable(QWidget *parent) : QTableWidget(parent)
{
}

void CVTable::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    if (this->editTriggers() != QAbstractItemView::NoEditTriggers && !this->selectedRanges().isEmpty())
        menu.addAction(QIcon(":/icon/cut.ico"), "剪切", this, &CVTable::cutSelectedCells);

    if (!this->selectedRanges().isEmpty())
        menu.addAction(QIcon(":/icon/copy.ico"), "复制", this, &CVTable::copySelectedCells);

    if (this->editTriggers() != QAbstractItemView::NoEditTriggers)
    {
        menu.addAction(QIcon(":/icon/paste.ico"), "粘贴", this, &CVTable::pasteSelectedCells);
        menu.addAction(QIcon(":/icon/clear.ico"), "清空", this, &CVTable::removeSelectedCells);
    }

    menu.exec(event->globalPos());
}

void CVTable::keyPressEvent(QKeyEvent *event)
{
    // 表格不可编辑
    if (this->editTriggers() == QAbstractItemView::NoEditTriggers)
    {
        if (event->modifiers() == Qt::ControlModifier &&
            ((event->key() == Qt::Key_C) || (event->key() == Qt::Key_X)))
        {
            // 复制选中的单元格内容
            copySelectedCells();
            event->accept();
        }
        else
        {
            QTableWidget::keyPressEvent(event);
        }
        return;
    }

    // 表格可编辑
    // 修饰键是Ctrl
    if (event->modifiers() == Qt::ControlModifier)
    {
        switch (event->key())
        {
        case Qt::Key_X:
            // 剪切选中的单元格内容
            cutSelectedCells();
            event->accept();
            break;

        case Qt::Key_C:
            // 复制选中的单元格内容
            copySelectedCells();
            event->accept();
            break;

        case Qt::Key_V:
            // 粘贴剪贴板中的内容
            pasteSelectedCells();
            event->accept();
            break;
        }
        return;
    }

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        // 清空选中的单元格内容
        removeSelectedCells();
        event->accept();
        return;
    }
    QTableWidget::keyPressEvent(event);
}

void CVTable::cutSelectedCells()
{
    // 复制选中的单元格内容
    copySelectedCells();
    // 清空选中的单元格内容
    removeSelectedCells();
}

void CVTable::copySelectedCells()
{
    QClipboard *clipboard = QApplication::clipboard();
    QList<QTableWidgetItem *> selectedItems = this->selectedItems();
    if (selectedItems.isEmpty())
        return;

    // 构造剪贴板文本
    QString text;
    QMap<int, int> rows, cols;
    foreach (QTableWidgetItem *item, selectedItems)
    {
        rows[item->row()] = 0;
        cols[item->column()] = 0;
    }
    for (int row : rows.keys())
    {
        for (int col : cols.keys())
        {
            QTableWidgetItem *item = this->item(row, col);
            if (item)
                text += item->text() + "\t";
            else
                text += "\t";
        }
        text.chop(1); // 删除最后一个制表符
        text += "\n";
    }

    clipboard->setText(text);
}

void CVTable::pasteSelectedCells()
{
    const QString &textToPast = QApplication::clipboard()->text();
    if (textToPast.isEmpty())
        return;
    QStringList tableRowDataList = textToPast.split("\n"); //, QString::SkipEmptyParts);

    // 获取当前选中的左上角单元格的索引
    int currentIndexRow = this->selectedRanges()[0].topRow();
    int currentIndexColumn = this->selectedRanges()[0].leftColumn();

    // 计算可以粘贴的最大行数和列数
    int maxRows = qMin(this->rowCount() - currentIndexRow, tableRowDataList.length() - 1);
    int maxCols = qMin(this->columnCount() - currentIndexColumn, tableRowDataList.at(0).split("\t").length());

    for (int i = 0; i < maxRows; ++i)
    {
        QStringList rowDataList = tableRowDataList.at(i).split("\t");
        for (int k = 0; k < maxCols; k++)
        {
            QTableWidgetItem *item = this->item(i + currentIndexRow, k + currentIndexColumn);
            if (item)
                item->setText(rowDataList.at(k));
            else
                this->setItem(i + currentIndexRow, k + currentIndexColumn, new QTableWidgetItem(rowDataList.at(k)));
        }
    }
}

void CVTable::removeSelectedCells()
{
    QList<QTableWidgetItem *> selectedItems = this->selectedItems();
    if (selectedItems.isEmpty())
        return;

    foreach (QTableWidgetItem *item, selectedItems)
    {
        item->setText("");
    }
}
