/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "tolistviewformattercsv.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

#include <qregexp.h>
#include <iostream>
#include <vector>

namespace
{
toListViewFormatter* createCSV()
{
    return new toListViewFormatterCSV();
}
const bool registered = toListViewFormatterFactory::Instance().Register(
                            toListViewFormatterIdentifier::CSV, createCSV);
}


toListViewFormatterCSV::toListViewFormatterCSV() : toListViewFormatter()
{
}

toListViewFormatterCSV::~toListViewFormatterCSV()
{
}

QString toListViewFormatterCSV::QuoteString(const QString &str)
{
    static QRegExp quote(QString::fromLatin1("\""));
    QString t = str;
    t.replace(quote, QString::fromLatin1("\"\""));
    return t;
}

QString toListViewFormatterCSV::getFormattedString(toListView& tListView)
{
    int column_count = tListView.columns();
    QString separator = tListView.getSep();
    QString delimiter = tListView.getDel();

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
            output += QString::fromLatin1("%1%2%3%4").
                      arg(delimiter).
                      arg(QuoteString((tListView.headerItem())->text(j))).
                      arg(delimiter).
                      arg(separator);
        if (output.length() > 0)
            output = output.left(output.length() - separator.length());

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

                line += QString::fromLatin1("%1%2%3%4").
                        arg(delimiter,
                            QuoteString(text),
                            delimiter,
                            separator);
            }
            line = line.left(line.length() - separator.length());

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


QString toListViewFormatterCSV::getFormattedString(toExportSettings &settings,
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
            output += QString::fromLatin1("%1%2%3%4").
                      arg(delimiter).
                      arg(QuoteString(model->headerData(
                                          j,
                                          Qt::Horizontal,
                                          Qt::DisplayRole).toString())).
                      arg(delimiter).
                      arg(separator);
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

            line += QString::fromLatin1("%1%2%3%4").
                    arg(delimiter,
                        QuoteString(text),
                        delimiter,
                        separator);
        }

        line = line.left(line.length() - separator.length());
        output += line;
        endLine(output);
    }

    return output;
}
