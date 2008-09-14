/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "tolistviewformattertabdel.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

#include <iostream>

namespace
{
toListViewFormatter* createTabDel()
{
    return new toListViewFormatterTabDel();
}
const bool registered = toListViewFormatterFactory::Instance().Register(
                            toListViewFormatterIdentifier::TAB_DELIMITED, createTabDel);
}


toListViewFormatterTabDel::toListViewFormatterTabDel() : toListViewFormatter()
{
}

toListViewFormatterTabDel::~toListViewFormatterTabDel()
{
}

QString toListViewFormatterTabDel::getFormattedString(toListView& tListView)
{
    int column_count = tListView.columns();
    QString separator = tListView.getSep();
    QString delimiter = tListView.getDel();

    QVector<int> sizes(column_count);
    QString output;
    QString indent;
    QString bgcolor;

    if (tListView.getIncludeHeader())
    {
        if (bgcolor.isEmpty())
            bgcolor = QString::fromLatin1("nonull");
        else
            bgcolor = QString::null;
        for (int j = 0;j < column_count;j++)
            output += QString::fromLatin1("%1\t").arg((tListView.headerItem())->text(j));
        output = output.left(output.length() - 1);
        endLine(output);
    }

    toTreeWidgetItem *next = NULL;

    for (toTreeWidgetItem *item = tListView.firstChild();item;item = next)
    {
        if (!tListView.getOnlySelection() || item->isSelected())
        {

            toResultViewItem * resItem = dynamic_cast<toResultViewItem *>(item);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);

            if (bgcolor.isEmpty())
                bgcolor = QString::fromLatin1(" BGCOLOR=#cfcfff");
            else
                bgcolor = QString::null;
            QString line;
            for (int i = 0;i < column_count;i++)
            {
                QString text;

                if (resItem)
                    text = resItem->allText(i);
                else if (chkItem)
                    text = chkItem->allText(i);
                else
                    text = item->text(i);

                line += indent;
                line += QString::fromLatin1("%1\t").arg(text);
            }
            line = line.left(line.length() - 1);
            endLine(output);
            output += line;
        }

        if (item->firstChild())
        {
            indent += QString::fromLatin1(" ");
            next = item->firstChild();
        }
        else if (item->nextSibling())
            next = item->nextSibling();
        else
        {
            next = item;
            do
            {
                next = next->parent();
                indent.truncate(indent.length() - 1);
            }
            while (next && !next->nextSibling());
            if (next)
                next = next->nextSibling();
        }
    }

    return output;
}


QString toListViewFormatterTabDel::getFormattedString(toExportSettings &settings,
        const toResultModel *model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();
    QString separator = settings.Separator;
    QString delimiter = settings.Delimiter;

    QString output;
    QString indent;

    if (settings.IncludeHeader)
    {
        for (int j = 0; j < columns; j++)
            output += QString("%1\t").arg(model->headerData(
                                              j,
                                              Qt::Horizontal,
                                              Qt::DisplayRole).toString());
        if (output.length() > 0)
            output = output.left(output.length() - separator.length());

        endLine(output);
    }

    QVector<int> slist = selectedList(settings.selected);
    for (int row = 0; row < rows; row++)
    {
        if (settings.OnlySelection && !slist.contains(row))
            continue;

        QString line;
        for (int i = 0; i < columns; i++)
        {
            QString text = model->data(row, i).toString();
            line += indent;
            line += QString::fromLatin1("%1\t").arg(text);
        }

        line = line.left(line.length() - separator.length());
        output += line;
        endLine(output);
    }

    return output;
}
