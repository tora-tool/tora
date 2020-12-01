
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

#include "widgets/toscintilla.h"
#include "core/toeditorconfiguration.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/tologger.h"
#include "core/tomainwindow.h"
#include "core/toconf.h"
#include "core/tocontextmenu.h"
#include "core/toeditmenu.h"
#include "core/tofilemenu.h"
#include "widgets/tosearch.h"

#include "ts_log/ts_log_utils.h"

#include <QApplication>
#include <QtGui/QClipboard>
#include <QtXml/QDomDocument>
#include <QShortcut>
#include <QtCore/QtDebug>
#include <QMenu>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>
#include <QToolTip>

#include <Qsci/qscilexersql.h>
#include "core/tostyle.h"

void QSciMessage::notify()
{
    Utils::toStatusMessage(text, true);
}

toScintilla::toScintilla(QWidget *parent, const char *name)
    : QsciScintilla(parent)
    , DragStart()
    , m_searchText()
    , m_flags()
    , m_searchIndicator(9) // see QsciScintilla docs
    , m_showTooTips(false)
{
    using namespace ToConfiguration;
    if (name)
        setObjectName(name);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QPalette const& palete = QApplication::palette();

    super::setFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfTextFont).toString()));
    super::setMarginLineNumbers(0, true);
    super::setCallTipsStyle(CallTipsNone);
    super::setMarginsBackgroundColor(palete.color(QPalette::AlternateBase));

    // WARNING: it looks like this hack is mandatory for macosx. Oracle simply
    // doesn't understand mac's eols, so force to UNIX is a functional workaround
#ifdef Q_OS_MAC
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

    connect(this, SIGNAL(linesChanged()), this, SLOT(slotLinesChanged()));
    connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(setCoordinates(int, int)));

    // sets default tab width
    super::setTabWidth(toConfigurationNewSingle::Instance().option(Editor::TabStopInt).toInt());
    super::setIndentationsUseTabs(!toConfigurationNewSingle::Instance().option(Editor::UseSpacesForIndentBool).toBool());

    super::setUtf8(true);
    setAcceptDrops(true); // QWidget::setAcceptDrops

    super::setMarginWidth(0, QString::fromLatin1("00"));
}

toScintilla::~toScintilla()
{
//	toEditWidget::lostFocus();
}

long toScintilla::currentPosition() const
{
    long pos = SendScintilla(SCI_GETCURRENTPOS);
    return pos;
}

QString toScintilla::wordAtPosition(int position, bool onlyWordCharacters /* = true */) const
{
    if (position < 0)
        return QString();

    long start_pos = SendScintilla(SCI_WORDSTARTPOSITION, position, onlyWordCharacters);
    long end_pos = SendScintilla(SCI_WORDENDPOSITION, position, onlyWordCharacters);

    int style1 = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
    int style2 = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, end_pos) & 0x1f;

    // QScintilla returned single word within quotes
    if ( style1 == QsciLexerSQL::DoubleQuotedString || style1 == QsciLexerSQL::SingleQuotedString)
    {
        start_pos = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, start_pos);
        int style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
        while ( style == style1)
        {
            start_pos = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, start_pos);
            style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;
        }
        start_pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, start_pos);

        end_pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, end_pos);
        style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, end_pos) & 0x1f;
        while ( style == style1)
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
QString toScintilla::wordAtLineIndex(int line, int index) const
{
    return wordAtPosition(positionFromLineIndex(line, index));
}

// Convert a Scintilla string to a Qt Unicode string.
QString toScintilla::convertTextS2Q(const char *s) const
{
    if (isUtf8())
        return QString::fromUtf8(s);

    return QString::fromLatin1(s);
}

void toScintilla::slotLinesChanged()
{
    int x = QString::number(lines()).length() + 1;
    setMarginWidth(0, QString().fill('0', x));

}
void toScintilla::setCoordinates(int line, int column)
{
    toGlobalEventSingle::Instance().setCoordinates(line + 1, column + 1);
}

void toScintilla::setWordWrap(bool enable)
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

// implicit tooltip implementation - show word under cursor
bool toScintilla::showToolTip(toScintilla::ToolTipData const& t)
{
    int word_len = t.wordEnd - t.wordStart;
    if (word_len <= 0)
        return false;
    char *buf = new char[word_len + 1];
    SendScintilla(SCI_GETTEXTRANGE, t.wordStart, t.wordEnd, buf);
    QString word = bytesAsText(buf);
    delete[] buf;

    QToolTip::showText(t.globalPos, word, viewport(), t.rect);
    return true;
}

