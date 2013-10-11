
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

#include "editor/tomarkedtext.h"
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/tologger.h"

#include <QtGui/QClipboard>
#include <QtGui/QPrintDialog>
#include <QtXml/QDomDocument>
#include <QtGui/QShortcut>
#include <QtCore/QtDebug>
#include <QtGui/QMenu>

#include <Qsci/qsciprinter.h>
#include <Qsci/qscilexersql.h>

#include "icons/undo.xpm"
#include "icons/redo.xpm"
#include "icons/copy.xpm"
#include "icons/cut.xpm"
#include "icons/paste.xpm"


#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)

void QSciMessage::notify()
{
	Utils::toStatusMessage(text, true);
}

toMarkedText::toMarkedText(QWidget *parent, const char *name)
	: QsciScintilla(parent)
	, DragStart()
	, m_searchText()
	, m_flags()
	, m_searchIndicator(9) // see QsciScintilla docs
	, m_origContent()
{
    if (name)
        setObjectName(name);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    super::setFont(Utils::toStringToFont(toConfigurationSingle::Instance().textFont()));
    super::setMarginLineNumbers(0, true);
    super::setCallTipsStyle(CallTipsNone);

    // WARNING: it looks like this hack is mandatory for macosx. Oracle simly
    // doesn't understand mac's eols, so force to UNIX is a functional workaround
#ifdef Q_WS_MAC
    super::setEolMode(QsciScintilla::EolUnix);
#endif

    // it allows to use S&R in eg. error message output
    //QShortcut *searchReplaceShortcut = new QShortcut(QKeySequence::Find, parent, SLOT(searchReplace()), SLOT(searchReplace()));

    // search all occurrences
    // allow indicator painting *under* the text (but it makes editor slower a bit...)
    // It paints a colored box under the text for all occurrences of m_searchText.
    super::indicatorDefine(QsciScintilla::RoundBoxIndicator, m_searchIndicator);
    // TODO/FIXME: make it configurable
    super::setIndicatorForegroundColor(Qt::red, m_searchIndicator);
    super::setIndicatorOutlineColor(Qt::black, m_searchIndicator);
    super::setIndicatorDrawUnder(true, m_searchIndicator);
    // end of search all occurrences

    connect(this, SIGNAL(linesChanged()), this, SLOT(linesChanged()));

    // sets default tab width
    super::setTabWidth(toConfigurationSingle::Instance().tabStop());
    super::setIndentationsUseTabs(!toConfigurationSingle::Instance().useSpacesForIndent());

    super::setUtf8(true);
    setAcceptDrops(true); // QWidget::setAcceptDrops

    super::setMarginWidth(0, QString::fromAscii("00"));
}

toMarkedText::~toMarkedText()
{
//	toEditWidget::lostFocus();
}

QString toMarkedText::wordAtPosition(int position) const
{
    if (position < 0)
        return QString();

    long start_pos = SendScintilla(SCI_WORDSTARTPOSITION, position, true);
    long end_pos = SendScintilla(SCI_WORDENDPOSITION, position, true);

	int style1 = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
    int style2 = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, end_pos) & 0x1f;

    // QScintilla returned single word within quotes
    if( style1 == QsciLexerSQL::DoubleQuotedString || style1 == QsciLexerSQL::SingleQuotedString)
    {
    	start_pos = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, start_pos);
    	int style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
    	while( style == style1)
    	{
    		start_pos = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, start_pos);
        	style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
    	}
    	start_pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, start_pos);

    	end_pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, end_pos);
    	style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, end_pos) & 0x1f;
    	while( style == style1)
    	{
    		end_pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, end_pos);
        	style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, end_pos) & 0x1f;
    	}
    }

    int word_len = end_pos - start_pos;
    if (word_len <= 0)
        return QString();

    char *buf = new char[word_len + 1];
    SendScintilla(SCI_GETTEXTRANGE, start_pos, end_pos, buf);
    QString word = convertTextS2Q(buf);
    delete[] buf;

    return word;
}

// Return the word at the given coordinates.
QString toMarkedText::wordAtLineIndex(int line, int index) const
{
    return wordAtPosition(positionFromLineIndex(line, index));
}

// Convert a Scintilla string to a Qt Unicode string.
QString toMarkedText::convertTextS2Q(const char *s) const
{
    if (isUtf8())
        return QString::fromUtf8(s);

    return QString::fromLatin1(s);
}

void toMarkedText::linesChanged()
{
    int x = QString::number(lines()).length() + 1;
    setMarginWidth(0, QString().fill('0', x));

}

