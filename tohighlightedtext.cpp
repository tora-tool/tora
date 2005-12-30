/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"
#include "totool.h"

#include <ctype.h>

#include <qapplication.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qsimplerichtext.h>
#include <qstylesheet.h>
#include <qtimer.h>

#include <qextscintillalexersql.h>

#include "todefaultkeywords.h"

#include "tohighlightedtext.moc"

// Default SQL lexer
static QextScintillaLexerSQL sqlLexer(0);



toSyntaxAnalyzer::toSyntaxAnalyzer(const char **keywords)
{
    for (int i = 0;keywords[i];i++)
    {
        std::list<const char *> &curKey = Keywords[(unsigned char)char(toupper(*keywords[i]))];
        curKey.insert(curKey.end(), keywords[i]);
    }
    ColorsUpdated = false;
}

toSyntaxAnalyzer::posibleHit::posibleHit(const char *text)
{
    Pos = 1;
    Text = text;
}

QColor toSyntaxAnalyzer::getColor(toSyntaxAnalyzer::infoType typ)
{
    if (!ColorsUpdated)
    {
        updateSettings();
        ColorsUpdated = true;
    }
    return Colors[typ];
}

#define ISIDENT(c) (isalnum(c)||(c)=='_'||(c)=='%'||(c)=='$'||(c)=='#')

std::list<toSyntaxAnalyzer::highlightInfo> toSyntaxAnalyzer::analyzeLine(const QString &str,
        toSyntaxAnalyzer::infoType in,
        toSyntaxAnalyzer::infoType &out)
{
    std::list<highlightInfo> highs;
    std::list<posibleHit> search;

    bool inWord;
    bool wasWord = false;
    int multiComment = -1;
    int inString = -1;
    QChar endString;

    if (in == String)
    {
        inString = 0;
        endString = '\'';
    }
    else if (in == Comment)
    {
        multiComment = 0;
    }

    char c;
    char nc = str[0].latin1();
    for (int i = 0;i < int(str.length());i++)
    {
        std::list<posibleHit>::iterator j = search.begin();

        c = nc;
        if (int(str.length()) > i)
            nc = str[i + 1].latin1();
        else
            nc = ' ';

        bool nextSymbol = ISIDENT(nc);
        if (multiComment >= 0)
        {
            if (c == '*' && nc == '/')
            {
                highs.insert(highs.end(), highlightInfo(multiComment, Comment));
                highs.insert(highs.end(), highlightInfo(i + 2));
                multiComment = -1;
            }
        }
        else if (inString >= 0)
        {
            if (c == endString)
            {
                highs.insert(highs.end(), highlightInfo(inString, String));
                highs.insert(highs.end(), highlightInfo(i + 1));
                inString = -1;
            }
        }
        else if (c == '\'' || c == '\"')
        {
            inString = i;
            endString = str[i];
            search.clear();
            wasWord = false;
        }
        else if (c == '-' && nc == '-')
        {
            highs.insert(highs.end(), highlightInfo(i, Comment));
            highs.insert(highs.end(), highlightInfo(str.length() + 1));
            out = Normal;
            return highs;
        }
        else if (c == '/' && nc == '/')
        {
            highs.insert(highs.end(), highlightInfo(i, Comment));
            highs.insert(highs.end(), highlightInfo(str.length() + 1));
            out = Normal;
            return highs;
        }
        else if (c == '/' && nc == '*')
        {
            multiComment = i;
            search.clear();
            wasWord = false;
        }
        else
        {
            std::list<posibleHit> newHits;
            while (j != search.end())
            {
                posibleHit &cur = (*j);
                if (cur.Text[cur.Pos] == toupper(c))
                {
                    cur.Pos++;
                    if (!cur.Text[cur.Pos] && !nextSymbol)
                    {
                        newHits.clear();
                        highs.insert(highs.end(), highlightInfo(i - cur.Pos, Keyword));
                        highs.insert(highs.end(), highlightInfo(i + 1));
                        break;
                    }
                    newHits.insert(newHits.end(), cur);
                }
                j++;
            }
            search = newHits;
            if (ISIDENT(c))
                inWord = true;
            else
                inWord = false;

            if (!wasWord && inWord)
            {
                std::list<const char *> &curKey = Keywords[(unsigned char)char(toupper(c))];
                for (std::list<const char *>::iterator j = curKey.begin();
                        j != curKey.end();j++)
                {
                    if (strlen(*j) == 1)
                    {
                        if (!nextSymbol)
                        {
                            highs.insert(highs.end(), highlightInfo(i, Keyword));
                            highs.insert(highs.end(), highlightInfo(i));
                        }
                    }
                    else
                        search.insert(search.end(), posibleHit(*j));
                }
            }
            wasWord = inWord;
        }
    }
    if (inString >= 0)
    {
        if (endString == '\'')
        {
            out = String;
            highs.insert(highs.end(), highlightInfo(inString, String));
        }
        else
        {
            out = Normal;
            highs.insert(highs.end(), highlightInfo(inString, Error));
        }
        highs.insert(highs.end(), highlightInfo(str.length() + 1));
    }
    else if (multiComment >= 0)
    {
        highs.insert(highs.end(), highlightInfo(multiComment, Comment));
        highs.insert(highs.end(), highlightInfo(str.length() + 1));
        out = Comment;
    }
    else
        out = Normal;

    return highs;
}

static toSyntaxAnalyzer DefaultAnalyzer(DefaultKeywords);

toSyntaxAnalyzer &toSyntaxAnalyzer::defaultAnalyzer(void)
{
    return DefaultAnalyzer;
}