bool toScintilla::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip && m_showTooTips)
    {
        ToolTipData t;

        //https://riverbankcomputing.com/pipermail/qscintilla/2008-November/000381.html
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        t.globalPos = helpEvent->globalPos();
        t.position = helpEvent->pos();

        t.textPosition = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, t.position.x(), t.position.y());
        if (t.textPosition == -1)
            return true;

        t.wordStart = SendScintilla(SCI_WORDSTARTPOSITION, t.textPosition, true);
        t.wordEnd   = SendScintilla(SCI_WORDENDPOSITION, t.textPosition, true);
        if (t.wordStart == t.wordEnd)
            return true;

        t.x_start = SendScintilla(SCI_POINTXFROMPOSITION, 0, t.wordStart);
        t.y_start = SendScintilla(SCI_POINTYFROMPOSITION, 0, t.wordStart);
        t.x_end   = SendScintilla(SCI_POINTXFROMPOSITION, 0, t.wordEnd);
        t.line    = SendScintilla(SCI_LINEFROMPOSITION, t.wordStart);
        t.height  = SendScintilla(SCI_TEXTHEIGHT, t.line);
        t.rect    = QRect(t.x_start, t.y_start, t.x_end - t.x_start, t.height);

        if (showToolTip(t))
        {
            return true;
        } else {
            QToolTip::hideText();
            event->ignore();
        }
    }
    return QsciScintilla::event(event);
}

void toScintilla::copy()
{
    QsciScintilla::copy();
    QMimeData *md = new QMimeData();
    QString txt = QApplication::clipboard()->mimeData()->text();
    QString mime;
    md->setText(txt);

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    mime = getSelectionAsHTML();
#elif defined(Q_OS_WIN)
    mime = getSelectionAsRTF();
#endif

    if (mime.isEmpty())
        return;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    // md->setData(QLatin1String("text/html"), html.toUtf8()); MAC?
    md->setHtml(mime);
    TLOG(0, toDecorator, __HERE__) << "html:" << mime << std::endl;
#elif defined(Q_OS_WIN)
    md->setData(QLatin1String("text/rtf"), mime.toUtf8());
    md->setData(QLatin1String("Rich Text Format"), mime.toUtf8());
    TLOG(0, toDecorator, __HERE__) << "rtf:" << mime << std::endl;
#endif

    QApplication::clipboard()->setMimeData(md, QClipboard::Clipboard);
}

void toScintilla::paste()
{
#ifdef QT_DEBUG
    QMimeData const *md = QApplication::clipboard()->mimeData();
    TLOG(0, toDecorator, __HERE__) << md->formats().join("'") << std::endl;
    Q_FOREACH(QString format, md->formats())
    {
        QByteArray ba = md->data(format);
        TLOG(0, toNoDecorator, __HERE__) << format << std::endl
                                         << ba.data() << std::endl;
    }
#endif
    QsciScintilla::paste();
}

void toScintilla::newLine(void)
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

    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::AutoIndentBool).toBool())
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
                insertAndSelect(ind, false);
        }
    }
}

void toScintilla::dropEvent(QDropEvent *e)
{
    if (e->source() == this || e->source() == viewport())
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

void toScintilla::mousePressEvent(QMouseEvent *e)
{
    DragStart = QPoint();
    if (e->button() == Qt::LeftButton && geometry().contains(e->pos()))
    {
        if (!toConfigurationNewSingle::Instance().option(ToConfiguration::Main::EditDragDrop).toBool())
        {
            // would normally be a drag request. clear selection.
            if (QsciScintilla::hasSelectedText())
                QsciScintilla::selectAll(false);
        }
        else if (QsciScintilla::hasSelectedText())
            DragStart = e->pos();
    }

    QsciScintilla::mousePressEvent(e);
}

void toScintilla::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Copy))
    {
        // "Override" scintilla's default copy behavior
        copy();
        e->accept();
        return;
    } else if (Utils::toCheckKeyEvent(e, toEditMenuSingle::Instance().searchReplaceAct->shortcut())) {
        toSearchReplaceDockletSingle::Instance().activate();
        e->accept();
        return;
//    } else if (Utils::toCheckKeyEvent(e, toFileMenuSingle::Instance().saveAct->shortcut()) && flagSet().Save) {
//        editSave(false);
//        e->accept();
//        return;
//    } else if (Utils::toCheckKeyEvent(e, toFileMenuSingle::Instance().saveAsAct->shortcut()) && flagSet().Save) {
//        editSave(true);
//        e->accept();
//        return;
    }
    super::keyPressEvent(e);
}

void toScintilla::findPosition(int index, int &line, int &col)
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

void toScintilla::gotoPosition(long pos)
{
    SendScintilla(QsciScintilla::SCI_GOTOPOS, pos);
}

void toScintilla::gotoLine(int line)
{
    SendScintilla(QsciScintilla::SCI_GOTOLINE, line);
}

long toScintilla::positionAfter(long pos, int offset)
{
    // Allow for multi-byte characters.
    for (int i = 0; i < offset; i++)
        pos = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, pos);
    return pos;
}

void toScintilla::setSelection(long posFrom, long posTo)
{
    SendScintilla(SCI_SETSEL, posFrom, posTo);
}

bool toScintilla::findText(const QString &searchText, const QString &replaceText, Search::SearchFlags flags)
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

void toScintilla::findStop()
{
    clearIndicatorRange(0, 0, lines(), lineLength(lines()-1), m_searchIndicator);
}