void toMarkedText::setWordWrap(bool enable)
{
    if (enable)
    {
        setWrapMode(QsciScintilla::WrapWord);
        setWrapVisualFlags(QsciScintilla::WrapFlagByBorder,
                           QsciScintilla::WrapFlagByBorder);
    }
    else
    {
        setWrapMode(QsciScintilla::WrapNone);
        setWrapVisualFlags(QsciScintilla::WrapFlagNone,
                           QsciScintilla::WrapFlagNone);
    }
}

void toMarkedText::setXMLWrap(bool wrap)
{
    if (wrap)
    {
        QDomDocument d;
        if (m_origContent.isEmpty())
            m_origContent = text();
        if (d.setContent(m_origContent))
            setText(d.toString(2));
        else
            m_origContent.clear();
    }
    else
    {
        if (!m_origContent.isEmpty())
        {
            setText(m_origContent);
            m_origContent.clear();
        }
    }
}

void toMarkedText::print(const QString  &fname)
{
    QsciPrinter printer;

    QPrintDialog dialog(&printer, this);
    dialog.setMinMax(1, 1000);
    dialog.setFromTo(1, 1000);

    if (!fname.isEmpty())
    {
        QFileInfo info(fname);
        dialog.setWindowTitle(tr("Print %1").arg(info.fileName()));
        printer.setOutputFileName(info.path() +
                                  QString("/") +
                                  info.baseName() +
                                  ".pdf");
    }
    else
        dialog.setWindowTitle(tr("Print Document"));

    // printRange() not handling this and not sure what to do about it
//     if(hasSelectedText())
//         dialog.addEnabledOption(QAbstractPrintDialog::PrintSelection);

    if (!dialog.exec())
        return;

    printer.setCreator(tr(TOAPPNAME));

    // they show up in the print
    setMarginLineNumbers(0, false);
    printer.printRange(this);
    setMarginLineNumbers(0, true);
}

void toMarkedText::newLine(void)
{
    // new line
    switch (eolMode())
    {
    case EolWindows:
        insert ("\n\r");
        break;

    case EolMac:
        insert ("\n");
        break;

    default:
        // Unix is default one
        insert ("\r");
        break;
    }

    if (toConfigurationSingle::Instance().autoIndent())
    {
        int curline, curcol;
        getCursorPosition (&curline, &curcol);
        if (curline > 0)
        {
            QString str = text(curline - 1);
            QString ind;
            for (int i = 0; i < str.length() && str.at(i).isSpace(); i++)
                ind += str.at(i);
            if (ind.length())
                insert(ind, false);
        }
    }
}

void toMarkedText::dropEvent(QDropEvent *e)
{
    if(e->source() == this || e->source() == viewport())
    {
        QPoint point = e->pos() - DragStart;
        // forces a reasonable drag distance
        if (point.manhattanLength() < QApplication::startDragDistance())
        {
            e->ignore();
            return;
        }

        e->acceptProposedAction();
        beginUndoAction();

        QString selection = selectedText();
        removeSelectedText();

        long position = SendScintilla(SCI_POSITIONFROMPOINT,
                                      e->pos().x(),
                                      e->pos().y());
        SendScintilla(SCI_SETCURRENTPOS, position);

        int line, index;
        getCursorPosition(&line, &index);
        insertAt(selection, line, index);
        setSelection(line, index, line, index + selection.length());

        endUndoAction();
    }
    else
        QsciScintilla::dropEvent(e);

    setFocus();
}

void toMarkedText::mousePressEvent(QMouseEvent *e)
{
    DragStart = QPoint();
    if(e->button() == Qt::LeftButton && geometry().contains(e->pos()))
    {
        if(!toConfigurationSingle::Instance().editDragDrop())
        {
            // would normally be a drag request. clear selection.
            if(QsciScintilla::hasSelectedText())
                QsciScintilla::selectAll(false);
        }
        else if(QsciScintilla::hasSelectedText())
            DragStart = e->pos();
    }

    QsciScintilla::mousePressEvent(e);
}

