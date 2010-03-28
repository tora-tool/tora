
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

#include "tohighlightedtext.h"
#include "tosqlparse.h"

#include <qapplication.h>
#include <QDebug>

#include <stdio.h>

bool toMonolithic(void)
{
    return false;
}

void printSt(toSQLParse::statement &stat, int level)
{
    for (int i = 0; i < level; i++)
        printf("    ");

    switch (stat.StatementClass)
    {
    case toSQLParse::statement::unknown:
        printf("[U]");
        break;
    case toSQLParse::statement::ddldml:
        printf("[D]");
        break;
    case toSQLParse::statement::plsqlblock:
        printf("[P]");
    default:
        printf("[B]");
    }

    switch (stat.Type)
    {
    case toSQLParse::statement::Block:
        printf("Block:");
        break;
    case toSQLParse::statement::Statement:
        printf("Statement:");
        break;
    case toSQLParse::statement::List:
        printf("List:");
        break;
    case toSQLParse::statement::Keyword:
        printf("Keyword:");
        break;
    case toSQLParse::statement::Token:
        printf("Token:");
        break;
    case toSQLParse::statement::EndOfStatement:
        printf("EndOfStatement:");
        break;
    case toSQLParse::statement::Raw:
        printf("Raw:");
        break;
    }
    printf("%s (%d)\n", (const char *)stat.String.toUtf8(), stat.Line);
    if (!stat.Comment.isNull())
    {
        for (int i = 0; i < level; i++)
            printf("    ");
        printf("Comment:%s\n", (const char *)stat.Comment.toUtf8());
    }
    for (std::list<toSQLParse::statement>::iterator i = stat.subTokens().begin();
            i != stat.subTokens().end();
            i++)
        printSt(*i, level + 1);
}

QString decodeClass(int sc)
{
    if (sc == toSQLParse::statement::unknown)
        return "Unknown";
    else if (sc == toSQLParse::statement::ddldml)
        return "Ddldml";
    else if (sc == toSQLParse::statement::plsqlblock)
        return "Plsqlblock";
    else
        return "Bubamara!";
} // decodeClass