#if 0
// TODO: this part is waiting for QScintilla backend feature (yet unimplemented).
void toScintilla::setSelectionType(int aType)
{
    TLOG(2, toDecorator, __HERE__) << "setSelectionType" << SendScintilla(SCI_GETSELECTIONMODE) << aType;
    TLOG(2, toDecorator, __HERE__) << SendScintilla(SCI_SETSELECTIONMODE, aType);
    TLOG(2, toDecorator, __HERE__) << "setSelectionType" << SendScintilla(SCI_GETSELECTIONMODE) << aType;
}
#endif

void toScintilla::focusInEvent (QFocusEvent *e)
{
    TLOG(9, toDecorator, __HERE__) << this << std::endl;
    super::focusInEvent(e);
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    toGlobalEventSingle::Instance().setCoordinates(curline + 1, curcol + 1);
    toEditWidget::gotFocus();
}

void toScintilla::focusOutEvent (QFocusEvent *e)
{
    TLOG(9, toDecorator, __HERE__) << this << std::endl;
    super::focusOutEvent(e);
    toEditWidget::lostFocus();
}

void toScintilla::contextMenuEvent(QContextMenuEvent *e)
{
    // create menu
    QMenu *popup = new QMenu(this);

    // Handle parent widget's context menu fields
    toContextMenuHandler::traverse(this, popup);

    populateContextMenu(popup);

    // Display and "run" the menu
    e->accept();
    popup->exec(e->globalPos());
    delete popup;
}

void toScintilla::populateContextMenu(QMenu *popup)
{
    // Handle my own context menu fields
    toEditMenu &editMenu = toEditMenuSingle::Instance();
    editMenu.menuAboutToShow();
    const bool isEmptyDocument = (lines() == 0);

    if (!isReadOnly())
    {
        popup->addAction(editMenu.undoAct);
        popup->addAction(editMenu.redoAct);

        popup->addSeparator();

        popup->addAction(editMenu.cutAct);
    }

    popup->addAction(editMenu.copyAct);

    if (!isReadOnly())
    {
        popup->addAction(editMenu.pasteAct);

        popup->addSeparator();
    }

    popup->addAction(editMenu.selectAllAct);
}

bool toScintilla::editOpen(const QString &file) { throw __QHERE__; };

bool toScintilla::editSave(bool askfile)
{
    QString fn = Utils::toSaveFilename(QString(), QString(), this);
    if (!fn.isEmpty() && Utils::toWriteFile(fn, text()))
    {
        setModified(false);
        return true;
    }
    return false;
};

void toScintilla::editUndo(void)
{
    undo();
};

void toScintilla::editRedo(void)
{
    redo();
};

void toScintilla::editCut(void)
{
    cut();
};

void toScintilla::editCopy(void)
{
    copy();
}

void toScintilla::editPaste(void)
{
    paste();
}

void toScintilla::editSelectAll(void)
{
    selectAll(true);
}

void toScintilla::editReadAll(void) { throw __QHERE__; };

QString toScintilla::editText()
{
    return text();
}

toEditWidget::FlagSetStruct toScintilla::flagSet()
{
    if (isReadOnly())
    {
        FlagSet.Save = true;
        FlagSet.Copy = hasSelectedText();
        FlagSet.Paste = false;
        FlagSet.Search = true;
        FlagSet.SelectAll = true;
    }
    else
    {
        FlagSet.Save = true;
        FlagSet.Undo = isUndoAvailable();
        FlagSet.Redo = isRedoAvailable();
        FlagSet.Cut  = hasSelectedText();
        FlagSet.Copy = hasSelectedText();
        FlagSet.Paste = true;
        FlagSet.Search = true;
        FlagSet.SelectAll = true;
    }
    return FlagSet;
}

bool toScintilla::handleSearching(QString const& search, QString const& replace, Search::SearchFlags flags)
{
    return findText(search, replace, flags);
}

