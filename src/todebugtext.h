
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#ifndef TODEBUGTEXT_H
#define TODEBUGTEXT_H

#include "config.h"
#include "tohighlightedtext.h"

#include <totreewidget.h>
//Added by qt3to4:
#include <QString>
#include <QMouseEvent>

#include <algorithm>

class toConnection;
class toDebug;

class toBreakpointItem : public toTreeWidgetItem
{
    int Line;
    int Namespace;
public:
    toBreakpointItem(toTreeWidget *parent, toTreeWidgetItem *after,
                     const QString &schema, const QString &type, const QString &object, int line);
    void setBreakpoint(void);
    void clearBreakpoint(void);
    void disableBreakpoint(void);
    void enableBreakpoint(void);
    int line()
    {
        return Line;
    }
};

class toDebugText : public toHighlightedText
{
    Q_OBJECT
    QString Schema;
    QString Object;
    QString Type;
    int LastX;
    toDebug *Debugger;

    toTreeWidget *Breakpoints;
    bool NoBreakpoints;
    toBreakpointItem *FirstItem;
    toBreakpointItem *CurrentItem;

    bool checkItem(toBreakpointItem *item);
    bool hasBreakpoint(int row);
    int breakMarker;
    int disabledBreakMarker;
    static int ID;
public:
    toDebugText(toTreeWidget *breakpoints,
                QWidget *parent,
                toDebug *debugger);

    void toggleBreakpoint(int row = -1, bool enable = false);

    void setData(const QString &schema, const QString &type, const QString &data);
    const QString &schema(void) const
    {
        return Schema;
    }
    const QString &object(void) const
    {
        return Object;
    }
    void setType(const QString &type)
    {
        setData(Schema, type, Object);
    }
    void setSchema(const QString &schema)
    {
        setData(schema, Type, Object);
    }
    const QString &type(void) const
    {
        return Type;
    }
    void clear(void);

    bool readData(toConnection &connection, toTreeWidget *);
    bool readErrors(toConnection &connection);
    bool compile(void);

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
protected slots:
    virtual void toggleBreakpoint(int margin, int line, Qt::KeyboardModifiers state);
protected:
    virtual void mouseMoveEvent (QMouseEvent *me);

};

#endif
