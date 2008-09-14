
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

#ifndef TOLISTVIEWFORMATTER
#define TOLISTVIEWFORMATTER

#include <qstring.h>
#include <QModelIndexList>
#include <QVector>

class toListView;
class toResultModel;


/**
 * Class for storing settings for export.
 *
 */
class toExportSettings
{
public:
    bool    IncludeHeader;
    bool    OnlySelection;
    int     Type;
    QString Separator;
    QString Delimiter;

    QModelIndexList selected;

    toExportSettings(bool _header,
                     bool _selection,
                     int _type,
                     const QString &_sep,
                     const QString &_del)
    {
        IncludeHeader = _header;
        OnlySelection = _selection;
        Type          = _type;
        Separator     = _sep;
        Delimiter     = _del;
    }
};


class toListViewFormatter
{
public:
    toListViewFormatter();
    virtual ~toListViewFormatter();
    virtual QString getFormattedString(toListView& tListView) = 0;
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model) = 0;

protected:
    virtual void endLine(QString &output);
    // build a vector of selected rows for easy searching
    virtual QVector<int> selectedList(const QModelIndexList &selected);
};

#endif