QString toScintilla::getSelectionAsHTML()
{
    if (lexer() == NULL)
        return QString();

    static const QString SPAN_CLASS = QString::fromLatin1("<span class=\"S%1\">");

    clearIndicatorRange(0, 0, lines(), lineLength(lines()-1), m_searchIndicator);
    recolor();
    int tabSize = 4;
    int wysiwyg = 1;
    int tabs = 0;
    int onlyStylesUsed = 1;
    int titleFullPath = 0;

    int startPos = SendScintilla(SCI_GETSELECTIONSTART);
    int endPos = SendScintilla(SCI_GETSELECTIONEND);

    bool styleIsUsed[STYLE_MAX + 1];
    if (onlyStylesUsed)
    {
        int i;
        for (i = 0; i <= STYLE_MAX; i++)
        {
            styleIsUsed[i] = false;
        }
        // check the used styles
        for (i = startPos; i < endPos; i++)
        {
            styleIsUsed[getStyleAt(i) & 0x7F] = true;
        }
    }
    else
    {
        for (int i = 0; i <= STYLE_MAX; i++)
        {
            styleIsUsed[i] = true;
        }
    }
    styleIsUsed[STYLE_DEFAULT] = true;

    QString retval;
    retval += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
    retval += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
    retval += "<head>\n";
    retval += "<title></title>\n"; //static_cast<const char *>(filePath.Name().AsUTF8().c_str()));

    // Probably not used by robots, but making a little advertisement for those looking
    // at the source code doesn't hurt...
    retval += "<meta name=\"Generator\" content=\"SciTE - www.Scintilla.org\" />\n";
    retval += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";
    //retval += "<style type=\"text/css\">\n";

    QFont sdmono;
#if defined(Q_OS_WIN)
    sdmono = QFont("Verdana",9);
#elif defined(Q_OS_MAC)
    sdmono = QFont("Verdana", 12);
#else
    sdmono = QFont("Bitstream Vera Sans",8);
#endif


    QMap<int, QString> styles;

    for (int istyle = 0; istyle <= STYLE_MAX; istyle++)
    {
        if ((istyle > STYLE_DEFAULT) && (istyle <= STYLE_LASTPREDEFINED))
            continue;
        if (styleIsUsed[istyle])
        {
            QFont font = lexer()->font(istyle);
            int size = font.pointSize();
            bool italics = font.italic();
            int weight = font.weight();
            bool bold = font.bold();
            QString family = font.family();
            QColor fore = lexer()->color(istyle);
            QColor back = lexer()->paper(istyle);
            QString styledSpan = "<span style=\"";
            //			if (CurrentBuffer()->useMonoFont && sd.Font.length() && sdmono.Font.length()) {
            //				sd.Font = sdmono.Font;
            //				sd.Size = sdmono.Size;
            //				sd.italics = sdmono.italics;
            //				sd.weight = sdmono.weight;
            //			}

            if (istyle == STYLE_DEFAULT)
            {
//				retval += "span {\n";
            }
            else
            {
//				retval += QString(".S%1 {\n").arg(istyle);
            }

            if (italics)
            {
//				retval += "\tfont-style: italic;\n";
                styledSpan += "font-style: italic; ";
            }

            if (bold)
            {
//				retval += "\tfont-weight: bold;\n";
                styledSpan += "font-weight: bold; ";
            }
            if (wysiwyg && !family.isEmpty())
            {
//				retval += QString("\tfont-family: '%1';\n").arg(family);
                styledSpan += QString("font-family: '%1'; ").arg(family);
            }
            if (fore.isValid())
            {
//				retval += QString("\tcolor: %1;\n").arg(fore.name());
                styledSpan += QString("color: rgb(%1,%2,%3); ").arg(fore.red()).arg(fore.green()).arg(fore.blue());
            }
            else if (istyle == STYLE_DEFAULT)
            {
//				retval += "\tcolor: #000000;\n";
                styledSpan += QString("color: rgb(0,0,0); ");
            }
            if (back.isValid() &&  istyle != STYLE_DEFAULT)
            {
//				retval += QString("\tbackground: %1;\n").arg(back.name());
//				retval += QString("\ttext-decoration: inherit;\n");
                styledSpan += QString("background: rgb(%1,%2,%3); ").arg(back.red()).arg(back.green()).arg(back.blue());
            }
            if (wysiwyg && size)
            {
//				retval += QString("\tfont-size: %1pt;\n").arg(size);
                styledSpan += QString("font-size: %1pt").arg(size);
            }
//			retval += "}\n";
            styledSpan += "\">";
            styles.insert(istyle, styledSpan);
        }
    }

    retval += "</style>\n";
    retval += "</head>\n";

    //	if (!bgColour.isEmpty())
    //		retval += QString("<body bgcolor=\"%1\">\n").arg(bgColour);
    //	else
    retval += "<body>\n";

    int level = (getLevelAt(0) & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;
    int newLevel;
    int styleCurrent = getStyleAt(0);
    bool inStyleSpan = false;
    bool inFoldSpan = false;
    // Global span for default attributes
    if (wysiwyg)
    {
        retval += "<span>";
    }
    else
    {
        retval += "<pre>";
    }

    if (styleIsUsed[styleCurrent])
    {
        if (styles.contains(styleCurrent))
            retval += styles.value(styleCurrent);
        else
            retval += SPAN_CLASS.arg(styleCurrent);
        inStyleSpan = true;
    }
    // Else, this style has no definition (beside default one):
    // no span for it, except the global one

    int column = 0;
    for (int i = startPos; i < endPos; i++)
    {
        char ch = getByteAt(i);
        int style = getStyleAt(i);

        if (style != styleCurrent)
        {
            if (inStyleSpan)
            {
                retval += "</span>";
                inStyleSpan = false;
            }
            if (ch != '\r' && ch != '\n')  	// No need of a span for the EOL
            {
                if (styleIsUsed[style])
                {
                    if (styles.contains(style))
                        retval += styles.value(style);
                    else
                        retval += SPAN_CLASS.arg(style);
                    inStyleSpan = true;
                }
                styleCurrent = style;
            }
        }
        switch (ch)
        {
            case ' ':
                {
                    if (wysiwyg)
                    {
                        char prevCh = '\0';
                        if (column == 0)  	// At start of line, must put a &nbsp; because regular space will be collapsed
                        {
                            prevCh = ' ';
                        }
                        while (i < endPos && getByteAt(i) == ' ')
                        {
                            if (prevCh != ' ')
                            {
                                retval += ' ';
                            }
                            else
                            {
                                retval += "&nbsp;";
                            }
                            prevCh = getByteAt(i);
                            i++;
                            column++;
                        }
                        i--; // the last incrementation will be done by the for loop
                    }
                    else
                    {
                        retval += ' ';
                        column++;
                    }
                }
                break;
            case '\t':
                {
                    int ts = tabSize - (column % tabSize);
                    if (wysiwyg)
                    {
                        for (int itab = 0; itab < ts; itab++)
                        {
                            if (itab % 2)
                            {
                                retval += ' ';
                            }
                            else
                            {
                                retval += "&nbsp;";
                            }
                        }
                        column += ts;
                    }
                    else
                    {
                        if (tabs)
                        {
                            retval += ch;
                            column++;
                        }
                        else
                        {
                            for (int itab = 0; itab < ts; itab++)
                            {
                                retval += ' ';
                            }
                            column += ts;
                        }
                    }
                }
                break;
            case '\r':
            case '\n':
                {
                    if (inStyleSpan)
                    {
                        retval += "</span>";
                        inStyleSpan = false;
                    }
                    if (inFoldSpan)
                    {
                        retval += "</span>";
                        inFoldSpan = false;
                    }
                    if (ch == '\r' && getByteAt(i + 1) == '\n')
                    {
                        i++;	// CR+LF line ending, skip the "extra" EOL char
                    }
                    column = 0;
                    if (wysiwyg)
                    {
                        retval += "<br />";
                    }

                    styleCurrent = getStyleAt(i + 1);
                    retval += '\n';

                    if (styleIsUsed[styleCurrent] && getByteAt(i + 1) != '\r' && getByteAt(i + 1) != '\n')
                    {
                        // We know it's the correct next style,
                        // but no (empty) span for an empty line
                        if (styles.contains(styleCurrent))
                            retval += styles.value(styleCurrent);
                        else
                            retval += SPAN_CLASS.arg(styleCurrent);
                        inStyleSpan = true;
                    }
                }
                break;
            case '<':
                retval += "&lt;";
                column++;
                break;
            case '>':
                retval += "&gt;";
                column++;
                break;
            case '&':
                retval += "&amp;";
                column++;
                break;
            default:
                retval += ch;
                column++;
        }
    }

    if (inStyleSpan)
    {
        retval += "</span>";
    }

    if (!wysiwyg)
    {
        retval += "</pre>";
    }
    else
    {
        retval += "</span>";
    }

    retval += "\n</body>\n</html>\n";
    return retval;
}

// extracts control words that are different between two styles
static QString  GetRTFStyleChange(QString const& last, QString const& current)
{
    // \f0\fs20\cf0\highlight0\b0\i0
    QString delta;
    QStringList lastL = last.split('\\', QString::SkipEmptyParts);
    QStringList currentL = current.split('\\', QString::SkipEmptyParts);

    // font face, size, color, background, bold, italic
    for (int i = 0; i < 6; i++)
    {
        if ( lastL.at(i) != currentL.at(i))   // changed
        {
            delta += '\\';
            delta += currentL.at(i);
        }
    }
    if (!delta.isEmpty())
        delta += " ";
    return delta;
}

QString toScintilla::getSelectionAsRTF()
{
    if (lexer() == NULL)
        return QString();

    static const QString RTF_HEADEROPEN = "{\\rtf1\\ansi\\deff0\\deftab720";
    static const QString RTF_FONTDEFOPEN = "{\\fonttbl";
    static const QString RTF_FONTDEF = "{\\f%1\\fnil\\fcharset%2 %3;}";
    static const QString RTF_FONTDEFCLOSE = "}";
    static const QString RTF_COLORDEFOPEN = "{\\colortbl";
    static const QString RTF_COLORDEF = "\\red%1\\green%2\\blue%3;";
    static const QString RTF_COLORDEFCLOSE = "}";
    static const QString RTF_HEADERCLOSE = "\n";
    static const QString RTF_BODYOPEN = "";
    static const QString RTF_BODYCLOSE = "}";

    static const QString RTF_SETFONTFACE = "\\f%1";
    static const QString RTF_SETFONTSIZE = "\\fs%1";
    static const QString RTF_SETCOLOR = "\\cf%1";
    static const QString RTF_SETBACKGROUND = "\\highlight%1";
    static const QString RTF_BOLD_ON = "\\b";
    static const QString RTF_BOLD_OFF = "\\b0";
    static const QString RTF_ITALIC_ON = "\\i";
    static const QString RTF_ITALIC_OFF = "\\i0";
    static const QString RTF_UNDERLINE_ON = "\\ul";
    static const QString RTF_UNDERLINE_OFF = "\\ulnone";
    static const QString RTF_STRIKE_ON = "\\i";
    static const QString RTF_STRIKE_OFF = "\\strike0";

    static const QString RTF_EOLN = "\\par\n";
    static const QString RTF_TAB = "\\tab ";

    static const int MAX_STYLEDEF = 128;
    static const int MAX_FONTDEF  = 64;
    static const int MAX_COLORDEF = 8;
    static const QString RTF_FONTFACE = "Courier New";
    static const QString RTF_COLOR = "#000000";

    QString fp;
    int startPos = SendScintilla(SCI_GETSELECTIONSTART);
    int endPos = SendScintilla(SCI_GETSELECTIONEND);

    clearIndicatorRange(0, 0, lines(), lineLength(lines()-1), m_searchIndicator);
    recolor();

    // Read the default settings
    toStyle defaultStyle("", lexer()->color(STYLE_DEFAULT), lexer()->paper(STYLE_DEFAULT), lexer()->font(STYLE_DEFAULT));

    bool tabs = true;
    int tabSize = 4;
    int wysiwyg = 1;

    QFont sdmono;
#if defined(Q_OS_WIN)
    sdmono = QFont("Verdana",9);
#elif defined(Q_OS_MAC)
    sdmono = QFont("Verdana", 12);
#else
    sdmono = QFont("Bitstream Vera Sans",8);
#endif

    QString fontFace = sdmono.family();
    if (fontFace.length())
    {
        defaultStyle.Font = fontFace;
    }
    else if (defaultStyle.Font.family().isEmpty())
    {
        defaultStyle.Font = RTF_FONTFACE;
    }
    int fontSize = sdmono.pointSize();
    if (fontSize > 0)
    {
        defaultStyle.Size = fontSize << 1;
    }
    else if (defaultStyle.Size == 0)
    {
        defaultStyle.Size = 10 << 1;
    }
    else
    {
        defaultStyle.Size <<= 1;
    }

    QString styles[STYLE_DEFAULT + 1];
    QFont fonts[STYLE_DEFAULT + 1];
    QColor colors[STYLE_DEFAULT + 1];
    QString lastStyle, deltaStyle;
    int fontCount = 1, colorCount = 2, i;

    fp += RTF_HEADEROPEN;
    {
        // fonts definitions
        fp += RTF_FONTDEFOPEN;

        fonts[0] = defaultStyle.Font;

        unsigned int characterset = 1; //props.GetInt("character.set", SC_CHARSET_DEFAULT);
        fp += RTF_FONTDEF.arg(0).arg(characterset).arg(defaultStyle.Font.family());

        colors[0] = defaultStyle.FGColor;
        colors[1] = defaultStyle.BGColor;

        for (int istyle = 0; istyle < STYLE_DEFAULT; istyle++)
        {
            toStyle sd("", lexer()->color(istyle), lexer()->paper(istyle), lexer()->font(istyle));

            for (i = 0; i < fontCount; i++)
                if (sd.Font == fonts[i])
                    break;
            if (i >= fontCount)
            {
                fonts[fontCount++] = sd.Font;
                fp += RTF_FONTDEF.arg(i).arg(characterset).arg(sd.Font.family());
            }
            lastStyle += RTF_SETFONTFACE.arg(i);
            //lastStyle += RTF_SETFONTFACE + "0";

            lastStyle += RTF_SETFONTSIZE.arg( sd.Size ? sd.Size << 1 : defaultStyle.Size);

            for (i = 0; i < colorCount; i++)
                if (sd.FGColor == colors[i])
                    break;
            if (i >= colorCount)
                colors[colorCount++] = sd.FGColor;
            lastStyle += RTF_SETCOLOR.arg(i);
            //lastStyle += RTF_SETCOLOR "0";	// Default fore

            for (i = 0; i < colorCount; i++)
                if (sd.BGColor == colors[i])
                    break;
            if (i >= colorCount)
                colors[colorCount++] = sd.BGColor;
            lastStyle += RTF_SETBACKGROUND.arg(i);
            //lastStyle += RTF_SETBACKGROUND + "1";	// Default back (use only one default background)


            lastStyle += sd.Bold ? RTF_BOLD_ON : RTF_BOLD_OFF;
            lastStyle += sd.Italics ? RTF_ITALIC_ON : RTF_ITALIC_OFF;

            styles[istyle] = lastStyle;
            lastStyle.truncate(0);
        }
        styles[STYLE_DEFAULT] = RTF_SETFONTFACE.arg("0")
                                + RTF_SETFONTSIZE.arg(defaultStyle.Size)
                                + RTF_SETCOLOR.arg("0")
                                + RTF_SETBACKGROUND.arg("1")
                                + RTF_BOLD_OFF
                                + RTF_ITALIC_OFF;
        fp += RTF_FONTDEFCLOSE;
    }

    fp += RTF_COLORDEFOPEN;
    for (i = 0; i < colorCount; i++)
    {
        fp += RTF_COLORDEF.arg(colors[i].red()).arg(colors[i].green()).arg(colors[i].blue());
    }
    fp += RTF_COLORDEFCLOSE;

    fp += RTF_HEADERCLOSE;

    fp += RTF_BODYOPEN
          + RTF_SETFONTFACE.arg("0")
          + RTF_SETFONTSIZE.arg(defaultStyle.Size)
          + RTF_SETCOLOR.arg("0 ");
    lastStyle = styles[STYLE_DEFAULT];
    bool prevCR = false;
    int styleCurrent = -1;
    int column = 0;
    for (i = startPos; i < endPos; i++)
    {
        char ch = getByteAt(i);
        int style = getStyleAt(i);
        if (style > STYLE_DEFAULT)
            style = 0;
        if (style != styleCurrent)
        {
            deltaStyle = GetRTFStyleChange(lastStyle, styles[style]);
            fp += deltaStyle;
            lastStyle = styles[style];
            styleCurrent = style;
        }
        switch (ch)
        {
            case '{':
                fp +="\\{";
                break;
            case '}':
                fp +="\\}";
                break;
            case '\\':
                fp +="\\\\";
                break;
            case '\t':
                if (tabs)
                {
                    fp +=RTF_TAB;
                }
                else
                {
                    int ts = tabSize - (column % tabSize);
                    for (int itab = 0; itab < ts; itab++)
                    {
                        fp += ' ';
                    }
                    column += ts - 1;
                }
                break;
            case '\n':
                if (!prevCR)
                {
                    fp +=RTF_EOLN;
                    column = -1;
                }
                break;
            case '\r':
                fp +=RTF_EOLN;
                column = -1;
                break;
            default:
                fp += ch;
        }
        column++;
        prevCR = ch == '\r';
    }
    fp += RTF_BODYCLOSE;

    return fp;
}

void toScintilla::insertAndSelect(const QString &str, bool select)
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

int toScintilla::PrevWordStart(int pos)
{
	return NextWordStart(pos, -1);
}

int toScintilla::PrevWordEnd(int pos)
{
	return NextWordEnd(pos, -1);
}

/**
 * Find the start of the next word in either a forward (delta >= 0) or backwards direction
 * (delta < 0).
 * This is looking for a transition between character classes although there is also some
 * additional movement to transit white space.
 * Used by cursor movement by word commands.
 */
int toScintilla::NextWordStart(int pos, int delta)
{
    int length = text().length(); // get length in chars. (while pure length() return number of bytes)
    if (delta < 0)
    {
        while (pos > 0 && (m_charClasifier.GetClass(getByteAt(pos - 1)) == CharClassify::ccSpace))
            pos--;
        if (pos > 0)
        {
            CharClassify::cc ccStart = m_charClasifier.GetClass(getByteAt(pos-1));
            while (pos > 0 && (m_charClasifier.GetClass(getByteAt(pos - 1)) == ccStart))
            {
                pos--;
            }
        }
    }
    else
    {
        CharClassify::cc ccStart = m_charClasifier.GetClass(getByteAt(pos));
        while (pos < (length) && (m_charClasifier.GetClass(getByteAt(pos)) == ccStart))
            pos++;
        while (pos < (length) && (m_charClasifier.GetClass(getByteAt(pos)) == CharClassify::ccSpace))
            pos++;
    }
    return pos;
}

/**
 * Find the end of the next word in either a forward (delta >= 0) or backwards direction
 * (delta < 0).
 * This is looking for a transition between character classes although there is also some
 * additional movement to transit white space.
 * Used by cursor movement by word commands.
 */
int toScintilla::NextWordEnd(int pos, int delta)
{
    int length = text().length(); // get length in chars. (while pure length() return number of bytes)
    if (delta < 0)
    {
        if (pos > 0)
        {
            CharClassify::cc ccStart = m_charClasifier.GetClass(getByteAt(pos-1));
            if (ccStart != CharClassify::ccSpace)
            {
                while (pos > 0 && m_charClasifier.GetClass(getByteAt(pos - 1)) == ccStart)
                {
                    pos--;
                }
            }
            while (pos > 0 && m_charClasifier.GetClass(getByteAt(pos - 1)) == CharClassify::ccSpace)
            {
                pos--;
            }
        }
    }
    else
    {
        while (pos < length && m_charClasifier.GetClass(getByteAt(pos)) == CharClassify::ccSpace)
        {
            pos++;
        }
        if (pos < length)
        {
            CharClassify::cc ccStart = m_charClasifier.GetClass(getByteAt(pos));
            while (pos < length && m_charClasifier.GetClass(getByteAt(pos)) == ccStart)
            {
                pos++;
            }
        }
    }
    return pos;
}

char toScintilla::getByteAt(int pos)
{
    char ch = SendScintilla(SCI_GETCHARAT, pos);
    return ch;
}

int toScintilla::getStyleAt(int pos)
{
    int style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos);
    return style;
}

