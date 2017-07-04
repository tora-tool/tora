
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

#include "tools/toresultfield.h"

#include "core/toeventquery.h"
#include "core/utils.h"
#include "editor/toscintilla.h"

#ifdef TORA_EXPERIMENTAL
#include "parsing/tsqllexer.h"
#include "parsing/tsqlparse.h"
#endif

toResultField::toResultField(QWidget *parent, const char *name)
    : toHighlightedEditor(parent, name)
{
    sciEditor()->setReadOnly(true);
    FlagSet.Paste = FlagSet.Cut = false;
    Query = NULL;
    whichResultField = 1;
}

toResultField::~toResultField()
{
    delete Query;
}

void toResultField::query(const QString &sql, toQueryParams const& param)
{
    if (!setSqlAndParams(sql, param))
        return ;

    setFilename(QString::null);

    try
    {
        clear();
        Unapplied = QString::null;
        if (Query)
        {
            delete Query;
            Query = NULL;
        }

        Query = new toEventQuery(this
                                 , connection()
                                 , sql
                                 , param
                                 , toEventQuery::READ_ALL);
        connect(Query, SIGNAL(dataAvailable(toEventQuery*)), this, SLOT(slotPoll()));
        connect(Query, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(slotQueryDone()));
        Query->start();
    }
    TOCATCH
}

void toResultField::clearData()
{
    this->clear();
}

#define THRESHOLD 10240

void toResultField::slotPoll(void)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return ;
        if (Query)
        {
            int fieldNo = 1;
            while (Query->hasMore())
            {
                // For some MySQL statements (say "show create function aaa.bbb") more than one column is returned
                // and it is not possible to control that (or I do not know how to do it). This workaround will get
                // a required field (say 3rd) from a result set returned.
                QString val = (QString)Query->readValue();

                if (fieldNo == whichResultField) // by default this would be set to 1 in constructor
                    Unapplied += val;
                fieldNo++;
                if (fieldNo > Query->columnCount())
                    fieldNo = 1;
            }
            if (Unapplied.length() > THRESHOLD)
            {
                sciEditor()->append(Unapplied);
                Unapplied = QString::null;
            }
        }
    }
    catch (const QString &exc)
    {
        Query->deleteLater();
        Query = NULL;
        Utils::toStatusMessage(exc);
    }
}

void toResultField::slotQueryDone(void)
{
    delete Query;
    Query = NULL;

    try
    {
        // Code is formatted if it is set in preferences (Preferences->Editor Extensions) to
        // indent (format) a read only code AND! if it is not a MySQL code because current
        // TOra code parser/indenter does not work correctly with MySQL code (routines)
        // TODO: toSQLParse disabled
        //        if (toConfigurationSingle::Instance().autoIndentRo() && !connection().providerIs("QMYSQL") && !connection().providerIs("Teradata"))
        //            setText(toSQLParse::indent(text() + Unapplied));
        //        else
        sciEditor()->append(Unapplied);
    }
    TOCATCH
    Unapplied = QString::null;

    ////////////////////////////////////////////////////////////////////////////////////////
    int line, col;
    QString buffer;
    QString firstWord, currentWord;
    QString txt = sciEditor()->text();
    sciEditor()->getCursorPosition(&line, &col);

#ifdef TORA_EXPERIMENTAL
    try
    {
        static QSet<QString> PLSQL_INTRODUCERS
        {
            "ANALYZE",            "BEGIN",            "CALL",
            "COMMIT",            "CREATE",            "DECLARE",
            "DROP",            "EXPLAIN",            "FUNCTION",
            "GRANT",            "LOCK",            "PACKAGE",
            "PACKAGE",            "PROCEDURE",            "ROLLBACK",
            "SAVEPOINT",            "SET",            "TRUNCATE"
        };
        static QSet<QString> DML_INTRODUCERS
        {
            "WITH", "SELECT", "INSERT", "UPDATE", "DELETE", "MERGE"
        };

        do
        {
            std::unique_ptr<SQLParser::Statement> stat;
            std::unique_ptr<SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance ().create ("OracleGuiLexer", txt, "");
            if (firstWord.isEmpty ())
                break;
            firstWord = lexer->firstWord ();
            currentWord = lexer->wordAt(SQLLexer::Position(line, col));

            if (PLSQL_INTRODUCERS.contains(firstWord.toUpper()))
            {
                std::cout << "PLSQL:" << std::endl;
                if (firstWord == "PACKAGE" || firstWord == "PROCEDURE" || firstWord == "FUNCTION")
                    txt = QString ("CREATE OR REPLACE ") + txt;
                stat = StatementFactTwoParmSing::Instance ().create ("OraclePLSQL", txt, "");
                std::cout << stat->root ()->toStringRecursive ().toStdString () << std::endl;
            }
            else if (DML_INTRODUCERS.contains(firstWord.toUpper()))
            {
                std::cout << "SQL:" << std::endl;
                stat = StatementFactTwoParmSing::Instance ().create ("OracleDML", txt, "");
                std::cout << stat->root ()->toStringRecursive ().toStdString () << std::endl;
            }
            else
            {
                std::cout << "Unknown:" << firstWord.QString::toStdString () << std::endl;
                throw SQLParser::ParseException ();
            }

            //QString declName;
            foreach( QString declName, stat->allDeclarations()){
            QList<const SQLParser::Token*> decl = stat->declarations(declName);
            foreach(SQLParser::Token const*t, decl)
            {

                buffer += QString("%1: %2%3\n")
                .arg(declName)
                .arg(t->getTokenTypeString())
                .arg(t->getPosition().toString());
            }
        }
    }while(false);
}
catch ( SQLParser::ParseException const &e)
{
    buffer = "Parser error\n";
}
    if (!buffer.isEmpty ())
        TOMessageBox::information (this, currentWord, buffer);
#endif
} // queryDone
