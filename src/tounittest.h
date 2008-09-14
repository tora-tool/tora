/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOUNITTEST_H
#define TOUNITTEST_H

#include "totool.h"

class toWorksheetWidget;
class toResultTableView;
class toOutput;
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
    bar := 2;
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
        virtual bool canHandle(toConnection &conn);

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
        toWorksheetWidget * worksheet;
        //! A "result table" for displaying all callable PL/SQL blocks
        toResultTableView * codeList;
        //! A "result table" for displaying the package methods
        toResultTableView * packageList;

        toResultCombo *Schema;

        //! Spool output
        toOutput * output;

        QSplitter * splitter;
        QSplitter * codeSplitter;
        QSplitter * editorSplitter;

    private slots:
        //! Prepare for SQL test script creation. It's finished in the handleDone().
        void packageList_selectionChanged();
        //! Allow user to chose one of PL/SQL code unit.
        void codeList_selectionChanged();
        //! Setup and/or update the codeList
        void refreshCodeList();
        //! Set m_owner on schema change
        void changeSchema(const QString &);
        //! Finish the test script and show it in the worksheet instance.
        void handleDone();
};

#endif
