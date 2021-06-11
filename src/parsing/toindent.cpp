
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

#include "parsing/toindent.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"
#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"

#include <QRegExp>
#include <QRegularExpression>

using namespace SQLParser;
using namespace ToConfiguration;

// internal utility class
class LineBuffer : public QList<Token const*>
{
public:
    LineBuffer() : linePos(0) {};

    void append(Token const* token); // append token into this buffer/queue list
    unsigned lineLenght(); // return the length on all tokens in this list (including estimated number of spaces)
    LineBuffer split(toIndent::Mode = toIndent::WidthMode);    // split this buffer, i.e. perform a line-break
    QString toString();
protected:
    QString formatToken(Token const *token);
    unsigned linePos;
};


toIndent::toIndent(int adj) : adjustment(adj)
{
    setup();
}

toIndent::toIndent(QMap<QString, QVariant> const &params, int adj) : adjustment(adj)
{
    setup();
    if (params.contains("IndentLineWidthInt")) IndentLineWidthInt = params.value("IndentLineWidthInt").toInt();
    if (params.contains("IndentWidthInt"))     IndentWidthInt     = params.value("IndentWidthInt").toInt();
    /*----------------------------*/
    if (params.contains("ReUseNewlinesBool")) ReUseNewlinesBool = params.value("ReUseNewlinesBool").toBool();
    if (params.contains("BreakOnSelectBool")) BreakOnSelectBool = params.value("BreakOnSelectBool").toBool();
    if (params.contains("BreakOnFromBool"))   BreakOnFromBool   = params.value("BreakOnFromBool").toBool();
    if (params.contains("BreakOnWhereBool"))  BreakOnWhereBool  = params.value("BreakOnWhereBool").toBool();
    if (params.contains("BreakOnGroupBool"))  BreakOnGroupBool  = params.value("BreakOnGroupBool").toBool();
    if (params.contains("BreakOnOrderBool"))  BreakOnOrderBool  = params.value("BreakOnOrderBool").toBool();
    if (params.contains("BreakOnModelBool"))  BreakOnModelBool  = params.value("BreakOnModelBool").toBool();
    if (params.contains("BreakOnPivotBool"))  BreakOnPivotBool  = params.value("BreakOnPivotBool").toBool();
    if (params.contains("BreakOnLimitBool"))  BreakOnLimitBool  = params.value("BreakOnLimitBool").toBool();
    if (params.contains("BreakOnJoinBool"))   BreakOnJoinBool   = params.value("BreakOnJoinBool").toBool();
    /*----------------------------*/
    if (params.contains("WidthModeBool"))     WidthModeBool     = params.value("WidthModeBool").toBool();
}

void toIndent::setup()
{
    IndentLineWidthInt  = toConfigurationNewSingle::Instance().option(Editor::IndentLineWidthInt).toInt();
    IndentWidthInt      = toConfigurationNewSingle::Instance().option(Editor::IndentWidthInt).toInt();
    /*----------------------------*/
    ReUseNewlinesBool   = toConfigurationNewSingle::Instance().option(Editor::ReUseNewlinesBool).toBool();
    BreakOnSelectBool   = toConfigurationNewSingle::Instance().option(Editor::BreakOnSelectBool).toBool();
    BreakOnFromBool     = toConfigurationNewSingle::Instance().option(Editor::BreakOnFromBool).toBool();
    BreakOnWhereBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnWhereBool).toBool();
    BreakOnGroupBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnGroupBool).toBool();
    BreakOnOrderBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnOrderBool).toBool();
    BreakOnModelBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnModelBool).toBool();
    BreakOnPivotBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnPivotBool).toBool();
    BreakOnLimitBool    = toConfigurationNewSingle::Instance().option(Editor::BreakOnLimitBool).toBool();
    BreakOnJoinBool     = toConfigurationNewSingle::Instance().option(Editor::BreakOnJoinBool).toBool();
    /*----------------------------*/
    WidthModeBool       = toConfigurationNewSingle::Instance().option(Editor::WidthModeBool).toBool();
}

QSet<QString> toIndent::KEYWORDS = QSet<QString>() // keywords - should have spaces around them
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