void toMarkedText::keyPressEvent(QKeyEvent *e)
{
//    if (Search)
//    {
//        bool ok = false;
//        if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Backspace)
//        {
//            int len = SearchString.length();
//            if (len > 0)
//                SearchString.truncate(len - 1);
//            ok = true;
//        }
//        else if (e->key() != Qt::Key_Escape)
//        {
//            QString t = e->text();
//            if (t.length())
//            {
//                SearchString += t;
//                ok = true;
//            }
//            else if (e->key() == Qt::Key_Shift ||
//                     e->key() == Qt::Key_Control ||
//                     e->key() == Qt::Key_Meta ||
//                     e->key() == Qt::Key_Alt)
//            {
//                ok = true;
//            }
//        }
//
//        if (ok)
//        {
//            incrementalSearch(m_searchDirection, false);
//            e->accept();
//            return ;
//        }
//        else
//        {
//            Search = false;
//            LastSearch = SearchString;
//            Utils::toStatusMessage(QString::null);
//        }
//    }
    QsciScintilla::keyPressEvent(e);
}

#ifdef TORA3_GRAPH
void toMarkedText::exportData(std::map<QString, QString> &data, const QString &prefix)
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

void toMarkedText::importData(std::map<QString, QString> &data, const QString &prefix)
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

/*
static int FindIndex(const QString &str, int line, int col)
{
    int pos = 0;
    for (int i = 0;i < line;i++)
    {
        pos = str.indexOf('\n', pos);
        if (pos < 0)
            return pos;
        pos++;
    }
    return pos + col;
}*/

void toMarkedText::findPosition(int index, int &line, int &col)
{
    int pos = 0;
    for (int i = 0; i < lines(); i++)
    {
        QString str = text(i);
        if (str.length() + pos >= index)
        {
            line = i;
            col = index - pos;
            return ;
        }
        pos += str.length();
    }
    col = -1;
    line = -1;
    return ;
}


bool toMarkedText::findText(const QString &searchText, const QString &replaceText, Search::SearchFlags flags)
{
    int line, index;
    bool found = false;

    getCursorPosition(&line, &index);

  	found = findFirst(searchText,
                      (bool)(flags & Search::Regexp),
                      (bool)(flags & Search::CaseSensitive),
                      (bool)(flags & Search::WholeWords),
                      true, //bool   wrap,
                      (bool)(flags & Search::Forward),
                      line,
                      index,
                      true //bool   show = true
                     );

    if (m_searchText != searchText || m_flags != flags)
    {
    	m_searchText = searchText;
    	m_flags = flags;

        // find and highlight all occurrences of m_searchText
        // from the beginning to the end
        int from = 0;
        int to = text().length();

        // clear previously used marked text
        clearIndicatorRange(0, 0, lines(), lineLength(lines()-1), m_searchIndicator);

        if (!found)
            return found;

        // set searching flags
        int searchFlags = 0;
        //QsciScintilla::SCFIND_WORDSTART;
        if (flags & Search::Regexp)
            searchFlags |= QsciScintilla::SCFIND_REGEXP;
        if (flags & Search::CaseSensitive)
            searchFlags |= QsciScintilla::SCFIND_MATCHCASE;
        if (flags & Search::WholeWords)
            searchFlags |= QsciScintilla::SCFIND_WHOLEWORD;

        SendScintilla(QsciScintilla::SCI_SETINDICATORCURRENT, m_searchIndicator);

        while (from < to)
        {
            // set searching range
            SendScintilla(QsciScintilla::SCI_SETTARGETSTART, from);
            SendScintilla(QsciScintilla::SCI_SETTARGETEND, to);

            SendScintilla(QsciScintilla::SCI_SETSEARCHFLAGS, searchFlags);
            from = SendScintilla(QsciScintilla::SCI_SEARCHINTARGET,
                                 m_searchText.length(), m_searchText.toUtf8().data());

            // SCI_SEARCHINTARGET returns -1 when it doesn't find anything
            if (from == -1)
            {
                break;
            }

            int end = SendScintilla(QsciScintilla::SCI_GETTARGETEND);
            // mark current occurrence of searchText
            SendScintilla(QsciScintilla::SCI_INDICATORFILLRANGE, from, end - from);
            from = end;
        } // while
    }

    if (!isReadOnly() && found && searchText != replaceText)
    {
    	if (m_flags & Search::Replace)
    	{
    		this->replace(replaceText);
    	}
    	else if (m_flags & Search::ReplaceAll)
    	{
    		QsciScintilla::beginUndoAction();
    		while (findNext())
    		    this->replace(replaceText);
    		QsciScintilla::endUndoAction();
    	}
    }

    return found; // TODO/FIXME: what to do with a retval?
}

