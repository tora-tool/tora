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

#ifndef TOSCRIPT_H
#define TOSCRIPT_H

#include "config.h"
#include "tohelp.h"
#include "ui_toscriptui.h"

#include <list>
#include <algorithm>

class QTextBrowser;
class toConnection;
class toExtract;
class toListView;
class toScriptUI;
class toWorksheet;

class toScript : public toToolWidget {
    Q_OBJECT;

    Ui::toScriptUI *ScriptUI;
    toWorksheet *Worksheet;
    toListView *DropList;
    toListView *CreateList;
    toListView *SearchList;
	QTextBrowser *Report;

    void setupExtract(toExtract &);
    void changeConnection(int, bool source);
    void changeSchema(int, bool source);
    std::list<QString> createObjectList(toTreeWidget *);
    void fillDifference(std::list<QString> &objects, toTreeWidget *list);
    void readOwnerObjects(toTreeWidgetItem *item, toConnection &conn);

    struct PrefixString
    {
public:
        PrefixString(std::list<QString> & l, QString s);
        void operator()(QString& tmp);
private:
        std::list<QString> & _l;
        QString _s;
    };

protected:
    void closeEvent(QCloseEvent *event);

public:
    toScript(QWidget *parent, toConnection &connection);
    virtual ~toScript();

public slots:
    void execute(void);
    void changeMode(int);
    void changeSource(int val)
    {
        changeConnection(val, true);
    }
    void changeDestination(int val)
    {
        changeConnection(val, false);
    }
    void keepOn(toTreeWidgetItem *item);
    void objectClicked(toTreeWidgetItem *item);
    void changeSourceSchema(int val)
    {
        changeSchema(val, true);
    }
    void changeDestinationSchema(int val)
    {
        changeSchema(val, false);
    }
    void newSize(void);
    void removeSize(void);
    void browseFile(void);

    void expandSource(toTreeWidgetItem *item);
    void expandDestination(toTreeWidgetItem *item);
    void addConnection(const QString &name);
    void delConnection(const QString &name);
};

inline void toScript::PrefixString::operator()(QString& txt)
{
    _l.push_back(_s + txt);
}

inline toScript::PrefixString::PrefixString(std::list<QString> & l, QString s) : _l(l), _s(s)
{}

#endif
