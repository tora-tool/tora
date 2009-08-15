
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"
#include "totool.h"

#include <ctype.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qtimer.h>
#include <qkeysequence.h>
#include <Qsci/qsciapis.h>
#include <qnamespace.h>
#include <QListWidget>
#include <QtDebug>

#include <Qsci/qscilexersql.h>
#include <qpoint.h>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QFile>
#include <QMessageBox>

#include "todefaultkeywords.h"


// Default SQL lexer
// static QsciLexerSQL sqlLexer;
// this definition (via function) fixes the font screwing
// in the GUI. It's stronlgy suggested to handle Qt stuff
// *after* QApplication initialization.
QsciLexerSQL * sqlLexer()
{
    static QsciLexerSQL * _sqlLexer;
    if (!_sqlLexer)
        _sqlLexer = new QsciLexerSQL();
    return _sqlLexer;
}


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

/*
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
    char nc = str[0].toLatin1();
    for (int i = 0;i < int(str.length());i++)
    {
        std::list<posibleHit>::iterator j = search.begin();

        c = nc;
        if (int(str.length()) > i)
            nc = str[i + 1].toLatin1();
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
            out = Default;
            return highs;
        }
        else if (c == '/' && nc == '/')
        {
            highs.insert(highs.end(), highlightInfo(i, Comment));
            highs.insert(highs.end(), highlightInfo(str.length() + 1));
            out = Default;
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
            out = Default;
            //highs.insert(highs.end(), highlightInfo(inString, Error));
            highs.insert(highs.end(), highlightInfo(inString, Comment));
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
        out = Default;

    return highs;
}
*/

static toSyntaxAnalyzer DefaultAnalyzer(DefaultKeywords);

toSyntaxAnalyzer &toSyntaxAnalyzer::defaultAnalyzer(void)
{
    return DefaultAnalyzer;
}

bool toSyntaxAnalyzer::reservedWord(const QString &str)
{
    if (str.length() == 0)
        return false;
    QString t = str.toUpper();
    std::list<const char *> &curKey = Keywords[(unsigned char)char(str[0].toLatin1())];
    for (std::list<const char *>::iterator i = curKey.begin();i != curKey.end();i++)
        if (t == (*i))
            return true;
    return false;
}

toComplPopup::toComplPopup(toHighlightedText* edit)
        : QWidget(edit, Qt::Popup)
{
    List   = new QListWidget(this);
    Editor = edit;

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    vbox->addWidget(List);
    setLayout(vbox);
    setFocusProxy(List);
}

toComplPopup::~toComplPopup()
{
}


void toComplPopup::hide()
{
    if (parentWidget())
        parentWidget()->setFocus();
    QWidget::hide();
}


void toComplPopup::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }

    if ((!e->text().isNull() && e->text().length() > 0 && e->key() != Qt::Key_Return
            && e->text() != " ") || e->key() == Qt::Key_Backspace)
    {
        this->Editor->keyPressEvent(e);
        this->Editor->autoCompleteFromAPIs();
    }
    else if (!e->text().isNull() && e->text().length() > 0 && e->text() == " ")
    {
        this->Editor->keyPressEvent(e);
        this->hide();
    }
    else
        QWidget::keyPressEvent(e);

}


