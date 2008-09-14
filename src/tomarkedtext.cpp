
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tosearchreplace.h"
#include "totool.h"

#include <Qsci/qsciprinter.h>

#include <qapplication.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qprinter.h>
#include <qclipboard.h>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QString>
#include <QDropEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QDir>
#include <QApplication>
#include <QPrintDialog>

#include "icons/undo.xpm"
#include "icons/redo.xpm"
#include "icons/copy.xpm"
#include "icons/cut.xpm"
#include "icons/paste.xpm"


#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)


toMarkedText::toMarkedText(QWidget *parent, const char *name)
        : QsciScintilla(parent), toEditWidget()
{
    if (name)
        setObjectName(name);
    setEdit();
    Search = false;
    setFont(toStringToFont(toConfigurationSingle::Instance().textFont()));
    setMarginLineNumbers(0, true);
    setCallTipsStyle(CallTipsNone);

    connect(this, SIGNAL(textChanged()), this, SLOT(setTextChanged()));
    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(setCopyAvailable(bool)));
    connect(this, SIGNAL(linesChanged()), this, SLOT(linesChanged()));

    CursorTimerID = -1;

    // sets default tab width
    setTabWidth(defaultTabWidth());
    setIndentationsUseTabs(!defaultTabSpaces());

    this->setUtf8(true);
    setAcceptDrops(true);
}

toMarkedText::~toMarkedText()
{
}

int toMarkedText::defaultTabWidth()
{
    return toConfigurationSingle::Instance().tabStop();
}

void toMarkedText::setDefaultTabWidth(int width)
{
    toConfigurationSingle::Instance().setTabStop(width);
    toConfigurationSingle::Instance().saveConfig();
}

bool toMarkedText::defaultTabSpaces(void)
{
    return toConfigurationSingle::Instance().tabSpaces();
}

void toMarkedText::setDefaultTabSpaces(bool on)
{
    toConfigurationSingle::Instance().setTabSpaces(on);
    toConfigurationSingle::Instance().saveConfig();
}

void toMarkedText::setCopyAvailable(bool yes)
{
    setEdit();
}

void toMarkedText::linesChanged()
{
    int x = QString::number(lines()).length() + 1;
    setMarginWidth(0, QString().fill('0', x));
}

void toMarkedText::setEdit(void)
{
    if (isReadOnly())
    {
        toEditWidget::setEdit(false, true, true,
                              false, false,
                              false, hasSelectedText(), false,
                              true, true, false);
    }
    else
    {
        toEditWidget::setEdit(true, true, true,
                              undoEnabled(), redoEnabled(),
                              hasSelectedText(), hasSelectedText(), true,
                              true, true, false);
    }
    toMain::editEnable(this);
}

void toMarkedText::focusInEvent (QFocusEvent *e)
{
    receivedFocus();
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    toMainWidget()->setCoordinates(curline + 1, curcol + 1);
    QsciScintilla::focusInEvent(e);
    if (CursorTimerID < 0)
        CursorTimerID = startTimer(500);
}

void toMarkedText::timerEvent(QTimerEvent *e)
{
    if (CursorTimerID == e->timerId())
    {
        int curline, curcol;
        getCursorPosition (&curline, &curcol);
        toMainWidget()->setCoordinates(curline + 1, curcol + 1);
    }
    else
        QsciScintilla::timerEvent(e);
}

void toMarkedText::focusOutEvent (QFocusEvent *e)
{
    if (Search)
    {
        Search = false;
        LastSearch = SearchString;
        toStatusMessage(QString::null);
    }
    if (CursorTimerID >= 0)
    {
        killTimer(CursorTimerID);
        CursorTimerID = -1;
    }
    QsciScintilla::focusOutEvent(e);
}


void toMarkedText::editPrint(void)
{
    QsciPrinter printer;

    QPrintDialog dialog(&printer, this);
    dialog.setMinMax(1, 1000);
    dialog.setFromTo(1, 1000);

    if(!Filename.isEmpty()) {
        QFileInfo info(Filename);
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

    if(!dialog.exec())
        return;

    printer.setCreator(tr(TOAPPNAME));

    // they show up in the print
    setMarginLineNumbers(0, false);
    printer.printRange(this);
    setMarginLineNumbers(0, true);
}

void toMarkedText::openFilename(const QString &file)
{
    QString data = toReadFile(file);
    setText(data);
    setFilename(file);
    setModified(false);
    toMainWidget()->addRecentFile(file);
    toStatusMessage(tr("File opened successfully"), false, false);
}

bool toMarkedText::editOpen(QString suggestedFile)
{
    if (isModified())
    {
        int ret = TOMessageBox::information(this,
                                            tr("Save changes?"),
                                            tr("The editor has been changed, do you want to save them\n"
                                               "before opening a new file?"),
                                            tr("&Yes"), tr("&No"), tr("Cancel"), 0, 2);
        if (ret == 2)
            return false;
        else if (ret == 0)
            if (!editSave(false))
                return false;
    }

    QString fname;
    if (suggestedFile != QString::null)
        fname = suggestedFile;
    else
    {
        QFileInfo file(filename());
        fname = toOpenFilename(file.dir().path(), QString::null, this);
    }
    if (!fname.isEmpty())
    {
        try
        {
            openFilename(fname);
            emit fileOpened();
            return true;
        }
        TOCATCH
    }
    return false;
}

bool toMarkedText::editSave(bool askfile)
{
    QFileInfo file(filename());
    QString fn = filename();
    if (askfile || fn.isEmpty())
        fn = toSaveFilename(file.dir().path(), QString::null, this);
    if (!fn.isEmpty())
    {
        if (!toWriteFile(fn, text()))
            return false;
        toMainWidget()->addRecentFile(fn);
        setFilename(fn);
        setModified(false);
        return true;
    }
    return false;
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
            for (int i = 0;i < str.length() && str.at(i).isSpace();i++)
                ind += str.at(i);
            if (ind.length())
                insert(ind, false);
        }
    }
}

