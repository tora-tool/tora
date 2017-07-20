
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

#include "core/totool.h"
#include "editor/todebugtext.h"
//#include "tosqlparse.h"
//#include "tools/toplsqleditor.h"

class QTreeWidget;
class QSplitter;
class QTreeWidgetItem;

class toTreeWidgetItem;
class toTreeWidget;
class toPLSQL;
class toPLSQLWidget;

/*! \brief An editor widget for PL/SQL Editor.
*/
class toPLSQLEditor : public toDebugText
{
        Q_OBJECT
        QString Schema;
        QString Object;
        QString Type;

        static int ID;

        // Reimplement this in order to handle saving of
        // package specification and body in one file
        virtual bool editSave(bool askfile);
        // Pointer to a parent PLSQLEditor tool.
        // This is later used when saving one part of package (spec or body) to
        // find another part and save it as well (if set like that in preferences).
        toPLSQL *Editor;

        // Error raised when CALLING compilation (as opposed to errors found during compilation)
        // Example: ORA-02303: cannot drop or replace a type with type or table dependents
        QString compilation_error;

    public:
        toPLSQLEditor(QWidget *parent = 0);
        toPLSQLWidget * parent_widget;

        void setEditor(toPLSQL * pEditor)
        {
            Editor = pEditor;
        }

        enum CompilationType {Production,
                              Warning
                             };

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

        bool readData(toConnection &connection/*, toTreeWidget **/);
        bool readErrors(toConnection &connection);
        bool compile(CompilationType t);

    signals:
        void errorsChanged(const QString & type, const QMultiMap<int, QString> & values, const bool cleanup = false);
        void warningsChanged(const QMap<int, QString> values);
        void contentChanged();

};

/*! \brief A main widget for PL/SQL Editor.
It handles all Content and Messages cooperation with Editor.
*/
class toPLSQLWidget : public QWidget
{
        Q_OBJECT

    public slots:
        void applyResult(const QString &, const QMultiMap<int, QString>&, const bool = false);
        // expands or colapses results pane depending on number of errors, warnings etc.
        void resizeResults(void);

    private:
        //toTreeWidget * m_contents;
        QTreeWidget * m_result;
        toPLSQLEditor * m_editor;

        QSplitter * m_splitter;
        //QSplitter * m_contentSplitter;

        QTreeWidgetItem * m_errItem;    // main branch of errors
        QTreeWidgetItem * m_warnItem;   // main branch of warnings
        QTreeWidgetItem * m_staticItem; // main branch of static check observations

        // Count of errors, warnings and static check observations. Values are set after
        // compilation in function readErrors and after static check execution.
        // Count is used to auto-hide/display pane displaying list of errors, warnings etc.
        // Placed here because values are used not only after compiling, but also after
        // static code check and vice versa.
        int errorCount, warningCount, staticCount;

// TODO parse
//    void updateArguments(toSQLParse::statement &statements,
//                         toTreeWidgetItem *parent);
//    void updateContent(toSQLParse::statement &statements,
//                       toTreeWidgetItem *parent,
//                       const QString &id = QString::null);
        void updateContent(toPLSQLEditor* editor);
        // removes all errors, warnings and static check messages from result pane
        void cleanupResults(const QString & type = NULL);
        void setCount(const QString & type, const int count);

    public:
        toPLSQLWidget(QWidget * parent = 0);
        ~toPLSQLWidget();

		toPLSQLEditor* editor()
        {
            return m_editor;
        };

    private slots:
        void goToError(QTreeWidgetItem *, QTreeWidgetItem *);
        void updateContent()
        {
            updateContent(m_editor);
        };
        void changeContent(toTreeWidgetItem *);

};