toHighlightedText::toHighlightedText(QWidget *parent, const char *name)
        : toMarkedText(parent, name),
        lexer(0)
{
    sqlLexer()->setDefaultFont(toStringToFont(toConfigurationSingle::Instance().codeFont()));

    // set default keywords for code completion
    QFile api(":/templates/completion.api");
    defaultCompletion.clear();
    if (!api.open(QIODevice::ReadOnly | QIODevice::Text))
        QMessageBox::warning(this, tr("Init error"),
                             tr("Cannot read code completion API from %1").arg(api.fileName()));
    else
    {
        while (!api.atEnd())
        {
            QString s(api.readLine());
            defaultCompletion.append(s.trimmed());
        }
    }

    // set default SQL lexer (syntax colouring as well)
    setLexer (sqlLexer());

    setFolding(QsciScintilla::BoxedFoldStyle);
    sqlLexer()->setFoldComments(true);
    sqlLexer()->setFoldCompact(false);

    // enable syntax colouring
    setSyntaxColoring (true);

    // set the font
    setFont(toStringToFont(toConfigurationSingle::Instance().codeFont()));

    m_errorMarginHandle = markerDefine(QsciScintilla::Circle);
    m_errorHandle = markerDefine(QsciScintilla::Background);

    m_debugMarginHandle = markerDefine(QsciScintilla::Rectangle);
    m_debugHandle = markerDefine(QsciScintilla::Background);

    m_currentLineMarginHandle = markerDefine(QsciScintilla::RightArrow);
    m_currentLineHandle = markerDefine(QsciScintilla::Background);

    m_bookmarkMarginHandle = markerDefine(QsciScintilla::RightTriangle);
    m_bookmarkHandle = markerDefine(QsciScintilla::Background);

    updateSyntaxColor(toSyntaxAnalyzer::DebugBg);
    updateSyntaxColor(toSyntaxAnalyzer::ErrorBg);
    updateSyntaxColor(toSyntaxAnalyzer::CurrentLineMarker);

    // handle "max text width" mark
    if (toConfigurationSingle::Instance().useMaxTextWidthMark())
    {
        setEdgeColumn(toConfigurationSingle::Instance().maxTextWidthMark());
        setEdgeColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).darker(150));
        setEdgeMode(QsciScintilla::EdgeLine);
    }
    else
        setEdgeMode(QsciScintilla::EdgeNone);

    setAutoIndent(true);
    connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(setStatusMessage(void )));
    complAPI = new QsciAPIs(lexer);
    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(autoCompleteFromAPIs()) );
    popup = new toComplPopup(this);
    popup->hide();
    connect(popup->list(),
            SIGNAL(itemClicked(QListWidgetItem*)),
            this,
            SLOT(completeFromAPI(QListWidgetItem*)));
    connect(popup->list(),
            SIGNAL(itemActivated(QListWidgetItem*)),
            this,
            SLOT(completeFromAPI(QListWidgetItem*)));
}

void toHighlightedText::keyPressEvent(QKeyEvent * e)
{
    // handle editor shortcuts with TAB
    // It uses qscintilla lowlevel API to handle "word unde cursor"
    // This code is taken from sqliteman.com
    if (toConfigurationSingle::Instance().useEditorShortcuts()
        && e->key() == Qt::Key_Tab)
    {
        int pos = SendScintilla(SCI_GETCURRENTPOS);
        int start = SendScintilla(SCI_WORDSTARTPOSITION, pos,true);
        int end = SendScintilla(SCI_WORDENDPOSITION, pos, true);
        SendScintilla(SCI_SETSELECTIONSTART, start, true);
        SendScintilla(SCI_SETSELECTIONEND, end, true);
        QString key(selectedText());
        EditorShortcutsMap shorts(toConfigurationSingle::Instance().editorShortcuts());
        if (shorts.contains(key))
        {
            removeSelectedText();
            insert(shorts.value(key).toString());
            SendScintilla(SCI_SETCURRENTPOS,
                           SendScintilla(SCI_GETCURRENTPOS) +
                           shorts.value(key).toString().length());
            pos = SendScintilla(SCI_GETCURRENTPOS);
            SendScintilla(SCI_SETSELECTIONSTART, pos,true);
            SendScintilla(SCI_SETSELECTIONEND, pos, true);
            return;
        }
        SendScintilla(SCI_SETSELECTIONSTART, pos,true);
        SendScintilla(SCI_SETSELECTIONEND, pos, true);
    }
    toMarkedText::keyPressEvent(e);
}

toHighlightedText::~toHighlightedText()
{
    if (complAPI)
        delete complAPI;
    if (popup)
        delete popup;
}

void toHighlightedText::positionChanged(int row, int col)
{
    if (col > 0 && this->text(row)[col-1] == '.')
    {
        timer->start(500);
    }
    else
    {
        if (timer->isActive())
            timer->stop();
    }
    // current line marker
    markerDeleteAll(m_currentLineHandle);
    markerDeleteAll(m_currentLineMarginHandle);
    markerAdd(row, m_currentLineHandle);
    markerAdd(row, m_currentLineMarginHandle);
}

static QString UpperIdent(const QString &str)
{
    if (str.length() > 0 && str[0] == '\"')
        return str;
    else
        return str.toUpper();
}