QSet<QString> toIndent::JOIN = QSet<QString>() // keywords that can start join
						<< "JOIN"
						<< "LEFT"
						<< "RIGHT"
						<< "CROSS"
						<< "INNER"
						<< "OUTER"
						<< "FULL"
						<< "NATURAL";

static void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list);

void toIndent::tagToken(Token const*token)
{
    static QRegExp TRAILING_NEWLINE("^.*[\\n\\r]+$");

    int depth = token->metadata().value("INDENT_DEPTH").toInt();
    int line = token->getPosition().getLine();
    SQLParser::Token::TokenType tt = token->getTokenType();
    QString word = token->toString();

    // mark token / trailing newline from single line comment in toString
    if (tt == SQLParser::Token::TokenType::X_COMMENT) // comment or white char
    {
        if (TRAILING_NEWLINE.exactMatch(word))
        {
            token->metadata().insert("TRAILING_NEWLINE", 1);
            //token->metadata().insert("LINEBREAK", 1);
        }
    }

    if (KEYWORDS.contains(word.toUpper()))
    {
        token->metadata().insert("KEYWORD", 1); // KEYWORDS should always be surrounded by spaces
    }

    if (word == ".")
    {
        token->metadata().insert("NO_SPACE_BEFORE", 1);
        token->metadata().insert("NO_SPACE_AFTER", 1);
    }

    if (word == ",")
    {
        token->metadata().insert("NO_SPACE_BEFORE", 1);
    }

    if (word == "(")
    {
        token->metadata().insert("NO_SPACE_BEFORE", 1);
        token->metadata().insert("GLUE", 1); // there should not be space between two tokens habinv GLUE prop. set
    }

    if (word == "+")
    {
        token->metadata().insert("GLUE", 1);
    }

    if (word == ")")
    {
        token->metadata().insert("GLUE", 1);
    }

    if (BreakOnSelectBool  && word.toUpper() == "SELECT") token->metadata().insert("LINEBREAK", 1);
    if (BreakOnFromBool    && word.toUpper() == "FROM")   token->metadata().insert("LINEBREAK", 1);
    if (BreakOnWhereBool   && word.toUpper() == "WHERE")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnGroupBool   && word.toUpper() == "GROUP")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnOrderBool   && word.toUpper() == "ORDER")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnModelBool   && word.toUpper() == "MODEL")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnPivotBool   && word.toUpper() == "PIVOT")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnLimitBool   && word.toUpper() == "LIMIT")  token->metadata().insert("LINEBREAK", 1);
    if (BreakOnJoinBool    && JOIN.contains(word.toUpper()) && token->metadata().contains("SUBTREE_START"))
    {
        token->metadata().insert("LINEBREAK", 1);
    }
}

