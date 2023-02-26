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
    if (this->editTriggers() == QAbstractItemView::NoEditTriggers)
    {
        // 表格不可编辑
        if (event->matches(QKeySequence::Copy))
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
    if (event->matches(QKeySequence::Cut))
    {
        // 剪切选中的单元格内容
        cutSelectedCells();
        event->accept();
    }
    else if (event->matches(QKeySequence::Paste))
    {
        // 粘贴剪贴板中的内容
        pasteSelectedCells();
        event->accept();
    }
    else if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        // 清空选中的单元格内容
        removeSelectedCells();
        event->accept();
    }
    else
    {
        QTableWidget::keyPressEvent(event);
    }
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
    const QString &text_to_past = QApplication::clipboard()->text();
    if (text_to_past.isEmpty())
        return;
    QStringList table_row_data_list = text_to_past.split("\n"); //, QString::SkipEmptyParts);
    QModelIndex current_index = this->currentIndex();

    // 计算可以粘贴的最大行数和列数
    int maxRows = qMin(this->rowCount() - current_index.row(), table_row_data_list.length());
    int maxCols = qMin(this->columnCount() - current_index.column(), table_row_data_list.at(0).split("\t").length());

    for (int i = 0; i < maxRows; ++i)
    {
        QStringList row_data_list = table_row_data_list.at(i).split("\t");
        for (int k = 0; k < maxCols; k++)
        {
            QTableWidgetItem *item = this->item(i + current_index.row(), k + current_index.column());
            if (item)
                item->setText(row_data_list.at(k));
            else
                this->setItem(i + current_index.row(), k + current_index.column(), new QTableWidgetItem(row_data_list.at(k)));
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