void toHighlightedText::autoCompleteFromAPIs()
{
    timer->stop(); // it's a must to prevent infinite reopening

    QListWidget *list = popup->list();
    QString partial;
    QStringList compleList = this->getCompletionList(&partial);

    if (compleList.count() == 0)
        return;

    if (compleList.count() == 1 && compleList.first() == partial)
        this->completeWithText(compleList.first());
    else
    {
        long position, posx, posy;
        int curCol, curRow;
        this->getCursorPosition(&curRow, &curCol);
        position = this->SendScintilla(SCI_GETCURRENTPOS);
        posx = this->SendScintilla(SCI_POINTXFROMPOSITION, 0, position);
        posy = this->SendScintilla(SCI_POINTYFROMPOSITION, 0, position) +
               this->SendScintilla(SCI_TEXTHEIGHT, curRow);
        QPoint p(posx, posy);
        p = mapToGlobal(p);
        popup->move(p);
        list->clear();
        list->addItems(compleList);
        if (!partial.isNull() && partial.length() > 0)
        {
            int i;
            for (i = 0;i < list->model()->rowCount();i++)
            {
                if (list->item(i)->text().indexOf(partial) == 0)
                {
                    list->item(i)->setSelected(true);
                    list->setCurrentItem(list->item(i));
                    break;
                }
            }
        }

        // if there's no current selection, select the first
        // item. that way arrow keys work as intended.
        QList<QListWidgetItem *> selected = list->selectedItems();
        if (selected.size() < 1 && list->count() > 0)
        {
            list->item(0)->setSelected(true);
            list->setCurrentItem(list->item(0));
        }

        popup->show();
        popup->setFocus();
    }
}

bool toHighlightedText::invalidToken(int line, int col)
{
    bool ident = true;
    if (line < 0)
    {
        line = 0;
        col = 0;
    }
    while (line < lines())
    {
        QString cl = text(line);
        while (col < int(cl.length()))
        {
            QChar c = cl[col];
            if (!toIsIdent(c))
                ident = false;
            if (!ident && !c.isSpace())
                return c == '.';
            col++;
        }
        line++;
        col = 0;
    }
    return false;
}

/**
 * Sets the syntax colouring flag.
 */
void toHighlightedText::setSyntaxColoring(bool val)
{
    syntaxColoring = val;
    if (syntaxColoring)
    {
        QsciScintilla::setLexer(lexer);
        updateSyntaxColor(toSyntaxAnalyzer::Default);
        updateSyntaxColor(toSyntaxAnalyzer::Comment);
        updateSyntaxColor(toSyntaxAnalyzer::Number);
        updateSyntaxColor(toSyntaxAnalyzer::Keyword);
        updateSyntaxColor(toSyntaxAnalyzer::String);
        updateSyntaxColor(toSyntaxAnalyzer::DefaultBg);
        updateSyntaxColor(toSyntaxAnalyzer::CurrentLineMarker);

        update();
    }
    else
    {
        QsciScintilla::setLexer(0);
    }
}

/**
 * Sets the syntax colours for given type
 */
void toHighlightedText::updateSyntaxColor(toSyntaxAnalyzer::infoType t)
{
    QColor col = DefaultAnalyzer.getColor(t);

    switch (t)
    {
    case toSyntaxAnalyzer::Default:
        lexer->setColor(col, QsciLexerSQL::Default);
        //lexer->setColor(col, QsciLexerSQL::CommentLineHash);
        break;
    case toSyntaxAnalyzer::Comment:
        lexer->setColor(col, QsciLexerSQL::Comment);
        lexer->setColor(col, QsciLexerSQL::CommentLine);
        lexer->setColor(col, QsciLexerSQL::PlusPrompt);
        lexer->setColor(col, QsciLexerSQL::PlusComment);
        lexer->setColor(col, QsciLexerSQL::CommentDoc);
        lexer->setColor(col, QsciLexerSQL::CommentDocKeyword);
        break;
    case toSyntaxAnalyzer::Number:
        lexer->setColor(col, QsciLexerSQL::Number);
        break;
    case toSyntaxAnalyzer::Keyword:
        lexer->setColor(col, QsciLexerSQL::Keyword);
        lexer->setColor(col, QsciLexerSQL::PlusKeyword);
        lexer->setColor(col, QsciLexerSQL::Operator);
        break;
    case toSyntaxAnalyzer::String:
        lexer->setColor(col, QsciLexerSQL::DoubleQuotedString);
        lexer->setColor(col, QsciLexerSQL::SingleQuotedString);
        break;
    case toSyntaxAnalyzer::DefaultBg:
        lexer->setPaper(col);
        //lexer->setPaper(col, QsciLexerSQL::Default);
        break;
    case toSyntaxAnalyzer::ErrorBg:
        setMarkerBackgroundColor(col, m_errorHandle);
        break;
    case toSyntaxAnalyzer::DebugBg:
        setMarkerBackgroundColor(col, m_debugHandle);
        break;
    case toSyntaxAnalyzer::CurrentLineMarker:
        setMarkerBackgroundColor(col, m_currentLineHandle);
//         setMarkerBackgroundColor(col, m_currentLineMarginHandle);
        // TODO/FIXME?: make it configurable - color.
        setMarkerBackgroundColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).lighter(100),
                          m_bookmarkHandle);
        break;
    default:
        break;
    }
}