int toScintilla::getLevelAt(int line)
{
    int level = SendScintilla(QsciScintilla::SCI_GETFOLDLEVEL, line);
    return level;
}

wchar_t toScintilla::getWCharAt(int pos)
{
    //http://vacuproj.googlecode.com/svn/trunk/npscimoz/npscimoz/oldsrc/trunk.nsSciMoz.cxx
    char _retval[4];
    /*
     * This assumes that Scintilla is using an utf-8 byte-addressed buffer.
     *
     * Return the character that is represented by the utf-8 sequence at
     * the requested position (we could be in the middle of the sequence).
     */
    int byte, byte2, byte3;

    /*
     * Unroll 1 to 3 byte UTF-8 sequences.  See reference data at:
     * http://www.cl.cam.ac.uk/~mgk25/unicode.html
     * http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
     *
     * SendEditor must always be cast to return an unsigned char.
     */

    byte = (unsigned char) SendScintilla(SCI_GETCHARAT, pos);
    if (byte < 0x80)
    {
        /*
         * Characters in the ASCII charset.
         * Also treats \0 as a valid characters representing itself.
         */
        return byte;
    }

    while ((byte < 0xC0) && (byte >= 0x80) && (pos > 0))
    {
        /*
         * Naked trail byte.  We asked for an index in the middle of
         * a UTF-8 char sequence.  Back up to the beginning.  We should
         * end up with a start byte >= 0xC0 and <= 0xFD, but check against
         * 0x80 still in case we have a screwy buffer.
         *
         * We could store bytes as we walk backwards, but this shouldn't
         * be the common case.
         */
        byte = (unsigned char) SendScintilla(SCI_GETCHARAT, --pos);
    }

    if (byte < 0xC0)
    {
        /*
         * Handles properly formed UTF-8 characters between 0x01 and 0x7F.
         * Also treats \0 and naked trail bytes 0x80 to 0xBF as valid
         * characters representing themselves.
         */
    }
    else if (byte < 0xE0)
    {
        byte2 = (unsigned char) SendScintilla(SCI_GETCHARAT, pos+1);
        if ((byte2 & 0xC0) == 0x80)
        {
            /*
             * Two-byte-character lead-byte followed by a trail-byte.
             */
            byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
        }
        /*
         * A two-byte-character lead-byte not followed by trail-byte
         * represents itself.
         */
    }
    else if (byte < 0xF0)
    {
        byte2 = (unsigned char) SendScintilla(SCI_GETCHARAT, pos+1);
        byte3 = (unsigned char) SendScintilla(SCI_GETCHARAT, pos+2);
        if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80))
        {
            /*
             * Three-byte-character lead byte followed by two trail bytes.
             */

            byte = (((byte & 0x0F) << 12)
                    | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
        }
        /*
         * A three-byte-character lead-byte not followed by two trail-bytes
         * represents itself.
         */
    }
