
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
#include "tools/toworksheeteditor.h"
#include "tools/toworksheet.h"
#include "editor/tocomplpopup.h"
#include "core/toconfiguration.h"
#include "core/toeditorsetting.h"
#include "core/toconnection.h"
#include "core/toconnectiontraits.h"
#include "core/tologger.h"
#include "shortcuteditor/shortcutmodel.h"

#include <QListWidget>

using namespace ToConfiguration;

toWorksheetText::toWorksheetText(QWidget *parent, const char *name)
    : toSqlText(parent, name)
    , editorType(SciTe)
    , popup(new toComplPopup(this))
    , m_complAPI(NULL)
    , complTimer(new QTimer(this))
    , m_bookmarkHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
    , m_bookmarkMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightTriangle))
	, m_completeEnabled(toConfigurationNewSingle::Instance().option(Editor::UseSpacesForIndentBool).toBool())
{
	if (m_completeEnabled)
	{
		QsciScintilla::setAutoCompletionThreshold(0);
		QsciScintilla::setAutoCompletionSource(QsciScintilla::AcsAPIs);
	}
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
    if (super::lexer())
    {
        m_complAPI = super::lexer()->apis();
    }
    else
    {
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
    }
    else if (m_completeEnabled && e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_T)
    {
        autoCompleteFromAPIs();
        e->accept();
        return;
    }
    super::keyPressEvent(e);
}

void toWorksheetText::positionChanged(int row, int col)
{
    int position = currentPosition();
    wchar_t currentChar = getWCharAt(position);
	TLOG(0, toNoDecorator, __HERE__) << currentChar << std::endl;

	if (col <= 0)
    {
    	complTimer->stop();
    	return;
    }

	// Cursor is not at EOL, not before any word character
	if( currentChar != 0 && CharClass(currentChar) != CharClassify::ccWord && CharClass(currentChar) != CharClassify::ccSpace) {
    	complTimer->stop();
    	return;
    }

	for(int i=1, c=col; i<3 && c; i++, c--)
	{
		position = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, position);
		currentChar = getWCharAt(position);
		if (m_completeEnabled && currentChar == L'.')
		{
			complTimer->start(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::CodeCompleteDelayInt).toInt());
			return;
		}
		if (CharClass(currentChar) != CharClassify::ccWord)
			break;
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
    toConnection &connection = toConnection::currentConnection(this);

    TLOG(0, toTimeStart, __HERE__) << "Start" << std::endl;
    int position = currentPosition();
    toSqlText::Word schemaWord, tableWord;
    tableAtCursor(schemaWord, tableWord);

    QString schema = connection.getTraits().unQuote(schemaWord.text());          // possibly unquote schema name
    QString table = tableWord.start() > position ? QString() : tableWord.text(); // possible tableAtCursor matched some distant work (behind spaces)

    TLOG(0, toTimeDelta, __HERE__) << "Table at indexb: " << '"' << schemaWord.text() << '"' << ':' << '"' << tableWord.text() << '"' << std::endl;

    setSelection(schemaWord.start(), position);

    QStringList compleList;
    if (schema.isEmpty())
    	compleList = connection.getCache().completeEntry(toToolWidget::currentSchema(this), table);
    else
    	compleList = connection.getCache().completeEntry(schema, table);

    TLOG(0, toTimeDelta, __HERE__) << "Complete entry" << std::endl;

    if (compleList.size() <= 100) // Do not waste CPU on sorting huge completition list TODO: limit the amount of returned entries
    	compleList.sort();
    TLOG(0, toTimeDelta, __HERE__) << "Sort" << std::endl;

    if (compleList.isEmpty())
    {
    	this->SendScintilla(SCI_SETEMPTYSELECTION, position);
        return;
    }

    if (compleList.count() == 1 /*&& compleList.first() == partial*/)
    {
        completeWithText(compleList.first());
    }
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
        QListWidget *list = popup->list();
        list->clear();
        list->addItems(compleList);

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
        TLOG(0, toTimeTotal, __HERE__) << "End" << std::endl;
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
    // The text might be already selected by tableAtCursor
	if (!hasSelectedText())
	{
		setSelection(start, end);
	}
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
	TLOG(0, toTimeStart, __HERE__) << "Start" << std::endl;
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    QString word = wordAtLineIndex(curline, curcol);
    TLOG(0, toTimeDelta, __HERE__) << "Word at index: " << word << std::endl;
	QStringList retval = toConnection::currentConnection(this).getCache().completeEntry("" , word);
    TLOG(0, toTimeDelta, __HERE__) << "Complete entry" << std::endl;
	QStringList retval2;
	{
		//QWidget * parent = parentWidget();
		//QWidget * parent2 = parent->parentWidget();
		//if (toWorksheetEditor *editor = dynamic_cast<toWorksheetEditor*>(parentWidget()))
		//	if(toWorksheet *worksheet = dynamic_cast<toWorksheet*>(editor))
		//		retval2 = toConnection::currentConnection(this).getCache().completeEntry(worksheet->currentSchema()+'.' ,word);
		retval2 = toConnection::currentConnection(this).getCache().completeEntry(toToolWidget::currentSchema(this), word);
	}

    if (retval2.size() <= 100) // Do not waste CPU on sorting huge completition list TODO: limit the amount of returned entries
        retval2.sort();
    TLOG(0, toTimeDelta, __HERE__) << "Sort" << std::endl;
    Q_FOREACH(QString t, retval)
    {
        //TLOG(0, toNoDecorator, __HERE__) << " Tab: " << t << std::endl;
    }
    TLOG(0, toTimeTotal, __HERE__) << "End" << std::endl;
    return retval2;
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