void toHighlightedText::openFilename(const QString & file)
{
    toMarkedText::openFilename(file);

    m_bookmarks.clear();
    markerDeleteAll(m_bookmarkHandle);
    markerDeleteAll(m_bookmarkMarginHandle);
    setErrors(QMap<int,QString>());
}

/**
 * Set the lexer to use.
 * @param lexer to use,
 *        0 if no syntax colouring
 */
void toHighlightedText::setLexer(QsciLexer *lexer)
{
    if (lexer != 0)
    {
        this->lexer = lexer;
    }
    lexer->setColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::Default), -1);
    // refresh scintilla lexer
    setSyntaxColoring(syntaxColoring);
}

void toHighlightedText::setFont (const QFont & font)
{
    // Only sets fint lexer - one for all styles
    // this may (or may not) need to be changed in a future
    if (lexer)
    {
        lexer->setDefaultFont(font);
        lexer->setFont(font);

        /* this is workaround against qscintilla 1.6 setFont(font) bug */
        lexer->setFont(font, QsciLexerSQL::Default);
        lexer->setFont(font, QsciLexerSQL::Comment);
        lexer->setFont(font, QsciLexerSQL::CommentLine);
        lexer->setFont(font, QsciLexerSQL::PlusComment);
        lexer->setFont(font, QsciLexerSQL::CommentLineHash);
        lexer->setFont(font, QsciLexerSQL::CommentDocKeyword);
        lexer->setFont(font, QsciLexerSQL::CommentDocKeywordError);
        lexer->setFont(font, QsciLexerSQL::DoubleQuotedString);
        lexer->setFont(font, QsciLexerSQL::SingleQuotedString);
        lexer->setFont(font, QsciLexerSQL::PlusPrompt);

        update();
    }
}
void toHighlightedText::setCurrent(int current)
{
    setCursorPosition (current, 0);
    markerDeleteAll(m_debugHandle);
    markerDeleteAll(m_debugMarginHandle);
    if (current >= 0)
    {
        markerAdd(current, m_debugHandle);
        markerAdd(current, m_debugMarginHandle);
    }
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

bool toHighlightedText::hasErrors()
{
    if ( Errors.empty() )
        return (false);
    else
        return (true);
}

void toHighlightedText::nextError(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    for (QMap<int, QString>::iterator i = Errors.begin();i != Errors.end();i++)
    {
        if (i.key() > curline)
        {
            setCursorPosition(i.key(), 0);
            break;
        }
    }
}

void toHighlightedText::previousError(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    curcol = -1;
    for (QMap<int, QString>::iterator i = Errors.begin();i != Errors.end();i++)
    {
        if (i.key() >= curline)
        {
            if (curcol < 0)
                curcol = i.key();
            break;
        }
        curcol = i.key();
    }
    if (curcol >= 0)
        setCursorPosition(curcol, 0);
}

void toHighlightedText::handleBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);

    if (m_bookmarks.contains(curline))
    {
        markerDelete(curline, m_bookmarkHandle);
        markerDefine(curline, m_bookmarkMarginHandle);
        m_bookmarks.removeAll(curline);
    }
    else
    {
        markerAdd(curline, m_bookmarkHandle);
        markerAdd(curline, m_bookmarkMarginHandle);
        m_bookmarks.append(curline);
    }
    qSort(m_bookmarks);
}

void toHighlightedText::gotoPrevBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    --curline;

    int newline = -1;
    foreach(int i, m_bookmarks)
    {
        if (curline < i)
            break;
        newline = i;
    }
    if (newline >= 0)
        setCursorPosition(newline, 0);
}

void toHighlightedText::gotoNextBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    ++curline;

    int newline = -1;
    foreach(int i, m_bookmarks)
    {
        if (curline > i)
            continue;
        newline = i;
        break;
    }
    if (newline >= 0)
        setCursorPosition(newline, 0);
}

void toHighlightedText::setErrors(const QMap<int, QString> &errors)
{
    Errors = errors;
    setStatusMessage();
    markerDeleteAll(m_errorHandle);
    markerDeleteAll(m_errorMarginHandle);
    for (QMap<int, QString>::const_iterator i = Errors.begin();i != Errors.end();i++)
    {
        markerAdd(i.key(), m_errorHandle);
        markerAdd(i.key(), m_errorMarginHandle);
    }
}

void toHighlightedText::setStatusMessage(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    QMap<int, QString>::iterator err = Errors.find(curline);
    if (err == Errors.end())
        toStatusMessage(QString::null);
    else
        toStatusMessage(err.value(), true);
}

