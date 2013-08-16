
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

#include "core/toresultfield.h"
#include "core/toeventquery.h"
#include "core/toconfiguration.h"
#include "core/utils.h"

toResultField::toResultField(QWidget *parent, const char *name)
    : toHighlightedEditor(parent, name)
{
    setReadOnly(true);
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
        connect(Query, SIGNAL(done(toEventQuery*)), this, SLOT(slotQueryDone()));
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
            while (Query->hasMore())
            {
                // For some MySQL statements (say "show create function aaa.bbb") more than one column is returned
                // and it is not possible to control that (or I do not know how to do it). This workaround will get
                // a required field (say 3rd) from a result set returned.
                int fieldNo = whichResultField; // by default this would be set to 1 in constructor
                while (fieldNo > 1)
                {
                    fieldNo--;
                    Query->readValue();
                }
                Unapplied += Query->readValue();

                // Read any remaining columns for queries with specific field to fetch.
                // This is primarily used for MySQL statements like "show create..." which
                // return different uncontrollable number of fields for different users.
                // If remaining fields are not fetched polling thread will loop.
                if (whichResultField > 1)
                    while (!Query->eof())
                        Query->readValue();
            }
            if (Unapplied.length() > THRESHOLD)
            {
                append(Unapplied);
                Unapplied = QString::null;
            }
        }
    }
    catch (const QString &exc)
    {
        delete Query;
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
    	append(Unapplied);
    }
    TOCATCH
    Unapplied = QString::null;

    ////////////////////////////////////////////////////////////////////////////////////////
    int line, col;
    QString buffer;
    QString firstWord, currentWord;
    QString txt = text();
    getCursorPosition(&line, &col);

    //try {
    //  do {
    //      std::auto_ptr <SQLParser::Statement> stat;
    //      std::auto_ptr <SQLParser::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleSQL", txt, "");
    //      if(firstWord.isEmpty())
    //          break;
    //      firstWord = lexer->firstWord();
    //      currentWord = lexer->currentWord(line, col);

    //      if( QString::compare("CALL", firstWord, Qt::CaseInsensitive)      == 0 ||
    //          QString::compare("ANALYZE", firstWord, Qt::CaseInsensitive)   == 0 ||
    //          QString::compare("DECLARE", firstWord, Qt::CaseInsensitive)   == 0 ||
    //          QString::compare("BEGIN", firstWord, Qt::CaseInsensitive)     == 0 ||
    //          QString::compare("CREATE", firstWord, Qt::CaseInsensitive)    == 0 ||
    //          QString::compare("PROCEDURE", firstWord, Qt::CaseInsensitive)    == 0 ||
    //          QString::compare("FUNCTION", firstWord, Qt::CaseInsensitive)    == 0 ||
    //          QString::compare("PACKAGE", firstWord, Qt::CaseInsensitive)     == 0 ||
    //          QString::compare("LOCK", firstWord, Qt::CaseInsensitive)      == 0 ||
    //          QString::compare("EXPLAIN", firstWord, Qt::CaseInsensitive)   == 0 ||
    //          QString::compare("DROP", firstWord, Qt::CaseInsensitive)      == 0 ||
    //          QString::compare("COMMIT", firstWord, Qt::CaseInsensitive)    == 0 ||
    //          QString::compare("ROLLBACK", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("GRANT", firstWord, Qt::CaseInsensitive)     == 0 ||
    //          QString::compare("TRUNCATE", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("SAVEPOINT", firstWord, Qt::CaseInsensitive) == 0 ||
    //          QString::compare("PACKAGE", firstWord, Qt::CaseInsensitive) == 0 ||
    //          QString::compare("SET", firstWord, Qt::CaseInsensitive) == 0
    //          )
    //      {
    //          std::cout << "PLSQL:" << std::endl;
    //          if(firstWord == "PACKAGE" || firstWord == "PROCEDURE" || firstWord == "FUNCTION")
    //              txt = QString("CREATE OR REPLACE ") + txt;
    //          stat = StatementFactTwoParmSing::Instance().create("OraclePLSQL", txt, "");
    //          std::cout << stat->root()->toStringRecursive().toStdString() << std::endl;
    //      } else if( QString::compare("WITH", firstWord, Qt::CaseInsensitive)    == 0 ||
    //          QString::compare("SELECT", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("INSERT", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("UPDATE", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("DELETE", firstWord, Qt::CaseInsensitive)  == 0 ||
    //          QString::compare("MERGE", firstWord, Qt::CaseInsensitive)   == 0
    //          ) {
    //              std::cout << "SQL:" << std::endl;
    //              stat = StatementFactTwoParmSing::Instance().create("OracleSQL", txt, "");
    //              std::cout << stat->root()->toStringRecursive().toStdString() << std::endl;
    //      } else {
    //          std::cout << "Unknown:" << firstWord.QString::toStdString() << std::endl;
    //          throw SQLParser::ParseException();
    //      }

    //      //QString declName;
    //      foreach( QString declName, stat->allDeclarations())
    //      {
    //          QList<const SQLParser::Token*> decl = stat->declarations(declName);
    //          foreach(SQLParser::Token const*t, decl)
    //          {
    //
    //              buffer += QString("%1: %2%3\n")
    //                  .arg(declName)
    //                  .arg(t->getTokenTypeString())
    //                  .arg(t->getPosition().toString());
    //          }
    //      }
    //  } while(false);
    //} catch ( SQLParser::ParseException const &e)
    //{
    //  buffer = "Parser error\n";
    //}
    //if(!buffer.isEmpty())
    //  TOMessageBox::information (this, currentWord, buffer );
} // queryDone
