
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

#include "editor/toworksheettext.h"
#include "editor/tocomplpopup.h"
#include "core/toconfiguration_new.h"
#include "core/toeditorsetting.h"
#include "core/toconnection.h"
#include "core/tologger.h"
#include "shortcuteditor/shortcutmodel.h"

#include <QtGui/QListWidget>

using namespace ToConfiguration;

toWorksheetText::toWorksheetText(QWidget *parent, const char *name)
    : toSqlText(parent, name)
	, m_bookmarkMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightTriangle))
	, m_bookmarkHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
	, m_complAPI(NULL)
	, complTimer(new QTimer(this))
	, editorType(SciTe)
	, popup(new toComplPopup(this))
{
    QsciScintilla::setAutoCompletionThreshold(0);
    QsciScintilla::setAutoCompletionSource(QsciScintilla::AcsAPIs);
    QsciScintilla::setAutoIndent(true);

    // highlight caret line
    QsciScintilla::setCaretLineVisible(true);
#ifdef SCI_LEXER
    // This is only required until transparency fixes in QScintilla go into stable release
    QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, QsciScintilla::SC_ALPHA_NOALPHA);
#else
    QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, 100);
#endif

    // handle "max text width" mark
    if (toConfigurationNewSingle::Instance().option(Editor::UseMaxTextWidthMarkBool).toBool())
    {
    	QsciScintilla::setEdgeColumn(toConfigurationNewSingle::Instance().option(Editor::MaxTextWidthMarkInt).toInt());
    	// TODO setEdgeColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).darker(150));
    	QsciScintilla::setEdgeMode(QsciScintilla::EdgeLine);
    }
    else
    	QsciScintilla::setEdgeMode(QsciScintilla::EdgeNone);

    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    connect( complTimer, SIGNAL(timeout()), this, SLOT(autoCompleteFromAPIs()) );

	connect(&toEditorTypeButtonSingle::Instance(),
			SIGNAL(toggled(int)),
			this,
			SLOT(setEditorType(int)));

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

toWorksheetText::~toWorksheetText()
{
}

void toWorksheetText::setHighlighter(toSqlText::HighlighterTypeEnum e)
{
	super::setHighlighter(e);
	if(super::lexer())
	{
		m_complAPI = super::lexer()->apis();
	} else {
		m_complAPI = NULL;
	}
}

void toWorksheetText::keyPressEvent(QKeyEvent * e)
{
    // handle editor shortcuts with TAB
    // It uses qscintilla lowlevel API to handle "word under cursor"
    // This code is taken from sqliteman.com
    if (e->key() == Qt::Key_Tab && toConfigurationNewSingle::Instance().option(Editor::UseEditorShortcutsBool).toBool())
    {
        long pos = currentPosition();
        int start = SendScintilla(SCI_WORDSTARTPOSITION, pos, true);
        int end = SendScintilla(SCI_WORDENDPOSITION, pos, true);
        QString key(wordAtPosition(pos, true));
        EditorShortcutsMap shorts(toConfigurationNewSingle::Instance().option(Editor::EditorShortcutsMap).toMap());
        if (shorts.contains(key))
        {
            setSelection(start, end);
            removeSelectedText();
            insert(shorts.value(key).toString(), false);
            pos = SendScintilla(SCI_GETCURRENTPOS);
            SendScintilla(SCI_SETEMPTYSELECTION, pos + shorts.value(key).toByteArray().length());
    		e->accept();
            return;
        }
    } else if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_T) {
        autoCompleteFromAPIs();
        e->accept();
        return;
    }
	super::keyPressEvent(e);
}

void toWorksheetText::positionChanged(int row, int col)
{
	if (col > 0)
	{
		int position = this->SendScintilla(SCI_GETCURRENTPOS);
		position = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, position);
		char c = getByteAt(position);
		// TODO use getWCharAt and handle multibyte characters here
		if (c == '.')
			complTimer->start(500);
	}
	else
	{
		if (complTimer->isActive())
			complTimer->stop();
	}
// FIXME: disabled due repainting issues
//    current line marker (margin arrow)
//    markerDeleteAll(m_currentLineMarginHandle);
//    markerAdd(row, m_currentLineMarginHandle);
}

// the QScintilla way of autocomletition
#if 0
void toWorksheetText::autoCompleteFromAPIs()
{
    complTimer->stop(); // it's a must to prevent infinite reopening
	{
		toScintilla::autoCompleteFromAPIs();
		return;
	}
}
#endif