QStringList toHighlightedText::getCompletionList(QString* partial)
{
    int curline, curcol;
    // used as a flag to prevent completion popup when there is
    // an orphan comma. In short - be less agressive on popup.
    bool showDefault = false;
    QStringList toReturn;
    getCursorPosition (&curline, &curcol);

    QString line = text(curline);

    if (isReadOnly() || curcol == 0 || !toConfigurationSingle::Instance().codeCompletion())
        return toReturn;

    toSQLParse::editorTokenizer tokens(this, curcol, curline);
    if (curcol > 0 && line[curcol-1] != '.')
    {
        *partial = tokens.getToken(false);
        showDefault = true;
    }
    else
    {
        *partial = "";
    }

    QString name = tokens.getToken(false);
    QString owner;
    if (name == ".")
    {
        name = tokens.getToken(false);
    }

    QString token = tokens.getToken(false);

    if (token == ".")
        owner = tokens.getToken(false);
    else
    {
        QString cmp = UpperIdent(name);
        QString lastToken;
        while ((invalidToken(tokens.line(), tokens.offset() + token.length()) || UpperIdent(token) != cmp || lastToken == ".") && token != ";" && !token.isEmpty())
        {
            lastToken = token;
            token = tokens.getToken(false);
        }

        if (token == ";" || token.isEmpty())
        {
            tokens.setLine(curline);
            tokens.setOffset(curcol);
            token = tokens.getToken();
            while ((invalidToken(tokens.line(), tokens.offset()) || (UpperIdent(token) != cmp && lastToken != ".")) && token != ";" && !token.isEmpty())
                token = tokens.getToken();
            lastToken = token;
            tokens.getToken(false);
        }
        if (token != ";" && !token.isEmpty())
        {
            token = tokens.getToken(false);
            if (token != "TABLE" && token != "UPDATE" && token != "FROM" && token != "INTO" && (toIsIdent(token[0]) || token[0] == '\"'))
            {
                name = token;
                token = tokens.getToken(false);
                if (token == ".")
                    owner = tokens.getToken(false);
            }
            else if (token == ")")
                return toReturn;
        }
    }
    if (!owner.isEmpty())
    {
        name = owner + QString::fromLatin1(".") + name;
    }
    if (!name.isEmpty())
    {
        try
        {
            toConnection &conn = toCurrentConnection(this);
            toQDescList &desc = conn.columns(conn.realName(name, false));
            for (toQDescList::iterator i = desc.begin();i != desc.end();i++)
            {
                QString t;
                int ind = (*i).Name.indexOf("(");
                if (ind < 0)
                    ind = (*i).Name.indexOf("RETURNING") - 1; //it could be a function or procedure without parameters. -1 to remove the space
                if (ind >= 0)
                    t = conn.quote((*i).Name.mid(0, ind)) + (*i).Name.mid(ind);
                else
                    t = conn.quote((*i).Name);
                if (t.indexOf(*partial) == 0)
                    toReturn.append(t);
            }
        }
        catch (QString e)
        {
            qDebug() << "toHighlightedText::getCompletionList:" << e;
        }
        catch (...)
        {
            qDebug() << "toHighlightedText::getCompletionList: Unknown error.";
        }
    }
    // if is toReturn empty fill it with keywords...
    if (showDefault && toReturn.count() == 0)
    {
        for (int i = 0; i < defaultCompletion.size(); ++i)
        {
            if (defaultCompletion.at(i).startsWith(*partial, Qt::CaseInsensitive))
                toReturn.append(defaultCompletion.at(i));
        }
    }

    toReturn.sort();
    return toReturn;
}

void toHighlightedText::completeWithText(QString itemText)
{
    int curline, curcol, start, end;
    getCursorPosition (&curline, &curcol);
    QString line = text(curline);
    toSQLParse::editorTokenizer tokens(this, curcol, curline);
    if (line[curcol-1] != '.')
    {
        tokens.getToken(false);
        start = tokens.offset();
    }
    else
    {
        start = curcol;
    }
    if (line[curcol].isSpace())
    {
        end = curcol;
    }
    else
    {
        tokens.getToken(true);
        if (tokens.line() != curline)
            end = line.length();
        else
            end = tokens.offset();
    }
    disconnect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    setSelection(curline, start, curline, end);
    this->removeSelectedText();
    this->insert(itemText);
    this->setCursorPosition(curline, start + itemText.length());
    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
}


void toHighlightedText::completeFromAPI(QListWidgetItem* item)
{
    if (item)
        this->completeWithText(item->text());
    popup->hide();
}