int main(int argc, char **argv)
{
    //TODO: rewrite this with one list and structure :)
    // each test set contains:
    std::list<QString> testSet; // statement (or a set of statements) to be parsed
    std::list<int> testCount;   // count of statements (to be checked against parser result)
    std::list<int> testClass;   // statement class

    //===================================================
    // Test #1 Simplest DML statement ever
    testSet.insert(testSet.end(),
                   "select sysdate from dual;");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #2 Double simplest DML statement
    testSet.insert(testSet.end(),
                   "select sysdate from dual;"
                   "select sysdate from dual;");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #3 Simple DDL statement
    testSet.insert(testSet.end(),
                   "create table test ( col varchar(12) );");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #4 Simple DML statement with new line in text
    testSet.insert(testSet.end(),
                   "insert into prova3 (prova)\n"
                   "values ('This insert contains a ''\n"
                   "and now it goes to new line');");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #5 Procedure with exceptions and conditional statements
    testSet.insert(testSet.end(),
                   "create or replace PROCEDURE prova1\n"
                   "is\n"
                   "v_tmp NUMBER(1);\n"
                   "begin\n"
                   "begin\n"
                   "select 0 into v_tmp from dual;\n"
                   "exception\n"
                   "when 1 = 1 then\n"
                   "    v_tmp := 'Dadum';\n"
                   "when others then\n"
                   "if sqlcode=0 then\n"
                   "null;\n"
                   "else\n"
                   "null;\n"
                   "end if;\n"
                   "end;\n"
                   "\n"
                   "if v_tmp=0 then\n"
                   "null;\n"
                   "end if;\n"
                   "end;");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #6 Comment
    testSet.insert(testSet.end(), "\n"
                   "comment on column prova1.prova1 is 'This comment is\n"
                   "on more than one line';\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #7 Package specification
    testSet.insert(testSet.end(), "\n"
                   "PACKAGE oasSIMActivation AS\n"
                   " FUNCTION ParseCommand(Command VARCHAR2,inICC VARCHAR2,Param VARCHAR2) RETURN VARCHAR2;\n"
                   "\n"
                   " PROCEDURE InsertActions(inCommandType VARCHAR2,\n"
                   "    Sim oasSIM%ROWTYPE,\n"
                   "    inParam VARCHAR2);\n"
                   "\n"
                   " PROCEDURE InsertActions(CommandType VARCHAR2,\n"
                   "    inICC VARCHAR2,\n"
                   "    inParam VARCHAR2);\n"
                   "\n"
                   " PROCEDURE InsertActions(inCommandType VARCHAR2,\n"
                   "    Service oasService%ROWTYPE);\n"
                   "\n"
                   " PROCEDURE InsertActions(CommandType VARCHAR2,\n"
                   "    inCustomerID NUMBER,\n"
                   "    inSubID NUMBER,\n"
                   "    inServiceType VARCHAR2,\n"
                   "    Param VARCHAR2);\n"
                   "END;\n"
                  );
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #8 Anonymous block with case statement
    testSet.insert(testSet.end(), "\n"
                   "DECLARE\n"
                   "grade CHAR(1);\n"
                   "appraisal VARCHAR2(20);\n"
                   "BEGIN\n"
                   "CASE grade\n"
                   "WHEN 'A' THEN appraisal := 'Excellent';\n"
                   "WHEN 'B' THEN appraisal := 'Very Good';\n"
                   "WHEN 'C' THEN appraisal := 'Good';\n"
                   "WHEN 'D' THEN appraisal := 'Fair';\n"
                   "WHEN 'F' THEN appraisal := 'Poor';\n"
                   "ELSE appraisal := 'No such grade';\n"
                   "END CASE;\n"
                   "IF appraisal IS NULL THEN\n"
                   "NULL;\n"
                   "END IF;\n"
                   "END;\n"
                  );
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #9 Procedure with conditional statement
    testSet.insert(testSet.end(), "\n"
                   "CREATE OR REPLACE PROCEDURE A(p IN INTEGER) AS\n"
                   "BEGIN\n"
                   "IF a = 1 THEN NULL;\n"
                   "ELSIF a = 2 THEN NULL;\n"
                   "ELSE NULL;\n"
                   "END IF;\n"
                   "END;\n"
                  );
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #10 Procedure with case statement
    //          Note! Should generate only ONE statement (block)
    testSet.insert(testSet.end(), "\n"
                   "CREATE OR REPLACE PROCEDURE A AS\n"
                   "BEGIN\n"
                   "CASE a\n"
                   "WHEN 1 THEN NULL;\n"
                   "WHEN 2 THEN NULL;\n"
                   "ELSE NULL;\n"
                   "END CASE;\n"
                   "END;\n"
                  );
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #11 Anonynous block without declaration with query (incorrect syntax but should parse anyway:)
    testSet.insert(testSet.end(),
                   "BEGIN\n"
                   "IF 1 == 1 THEN\n"
                   "NULL;\n"
                   "END IF;\n"
                   "IF appraisal IS NULL THEN\n"
                   "NULL;\n"
                   "ELSE\n"
                   "SELECT CASE WHEN dummy='X' THEN 'A' ELSE 'B' END,  2 FROM dual;\n"
                   "END IF;\n"
                   "END;\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #12
    testSet.insert(testSet.end(),
                   "select count(case when dummy = 'Y' then dummy\n"
                   "             else null end) as tot_str\n"
                   "from dual;\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #13
    testSet.insert(testSet.end(),
                   "SET TRANSACTION READ ONLY\n"
                   "\n"
                   "PROMPT Hello\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #14
    testSet.insert(testSet.end(),
                   "/* Test comment\n"
                   "*/\n"
                   "INSERT INTO cdrProcess(ProcessID,\n"
                   "         StartDate,\n"
                   "         EnvDate,\n"
                   "         ProgramID,\n"
                   "         OSUser,\n"
                   "         SystemUser,\n"
                   "         ExecName,\n"
                   "         ExecVersion,\n"
                   "         ExecParameters,\n"
                   "         HostName)\n"
                   "VALUES (:1,\n"
                   " SYSDATE,\n"
                   " SYSDATE,\n"
                   " :2,\n"
                   " :3,\n"
                   " :4,\n"
                   " :5,\n"
                   " :6,\n"
                   " :7,\n"
                   " :8);\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #15
    testSet.insert(testSet.end(),
                   "CREATE or REPLACE Procedure TEST_SPR\n"
                   "(\n"
                   "    IN_TICKET_NUM   IN  VARCHAR2\n"
                   ")\n"
                   "IS\n"
                   "\n"
                   "BEGIN\n"
                   "\n"
                   "BEGIN\n"
                   "\n"
                   "for cur_rec in (select emp_id from employees) loop\n"
                   "\n"
                   " update employees set emp_id = emp_id + 1\n"
                   " where emp_id = cur_rec.emp_id;\n"
                   " commit;\n"
                   "end loop;\n"
                   " \n"
                   "END;\n"
                   "END TEST_SPR;\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #16
    testSet.insert(testSet.end(),
                   "SELECT owner,\n"
                   "       OBJECT,\n"
                   "       TYPE FROM v$access\n"
                   " WHERE sid=:f1<char[101]>\n"
                   " ORDER BY owner,\n"
                   "   OBJECT,\n"
                   "   TYPE;\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #17
    testSet.insert(testSet.end(),
                   "CREATE TABLE ess.EssCalLog (\n"
                   "        CalID  CHAR(5) NOT NULL,  -- Calender type\n"
                   " SeqID  NUMBER(8) NOT NULL,\n"
                   " ActStt  CHAR(1) NOT NULL\n"
                   "  CONSTRAINT EssCalLog_CK_ActStt CHECK (ActStt IN ('A','D') ),\n"
                   " LogRun  CHAR(1) NOT NULL  -- Should runs of this type be logged\n"
                   "  CONSTRAINT EssCalLog_CK_LogRun CHECK (LogRun IN ('Y','N') ),\n"
                   " PrcID  NUMBER(8) NOT NULL\n"
                   "  CONSTRAINT EssCalDay_FK_PrcID REFERENCES ess.EssPrc(PrcID),\n"
                   " Dsc  VARCHAR2(4000) NOT NULL, -- Description of this type\n"
                   " CONSTRAINT EssCal_PK PRIMARY KEY (CalID,SeqID)\n"
                   "  USING INDEX TABLESPACE Index02 -- A Comment\n"
                   ");\n"
                   "-- Another comment\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #18
    testSet.insert(testSet.end(),
                   "CREATE OR REPLACE procedure spTuxGetAccData (oRet                        OUT  NUMBER,\n"
                   "          oNumSwt                     OUT  NUMBER)\n"
                   "IS\n"
                   "  vYear  CHAR(4);\n"
                   "BEGIN\n"
                   "    <<label>>\n"
                   "    DECLARE\n"
                   "      oTrdStt NUMBER;\n"
                   "    BEGIN\n"
                   "      oTrdStt := 0;\n"
                   "    END;\n"
                   "\n"
                   "    EXCEPTION\n"
                   "        WHEN VALUE_ERROR THEN\n"
                   "     oRet := 3;\n"
                   " WHEN NO_DATA_FOUND THEN\n"
                   "     oRet := 2;\n"
                   " WHEN OTHERS THEN\n"
                   "     oRet := 1;\n"
                   "END;\n"
                   "CREATE OR REPLACE procedure spTuxGetAccData as\n"
                   "  vYear  CHAR(4);\n"
                   "begin\n"
                   "  null;\n"
                   "end;\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #19
    testSet.insert(testSet.end(),
                   "-------------------------------------------------------------------\n"
                   "--    EssCal, Current calendar view\n"
                   "\n"
                   "CREATE VIEW ess.EssCal AS\n"
                   "        SELECT CalID,\n"
                   "        LogRun,\n"
                   "        PrcID,\n"
                   "        Dsc\n"
                   "   FROM ess.EssCalLog a\n"
                   "  WHERE SeqID = (SELECT MAX(aa.SeqID) FROM EssCalLog aa WHERE aa.CalID = a.CalID)\n"
                   "    AND ActStt = 'A';\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #20
    testSet.insert(testSet.end(),
                   "    /* A little comment\n"
                   "     */\n"
                   "    SELECT /*+\n"
                   "FULL(a)\n"
                   "*/ a.TskCod TskCod -- Test comment\n"
                   "      ,a.CreEdt CreEdt,\n"
                   "       a.TspActOprID /* One comment OprID */ , -- Another comment\n"
                   "       COUNT(1) Tot,\n"
                   "       COUNT(a.TspActOprID) Lft,\n"
                   "       b.TraCod TraCod,\n"
                   "       SUM(b.FinAmt) FinAmt,\n"
                   "       TraCod\n"
                   "  FROM EssTsk a,EssTra b\n"
                   " WHERE ((a.TspActOprID = 'Test') OR a.TspActOprID IS NULL)\n"
                   "   AND DECODE(a.TspActOprID,NULL,NULL,a.TskID) = b.TskID(+)\n"
                   " GROUP BY a.TskCod,a.CreEdt,a.TspActOprID,b.TraCod\n"
                   "HAVING COUNT(a.TspActOprID) > 0;\n"
                   "SELECT a.Sid \"-Id\",\n"
                   "       a.Serial# \"-Serial#\",\n"
                   "       a.SchemaName \"Schema\",\n"
                   "       a.Status \"Status\",\n"
                   "       a.Server \"Server\",\n"
                   "       a.OsUser \"Osuser\",\n"
                   "       a.Machine \"Machine\",\n"
                   "       a.Program \"Program\",\n"
                   "       a.Type \"Type\",\n"
                   "       a.Module \"Module\",\n"
                   "       a.Action \"Action\",\n"
                   "       a.Client_Info \"Client Info\",\n"
                   "       b.Block_Gets \"-Block Gets\",\n"
                   "       b.Consistent_Gets \"-Consistent Gets\",\n"
                   "       b.Physical_Reads \"-Physical Reads\",\n"
                   "       b.Block_Changes \"-Block Changes\",\n"
                   "       b.Consistent_Changes \"-Consistent Changes\",\n"
                   "       c.Value*10 \"-CPU (ms)\",\n"
                   "       a.Process \"-Process\",\n"
                   "       a.SQL_Address||':'||SQL_Hash_Value \" SQL Address\",\n"
                   "       a.Prev_SQL_Addr||':'||Prev_Hash_Value \" Prev SQl Address\"\n"
                   "  FROM v$session a,\n"
                   "       v$sess_io b,\n"
                   "       v$sesstat c\n"
                   " WHERE a.sid = b.sid(+)\n"
                   "   AND a.sid = c.sid(+) AND (c.statistic# = 12 OR c.statistic# IS NULL)\n"
                   " ORDER BY a.Sid;\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #21
    testSet.insert(testSet.end(),
                   "select a.TskCod TskCod,\n"
                   "       count(1) Tot\n"
                   "  from (select * from EssTsk where PrsID >= '1940') ,EssTra b\n"
                   " where decode(a.TspActOprID,NULL,NULL,a.PrsID)+5 = b.PrsID(+)\n"
                   " group by a.TskCod,a.CreEdt,a.TspActOprID,b.TraCod\n"
                   "having count(a.TspActOprID) > 0;\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #22
    testSet.insert(testSet.end(),
                   "CREATE OR REPLACE procedure spTuxGetAccData (oRet OUT  NUMBER)\n"
                   "AS\n"
                   "  vYear  CHAR(4);\n"
                   "BEGIN\n"
                   "    DECLARE\n"
                   "      oTrdStt NUMBER;\n"
                   "    BEGIN\n"
                   "      oTrdStt := 0;\n"
                   "    END;\n"
                   "    EXCEPTION\n"
                   "        WHEN VALUE_ERROR THEN\n"
                   "     oRet := 3;\n"
                   "END;");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #23 BIG TEST! Run all statemens at once. Should separate statements correctly.
    testSet.insert(testSet.end(), "\n"
                   "create table test ( col varchar(12) );\n"
                   "insert into prova3 (prova)\n"
                   "values ('This insert contains a ''\n"
                   "and now it goes to new line');\n"
                   "create or replace PROCEDURE prova1\n"
                   "is\n"
                   "v_tmp NUMBER(1);\n"
                   "begin\n"
                   "begin\n"
                   "select 0 into v_tmp from dual;\n"
                   "exception\n"
                   "when 1 = 1 then\n"
                   "    v_tmp := 'Dadum';\n"
                   "when others then\n"
                   "if sqlcode=0 then\n"
                   "null;\n"
                   "else\n"
                   "null;\n"
                   "end if;\n"
                   "end;\n"
                   "\n"
                   "if v_tmp=0 then\n"
                   "null;\n"
                   "end if;\n"
                   "end;"
                   "comment on column prova1.prova1 is 'This comment is\n"
                   "on more than one line';\n"
                   "PACKAGE oasSIMActivation AS\n"
                   " FUNCTION ParseCommand(Command VARCHAR2,inICC VARCHAR2,Param VARCHAR2) RETURN VARCHAR2;\n"
                   "\n"
                   " PROCEDURE InsertActions(inCommandType VARCHAR2,\n"
                   "    Sim oasSIM%ROWTYPE,\n"
                   "    inParam VARCHAR2);\n"
                   "\n"
                   " PROCEDURE InsertActions(CommandType VARCHAR2,\n"
                   "    inICC VARCHAR2,\n"
                   "    inParam VARCHAR2);\n"
                   "\n"
                   " PROCEDURE InsertActions(inCommandType VARCHAR2,\n"
                   "    Service oasService%ROWTYPE);\n"
                   "\n"
                   " PROCEDURE InsertActions(CommandType VARCHAR2,\n"
                   "    inCustomerID NUMBER,\n"
                   "    inSubID NUMBER,\n"
                   "    inServiceType VARCHAR2,\n"
                   "    Param VARCHAR2);\n"
                   "END;\n"
                   "DECLARE\n"
                   "grade CHAR(1);\n"
                   "appraisal VARCHAR2(20);\n"
                   "BEGIN\n"
                   "CASE grade\n"
                   "WHEN 'A' THEN appraisal := 'Excellent';\n"
                   "WHEN 'B' THEN appraisal := 'Very Good';\n"
                   "WHEN 'C' THEN appraisal := 'Good';\n"
                   "WHEN 'D' THEN appraisal := 'Fair';\n"
                   "WHEN 'F' THEN appraisal := 'Poor';\n"
                   "ELSE appraisal := 'No such grade';\n"
                   "END;\n"
                   "IF appraisal IS NULL THEN\n"
                   "NULL;\n"
                   "END IF;\n"
                   "END;\n"
                   "\n"
                   "BEGIN\n"
                   "IF 1 == 1 THEN\n"
                   "NULL;\n"
                   "END IF;\n"
                   "IF appraisal IS NULL THEN\n"
                   "NULL;\n"
                   "ELSE\n"
                   "SELECT CASE WHEN dummy='X' THEN 'A' ELSE 'B' END,  2 FROM dual;\n"
                   "END IF;\n"
                   "END;\n"
                   "\n"
                   "select count(case when dummy = 'Y' then dummy\n"
                   "             else null end) as tot_str\n"
                   "from dual;\n"
                   "\n"
                   "SET TRANSACTION READ ONLY\n"
                   "\n"
                   "PROMPT Hello\n"
                   "\n"
                   "/* Test comment\n"
                   "*/\n"
                   "INSERT INTO cdrProcess(ProcessID,\n"
                   "         StartDate,\n"
                   "         EnvDate,\n"
                   "         ProgramID,\n"
                   "         OSUser,\n"
                   "         SystemUser,\n"
                   "         ExecName,\n"
                   "         ExecVersion,\n"
                   "         ExecParameters,\n"
                   "         HostName)\n"
                   "VALUES (:1,\n"
                   " SYSDATE,\n"
                   " SYSDATE,\n"
                   " :2,\n"
                   " :3,\n"
                   " :4,\n"
                   " :5,\n"
                   " :6,\n"
                   " :7,\n"
                   " :8);\n"
                   "\n"
                   "CREATE or REPLACE Procedure TEST_SPR\n"
                   "(\n"
                   "    IN_TICKET_NUM   IN  VARCHAR2\n"
                   ")\n"
                   "IS\n"
                   "\n"
                   "BEGIN\n"
                   "\n"
                   "BEGIN\n"
                   "\n"
                   "for cur_rec in (select emp_id from employees) loop\n"
                   "\n"
                   " update employees set emp_id = emp_id + 1\n"
                   " where emp_id = cur_rec.emp_id;\n"
                   " commit;\n"
                   "end loop;\n"
                   " \n"
                   "END;\n"
                   "END TEST_SPR;\n"
                   "\n"
                   "SELECT owner,\n"
                   "       OBJECT,\n"
                   "       TYPE FROM v$access\n"
                   " WHERE sid=:f1<char[101]>\n"
                   " ORDER BY owner,\n"
                   "   OBJECT,\n"
                   "   TYPE;\n"
                   "\n"
                   "CREATE TABLE ess.EssCalLog (\n"
                   "        CalID  CHAR(5) NOT NULL,  -- Calender type\n"
                   " SeqID  NUMBER(8) NOT NULL,\n"
                   " ActStt  CHAR(1) NOT NULL\n"
                   "  CONSTRAINT EssCalLog_CK_ActStt CHECK (ActStt IN ('A','D') ),\n"
                   " LogRun  CHAR(1) NOT NULL  -- Should runs of this type be logged\n"
                   "  CONSTRAINT EssCalLog_CK_LogRun CHECK (LogRun IN ('Y','N') ),\n"
                   " PrcID  NUMBER(8) NOT NULL\n"
                   "  CONSTRAINT EssCalDay_FK_PrcID REFERENCES ess.EssPrc(PrcID),\n"
                   " Dsc  VARCHAR2(4000) NOT NULL, -- Description of this type\n"
                   " CONSTRAINT EssCal_PK PRIMARY KEY (CalID,SeqID)\n"
                   "  USING INDEX TABLESPACE Index02 -- A Comment\n"
                   ");\n"
                   "-- Another comment\n"
                   "\n"
                   "CREATE OR REPLACE procedure spTuxGetAccData (oRet                        OUT  NUMBER,\n"
                   "          oNumSwt                     OUT  NUMBER)\n"
                   "IS\n"
                   "  vYear  CHAR(4);\n"
                   "BEGIN\n"
                   "    <<label>>\n"
                   "    DECLARE\n"
                   "      oTrdStt NUMBER;\n"
                   "    BEGIN\n"
                   "      oTrdStt := 0;\n"
                   "    END;\n"
                   "\n"
                   "    EXCEPTION\n"
                   "        WHEN VALUE_ERROR THEN\n"
                   "     oRet := 3;\n"
                   " WHEN NO_DATA_FOUND THEN\n"
                   "     oRet := 2;\n"
                   " WHEN OTHERS THEN\n"
                   "     oRet := 1;\n"
                   "END;\n"
                   "CREATE OR REPLACE procedure spTuxGetAccData as\n"
                   "  vYear  CHAR(4);\n"
                   "begin\n"
                   "  null;\n"
                   "end;\n"
                   "-------------------------------------------------------------------\n"
                   "--    EssCal, Current calendar view\n"
                   "\n"
                   "CREATE VIEW ess.EssCal AS\n"
                   "        SELECT CalID,\n"
                   "        LogRun,\n"
                   "        PrcID,\n"
                   "        Dsc\n"
                   "   FROM ess.EssCalLog a\n"
                   "  WHERE SeqID = (SELECT MAX(aa.SeqID) FROM EssCalLog aa WHERE aa.CalID = a.CalID)\n"
                   "    AND ActStt = 'A';\n"
                   "\n"
                   "    /* A little comment\n"
                   "     */\n"
                   "    SELECT /*+\n"
                   "FULL(a)\n"
                   "*/ a.TskCod TskCod -- Test comment\n"
                   "      ,a.CreEdt CreEdt,\n"
                   "       a.TspActOprID /* One comment OprID */ , -- Another comment\n"
                   "       COUNT(1) Tot,\n"
                   "       COUNT(a.TspActOprID) Lft,\n"
                   "       b.TraCod TraCod,\n"
                   "       SUM(b.FinAmt) FinAmt,\n"
                   "       TraCod\n"
                   "  FROM EssTsk a,EssTra b\n"
                   " WHERE ((a.TspActOprID = 'Test') OR a.TspActOprID IS NULL)\n"
                   "   AND DECODE(a.TspActOprID,NULL,NULL,a.TskID) = b.TskID(+)\n"
                   " GROUP BY a.TskCod,a.CreEdt,a.TspActOprID,b.TraCod\n"
                   "HAVING COUNT(a.TspActOprID) > 0;\n"
                   "SELECT a.Sid \"-Id\",\n"
                   "       a.Serial# \"-Serial#\",\n"
                   "       a.SchemaName \"Schema\",\n"
                   "       a.Status \"Status\",\n"
                   "       a.Server \"Server\",\n"
                   "       a.OsUser \"Osuser\",\n"
                   "       a.Machine \"Machine\",\n"
                   "       a.Program \"Program\",\n"
                   "       a.Type \"Type\",\n"
                   "       a.Module \"Module\",\n"
                   "       a.Action \"Action\",\n"
                   "       a.Client_Info \"Client Info\",\n"
                   "       b.Block_Gets \"-Block Gets\",\n"
                   "       b.Consistent_Gets \"-Consistent Gets\",\n"
                   "       b.Physical_Reads \"-Physical Reads\",\n"
                   "       b.Block_Changes \"-Block Changes\",\n"
                   "       b.Consistent_Changes \"-Consistent Changes\",\n"
                   "       c.Value*10 \"-CPU (ms)\",\n"
                   "       a.Process \"-Process\",\n"
                   "       a.SQL_Address||':'||SQL_Hash_Value \" SQL Address\",\n"
                   "       a.Prev_SQL_Addr||':'||Prev_Hash_Value \" Prev SQl Address\"\n"
                   "  FROM v$session a,\n"
                   "       v$sess_io b,\n"
                   "       v$sesstat c\n"
                   " WHERE a.sid = b.sid(+)\n"
                   "   AND a.sid = c.sid(+) AND (c.statistic# = 12 OR c.statistic# IS NULL)\n"
                   " ORDER BY a.Sid;\n"
                   "select a.TskCod TskCod,\n"
                   "       count(1) Tot\n"
                   "  from (select * from EssTsk where PrsID >= '1940') ,EssTra b\n"
                   " where decode(a.TspActOprID,NULL,NULL,a.PrsID)+5 = b.PrsID(+)\n"
                   " group by a.TskCod,a.CreEdt,a.TspActOprID,b.TraCod\n"
                   "having count(a.TspActOprID) > 0;\n"
                   "\n"
                   "CREATE OR REPLACE procedure spTuxGetAccData (oRet OUT  NUMBER)\n"
                   "AS\n"
                   "  vYear  CHAR(4);\n"
                   "BEGIN\n"
                   "    DECLARE\n"
                   "      oTrdStt NUMBER;\n"
                   "    BEGIN\n"
                   "      oTrdStt := 0;\n"
                   "    END;\n"
                   "    EXCEPTION\n"
                   "        WHEN VALUE_ERROR THEN\n"
                   "     oRet := 3;\n"
                   "END;");
    testCount.insert(testCount.end(), 21);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml); // comment
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock); // package spec
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock); // anonymous block
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock); // anonymous block (without declare)
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml); // create table ess.EssCalLog...
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #24 Insert statements ending with "/" rather than ";"
    testSet.insert(testSet.end(),
                   "insert into a values ('aaa', 222)\n"
                   "/\n"
                   "insert into a values ('bbb', 333)\n"
                   "/\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #25 Two procedures separated with "/"
    testSet.insert(testSet.end(),
                   "create procedure a1 as\n"
                   "begin null; end;\n"
                   "/\n"
                   "create procedure a2 as\n"
                   "begin null; end;\n"
                   "/\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);
    testClass.insert(testClass.end(), toSQLParse::statement::plsqlblock);

    //===================================================
    // Test #26 Statement with slash in it
    testSet.insert(testSet.end(),
                   "select 2/1 from dual;");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #27 Update statements containing "/"'s ending with "/" rather than ";"
    testSet.insert(testSet.end(),
                   "update a set b=2/1 where c='qqq'\n"
                   "/\n"
                   "update a set b=3/2, c=2/1 where q='qq/qq'\n"
                   "/\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #28 Merge statement
    // Note: Semicolon after first merge is only required for reparse test
    //       Initial parse test should be ok without it. Reparse has problems
    //       with it because parser separates statements and removes any "/"
    //       signs which makes these two merge statements as one on second parse.
    testSet.insert(testSet.end(),
                   "MERGE INTO a\n"
                   "USING (\n"
                   " SELECT 1 id from dual) b on (a.id = b.id)\n"
                   "WHEN MATCHED THEN\n"
                   " UPDATE SET a.c = a.c +1\n"
                   " DELETE WHERE (a.c > 2)\n"
                   "WHEN NOT MATCHED THEN\n"
                   " INSERT (a.id, a.c)\n"
                   " VALUES (b.id, 0)\n"
                   "/"
                   "MERGE INTO a\n"
                   "USING (\n"
                   " SELECT 1 id from dual) b on (a.id = b.id)\n"
                   "WHEN MATCHED THEN\n"
                   " UPDATE SET a.c = a.c +1\n"
                   " DELETE WHERE (a.c > 2)\n"
                   "WHEN NOT MATCHED THEN\n"
                   " INSERT (a.id, a.c)\n"
                   " VALUES (b.id, 0)\n"
                   "/\n");
    testCount.insert(testCount.end(), 2);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    //===================================================
    // Test #29 Query statement with "with" structure
    testSet.insert(testSet.end(),
                   "WITH p AS (SELECT 1/2 as half_a FROM dual),\n"
                   "q AS(SELECT 1/2 half_b FROM dual)\n"
                   "SELECT half_a/half_b FROM p, q;\n"
                   "/\n");
    testCount.insert(testCount.end(), 1);
    testClass.insert(testClass.end(), toSQLParse::statement::ddldml);

    QApplication test(argc, argv);
    toMarkedText text(NULL);

    int n = 1;
    int e = 0; // error count
    QString errors;
    std::list<int>::iterator cnt = testCount.begin();
    std::list<int>::iterator cls = testClass.begin();

    for (std::list<QString>::iterator i = testSet.begin(); i != testSet.end(); i++)
    {
        printf("Start test %i ", n);
        text.setText(*i);

        //printf("Initialising editorTokenizer\n");
        toSQLParse::editorTokenizer tokens(&text);

        //printf("Parsing statement\n");
        std::list<toSQLParse::statement> stat = toSQLParse::parse(tokens);

        //printf("===== List of parsed statements =====\n");
        int statementCount = 0;
        for (std::list<toSQLParse::statement>::iterator i = stat.begin(); i != stat.end(); i++)
        {
            //printf("----- Parsed statement ----- (parse test main())\n");
            //printSt(*i, 0);
            //printf("----- End of parsed statement -----\n");

            // check if statement class was identified correctly
            if (i->StatementClass != *cls)
            {
                e++;
                errors = errors + "Incorrect statement class on test #" + QString::number(n) +
                         ". Expected " + decodeClass(*cls) + " got " + decodeClass(i->StatementClass) + "\n";
                printf("[Error]");
            }
            else
            {
                printf("[OK]");
            }
            statementCount++;
            cls++;
        }
        //printf("===== End of list of parsed statements =====\n");
        //printf("Number of statements: %i, should have been %i\n", statementCount, *cnt);

        // check if correct number of statements was identified
        if (statementCount != *cnt)
        {
            e++;
            errors = errors + "Incorrect number of parsed statements on test #" + QString::number(n) +
                     ". Expected " + QString::number(*cnt) + " got " + QString::number(statementCount) + "\n";
            printf("[Error]");
        }
        else
        {
            printf("[OK]");
        }

        //QString firstparse = toSQLParse::indent(stat);
        //qDebug() << "parsed=" << QString("\n") + firstparse;

        QString firstparse = toSQLParse::indent(stat);
        // Note that secondparse will go directly to stringTokenizer::getToken
        // bypassing editorTokenizer
        QString secondparse = toSQLParse::indent(firstparse);

        //printf("First\n\n%s\n", (const char *)firstparse.toUtf8());

        if (firstparse != secondparse)
        {
            //printf("Reparse doesn't match\n");
            //printf("Second\n\n%s\n", (const char *)secondparse.toUtf8());
            e++;
            errors = errors + "Reparse doesn't match for test #" + QString::number(n) + "\n";
            printf("[ERROR]\n");
        }
        else
        {
            printf("[OK]\n");
        }

        n++;
        cnt++;
    }
    if (e == 0)
    {
        printf("Done. No errors!!!\n");
    }
    else
    {
        printf("Done. Number of errors: %i\n", e);
        qDebug() << errors;
    }

    return 0;
}