bool toSyntaxAnalyzer::reservedWord(const QString &str)
{
    if (str.length() == 0)
        return false;
    QString t = str.upper();
    std::list<const char *> &curKey = Keywords[(unsigned char)char(str[0].latin1())];
    for (std::list<const char *>::iterator i = curKey.begin();i != curKey.end();i++)
        if (t == (*i))
            return true;
    return false;
}


toHighlightedText::toHighlightedText(QWidget *parent, const char *name)
        : toMarkedText(parent, name), lexer(0), syntaxColoring(false)
{
    sqlLexer.setDefaultFont(toStringToFont(toTool::globalConfig(CONF_CODE, "")));
    
    // set default SQL lexer (syntax colouring as well)
    setLexer (&sqlLexer);
    
    // enable line numbers
    setMarginLineNumbers (0, true);
    
    // enable syntax colouring
    setSyntaxColoring (true);
    
    // set the font
    setFont(toStringToFont(toTool::globalConfig(CONF_CODE, "")));

    errorMarker=markerDefine(Circle,4);
    setMarkerBackgroundColor(Qt::red,errorMarker);
    debugMarker=markerDefine(Rectangle,8);
    setMarkerBackgroundColor(Qt::darkGreen,debugMarker);
    setMarkerBackgroundColor(Qt::red,errorMarker);
    setMarginMarkerMask(1,0);
    connect(this,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(setStatusMessage(void )));
}

toHighlightedText::~toHighlightedText() 
{
    
}

/** 
 * Sets the syntax colouring flag.
 */
void toHighlightedText::setSyntaxColoring(bool val)
{
    syntaxColoring = val;
    if (syntaxColoring) {
        QextScintilla::setLexer(lexer);
        update();
    }
    else {
        QextScintilla::setLexer(0);
    }
}

/**
 * Set the lexer to use. 
 * @param lexer to use,
 *        0 if no syntax colouring
 */
void toHighlightedText::setLexer(QextScintillaLexer *lexer) 
{
    if (lexer != 0) {
        this->lexer = lexer;
    }
    // refresh scintilla lexer
    setSyntaxColoring(syntaxColoring);
}

void toHighlightedText::setFont (const QFont & font)
{
    // Only sets fint lexer - one for all styles
    // this may (or may not) need to be changed in a future
    if (lexer) {
        lexer->setDefaultFont(font);
        lexer->setFont(font);
        update();
    }
}
void toHighlightedText::setCurrent(int current)
    {
        setCursorPosition (current, 0);
        markerDeleteAll(debugMarker);
        if(current>=0)
          markerAdd(current,debugMarker);
    }
void toHighlightedText::tableAtCursor(QString &owner, QString &table, bool mark)
{
    try
    {
        toConnection &conn = toCurrentConnection(this);
        int curline, curcol;
        getCursorPosition (&curline, &curcol);

        QString token = text(curline);
        toSQLParse::editorTokenizer tokens(this, curcol, curline);
        if (curcol > 0 && toIsIdent(token[curcol - 1]))
            token = tokens.getToken(false);
        else
            token = QString::null;

        toSQLParse::editorTokenizer lastTokens(this, tokens.offset(), tokens.line());
        token = tokens.getToken(false);
        if (token == ".")
        {
            lastTokens.setLine(tokens.line());
            lastTokens.setOffset(tokens.offset());
            owner = conn.unQuote(tokens.getToken(false));
            lastTokens.getToken(true);
            table += conn.unQuote(lastTokens.getToken(true));
        }
        else
        {
            tokens.setLine(lastTokens.line());
            tokens.setOffset(lastTokens.offset());
            owner = conn.unQuote(lastTokens.getToken(true));
            int tmplastline = lastTokens.line();
            int tmplastcol = lastTokens.offset();
            token = lastTokens.getToken(true);
            if (token == ".")
                table = conn.unQuote(lastTokens.getToken(true));
            else
            {
                lastTokens.setLine(tmplastline);
                lastTokens.setOffset(tmplastcol);
                table = owner;
                owner = QString::null;
            }
        }
        if (mark)
        {
            if (lastTokens.line() >= lines())
            {
                lastTokens.setLine(lines() - 1);
                lastTokens.setOffset(text(lines() - 1).length());
            }
            setSelection(tokens.line(), tokens.offset(), lastTokens.line(), lastTokens.offset());
        }
    }
    catch (...)
    {}
}

 bool toHighlightedText::hasErrors(){
     if ( Errors.empty() )
         return (false); 
     else
         return (true);
}

void toHighlightedText::nextError(void){
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    for (std::map<int, QString>::iterator i = Errors.begin();i != Errors.end();i++){   
        if ((*i).first > curline){
            setCursorPosition((*i).first, 0);
            break;
        }
    }
}

void toHighlightedText::previousError(void){
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    curcol = -1;
    for (std::map<int, QString>::iterator i = Errors.begin();i != Errors.end();i++){
        if ((*i).first >= curline){
            if (curcol < 0)
                curcol = (*i).first;
            break;
        }
        curcol = (*i).first;
    }
    if (curcol >= 0)
        setCursorPosition(curcol, 0);
}

void toHighlightedText::setErrors(const std::map<int, QString> &errors)
{
    Errors = errors;
    setStatusMessage();
    markerDeleteAll(errorMarker);
    for (std::map<int, QString>::iterator i = Errors.begin();i != Errors.end();i++){
       markerAdd((*i).first,errorMarker);
    }
}

void toHighlightedText::setStatusMessage(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    std::map<int, QString>::iterator err = Errors.find(curline);
    if (err == Errors.end())
        toStatusMessage(QString::null);
    else
        toStatusMessage((*err).second, true);
}