#if 0
// TODO: this part is waiting for QScintilla backend feature (yet unimplemented).
void toMarkedText::setSelectionType(int aType)
{
    TLOG(2, toDecorator, __HERE__) << "setSelectionType" << SendScintilla(SCI_GETSELECTIONMODE) << aType;
    TLOG(2, toDecorator, __HERE__) << SendScintilla(SCI_SETSELECTIONMODE, aType);
    TLOG(2, toDecorator, __HERE__) << "setSelectionType" << SendScintilla(SCI_GETSELECTIONMODE) << aType;
}
#endif

void toMarkedText::focusInEvent (QFocusEvent *e)
{
    TLOG(9, toDecorator, __HERE__) << this << std::endl;
    QsciScintilla::focusInEvent(e);
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    toGlobalEventSingle::Instance().setCoordinates(curline + 1, curcol + 1);

    emit gotFocus();
}

void toMarkedText::focusOutEvent (QFocusEvent *e)
{
    qDebug() << ">>> toMarkedText::focusOutEvent" << this;
    QsciScintilla::focusOutEvent(e);
    emit lostFocus();
}

void toMarkedText::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<toMarkedText> that = this;
    QPointer<QMenu> popup = createPopupMenu( e->pos() );
    if (!popup)
        return;

    e->accept();

    // NOTE: this emit exist for compatibility with older
    //       version of TOra. It will have to be removed
    //       once the way around it will be figured out
    emit displayMenu(popup);

    popup->exec(e->globalPos());
    delete popup;
}

/**
 * This function is called to create a right mouse button popup menu
 * at the specified position. If you want to create a custom popup menu,
 * reimplement this function and return the created popup menu. Ownership
 * of the popup menu is transferred to the caller.
 */
QMenu *toMarkedText::createPopupMenu(const QPoint& pos)
{
    Q_UNUSED(pos);

    const bool isEmptyDocument = (lines() == 0);

    // create menu
    QMenu   *popup = new QMenu(this);
    QAction *action;

    if (!isReadOnly())
    {
        action = popup->addAction(QIcon(undo_xpm), tr("&Undo"), this, SLOT(undo()));
        action->setShortcut(QKeySequence::Undo);
        action->setEnabled(isUndoAvailable());

        action = popup->addAction(QIcon(redo_xpm), tr("&Redo"), this, SLOT(redo()));
        action->setShortcut(QKeySequence::Redo);
        action->setEnabled(isRedoAvailable());

        popup->addSeparator();

        action = popup->addAction(QIcon(cut_xpm), tr("Cu&t"), this, SLOT(cut()));
        action->setShortcut(QKeySequence::Cut);
        action->setToolTip(tr("Cut to clipboard"));
        action->setEnabled(hasSelectedText());

        action = popup->addAction(QIcon(copy_xpm),
                                  tr("&Copy"),
                                  this,
                                  SLOT(copy()));
        action->setShortcut(QKeySequence::Copy);
        action->setToolTip(tr("Copy to clipboard"));
        action->setEnabled(hasSelectedText());

        action = popup->addAction(QIcon(paste_xpm),
                                  tr("&Paste"),
                                  this,
                                  SLOT(paste()));
        action->setShortcut(QKeySequence::Paste);
        action->setToolTip(tr("Paste from clipboard"));
        action->setEnabled(!QApplication::clipboard()->text(
                               QClipboard::Clipboard).isEmpty());

        action = popup->addAction(tr("Clear"),
                                  parent(),
                                  SLOT(clear()));
        action->setToolTip(tr("Clear editor"));
        action->setEnabled(!isEmptyDocument);

        popup->addSeparator();
    }

    action = popup->addAction(tr("Select &All"),
                              this,
                              SLOT(selectAll()));
    action->setShortcut(QKeySequence::SelectAll);
    action->setEnabled(!isEmptyDocument);

    return popup;
}

void toMarkedText::insert(const QString &str, bool select)
{
    int lineFrom;
    int indexFrom;

    // Make this work as one undo so user doesn't see his code disappear first.
    QsciScintilla::beginUndoAction();

    QsciScintilla::removeSelectedText();

    if (select)
        getCursorPosition(&lineFrom, &indexFrom);

    // insert() doesn't work as advertised.
    // docs say: "The new current position if after the inserted text."

    // I thought it would mean that the cursor would be at the end of
    // the inserted text. Now I'm not really sure what the heck that
    // means.

    QsciScintilla::insert(str);

    // get new position and select if requested
    if (select)
        setSelection(lineFrom, indexFrom, lineFrom, indexFrom + str.length());

    QsciScintilla::endUndoAction();
}
