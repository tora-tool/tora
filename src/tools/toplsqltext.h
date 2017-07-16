
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#pragma once

#include "widgets/totoolwidget.h"
//#include "editor/tosqltext.h"
//#include "tosqlparse.h"
//#include "tools/toplsqltext.h"

class QTreeView;
class QTabWidget;
class QModelIndex;
class QSplitter;
class QAction;
class QMenu;

class toCodeModel;
class toPLSQLWidget;
class toTreeWidgetItem;
class toPLSQLEditor;
class toResultSchema;

#if TORA3_MEMOEDITOR
/*! \brief Advanced PL/SQL Editor. It's based on
Debugger code. But it can compile PL/SQL unit in all cases.
Even if is the user without debug grants/roles.

It can compile for "production" and for "warnings" as well.
There is implemented navigation for bugs etc.
*/
class toPLSQL : public toToolWidget
{
        Q_OBJECT;

        toConnection &conn;

        // Toolbar
        void createActions(void);

        toResultSchema *Schema;
        QAction   *refreshAct;
        QAction   *describeAct;
        QAction   *newSheetAct;
        QAction   *compileAct;
        QAction   *compileWarnAct;
        QAction   *closeAct;
        QAction   *closeAllAct;
        QAction   *nextErrorAct;
        QAction   *previousErrorAct;
        QAction   *checkCodeAct;

        // Extra menu
        QMenu *ToolMenu;

        // Content pane
        QTreeView   *Objects;
        toCodeModel *CodeModel;
        QSplitter   *splitter;
        // Editors
        QTabWidget *Editors;

        void updateCurrent(void);
        QString editorName(const QString &schema,
                           const QString &object,
                           const QString &type);
        QString editorName(toPLSQLWidget *text);


        bool viewSource(const QString &schema,
                        const QString &name,
                        const QString &type,
                        int line,
                        bool current = false);

        void closeEditor(toPLSQLWidget* &editor);
        void parseResults(const QString buf, QMultiMap<int, QString> &res);

        // checks if given object has any source code
        bool hasCode(const QString &schema, const QString &type, const QString &name);

    protected:
        void closeEvent(QCloseEvent *) override;

    public:
        toPLSQL(QWidget *parent, toConnection &connection);

        virtual ~toPLSQL();

        toPLSQLWidget *currentEditor(void);
        QString schema(void) const override;
        toPLSQLEditor *getAnotherPart(QString &pSchema, QString &pObject, QString &pType);

    public slots:
        void compile(void);
        void compileWarn(void);
        void changeSchema(int);
        void refresh(void);
        void changePackage(const QModelIndex &);
        void changePackage(const QModelIndex &, const QModelIndex &);
        void prevError(void);
        void nextError(void);
        void showSource(toTreeWidgetItem *);
        void newSheet(void);
        void slotWindowActivated(toToolWidget *w) override;
        void closeEditor(void);
        void closeEditor(int);
        void closeAllEditor(void);
        void checkCode(void);
    private slots:
        void describe(void);
};

#endif
