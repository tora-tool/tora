
#include "parsing/toindent.h"
#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"
#include "core/utils.h"
#include "core/tologger.h"

using namespace SQLParser;

void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list);

QString toIndent::indent(QString const&input)
{
    QString retval;
    try
    {
        std::unique_ptr <Statement> ast = StatementFactTwoParmSing::Instance().create("OracleDML", input, "");
        Token const* root = ast->root();

        TLOG(8, toNoDecorator, __HERE__) << root->toLispStringRecursive() << std::endl;

        QList<SQLParser::Token const*> list;
        indentPriv(root, list);

        int lastLine = 1;
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
                retval.append(' ');
            }

            if(line > lastLine)
            {
                retval.append("\n");               // prepend newline if needed
                retval.append(QString(d2, ' '));   // indent line by tokens depth
            }
            retval.append(s);

            firstWord = false;
            lastWord = s;
            lastLine = line;
        }

        TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
        TLOG(8, toNoDecorator, __HERE__) << input << std::endl;
        TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
        TLOG(8, toNoDecorator, __HERE__) << retval << std::endl;
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

        std::unique_ptr <SQLLexer::Lexer> lexerOut = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", retval, "");
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
            return retval;
        } else {
            TLOG(8, toNoDecorator, __HERE__) << "NOT OK" << std::endl;
            throw QString::fromLocal8Bit("Indent failed");
        }

    }
    catch (const ParseException &e)
    {
        std::cerr << "ParseException: "<< std::endl << std::endl;
        throw QString::fromLatin1("ParseException");
    }
    catch (const QString &str)
    {
        std::cerr << "Unhandled exception: "<< std::endl << std::endl << qPrintable(str) << std::endl;
        throw QString::fromLatin1("OtherException")  + str;
    }
    return retval;
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