void toMarkedText::searchFound(int line, int col)
{
    setSelection (line, col + SearchString.length(), line, col);
    ensureCursorVisible();
    toStatusMessage(tr("Incremental search") + QString::fromLatin1(":") + SearchString, false, false);
}

void toMarkedText::incrementalSearch(bool forward, bool next)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    QString line;
    if (SearchFailed && next)
    {
        if (forward)
        {
            curline = 0;
            curcol = 0;
            next = false;
            line = text(curline);
        }
        else
        {
            curline = lines() - 1;
            line = text(curline);
            curcol = line.length();
            next = false;
        }
        SearchFailed = false;
    }
    else
        line = text(curline);
    if (forward)
    {
        if (next)
            curcol++;
        if (curcol + SearchString.length() <= line.length())
        {
            int pos = line.indexOf(SearchString, curcol, Qt::CaseInsensitive);
            if (pos >= 0)
            {
                searchFound(curline, pos);
                return ;
            }
        }
        for (curline++;curline < lines();curline++)
        {
            int pos = text(curline).indexOf(SearchString, 0, Qt::CaseInsensitive);
            if (pos >= 0)
            {
                searchFound(curline, pos);
                return ;
            }
        }
    }
    else
    {
        if (next)
            curcol--;
        if (curcol >= 0)
        {
            int pos = line.lastIndexOf(SearchString, curcol, Qt::CaseInsensitive);
            if (pos >= 0)
            {
                searchFound(curline, pos);
                return ;
            }
        }
        for (curline--;curline >= 0;curline--)
        {
            int pos = text(curline).lastIndexOf(SearchString, -1, Qt::CaseInsensitive);
            if (pos >= 0)
            {
                searchFound(curline, pos);
                return ;
            }
        }
    }
    toStatusMessage(tr("Incremental search") + QString::fromLatin1(":") + SearchString + QString::fromLatin1(" (failed)"), false, false);
    SearchFailed = true;
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
    if (Search)
    {
        Search = false;
        LastSearch = SearchString;
        toStatusMessage(QString::null);
    }

    DragStart = QPoint();
    if(e->button() == Qt::LeftButton && geometry().contains(e->pos())) {
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

void toMarkedText::incrementalSearch(bool forward)
{
    SearchForward = forward;
    if (!Search)
    {
        Search = true;
        SearchFailed = false;
        SearchString = QString::null;
        toStatusMessage(tr("Incremental search") + QString::fromLatin1(":"), false, false);
    }
    else if (Search)
    {
        if (!SearchString.length())
            SearchString = LastSearch;
        if (SearchString.length())
            incrementalSearch(SearchForward, true);
    }
}

void toMarkedText::keyPressEvent(QKeyEvent *e)
{
    if (Search)
    {
        bool ok = false;
        if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Backspace)
        {
            int len = SearchString.length();
            if (len > 0)
                SearchString.truncate(len - 1);
            ok = true;
        }
        else if (e->key() != Qt::Key_Escape)
        {
            QString t = e->text();
            if (t.length())
            {
                SearchString += t;
                ok = true;
            }
            else if (e->key() == Qt::Key_Shift ||
                     e->key() == Qt::Key_Control ||
                     e->key() == Qt::Key_Meta ||
                     e->key() == Qt::Key_Alt)
            {
                ok = true;
            }
        }

        if (ok)
        {
            incrementalSearch(SearchForward, false);
            e->accept();
            return ;
        }
        else
        {
            Search = false;
            LastSearch = SearchString;
            toStatusMessage(QString::null);
        }
    }
    QsciScintilla::keyPressEvent(e);
}

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
}

void toMarkedText::findPosition(int index, int &line, int &col)
{
    int pos = 0;
    for (int i = 0;i < lines();i++)
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

bool toMarkedText::searchNext(toSearchReplace *search)
{
    QString text = toMarkedText::text();

    int col;
    int line;
    getCursorPosition(&line, &col);
    int pos = FindIndex(text, line, col) + 1;

    int endPos;
    if (search->findString(text, pos, endPos))
    {
        int endCol;
        int endLine;
        findPosition(pos, line, col);
        findPosition(endPos, endLine, endCol);
        setSelection(line, col, endLine, endCol);
        ensureCursorVisible();
        return true;
    }

    return false;
}

void toMarkedText::searchReplace(const QString &newData)
{
    if (!isReadOnly())
        insert(newData);
}

bool toMarkedText::searchCanReplace(bool all)
{
    if (isReadOnly())
        return false;
    if (all || hasSelectedText())
        return true;
    return false;
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

void toMarkedText::setTextChanged()
{
    redoEnabled(isRedoAvailable());
    undoEnabled(isUndoAvailable());
    if (hasSelectedText())
    {
        setEdit();
    }
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
        action = popup->addAction(QIcon(undo_xpm),
                                  tr("&Undo"),
                                  this,
                                  SLOT(undo()));
        action->setShortcut(QKeySequence::Undo);
        action->setEnabled(isUndoAvailable());

        action = popup->addAction(QIcon(redo_xpm),
                                  tr("&Redo"),
                                  this,
                                  SLOT(redo()));
        action->setShortcut(QKeySequence::Redo);
        action->setEnabled(isRedoAvailable());

        popup->addSeparator();

        action = popup->addAction(QIcon(cut_xpm),
                                  tr("Cu&t"),
                                  this,
                                  SLOT(cut()));
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
                                  this,
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
