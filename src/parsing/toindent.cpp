
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
        // When tokens have depth like:
        // 1 1 2 3 4 1 1 5
        //           ^--- this one is chosen
        Token const *minDepthToken(NULL), *lastDepthToken(NULL);

        QList<Token const*>::ConstIterator it = constBegin();
        for(; it != constEnd(); ++it )
        {
            if ((*it)->metadata().contains("LINEBREAK"))
            {
                lastDepthToken = *it;
                minDepthToken = *it;
                break;
            }

            if (!(*it)->metadata().contains("SUBTREE_START"))
            {
                lastDepthToken = *it;
                continue;
            }

            if (minDepthToken == NULL)
                lastDepthToken = minDepthToken = *it;
            else if ((*it)->metadata().value("INDENT_DEPTH").toInt() <= minDepthToken->metadata().value("INDENT_DEPTH").toInt()
                    && (*it)->metadata().value("INDENT_DEPTH").toInt() < lastDepthToken->metadata().value("INDENT_DEPTH").toInt())
                minDepthToken = *it;
            lastDepthToken = *it;
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

        Token const *prevToken = NULL;

        if (it != constEnd()) // process the 1st word in the list
        {
            Token const *t = *it;
            if (t->metadata().contains("LINEBREAK"))
            {
                retval.append("\n");
                int depth = t->metadata().value("INDENT_DEPTH").toInt();
                int indent = toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
                retval.append(QString(depth * indent, ' '));
            }
            retval.append(t->toString());
            prevToken = *it;
            it++;
        }

        for (; it != constEnd(); ++it) // process the rest
        {
            Token const *token = *it;
            // Case when token was labeled as NEWLINE
            if (token->metadata().contains("LINEBREAK"))
            {
                retval.append("\n");
                int depth = token->metadata().value("INDENT_DEPTH").toInt();
                int indent = toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
                retval.append(QString(depth * indent, ' '));
//            } else if(token->metadata().contains("SUBTREE_START")
//                    && token->metadata().value("SUBTREE_LENGTH").toInt() > 100) {
//                retval.append("\n");
//                int depth = token->metadata().value("INDENT_DEPTH").toInt();
//                int indent = toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
//                retval.append(QString(depth * indent, ' '));
            // regular token
            } else {
				QString w = token->toString();
                if (prevToken->metadata().contains("KEYWORD"))
                    goto APPEND_SPACE;
                if (token->metadata().contains("KEYWORD"))
                    goto APPEND_SPACE;
                if (prevToken->metadata().contains("GLUE") && token->metadata().contains("GLUE"))
                    goto APPEND_WORD;
                if (prevToken->metadata().contains("NO_SPACE_AFTER"))
                    goto APPEND_WORD;
                if (token->metadata().contains("NO_SPACE_BEFORE"))
                    goto APPEND_WORD;

APPEND_SPACE:
                    retval.append(QString(' ')); // prepend space
            }

APPEND_WORD:
            retval.append(token->toString());
            prevToken = token;
        }

        retval.append('\n');
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

        QSet<QString> KEYWORDS = QSet<QString>() // keywords - should have spaces around them
                                << "WITH"
                                << "AS"
                                << "SELECT"
                                << "FROM"
                                << "JOIN"
                                << "CROSS"
                                << "NATURAL"
                                << "INNER"
                                << "OUTER"
                                << "LEFT"
                                << "RIGHT"
                                << "PARTITION"
                                << "ON"
                                << "USING"
                                << "WHERE"
                                << "AND"
                                << "OR"
                                << "BETWEEN"
                                << "IN"
                                << "GROUP"
                                << "BY"
                                << "HAVING"
                                << "MODEL"
                                << "PIVOT"
                                << "XML"
                                << "FOR";

        TLOG(8, toNoDecorator, __HERE__) << "IDEPTH" << "\t" << "LINE" << "\t" << "TYPE" << "\t" << "SLEN"<< "\t" << "TOKEN" << std::endl;
        foreach(Token const *t, list)
        {
            int depth = t->metadata().value("INDENT_DEPTH").toInt();
            int line = t->getPosition().getLine();
            SQLParser::Token::TokenType tt = t->getTokenType();
            QString word = t->toString();

            // remove trailing new line from single line comment
            if (tt == SQLParser::Token::TokenType::X_COMMENT) // comment or white char
            {
                if (tt == SQLParser::Token::TokenType::X_COMMENT)
                {
                    word.remove(QRegExp("[\\n\\r]*$"));
                }
            }

            int slen = 0;
            if (t->metadata().contains("SUBTREE_LENGTH_TOKENS"))
                slen = t->metadata().value("SUBTREE_LENGTH_TOKENS").toInt();
            QString marker;
            if (t->metadata().contains("LEFT_SPACER"))
                marker = "<";
            if (t->metadata().contains("RIGHT_SPACER"))
                marker = ">";
            if (t->metadata().contains("SUBTREE_START"))
                marker.append('*');

            TLOG(8, toNoDecorator, __HERE__) << depth << "\t" << line << "\t" << tt << "\t" << slen << "\t" << marker << word << std::endl;

            if (word == ".")
            {
                t->metadata().insert("NO_SPACE_BEFORE", 1);
                t->metadata().insert("NO_SPACE_AFTER", 1);
            }

            if (word == ",")
            {
                t->metadata().insert("NO_SPACE_BEFORE", 1);
            }

            if (word == "(")
            {
                t->metadata().insert("NO_SPACE_BEFORE", 1);
                t->metadata().insert("GLUE", 1);
            }

            if (word == "+")
            {
                t->metadata().insert("GLUE", 1);
            }

            if (word == ")")
            {
                t->metadata().insert("GLUE", 1);
            }

            if (KEYWORDS.contains(word.toUpper()))
            {
                t->metadata().insert("KEYWORD", 1);
            }

            if(line > lastLine)
            {
                if (toConfigurationNewSingle::Instance().option(Editor::ReUseNewlinesBool).toBool())
                {
                    t->metadata().insert("LINEBREAK", true);
                }
            }

            lineBuf.append(t);

            if (lineBuf.lineLenght() > toConfigurationNewSingle::Instance().option(Editor::IndentWidthtInt).toInt())
            {
                LineBuffer oldLine = lineBuf.split();
                retval2.append(oldLine.toString());
            }

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

            if (ilist == slist)
            {
                TLOG(8, toNoDecorator, __HERE__) << "OK" << std::endl;
                return retval2;
            } else {
                TLOG(8, toNoDecorator, __HERE__) << "NOT OK" << std::endl;
                throw QString::fromLocal8Bit("Indent failed");
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

    Token const*t = root; // this sub-tree root
    unsigned indentDepth = 0;
    while(t->parent())    // iterate to real root, compute indent depth, ignore nodes having no text
    {
        if (!white.exactMatch(t->toString()))
            indentDepth++; // increase indentDepth every time parent token in non-empty
        t = t->parent();
    }
    indentDepth--; // indentDepth for root select token should be 0;

    QList<SQLParser::Token const*> me, pre, post;
    int preStrLen(0), meStrLen(0), postStrLen(0);
    foreach(Token const *t, root->prevTokens()) // set INDENT_DEPTH for all
    {
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", indentDepth);
        t->metadata().insert("LEFT_SPACER", true);
        me.append(t);
        preStrLen += t->toString().length() + 1;
    }

    if (!white.exactMatch(root->toString()))
    {
        root->metadata().insert("INDENT_DEPTH", indentDepth);
        me.append(root);
        meStrLen += root->toString().length() + 1;
    }

    foreach(Token const* t, root->postTokens())
    {
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", indentDepth);
        t->metadata().insert("RIGHT_SPACER", true);
        me.append(t);
        postStrLen += t->toString().length() + 1;
    }

    // iterate over all children get their leaves lists
    int leftSonsLegth(0), rightSonsLenght(0); // total length of leaves on each side (in chars)
    foreach(QPointer<Token> child, root->getChildren())
    {
        Position child_position = child->getValidPosition();

        if(child_position < root->getPosition())
        {
            QList<SQLParser::Token const*> tempPre;
            indentPriv(child, tempPre);

            if (!tempPre.isEmpty())
                leftSonsLegth += tempPre.first()->metadata().value("SUBTREE_LENGTH").toInt();

            pre.append(tempPre);
        } else {
            QList<SQLParser::Token const*> tempPost;
            indentPriv(child, tempPost);

            if (!tempPost.isEmpty())
                rightSonsLenght += tempPost.first()->metadata().value("SUBTREE_LENGTH").toInt();

            post.append(tempPost);
        }
    }

    // this variable thisSubTree contains sorted is of all leaves in this sub-bree
    QList<SQLParser::Token const*> thisSubTree;
    thisSubTree.append(pre);
    thisSubTree.append(me);
    thisSubTree.append(post);

    if (!thisSubTree.isEmpty()) // might be empty in case of EOF token
    {
        // mark leftest and rightest leaves
        thisSubTree.first()->metadata().insert("SUBTREE_START", true); // mark 1st node
        thisSubTree.last()->metadata().insert("SUBTREE_END", true); // mark last node

        // the first (the leftest) token in subtree also contains information about this subree length (in chars)
        thisSubTree.first()->metadata().insert("SUBTREE_LENGTH_CHARS", leftSonsLegth + preStrLen + meStrLen + postStrLen + rightSonsLenght);
        thisSubTree.first()->metadata().insert("SUBTREE_LENGTH_TOKENS", thisSubTree.length());
    }

    list.append(thisSubTree);
}
