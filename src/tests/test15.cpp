
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

//#include "core/utils.h"
#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"
#include <QtCore/QDateTime>
#include <QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QLibrary>
#include <QtCore/QDebug>

#include <memory>
#include <iostream>
#include "core/utils.h"
#include "core/tologger.h"

void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list);

int main(int argc, char **argv)
{
    using namespace SQLParser;

    QString filename;
    if ( argc == 2)
    {
        filename = QString::fromLatin1(argv[1]);
    } else {
        return 0;
    }

    QApplication app(argc, argv);
    try
    {
        QString input = Utils::toReadFile(filename);
        std::unique_ptr <Statement> ast = StatementFactTwoParmSing::Instance().create("OracleDML", input, filename);
        Token const* root = ast->root();

        TLOG(8, toNoDecorator, __HERE__) << root->toLispStringRecursive() << std::endl;

        QList<SQLParser::Token const*> list;
        indentPriv(root, list);

        QString str;
        int lastLine = 0;
        QString lastWord;
        bool firstWord = true;
        foreach(Token const *t, list)
        {
            int d1 = t->depth();
            int d2 = t->metadata().value("INDENT_DEPTH").toInt();
            int line = t->getPosition().getLine();
            bool v = t->getValidPosition().isValid();
            SQLParser::Token::TokenType tt = t->getTokenType();
            QString s = t->toString();
            if (tt==SQLParser::Token::TokenType::X_COMMENT)
            {
                s.remove(QRegExp("[\\n\\r]"));
            }
            TLOG(8, toNoDecorator, __HERE__) << d1 << "\t" << d2 << "\t" << line << "\t" << tt << "\t" << s << std::endl;
            if (!firstWord)
            {
                str.append(' ');
            }

            if(line > lastLine)
            {
                str.append("\n");               // prepend newline if needed
                str.append(QString(d2, ' '));   // indent line by tokens depth
            }
            str.append(s);

            firstWord = false;
            lastWord = s;
            lastLine = line;
        }

        TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
		TLOG(8, toNoDecorator, __HERE__) << input << std::endl;
		TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
        TLOG(8, toNoDecorator, __HERE__) << str << std::endl;
        TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;

        QSet<SQLLexer::Token::TokenType> IGNORED = QSet<SQLLexer::Token::TokenType>()
                        << SQLLexer::Token::X_EOF
                        << SQLLexer::Token::X_EOL
                        << SQLLexer::Token::X_FAILURE
                        << SQLLexer::Token::X_ONE_LINE
                        << SQLLexer::Token::X_WHITE
                        << SQLLexer::Token::X_COMMENT
                        << SQLLexer::Token::X_COMMENT_ML
                        << SQLLexer::Token::X_COMMENT_ML_END;

        std::unique_ptr <SQLLexer::Lexer> lexerIn  = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", input, "");
        SQLLexer::Lexer::token_const_iterator i = lexerIn->begin();
        QList<SQLLexer::Token::TokenType> ilist;
        while ( i != lexerIn->end())
        {
            SQLLexer::Token::TokenType tt = i->getTokenType();
            if(!IGNORED.contains(tt))
            {
                std::cout << '\'' << qPrintable(i->getText()) << '\'' << "\t" << tt << std::endl;
                ilist.append(tt);
            }
            i++;
        }

        std::unique_ptr <SQLLexer::Lexer> lexerOut = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", str, "");
        SQLLexer::Lexer::token_const_iterator j = lexerOut->begin();
        QList<SQLLexer::Token::TokenType> slist;
        while ( j != lexerOut->end())
        {
            SQLLexer::Token::TokenType tt = j->getTokenType();
            if(!IGNORED.contains(tt))
            {
                std::cout << '\'' << qPrintable(j->getText()) << '\'' << "\t" << tt << std::endl;
                slist.append(tt);
            }
            j++;
        }

        if (ilist == slist)
        {
            TLOG(8, toNoDecorator, __HERE__) << "OK" << std::endl;
        } else {
            TLOG(8, toNoDecorator, __HERE__) << "NOT OK" << std::endl;
            return 1;
        }

    }
    catch (const ParseException &e)
    {
        std::cerr << "ParseException: "<< std::endl << std::endl;
    }
    catch (const QString &str)
    {
        std::cerr << "Unhandled exception: "<< std::endl << std::endl << qPrintable(str) << std::endl;
    }
    return 0;
}

void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list)
{
    using namespace SQLParser;
    QRegExp white("^[ \\n\\r\\t]*$");

    Token const*t = root;
    unsigned depth = 0;
    while(t->parent())
    {
        if (!white.exactMatch(t->toString()))
            depth++;
        t = t->parent();
    }

    QList<SQLParser::Token const*> me, pre, post;
    foreach(Token const *t, root->prevTokens())
    {
        if (!white.exactMatch(t->toString()))
        {
        t->metadata().insert("INDENT_DEPTH", depth);
        me.append(t);
        }
    }
    if (!white.exactMatch(root->toString()))
    {
        root->metadata().insert("INDENT_DEPTH", depth);
        me.append(root);
    }
    foreach(Token const* t, root->postTokens())
    {
        if (!white.exactMatch(t->toString()))
        {
            t->metadata().insert("INDENT_DEPTH", depth);
            me.append(t);
        }
    }

    foreach(QPointer<Token> child, root->getChildren())
    {
        Position child_position = child->getValidPosition();

        if(child_position < root->getPosition())
        {
            indentPriv(child, pre);
        } else {
            indentPriv(child, post);
        }
    }
    list.append(pre);
    list.append(me);
    list.append(post);
}
