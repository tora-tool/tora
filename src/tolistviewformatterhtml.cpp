
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

#include "tolistviewformatterhtml.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

#include <iostream>

namespace
{
toListViewFormatter* createHTML()
{
    return new toListViewFormatterHTML();
}
const bool registered = toListViewFormatterFactory::Instance().Register(
                            toListViewFormatterIdentifier::HTML, createHTML);
}


toListViewFormatterHTML::toListViewFormatterHTML() : toListViewFormatter()
{
}

toListViewFormatterHTML::~toListViewFormatterHTML()
{
}

QString toListViewFormatterHTML::getFormattedString(toListView& tListView)
{
    int column_count = tListView.columns();
    QString separator = tListView.getSep();
    QString delimiter = tListView.getDel();

    int *sizes = NULL;
    try
    {
        QString output;
        output = QString::fromLatin1("<HTML><HEAD><TITLE>%1</TITLE></HEAD><BODY><TABLE CELLSPACING=0 BORDER=0>").
                 arg(tListView.sqlName());

        QString indent;

        QString bgcolor;
        if (tListView.getIncludeHeader())
        {
            if (bgcolor.isEmpty())
                bgcolor = QString::fromLatin1("nonull");
            else
                bgcolor = QString::null;
            output += QString::fromLatin1("<TR BGCOLOR=#7f7f7f>");
            for (int j = 0; j < column_count; j++)
            {
                output += QString::fromLatin1("<TH ALIGN=LEFT BGCOLOR=#cfcfcf>");
                output += (tListView.headerItem())->text(j);
                output += QString::fromLatin1("</TH>");
            }
            if (tListView.getIncludeHeader())
                output += QString::fromLatin1("</TR>");

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
                line = QString::fromLatin1("<TR%1>").arg(bgcolor);

                for (int i = 0;i < column_count;i++)
                {
                    QString text;

                    if (resItem)
                        text = resItem->allText(i);
                    else if (chkItem)
                        text = chkItem->allText(i);
                    else
                        text = item->text(i);

                    line += QString::fromLatin1("<TD%1>").arg(bgcolor);
                    if (i == 0)
                        line += indent;

                    text.replace('&', "&amp;");
                    text.replace('<', "&lt;");
                    text.replace('>', "&gt");

                    line += QString::fromLatin1("<PRE>");
                    line += text;
                    line += QString::fromLatin1("</PRE>");
                    line += QString::fromLatin1("</TD>");
                }
                line += QString::fromLatin1("</TR>");

                endLine(line);
                output += line;
            }

            if (item->firstChild())
            {
                indent += QString::fromLatin1("&nbsp;");
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
                    indent.truncate(indent.length() - 5);
                }
                while (next && !next->nextSibling());
                if (next)
                    next = next->nextSibling();
            }
        }
        output += QString::fromLatin1("</TABLE></BODY></HTML>");
        delete[] sizes;
        return output;
    }
    catch (...)
    {
        delete[] sizes;
        throw;
    }
}


QString toListViewFormatterHTML::getFormattedString(toExportSettings &settings,
        const toResultModel *model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();
    QString separator = settings.Separator;
    QString delimiter = settings.Delimiter;

    QString output;
    QString indent;

    output = QString("<HTML><HEAD><TITLE>Export</TITLE></HEAD><BODY><TABLE>");
    endLine(output);

    QString bgcolor;
    if (settings.IncludeHeader)
    {
        output += QString("<TR>");
        endLine(output);

        for (int j = 0; j < columns; j++)
        {
            output += QString("\t<TH>");
            endLine(output);

            QString text = model->headerData(j,
                                             Qt::Horizontal,
                                             Qt::DisplayRole).toString();
            text.replace('&', "&amp;");
            text.replace('<', "&lt;");
            text.replace('>', "&gt");

            output += "\t\t" + text;

            endLine(output);
            output += QString("\t</TH>");
            endLine(output);
        }

        output += "</TR>";
        endLine(output);
    }

    QVector<int> slist = selectedList(settings.selected);
    for (int row = 0; row < rows; row++)
    {
        if (settings.OnlySelection && !slist.contains(row))
            continue;

        output += "<TR>";
        endLine(output);

        for (int i = 0; i < columns; i++)
        {
            output += QString("\t<TD>");
            endLine(output);

            QString text = model->data(row, i).toString();
            text.replace('&', "&amp;");
            text.replace('<', "&lt;");
            text.replace('>', "&gt");

            output += "\t\t" + text;
            endLine(output);

            output += QString("\t</TD>");
            endLine(output);
        }

        output += "</TR>";
        endLine(output);
    }

    output += "</TABLE></BODY></HTML>";
    return output;
}