#if 0
    /*
     * Byte represents a 4-6 byte sequence.  The rest of Komodo currently
     * won't support this, which makes this code very hard to test.
     * Leave it commented out until we have better 4-6 byte UTF-8 support.
     */
    else
    {
        /*
         * This is the general loop construct for building up Unicode
         * from UTF-8, and could be used for 1-6 byte len sequences.
         *
         * The following structure is used for mapping current UTF-8 byte
         * to number of bytes trail bytes.  It doesn't backtrack from
         * the middle of a UTF-8 sequence.
         */
        static const unsigned char totalBytes[256] =
        {
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
        };
        int ch, trail;

        trail = totalBytes[byte] - 1; // expected number of trail bytes
        if (trail > 0)
        {
            ch = byte & (0x3F >> trail);
            do
            {
                byte2 = (unsigned char) SendEditor(SCI_GETCHARAT, ++pos, 0);
                if ((byte2 & 0xC0) != 0x80)
                {
                    *_retval = (PRUnichar) byte;
                    return NS_OK;
                }
                ch <<= 6;
                ch |= (byte2 & 0x3F);
                trail--;
            }
            while (trail > 0);
            *_retval = (PRUnichar) ch;
            return NS_OK;
        }
    }
#endif

    return byte;
}

toScintilla::CharClassify::cc toScintilla::CharClass(char c)
{
    return m_charClasifier.GetClass(c);
}

