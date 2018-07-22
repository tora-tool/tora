
#include "parsing/toindent.h"
#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"

using namespace SQLParser;
using namespace ToConfiguration;

void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list);

class SpaceToken : public Token
{
    //Q_OBJECT;
public:
    SpaceToken(QObject *owner)
        : Token(NULL, Position(0,0), " ")
    {
        QObject::setParent(owner);
    };

    SpaceToken(QObject *owner, unsigned length)
        : Token(NULL, Position(0,0), QString(length, ' '))
    {
        QObject::setParent(owner);
    };

    const QString& toString() const override
    {
        return _mStr;
    }

    virtual ~SpaceToken(){};
};

class LineBuffer : public QList<Token const*>
{
public:
    LineBuffer() : linePos(0) {};

    void append(Token const* token)
    {
        linePos += token->toString().length();
        QList<Token const*>::append(token);
    }

    unsigned lineLenght()
    {
        return linePos;
    }

    LineBuffer split()
    {
        LineBuffer retval;

        // append first work into retval
        Token const* token = takeFirst();
        retval.append(token);
        linePos -= token->toString().length();

        // iterate over all buffer and find a token having lowest indent depth
        Token const* minDepthToken = NULL;

        QList<Token const*>::ConstIterator it = constBegin();
        for(; it != constEnd(); ++it )
        {
            if (SpaceToken const* t = dynamic_cast<SpaceToken const*>(*it))
                continue;
            if (minDepthToken == NULL)
                minDepthToken = *it;
            else if ((*it)->metadata().value("INDENT_DEPTH").toInt() <= minDepthToken->metadata().value("INDENT_DEPTH").toInt())
                minDepthToken = *it;
        }

        if (!minDepthToken)
            return retval;

        while(first() != minDepthToken)
        {
            Token const* token = takeFirst();
            retval.append(token);
            linePos -= token->toString().length();
        }
        return retval;
    }

    QString toString()
    {
        QString retval;
        QList<Token const*>::ConstIterator it = constBegin();
        for (; it != constEnd(); ++it)
        {
			Token const *t = *it;
            retval.append(t->toString());
        }
        return retval;
    }
public:
    unsigned linePos;
};

QString toIndent::indent(QString const&input)
{
    QObject owner; // the object responsible for disposal of all SpaceToken instances
    QString retvalX, retval2;
    try
    {
        std::unique_ptr <Statement> ast = StatementFactTwoParmSing::Instance().create("OracleDML", input, "");
        Token const* root = ast->root();

        TLOG(8, toNoDecorator, __HERE__) << root->toLispStringRecursive() << std::endl;

        QList<SQLParser::Token const*> list;
        // The variable list contains all non-white characters having property INDENT_DEPTH set to relative depth in AST tree
        indentPriv(root, list);

        int lastLine = 1;
        QString lastWord;
        bool firstWord = true;

        LineBuffer lineBuf;
        unsigned linePos = 0;

        TLOG(8, toNoDecorator, __HERE__) << "DEPTH" << "\t" << "IDEPTH" << "\t" << "LINE" << "\t" << "TYPE" << "\t" << "TOKEN" << std::endl;
        foreach(Token const *t, list)
        {
            int d1 = t->depth();
            int depth = t->metadata().value("INDENT_DEPTH").toInt();
            int line = t->getPosition().getLine();
            SQLParser::Token::TokenType tt = t->getTokenType();
            QString word = t->toString();

            // remove trailing new line from single line comment
            //            if (tt == SQLParser::Token::TokenType::X_COMMENT)
            //            {
            //                word.remove(QRegExp("[\\n\\r]"));
            //            }

            TLOG(8, toNoDecorator, __HERE__) << d1 << "\t" << depth << "\t" << line << "\t" << tt << "\t" << word << std::endl;

            if (!firstWord // prepend space before every word except the first one
                    && !(word == "." || lastWord == ".") // no space around dots
                    && !(word == ",")                    // no space before comma
                    )
            {
                //retval.append(" ");
                lineBuf.append(new SpaceToken(&owner));
            }

            if(line > lastLine)
            {
                if (toConfigurationNewSingle::Instance().option(Editor::ReUseNewlinesBool).toBool())
                {
                    //retval.append("\n");               // prepend newline if needed
                    //retval.append(QString(depth * toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt()
                    //                      , ' '));   // indent line by tokens depth
                    lineBuf.append(new SpaceToken(&owner, depth * toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt()));
                    linePos = depth * toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
                } else {
                    //retval.append(" ");
                    linePos++;
                    lineBuf.append(new SpaceToken(&owner));
                }
            }
            //retval.append(word);
            lineBuf.append(t);

            int w = toConfigurationNewSingle::Instance().option(Editor::IndentWidthtInt).toInt();
            if (lineBuf.lineLenght() > toConfigurationNewSingle::Instance().option(Editor::IndentWidthtInt).toInt())
            {
                LineBuffer oldLine = lineBuf.split();
                retval2.append(oldLine.toString());
                retval2.append("\n");
            }

            firstWord = false;
            lastWord = word;
            lastLine = line;
        }
        retval2.append(lineBuf.toString());

        // validation part
        {
            TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
            TLOG(8, toNoDecorator, __HERE__) << input << std::endl;
            TLOG(8, toNoDecorator, __HERE__) << "-------------------------------------------------------------------------------------------------------------" << std::endl;
            TLOG(8, toNoDecorator, __HERE__) << retval2 << std::endl;
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

            std::unique_ptr <SQLLexer::Lexer> lexerOut = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", retval2, "");
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

            //if (ilist == slist)
            {
                TLOG(8, toNoDecorator, __HERE__) << "OK" << std::endl;
                return retval2;
//            } else {
//                TLOG(8, toNoDecorator, __HERE__) << "NOT OK" << std::endl;
//                throw QString::fromLocal8Bit("Indent failed");
            }
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
    return retval2;
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
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", depth);
        me.append(t);
    }

    if (!white.exactMatch(root->toString()))
    {
        root->metadata().insert("INDENT_DEPTH", depth);
        me.append(root);
    }

    foreach(Token const* t, root->postTokens())
    {
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", depth);
        me.append(t);
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