QString toIndent::indent(QString const&input)
{
    QString retval;
    try
    {
        std::unique_ptr <Statement> ast = StatementFactTwoParmSing::Instance().create("OracleDML", input, "");
        Token const* root = ast->root();

        TLOG(8, toNoDecorator, __HERE__) << root->toLispStringRecursive() << std::endl;

        // The variable list contains all non-white characters having property INDENT_DEPTH set to relative depth in AST tree
        QList<SQLParser::Token const*> list;
        indentPriv(root, list);

        int lastLine = 1;
        QString lastWord;

        LineBuffer lineBuf;

        TLOG(8, toNoDecorator, __HERE__) << "IDEPTH" << "\t" << "LINE" << "\t" << "TYPE" << "\t" << "SLEN"<< "\t" << "MARK" << '\t' << "TOKEN" << std::endl;
        foreach(Token const *token, list)
        {
            int depth = token->metadata().value("INDENT_DEPTH").toInt();
            int line = token->getPosition().getLine();
            SQLParser::Token::TokenType tt = token->getTokenType();
            QString word = token->toString();

            tagToken(token); // tag token using TRAILING_NEWLINE, LINEBREAK, NO_SPACE_BEFORE, NO_SPACE_AFTER, ...

            { // some tracing output
                int slen = 0;
                if (token->metadata().contains("SUBTREE_LENGTH_TOKENS"))
                    slen = token->metadata().value("SUBTREE_LENGTH_TOKENS").toInt();

                QString marker;
                if (token->metadata().contains("SUBTREE_START"))
                    marker.append('*');
                if (token->metadata().contains("LEFT_SPACER"))
                    marker.append("<");
                if (token->metadata().contains("RIGHT_SPACER"))
                    marker.append(">");
                if (token->metadata().contains("TRAILING_NEWLINE"))
                    marker.append("_");
                TLOG(8, toNoDecorator, __HERE__) << depth << "\t" << line << "\t" << tt << "\t" << slen << "\t" << marker << '\t' << word << std::endl;
            }

            // This token is on a new line (and we reuse NEWLINES)
            if(line > lastLine &&  ReUseNewlinesBool)
            {
                token->metadata().insert("LINEBREAK", true); // mark this token as LINEBREAK
                lineBuf.append(token);

                LineBuffer oldLine = lineBuf.split(); // split the lineBuf

                int depth = oldLine.front()->metadata().value("INDENT_DEPTH").toInt();
                retval.append(QString(depth * this->IndentWidthInt + adjustment, ' ')); // prepend indentation spaces
                retval.append(oldLine.toString());                                // convert the old linebuf from string and append it to retval
                retval.append('\n');
            }
            // This token is on a new line - we do NOT reuse NEWLINES - but previous token was a single line comment
            else if (line > lastLine &&  !ReUseNewlinesBool && !lineBuf.isEmpty() && lineBuf.last()->metadata().contains("TRAILING_NEWLINE")) {
                token->metadata().insert("LINEBREAK", true); // mark this token as LINEBREAK
                lineBuf.append(token);

                LineBuffer oldLine = lineBuf.split(); // split the lineBuf

                int depth = oldLine.front()->metadata().value("INDENT_DEPTH").toInt();
                retval.append(QString(depth * this->IndentWidthInt + adjustment, ' ')); // prepend indentation spaces
                retval.append(oldLine.toString());                                // convert the old linebuf from string and append it to retval
                retval.append('\n');
            } else {
                lineBuf.append(token);
            }

            // The length of lineBuf exceeded the requested length
            if (lineBuf.lineLenght() > IndentLineWidthInt)
            {
                LineBuffer oldLine = lineBuf.split(); // split the lineBuf

                int depth = oldLine.front()->metadata().value("INDENT_DEPTH").toInt();
                retval.append(QString(depth * IndentWidthInt + adjustment, ' '));
                retval.append(oldLine.toString());
                retval.append('\n');
            }
            lastLine = line;
        }

        // append the last remaining part(last line)
        while (!lineBuf.isEmpty())
        {
            // The length of lineBuf exceeded the requested length
            if (lineBuf.lineLenght() > IndentLineWidthInt)
            {
                LineBuffer oldLine = lineBuf.split(); // split the lineBuf (in case there are ANY single line comments in lineBuf

                int depth = oldLine.front()->metadata().value("INDENT_DEPTH").toInt();
                retval.append(QString(depth * IndentWidthInt + adjustment, ' '));
                retval.append(oldLine.toString());
                retval.append('\n');
            } else {
                int depth = lineBuf.front()->metadata().value("INDENT_DEPTH").toInt();
                retval.append(QString(depth * IndentWidthInt + adjustment, ' '));
                retval.append(lineBuf.toString());
                retval.append('\n');
                break; // the last line was drained
            }
        }

        // validation part
        {
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
            // Use GUI LEXER (not parser) to convert the input string into list of tokens
            std::unique_ptr <SQLLexer::Lexer> lexerIn  = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", input, "");
            SQLLexer::Lexer::token_const_iterator i = lexerIn->begin();
            QList<SQLLexer::Token::TokenType> ilist;
            while ( i != lexerIn->end())
            {
                SQLLexer::Token::TokenType tt = i->getTokenType();
                if(!IGNORED.contains(tt)) // skip whitespaces
                    ilist.append(tt);
                i++;
            }

            // Use GUI LEXER (not parser) to convert the resulting string into list of tokens
            std::unique_ptr <SQLLexer::Lexer> lexerOut = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", retval, "");
            SQLLexer::Lexer::token_const_iterator j = lexerOut->begin();
            QList<SQLLexer::Token::TokenType> slist;
            while ( j != lexerOut->end())
            {
                SQLLexer::Token::TokenType tt = j->getTokenType();
                if(!IGNORED.contains(tt)) // skip whitespaces
                    slist.append(tt);
                j++;
            }

            // Compare the lists
            if (ilist == slist)
            {
                TLOG(8, toNoDecorator, __HERE__) << "OK" << std::endl;
                return retval;
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
        throw QString::fromLatin1("OtherException: ")  + str;
    }
    return retval;
}


void LineBuffer::append(Token const* token)
{
    linePos += token->toString().length();
    QList<Token const*>::append(token);
}

unsigned LineBuffer::lineLenght()
{
    return linePos;
}

LineBuffer LineBuffer::split(toIndent::Mode mode)
{
    LineBuffer retval;

    Token const* lastDepthToken;
    // append first word(s) into retval (all having the same depth)
    // consume all tokens having the same depth
    do
    {
        lastDepthToken = takeFirst();
		QString w = lastDepthToken->toString();
        retval.append(lastDepthToken);
        linePos -= lastDepthToken->toString().length();

        if (!empty() && first()->metadata().contains("LINEBREAK")) // break on linebreak
            break;
    }
    while (!empty() && first()->metadata().value("INDENT_DEPTH") == retval.last()->metadata().value("INDENT_DEPTH"));

    // width mode: (narrow mode not yet implemented)
    // iterate over all buffer and find a token having lowest indent depth
    // When tokens have depth like:
    // 1 3 2 1 1 2 3 4 1 1 5
    //                 ^--- this one is chosen
    // resulting into:
    // 1 3 2 1 1 2 3 4
    // 1 1 5
	//
	//
	// 4 4 4 5 6 7 8 8 6 7 8
	//       ^--- this one is chosen

    Token const *minDepthToken(NULL);
    QList<Token const*>::ConstIterator it = constBegin();
    for(; it != constEnd(); ++it )
    {
        if ((*it)->metadata().contains("LINEBREAK")) // break on linebreak
        {
            lastDepthToken = *it;
            minDepthToken = *it;
            break;
        }

        if (!(*it)->metadata().contains("SUBTREE_START")) // skip tokens whose do not start a subtree
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

    QString txt = retval.toString();
    return retval;
}

QString LineBuffer::toString()
{
    QString retval;
    QList<Token const*>::ConstIterator it = constBegin();

    Token const *prevToken = NULL;

    if (it != constEnd()) // process the 1st word in the list (do not prepend space)
    {
        retval.append(formatToken(*it));
        prevToken = *it;
        it++;
    }

    for (; it != constEnd(); ++it) // process the rest
    {
        Token const *token = *it;
        // abandoned piece of code, do not LINEBREAK short nested sub queries
        //
        // Case when token was labeled as NEWLINE
        //if (token->metadata().contains("LINEBREAK"))
        //{
        //    retval.append("\n");
        //    int depth = token->metadata().value("INDENT_DEPTH").toInt();
        //    int indent = toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
        //    retval.append(QString(depth * indent, ' '));
        //            } else if(token->metadata().contains("SUBTREE_START")
        //                    && token->metadata().value("SUBTREE_LENGTH").toInt() > 100) {
        //                retval.append("\n");
        //                int depth = token->metadata().value("INDENT_DEPTH").toInt();
        //                int indent = toConfigurationNewSingle::Instance().option(Editor::IndentDepthInt).toInt();
        //                retval.append(QString(depth * indent, ' '));
        // regular token
        //} else {
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

        APPEND_WORD:
        retval.append(formatToken(token));
        prevToken = token;
    }
    return retval;
}

QString LineBuffer::formatToken(Token const *token)
{
    QString word = token->toString();
    // remove trailing new line from single line comment
    if (token->metadata().contains("TRAILING_NEWLINE"))
        if (token->getTokenType() == SQLParser::Token::TokenType::X_COMMENT)
        {
            word.remove(QRegularExpression("[\\n\\r]*$"));
        }
    return word;
}

// static recursive function, turn AST tree into consecutive list of leaf/non-leaf tokens
// of of them will have stored some attributes in it (like INDENT_DEPTH for example
// aside from tokens present in AST tree, there are also some lexer tokes, with are excluded from AST tree
// those are add as "spacer" tokens
static void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list)
{
    using namespace SQLParser;
    QRegExp white("^[ \\n\\r\\t]*$");

    Token const *t = root; // this sub-tree's root
#if 0
    auto s = t->toString();
    //auto t = t->toStringRecursive();
    auto l = t->toLispStringRecursive();
    auto a = t->getTokenATypeName();
    auto b = t->getTokenTypeString();
#endif

    unsigned indentDepth = 0; // indentDepth counter
    while(t->parent())    // iterate to real root, compute indent depth, ignore nodes having no text
    {
        if (!white.exactMatch(t->toString()))
            indentDepth++; // increase indentDepth every time parent token in non-empty
        t = t->parent();
    }
    indentDepth--; // indentDepth for root select token should be 0;

    QList<SQLParser::Token const*> pre, me, post;
    int preStrLen(0), meStrLen(0), postStrLen(0); // total length of tokens in list pre, me, post

    // set INDENT_DEPTH for all pre spacer tokens (token on the left side from me)
    foreach(Token const *t, root->prevTokens())
    {
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", indentDepth);
        t->metadata().insert("LEFT_SPACER", true);
        me.append(t);
        preStrLen += t->toString().length() + 1;
    }

    // set INDENT_DEPTH to this subtree's root
    if (!white.exactMatch(root->toString()))
    {
        root->metadata().insert("INDENT_DEPTH", indentDepth);
        me.append(root);
        meStrLen += root->toString().length() + 1;
    }

    // set INDENT_DEPTH for all pre spacer tokens (token on the right side from me)
    foreach(Token const* t, root->postTokens())
    {
        if (white.exactMatch(t->toString()))
            continue;

        t->metadata().insert("INDENT_DEPTH", indentDepth);
        t->metadata().insert("RIGHT_SPACER", true);
        me.append(t);
        postStrLen += t->toString().length() + 1;
    }

    // recursively iterate over all CHILDREN get their leaves lists
    int leftSonsLegth(0), rightSonsLenght(0); // total length of leaves on each side (in chars)
    foreach(QPointer<Token> child, root->getChildren())
    {
        Position child_position = child->getValidPosition();

        if(child_position < root->getPosition())
        {
            QList<SQLParser::Token const*> tempPreChildSubTreeList;
            indentPriv(child, tempPreChildSubTreeList);
            if (!tempPreChildSubTreeList.isEmpty())
                leftSonsLegth += tempPreChildSubTreeList.first()->metadata().value("SUBTREE_LENGTH_CHARS").toInt();
            pre.append(tempPreChildSubTreeList);
        } else {
            QList<SQLParser::Token const*> tempPostChildSubTreeList;
            indentPriv(child, tempPostChildSubTreeList);
            if (!tempPostChildSubTreeList.isEmpty())
                rightSonsLenght += tempPostChildSubTreeList.first()->metadata().value("SUBTREE_LENGTH_CHARS").toInt();
            post.append(tempPostChildSubTreeList);
        }
    }

    // this variable thisSubTree contains sorted is of all leaves/tokens in this sub-bree
    QList<SQLParser::Token const*> thisSubTree;
    thisSubTree.append(pre);
    thisSubTree.append(me);
    thisSubTree.append(post);

    if (!thisSubTree.isEmpty()) // might be empty in case of EOF token
    {
        // mark leftest and rightest leaves
        thisSubTree.first()->metadata().insert("SUBTREE_START", true); // mark 1st node
        thisSubTree.last()->metadata().insert("SUBTREE_END", true); // mark last node

        // the first (the leftest) token in this subtree also contains information about this subree width (in chars/tokens)
        thisSubTree.first()->metadata().insert("SUBTREE_LENGTH_CHARS", leftSonsLegth + preStrLen + meStrLen + postStrLen + rightSonsLenght);
        thisSubTree.first()->metadata().insert("SUBTREE_LENGTH_TOKENS", thisSubTree.length());
    }

    list.append(thisSubTree);
}