toScintilla::CharClassify toScintilla::m_charClasifier;

toScintilla::CharClassify::CharClassify()
{
    SetDefaultCharClasses(true);
}

void toScintilla::CharClassify::SetDefaultCharClasses(bool includeWordClass)
{
    // Initialize all char classes to default values
    for (int ch = 0; ch < 256; ch++)
    {
        if (ch == '\r' || ch == '\n')
            charClass[ch] = ccNewLine;
        else if (ch < 0x20 || ch == ' ')
            charClass[ch] = ccSpace;
        else if (includeWordClass && (ch >= 0x80 || isalnum(ch) || ch == '_' || ch == '$' || ch == '#' || ch == ':' || ch == '@' ))
            charClass[ch] = ccWord;
        else
            charClass[ch] = ccPunctuation;
    }
}

void toScintilla::CharClassify::SetCharClasses(const unsigned char *chars, cc newCharClass)
{
    // Apply the newCharClass to the specifed chars
    if (chars)
    {
        while (*chars)
        {
            charClass[*chars] = static_cast<unsigned char>(newCharClass);
            chars++;
        }
    }
}

int toScintilla::CharClassify::GetCharsOfClass(cc characterClass, unsigned char *buffer)
{
    // Get characters belonging to the given char class; return the number
    // of characters (if the buffer is NULL, don't write to it).
    int count = 0;
    for (int ch = maxChar - 1; ch >= 0; --ch)
    {
        if (charClass[ch] == characterClass)
        {
            ++count;
            if (buffer)
            {
                *buffer = static_cast<unsigned char>(ch);
                buffer++;
            }
        }
    }
    return count;
}
