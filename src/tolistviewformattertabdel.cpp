/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "tolistviewformattertabdel.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"

#include <iostream>

namespace
{
toListViewFormatter* createTabDel()
{
    return new toListViewFormatterTabDel();
}
const bool registered = toListViewFormatterFactory::Instance().Register(toListViewFormatterIdentifier::TAB_DELIMITED, createTabDel);
}


toListViewFormatterTabDel::toListViewFormatterTabDel() : toListViewFormatter()
{
}

toListViewFormatterTabDel::~toListViewFormatterTabDel()
{
}

QString toListViewFormatterTabDel::getFormattedString(toListView& tListView)
{
    std::cout << "Yep,  im FORMATTERTABDEL\n";
    int column_count = tListView.columns();
    QString separator = tListView.getSep();
    QString delimiter = tListView.getDel();


    int *sizes = NULL;
    try
    {
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
#ifdef Q_OS_WIN32

            output += "\r\n";
#else

            output += "\n";
#endif

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
#ifdef Q_OS_WIN32

                line += "\r\n";
#else

                line += "\n";
#endif

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
        delete[] sizes;
        return output;
    }
    catch (...)
    {
        delete[] sizes;
        throw;
    }

}


QString toListViewFormatterTabDel::getFormattedString(toExportSettings &settings,
                                                      const toResultModel *model) {
    return QString::null;
}
