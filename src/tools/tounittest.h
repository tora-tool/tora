
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

#ifndef TOUNITTEST_H
#define TOUNITTEST_H

#include "widgets/totoolwidget.h"

class QSplitter;
class QTreeView;
class QModelIndex;

class toHighlightedEditor;
class toSqlText;
class toResultTableView;
class toCodeModel;
class toResultCombo;
class toResultModel;


/*! \brief The Unit Test tool for schema browser "second tab" (see toBrowser).
The tool area is split into two widgets:
PackageList (1) - it's the lost of package/function/procedure members,
e.g. all package's procedures etc. When user chose one of these members,
the test script is created in the worksheet (2).

This tool is available for Oracle only in this release (see canHandle()).

What is the Unit Test Script? The anonymous PL/SQL block with the following
structure:

DECLARE
    -- declarations of IN/OUT parameters
    foo number;
    bar varchar2(22);
BEGIN
    -- definitions of parameters
    foo := 1;
    bar := 'a';
    -- calling requested unit
    MYOWN.COOLPACKAGE.FOOBAR(
        foo => foo,
        bar => bar
        );
    -- printing of the results
    sys.dbms_output.put_line('foo: ' || foo);
    sys.dbms_output.put_line('bar: ' || bar);
END;

\note: All results are printed to the SERVEROUTPUT.

\warning: It can handle only basic data types currently.
No PLSQLdata (record of etc.) supported yet.

\author Petr Vanek <petr@scribus.info>
*/
class toUnitTest : public toToolWidget
{
        Q_OBJECT

    public:
        toUnitTest(QWidget * parent, toConnection &connection);
        ~toUnitTest();

        /** \brief Inform upstream parent object for what platform
        is this tool available. It's Oracle only this time.
        */
        virtual bool canHandle(const toConnection &conn);

    private:
        //! Current schema. Updated in the toBrowser::changeSecond()
        QString m_owner;
        //! Current object name. Updated in the toBrowser::changeSecond()
        QString m_name;
        //! Current object type. Updated in the toBrowser::changeSecond()
        QString m_type;

        /*! Internal result model to setup the testing script.
        USed for SQLPackageParams etc.*/
        toResultModel * m_model;

        //! SQL editor (worksheet) where is shown the test script.
        toSqlText *editor;
        //! A "result table" for displaying all callable PL/SQL blocks
        QTreeView * codeList;
        toCodeModel * codeModel;
        //! A "result table" for displaying the package methods
        toResultTableView * packageList;

        toResultCombo *Schema;

        QSplitter * splitter;
        QSplitter * codeSplitter;

    private slots:
        //! Prepare for SQL test script creation. It's finished in the handleDone().
        void slotPackageListSelectionChanged();
        //! Allow user to chose one of PL/SQL code unit.
        void slotChangePackage(const QModelIndex &);
        //! Setup and/or update the codeList
        void slotRefreshCodeList();
        //! Set m_owner on schema change
        void slotChangeSchema(const QString &);
        //! Finish the test script and show it in the worksheet instance.
        void slotHandleDone();

        virtual void slotWindowActivated(toToolWidget *widget) {};
};

#endif
