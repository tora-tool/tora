
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

#ifndef TOSCRIPT_H
#define TOSCRIPT_H

#include "config.h"
#include "tohelp.h"
#include "ui_toscriptui.h"

#include <list>
#include <algorithm>

class toExtract;
class toListView;
class toWorksheet;


/*! \brief DB objects exporter/comparator.
It's using toscriptui.ui layout.
\warning Beware of removing confContainer widget in toscriptui.ui file.
It's mandatory for compilation and it can be lost very easily during
designer's editations.
*/
class toScript : public toToolWidget
{
    Q_OBJECT;

    Ui::toScriptUI *ScriptUI;
    //! Additional widget/Tab widget. DDL scripts container.
    toWorksheet *Worksheet;
    //! Result lists for its action
    toListView *DropList;
    toListView *CreateList;
    toListView *SearchList;
    //! Text report summary widget/Tab
    QTextBrowser *Report;

    /*! Setup the selected (referenced) toExtract depending
    on GUI widgets. Including Resize tab.
    */
    void setupExtract(toExtract &);

    /*! Create commin string list with all selected objects
    for given objects selection
    */
    std::list<QString> createObjectList(QItemSelectionModel * selections);

    void fillDifference(std::list<QString> &objects, toTreeWidget *list);

    //! \brief Create separated strings for exporter
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
    //! Perform requested action (depending on the mode)
    void execute(void);

    //! Enable/disable GUI widgets depending on mode specified
    void changeMode(int);

    void keepOn(toTreeWidgetItem *item);

    //! Size tab - add new item
    void newSize(void);
    //! Size tab - remove current item
    void removeSize(void);

    /*! Ask user for output location. A file or a directory.
    It's stored in ScriptUI->Filename then
    */
    void browseFile(void);
};

inline void toScript::PrefixString::operator()(QString& txt)
{
    _l.push_back(_s + txt);
}

inline toScript::PrefixString::PrefixString(std::list<QString> & l, QString s) : _l(l), _s(s)
{}

#endif
