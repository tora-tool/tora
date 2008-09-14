
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tolistviewformattertext.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

#include <iostream>
#include <QVector>

namespace
{
toListViewFormatter* createText()
{
    return new toListViewFormatterText();
}
const bool registered = toListViewFormatterFactory::Instance().Register(toListViewFormatterIdentifier::TEXT, createText);
}

toListViewFormatterText::toListViewFormatterText() : toListViewFormatter()
{
}

toListViewFormatterText::~toListViewFormatterText()
{
}

QString toListViewFormatterText::getFormattedString(toListView& tListView)
{
    int column_count = tListView.columns();
    QString separator = tListView.getSep();
    QString delimiter = tListView.getDel();

    QVector<int> sizes(column_count);
    int level = 0;
    for (int i = 0;i < column_count;i++)
        if (tListView.getIncludeHeader())
            sizes[i] = (tListView.headerItem())->text(i).length();
        else
            sizes[i] = 0;

    {
        toTreeWidgetItem *next = NULL;
        for (toTreeWidgetItem *item = tListView.firstChild();item;item = next)
        {
            toResultViewItem * resItem = dynamic_cast<toResultViewItem *>(item);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);

            if (!tListView.getOnlySelection() || item->isSelected())
            {
                for (int i = 0;i < column_count;i++)
                {
                    int csiz;
                    if (resItem)
                        csiz = resItem->allText(i).length();
                    else if (chkItem)
                        csiz = chkItem->allText(i).length();
                    else
                        csiz = item->text(i).length();
                    if (i == 0)
                        csiz += level;
                    if (sizes[i] < csiz)
                        sizes[i] = csiz;
                }
            }

            if (item->firstChild())
            {
                level++;
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
                    level--;
                }
                while (next && !next->nextSibling());
                if (next)
                    next = next->nextSibling();
            }
        }
    }


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
            output += QString::fromLatin1("%1 ").arg((tListView.headerItem())->text(j), -sizes[j]);

        if (output.length() > 0)
            output = output.left(output.length() - 1);

        endLine(output);

        for (int k = 0;k < column_count;k++)
        {
            for (int l = 0;l < sizes[k];l++)
                output += QString::fromLatin1("=");
            if (k != column_count - 1)
                output += QString::fromLatin1(" ");
        }

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
                line += QString::fromLatin1("%1 ").arg(text, (i == 0 ? indent.length() : 0) - sizes[i]);
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


QString toListViewFormatterText::getFormattedString(toExportSettings &settings,
        const toResultModel *model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();
    QString separator = settings.Separator;
    QString delimiter = settings.Delimiter;

    QString output;
    QVector<int> sizes(columns);

    // must get widest length for each column

    // zero array or (if writing headers, set their size)
    for (int i = 0; i < columns; i++)
    {
        if (settings.IncludeHeader)
        {
            sizes[i] = model->headerData(
                           i,
                           Qt::Horizontal,
                           Qt::DisplayRole).toString().length();
        }
        else
            sizes[i] = 0;
    }

    // loop through model and get column widths
    for (int row = 0; row < rows; row++)
    {
        for (int column = 0; column < columns; column++)
        {
            QVariant data = model->data(row, column);
            QString v;
            if (data.isNull())
                v = "{null}";
            else
                v = data.toString();

            int len = v.length();
            if (len > sizes[column])
                sizes[column] = len;
        }
    }

    // write header data to fixed widths
    if (settings.IncludeHeader)
    {
        for (int column = 0; column < columns; column++)
        {
            QString value = model->headerData(
                                column,
                                Qt::Horizontal,
                                Qt::DisplayRole).toString();

            output += value;
            for (int left = value.length(); left <= sizes[column]; left++)
                output += ' ';
        }

        endLine(output);

        // write ==== border
        for (int column = 0; column < columns; column++)
        {
            for (int left = 0; left < sizes[column]; left++)
                output += '=';
            output += ' ';
        }

        endLine(output);
    }

    // write data
    QVector<int> slist = selectedList(settings.selected);
    for (int row = 0; row < rows; row++)
    {
        if (settings.OnlySelection && !slist.contains(row))
            continue;

        for (int column = 0; column < columns; column++)
        {
            QVariant data = model->data(row, column);
            QString value;
            if (data.isNull())
                value = "{null}";
            else
                value = data.toString();

            output += value;
            for (int left = value.length(); left <= sizes[column]; left++)
                output += ' ';
        }

        endLine(output);
    }

    return output;
}