// the Tora way of autocomletition
void toWorksheetText::autoCompleteFromAPIs()
{
    complTimer->stop(); // it's a must to prevent infinite reopening

    Utils::toBusy busy;
    QListWidget *list = popup->list();
    QString partial; // TODO never used, never assigned
    QStringList compleList = this->getCompletionList(partial);

    if (compleList.count() == 0)
        return;

    if (compleList.count() == 1 /*&& compleList.first() == partial*/)
    {
        completeWithText(compleList.first());
    } else {
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
//        if (!partial.isNull() && partial.length() > 0)
//        {
//            int i;
//            for (i = 0; i < list->model()->rowCount(); i++)
//            {
//                if (list->item(i)->text().indexOf(partial) == 0)
//                {
//                    list->item(i)->setSelected(true);
//                    list->setCurrentItem(list->item(i));
//                    break;
//                }
//            }
//        }

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

void toWorksheetText::completeFromAPI(QListWidgetItem* item)
{
    if (item)
    {
    	completeWithText(item->text());
    }
    popup->hide();
}

void toWorksheetText::completeWithText(QString const& text)
{
    long pos = currentPosition();
    int start = SendScintilla(SCI_WORDSTARTPOSITION, pos, true);
    int end = SendScintilla(SCI_WORDENDPOSITION, pos, true);
    setSelection(start, end);
    removeSelectedText();
    insert(text);
    SendScintilla(SCI_SETCURRENTPOS,
                  SendScintilla(SCI_GETCURRENTPOS) +
                  text.length());
    pos = SendScintilla(SCI_GETCURRENTPOS);
    SendScintilla(SCI_SETSELECTIONSTART, pos, true);
    SendScintilla(SCI_SETSELECTIONEND, pos, true);
}

void toWorksheetText::setEditorType(int)
{

}

void toWorksheetText::handleBookmark()
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

void toWorksheetText::gotoPrevBookmark()
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

void toWorksheetText::gotoNextBookmark()
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

QStringList toWorksheetText::getCompletionList(QString &partial)
{
    QStringList retval;
#if 0
    int curline, curcol;
    // used as a flag to prevent completion popup when there is
    // an orphan comma. In short - be less agressive on popup.
    bool showDefault = false;
    getCursorPosition (&curline, &curcol);

    QString line = text(curline);

    if (isReadOnly() || curcol == 0 || !toConfigurationSingle::Instance().codeCompletion())
        return retval;

    //throw QString("QStringList toHighlightedTextEditor::getCompletionList ... not implemented yet.");

    toSQLParse::editorTokenizer tokens(this, curcol, curline);
    if (curcol > 0 && line[curcol - 1] != '.')
    {
        partial = tokens.getToken(false);
        showDefault = true;
    }
    else
    {
        partial = "";
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
        while ((invalidToken(tokens.line(), tokens.offset() + token.length()) || UpperIdent(token) != cmp || lastToken == ".") && token != ";" && token != "~~~" && !token.isEmpty())
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
            if (token != "TABLE" && token != "UPDATE" && token != "FROM" && token != "INTO" && (Utils::toIsIdent(token[0]) || token[0] == '\"'))
            {
                name = token;
                token = tokens.getToken(false);
                if (token == ".")
                    owner = tokens.getToken(false);
            }
            else if (token == ")")
                return retval;
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
            toConnection &conn = toConnection::currentConnection(this);
            toConnection::objectName object = conn.realName(name, false);
            if(object.Type == "DATABASE")
            {
                std::list<toConnection::objectName> list = conn.tables(object);
                Q_FOREACH(toConnection::objectName table, list)
                {
                    QString t = conn.quote(table.Name, false);
                    if(t.indexOf(*partial) == 0)
                        retval.append(t);
                }
            }
            else
            {
                ///const toQDescList &desc = conn.columns(object);
                for (toQDescList::const_iterator i = desc.begin(); i != desc.end(); i++)
                {
                    QString t;
                    int ind = (*i).Name.indexOf("(");
                    if (ind < 0)
                        ind = (*i).Name.indexOf("RETURNING") - 1; //it could be a function or procedure without parameters. -1 to remove the space
                    if (ind >= 0)
                        t = conn.quote((*i).Name.mid(0, ind), false) + (*i).Name.mid(ind);
                    else
                        t = conn.quote((*i).Name, false);
                    if (t.indexOf(*partial) == 0)
                        retval.append(t);
                }
            }
        }
        catch (QString const &e)
        {
            TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList:" << e << std::endl;
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
            TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList: Unknown error.";
        }
    }

    // if is toReturn empty fill it with keywords...
    if (showDefault && retval.count() == 0)
    {
        for (int i = 0; i < defaultCompletion.size(); ++i)
        {
            if (defaultCompletion.at(i).startsWith(partial, Qt::CaseInsensitive))
                retval.append(defaultCompletion.at(i));
        }
    }

    retval.sort();
#endif
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    QString word = wordAtLineIndex(curline, curcol);
    retval = toConnection::currentConnection(this).getCache().completeEntry(word);
    retval.sort();
    Q_FOREACH(QString t, retval)
    {
    	TLOG(0, toNoDecorator, __HERE__) << " Tab: " << t << std::endl;
    }

    return retval;
}

void toWorksheetText::focusInEvent(QFocusEvent *e)
{
	toEditorTypeButtonSingle::Instance().setEnabled(true);
	toEditorTypeButtonSingle::Instance().setValue(editorType);
	super::focusInEvent(e);
}

void toWorksheetText::focusOutEvent(QFocusEvent *e)
{
	toEditorTypeButtonSingle::Instance().setDisabled(true);
	super::focusOutEvent(e);
}

#ifdef TORA3_SESSION
void toWorksheetText::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    data[prefix + ":Filename"] = Filename;
    data[prefix + ":Text"] = text();
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    data[prefix + ":Column"] = QString::number(curcol);
    data[prefix + ":Line"] = QString::number(curline);
    if (isModified())
        data[prefix + ":Edited"] = "Yes";
}

void toWorksheetText::importData(std::map<QString, QString> &data, const QString &prefix)
{
    QString txt = data[prefix + ":Text"];
    if (txt != text())
        setText(txt);
    Filename = data[prefix + ":Filename"];
    setCursorPosition(data[prefix + ":Line"].toInt(), data[prefix + ":Column"].toInt());
    if (data[prefix + ":Edited"].isEmpty())
        setModified(false);
}
#endif

toEditorTypeButton::toEditorTypeButton(QWidget *parent, const char *name)
	: toToggleButton(toWorksheetText::staticMetaObject.enumerator(toWorksheetText::staticMetaObject.indexOfEnumerator("EditorTypeEnum"))
	, parent
	, name
	)
{
}

toEditorTypeButton::toEditorTypeButton()
	: toToggleButton(toWorksheetText::staticMetaObject.enumerator(toWorksheetText::staticMetaObject.indexOfEnumerator("EditorTypeEnum"))
	, NULL
	)
{
}
