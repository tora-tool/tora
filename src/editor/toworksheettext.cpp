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
#include "tools/toworksheet.h"
#include "editor/tocomplpopup.h"
#include "core/toconnection.h"
#include "core/toconnectiontraits.h"
#include "core/tologger.h"
#include "core/toglobalevent.h"
#include "core/toeditorconfiguration.h"

#include "parsing/tsqlparse.h"

#include "shortcuteditor/shortcutmodel.h"

#include <QtCore/QFileSystemWatcher>
#include <QListWidget>
#include <QDir>

#include <algorithm>


using namespace ToConfiguration;
using namespace SQLParser;
using namespace std;

toWorksheetText::toWorksheetText(toWorksheet *worksheet, QWidget *parent, const char *name)
    : toSqlText(parent, name)
    , editorType(SciTe)
    , popup(new toComplPopup(this))
    , m_complAPI(NULL)
    , m_complTimer(new QTimer(this))
    , m_fsWatcher(new QFileSystemWatcher(this))
    , m_bookmarkHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
    , m_bookmarkMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightTriangle))
    , m_completeEnabled(toConfigurationNewSingle::Instance().option(Editor::CodeCompleteBool).toBool())
    , m_completeDelayed((toConfigurationNewSingle::Instance().option(Editor::CodeCompleteDelayInt).toInt() > 0))
    , m_parserTimer(new QTimer(this))
    , m_parserThread(new QThread(this))
{
    FlagSet.Open = true;

    if (m_completeEnabled && !m_completeDelayed)
    {
        QsciScintilla::setAutoCompletionThreshold(1); // start when a single leading word's char is typed
        QsciScintilla::setAutoCompletionUseSingle(QsciScintilla::AcusExplicit);
        QsciScintilla::setAutoCompletionSource(QsciScintilla::AcsAll); // AcsAll := AcsAPIs | AcsDocument
    }
    QsciScintilla::setAutoIndent(true);

    /* it is possible to select multiple ranges by holding down the Ctrl key while dragging with the mouse */
    SendScintilla(QsciScintilla::SCI_SETMULTIPLESELECTION, true);

    /*  When pasting into multiple selections,
     * the pasted text can go into just the main selection with SC_MULTIPASTE_ONCE=0
     * or into each selection with SC_MULTIPASTE_EACH=1. SC_MULTIPASTE_ONCE is the default. */
    SendScintilla(QsciScintilla::SCI_SETMULTIPASTE, 1);
    /*  Whether typing, new line, cursor left/right/up/down,
     * backspace, delete, home, and end work with multiple selections simultaneously.
     * Also allows selection and word and line deletion commands. */
    SendScintilla(QsciScintilla::SCI_SETADDITIONALSELECTIONTYPING, true);

    setCaretAlpha();
    connect(&m_caretVisible, SIGNAL(valueChanged(QVariant const&)), this, SLOT(setCaretAlpha()));
    connect(&m_caretAlpha, SIGNAL(valueChanged(QVariant const&)), this, SLOT(setCaretAlpha()));
    connect(m_fsWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(m_fsWatcher_fileChanged(const QString&)));

    // handle "max text width" mark
    if (toConfigurationNewSingle::Instance().option(Editor::UseMaxTextWidthMarkBool).toBool())
    {
        QsciScintilla::setEdgeColumn(toConfigurationNewSingle::Instance().option(Editor::MaxTextWidthMarkInt).toInt());
        // TODO setEdgeColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).darker(150));
        QsciScintilla::setEdgeMode(QsciScintilla::EdgeLine);
    }
    else
        QsciScintilla::setEdgeMode(QsciScintilla::EdgeNone);


    //connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    connect( m_complTimer, SIGNAL(timeout()), this, SLOT(slotCompletiotionTimout()) );

    connect(&toEditorTypeButtonSingle::Instance(),
            SIGNAL(toggled(int)),
            this,
            SLOT(setEditorType(int)));

    popup->hide();
    connect(popup->list(),
            SIGNAL(itemClicked(QListWidgetItem*)),
            this,
            SLOT(slotCompleteFromPopup(QListWidgetItem*)));
    connect(popup->list(),
            SIGNAL(itemActivated(QListWidgetItem*)),
            this,
            SLOT(slotCompleteFromPopup(QListWidgetItem*)));

    m_parserTimer->setInterval(5000);   // every 5s
    m_parserTimer->setSingleShot(true); // repeat only if bg thread responded
    m_parserThread->setObjectName("toWorksheetText ParserThread");
    m_worker = new toWorksheetTextWorker(NULL);
    m_worker->moveToThread(m_parserThread);
    connect(m_parserTimer, SIGNAL(timeout()), this, SLOT(statementProcess()));
    connect(this, SIGNAL(statementParsingRequested(QString)),  m_worker, SLOT(process(QString)));
    connect(m_worker, SIGNAL(processed(toDictionary)), this, SLOT(statementProcessed(toDictionary)));
    connect(m_worker, SIGNAL(finished()),  m_parserThread, SLOT(quit()));
    connect(m_worker, SIGNAL(finished()),  m_worker, SLOT(deleteLater()));
    connect(m_parserThread, SIGNAL(finished()),  m_parserThread, SLOT(deleteLater()));
    m_parserThread->start();
    scheduleParsing();
}

toWorksheetText::~toWorksheetText()
{
    m_parserThread->quit();
    m_parserThread->wait();
    delete m_parserThread;
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
    long currPosition = currentPosition();
    long nextPosition = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, currPosition);
    // handle editor shortcuts with TAB
    // It uses qscintilla lowlevel API to handle "word under cursor"
    // This code is taken from sqliteman.com
    if (e->key() == Qt::Key_Tab && toConfigurationNewSingle::Instance().option(Editor::UseEditorShortcutsBool).toBool())
    {
        long start = SendScintilla(SCI_WORDSTARTPOSITION, currPosition, true);
        long end = SendScintilla(SCI_WORDENDPOSITION, currPosition, true);
        QString key(wordAtPosition(currPosition, true));
        EditorShortcutsMap shorts(toConfigurationNewSingle::Instance().option(Editor::EditorShortcutsMap).toMap());
        if (shorts.contains(key))
        {
            setSelection(start, end);
            removeSelectedText();
            insert(shorts.value(key).toString());
            currPosition = SendScintilla(SCI_GETCURRENTPOS);
            SendScintilla(QsciScintillaBase::SCI_SETEMPTYSELECTION, currPosition + (unsigned)shorts.value(key).toByteArray().length());
            e->accept();
            return;
        }
    }
    else if (m_completeEnabled && e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Space)
    {
        autoCompleteFromDocument();
        e->accept();
        return;
    }
    else if (m_completeEnabled && e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_T)
    {
        toSqlText::Word firstWord, secondWord;
        tableAtCursor(firstWord, secondWord);

        QString context = firstWord.text();
        if (context.isEmpty())
            context = toToolWidget::currentSchema(this);

        autoCompleteTableName(context, secondWord);
        e->accept();
        return;
    }
    else if (m_completeEnabled && e->key() == Qt::Key_Period)
    {
        // Dot was pressed start completion timer, save current position
        m_complTimer->start(toConfigurationNewSingle::Instance().option(Editor::CodeCompleteDelayInt).toInt());
        m_complPosition = currentPosition();
        getCursorPosition(&m_complLine, &m_complLinePos);
    }
    super::keyPressEvent(e);
}

#if 0
void toWorksheetText::positionChanged(int row, int col)
{
    using namespace ToConfiguration;
    using cc = toScintilla::CharClassify::cc;
    using ChClassEnum = toScintilla::CharClassify;

    long currPosition, nextPosition;
    wchar_t currChar, nextChar;
    cc currClass, nextClass;
    
    if (col <= 0)
        goto no_complete;

    if (m_completeEnabled == false || m_completeEnabled == false)
        goto no_complete;

    currPosition = currentPosition();
    nextPosition = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, currPosition);

    currChar = getWCharAt(currPosition);
    nextChar = getWCharAt(nextPosition);

    currClass = CharClass(currChar);
    nextClass = CharClass(nextChar);

    TLOG(0, toNoDecorator, __HERE__) << currChar << std::endl;

    if (currChar == 0)
        goto no_complete;

    if ((currClass == ChClassEnum::ccWord || currClass == ChClassEnum::ccPunctuation) &&
            (nextClass == CharClassify::ccWord || nextClass == CharClassify::ccPunctuation))
        goto no_complete;

    // Cursor is not at EOL, not before any word character
    if (currClass != ChClassEnum::ccWord && currClass != ChClassEnum::ccSpace)
        goto no_complete;

    for(int i=1, c=col; i<3 && c; i++, c--)
    {
        currPosition = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, currPosition);
        currChar = getWCharAt(currPosition);
        if (currChar == L'.')
        {
            m_complTimer->start(toConfigurationNewSingle::Instance().option(Editor::CodeCompleteDelayInt).toInt());
            return;
        }
        if (currClass != CharClassify::ccWord)
            break;
    }

// FIXME: disabled due repainting issues
//    current line marker (margin arrow)
//    markerDeleteAll(m_currentLineMarginHandle);
//    markerAdd(row, m_currentLineMarginHandle);

no_complete:
    m_complTimer->stop();
}
#endif

void toWorksheetText::setCaretAlpha()
{
    // highlight caret line
    if ((bool)m_caretVisible)
    {
        QsciScintilla::setCaretLineVisible(true);
        // This is only required until transparency fixes in QScintilla go into stable release
        //QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, QsciScintilla::SC_ALPHA_NOALPHA);
        QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, (int)m_caretAlpha);
    } else {
        QsciScintilla::setCaretLineVisible(false);
    }
}

// the QScintilla way of autocomletition
#if 0
void toWorksheetText::autoCompleteFromAPIs()
{
    m_complTimer->stop(); // it's a must to prevent infinite reopening
    {
        toScintilla::autoCompleteFromAPIs();
        return;
    }
}
#endif

void toWorksheetText::slotCompletiotionTimout()
{
    TLOG(0, toTimeStart, __HERE__) << "Start" << std::endl;
    m_complTimer->stop(); // it's a must to prevent infinite reopening

    // Check whether our cursor position is close to period, which started this timer
    int curline, curcol;
    getCursorPosition (&curline, &curcol);

    if (curline != m_complLine)
        return;
    if (curcol - m_complLinePos >= 3)
        return;
    if (curcol < m_complLinePos)
        return;

    autoCompleteFromAPIs();
}

// the Tora way of autocomletition
void toWorksheetText::autoCompleteFromAPIs()
{
    TLOG(0, toTimeDelta, __HERE__) << "Start"  << std::endl;
    Utils::toBusy busy;
    toConnection &connection = toConnection::currentConnection(this);

    TLOG(0, toTimeDelta, __HERE__) << "Step" << std::endl;
    int position = currentPosition();
    toSqlText::Word firstWord, secondWord;
    tableAtCursor(firstWord, secondWord);

    QString worksheetSchema = toToolWidget::currentSchema(this);

    TLOG(0, toTimeDelta, __HERE__) << "Table at index: " << '"' << firstWord.text() << '"' << ':' << '"' << secondWord.text() << '"' << std::endl;

    // Disambiguate the 1st word, schema/table/alias
    if (!firstWord.text().isEmpty())
    {
        // firstWord is schema name, complete secondWord as table
        QStringList ul = connection.getCache().userList(toCache::OWNERS);
        if (ul.contains(firstWord.text().toUpper()))
        {
            autoCompleteTableName(firstWord.text().toUpper(), secondWord);
            return;
        }

        // firstWord is table alias, complete secondWord as column
        if (m_lastTranslations.contains(firstWord.text().toUpper()))
        {
            TLOG(0, toTimeDelta, __HERE__) << "Step a" << std::endl;
            autoCompleteColumnName(m_lastTranslations.value(firstWord.text().toUpper()), secondWord);
            return;
        }

        // firstWord might be a table name, complete secondWord as column
        toCache::ObjectRef table;
        table.context = worksheetSchema;
        table.second = QString();
        table.first  = secondWord.text().toUpper();
        toCache::CacheEntry const* e =  connection.getCache().findEntry(table);
        if (e)
        {
            TLOG(0, toTimeDelta, __HERE__) << "Step b" << std::endl;
            autoCompleteColumnName(firstWord.text().toUpper(), secondWord);
            return;
        }
    }
}

void toWorksheetText::autoCompleteTableName(QString const& context, toSqlText::Word const &secondWord)
{
    TLOG(0, toNoDecorator, __HERE__) << "autoCompleteTableName Start" << std::endl;

    toConnection &connection = toConnection::currentConnection(this);
    QStringList compleList = connection.getCache().completeEntry(context, secondWord.text().toUpper());

    if (compleList.size() <= 100) // Do not waste CPU on sorting huge completition list TODO: limit the amount of returned entries
        compleList.sort();

    int position = currentPosition();
    if (compleList.isEmpty())
    {
        this->SendScintilla(SCI_SETEMPTYSELECTION, position);
        return;
    }

    if (!secondWord.text().isEmpty())
        setSelection(secondWord.start(), position);

    if (compleList.count() == 1)
    {
        completeWithText(compleList.first());
    }
    else
    {
        displayCompletePopup(compleList);
    }
}

void toWorksheetText::autoCompleteColumnName(QString const& context, toSqlText::Word const &secondWord)
{
    TLOG(0, toTimeDelta, __HERE__) << "autoCompleteColumnName Start" << std::endl;
    toConnection &connection = toConnection::currentConnection(this);

    toCache::ObjectRef table;
    table.context = toToolWidget::currentSchema(this);
    table.first  = QString();
    table.second = context; // context is the table name

    QStringList compleList;

    toCache::CacheEntry const *e = connection.getCache().findEntry(table);
    if (e)
    {
        TLOG(0, toTimeDelta, __HERE__) << "autoCompleteColumnName Step a" << std::endl;
        // TODO this is sync db request evaluated in the main (UI) thread - no async approach yet
        connection.getCache().describeEntry(e);
        toQAdditionalDescriptions d  = e->description;
        toQColumnDescriptionList dl = d.value("COLUMNLIST").value<toQColumnDescriptionList>();

        foreach(toCache::ColumnDescription cd, dl)
        {
            TLOG(0, toNoDecorator, __HERE__) << cd.Name << std::endl;
            if (cd.Name.startsWith(secondWord.text().toUpper()))
                compleList.append(cd.Name);
        }
        TLOG(0, toTimeDelta, __HERE__) << "autoCompleteColumnName Step b" << std::endl;
    }
    compleList.sort();

    int position = currentPosition();
    if (compleList.isEmpty())
    {
        this->SendScintilla(SCI_SETEMPTYSELECTION, position);
        return;
    }

    if (!secondWord.text().isEmpty())
        setSelection(secondWord.start(), position);

    if (compleList.count() == 1)
    {
        completeWithText(compleList.first());
    }
    else
    {
        TLOG(0, toTimeDelta, __HERE__) << "autoCompleteColumnName Step c" << std::endl;
        displayCompletePopup(compleList);
    }
}

void toWorksheetText::slotCompleteFromPopup(QListWidgetItem* item)
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
                  (unsigned)text.length());
    pos = SendScintilla(SCI_GETCURRENTPOS);
    SendScintilla(SCI_SETSELECTIONSTART, pos, true);
    SendScintilla(SCI_SETSELECTIONEND, pos, true);
}

void toWorksheetText::displayCompletePopup(QStringList const& compleList)
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

    TLOG(0, toTimeTotal, __HERE__) << "End" << std::endl;
    popup->show();
    popup->setFocus();
}

QString const& toWorksheetText::filename(void) const
{
    return m_filename;
}

void toWorksheetText::setFilename(const QString &filename)
{
    m_filename = filename;
}

void toWorksheetText::openFilename(const QString &file)
{
#pragma message WARN("TODO/FIXME: clear markers!")
    fsWatcherClear();

    QString data = Utils::toReadFile(file);
    setText(data);
    setFilename(file);
    setModified(false);
    toGlobalEventSingle::Instance().addRecentFile(file);

    m_fsWatcher->addPath(file);

    Utils::toStatusMessage(tr("File opened successfully"), false, false);
}

bool toWorksheetText::editOpen(const QString &suggestedFile)
{
    int ret = 1;
    if (isModified())
    {
        // grab focus so user can see file and decide to save
        setFocus(Qt::OtherFocusReason);

        ret = TOMessageBox::information(this,
                                            tr("Save changes?"),
                                            tr("The editor has been changed, do you want to save them\n"
                                               "before opening a new file?"),
                                            tr("&Save"), tr("&Discard"), tr("New worksheet"), 0);
        if (ret < 2)
            return false;
        else if (ret == 0)
            if (!editSave(false))
                return false;
    }

    QString fname;
    if (!suggestedFile.isEmpty())
        fname = suggestedFile;
    else
        fname = Utils::toOpenFilename(QString(), this);

    if (!fname.isEmpty())
    {
        try
        {
            if (ret == 2)
                toGlobalEventSingle::Instance().editOpenFile(fname);
            else
            {
                openFilename(fname);
                emit fileOpened();
                emit fileOpened(fname);
            }
            return true;
        }
        TOCATCH
    }
    return false;
}

bool toWorksheetText::editSave(bool askfile)
{
    fsWatcherClear();
    bool ret = false;

    QString fn;
    QFileInfo file(filename());
    if (!filename().isEmpty() && file.exists() && file.isWritable())
        fn = file.absoluteFilePath();

    if (!filename().isEmpty() && fn.isEmpty() && file.dir().exists())
        fn = file.absoluteFilePath();

    if (askfile || fn.isEmpty())
        fn = Utils::toSaveFilename(fn, QString(), this);

    if (!fn.isEmpty() && Utils::toWriteFile(fn, text()))
    {
        toGlobalEventSingle::Instance().addRecentFile(fn);
        setFilename(fn);
        setModified(false);
        emit fileSaved(fn);

        m_fsWatcher->addPath(fn);
        ret = true;
    }
    return ret;
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
    std::sort(m_bookmarks.begin(), m_bookmarks.end());
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

#if 0
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
#endif

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

void toWorksheetText::m_fsWatcher_fileChanged(const QString & filename)
{
    m_fsWatcher->blockSignals(true);
    setFocus(Qt::OtherFocusReason);
    if (QMessageBox::question(this, tr("External File Modification"),
                              tr("File %1 was modified by an external application. Reload (your changes will be lost)?").arg(filename),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    try
    {
        openFilename(filename);
    }
    TOCATCH;

    m_fsWatcher->blockSignals(false);
}

void toWorksheetText::fsWatcherClear()
{
    QStringList l(m_fsWatcher->files());
    if (!l.empty())
        m_fsWatcher->removePaths(l);
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

void toWorksheetText::scheduleParsing()
{
    if (m_haveFocus && !m_parserTimer->isActive())
        m_parserTimer->start();
    super::scheduleParsing();
}

void toWorksheetText::unScheduleParsing()
{
    if (m_parserTimer->isActive())
        m_parserTimer->stop();
    super::unScheduleParsing();
}

void toWorksheetText::statementProcess()
{
    QString sql = currentStatement().sql;
    if (sql != m_lastSQL)
    {
        emit statementParsingRequested(sql);
        m_lastSQL = sql;
    }
}

void toWorksheetText::statementProcessed(toDictionary dict)
{
    if (!dict.isEmpty())
        m_lastTranslations = dict;
    scheduleParsing();
}

toWorksheetTextWorker::toWorksheetTextWorker(QObject *parent)
    : QObject(parent)
{

}

toWorksheetTextWorker::~toWorksheetTextWorker()
{
}

static void toASTWalkFilter(Statement &source, const std::function<void(Statement &source, Token const &n)>& visitor)
{
    SQLParser::Statement::token_const_iterator node;
    for (node = source.begin(); node != source.end(); ++node)
    {
        visitor(source, *node);
    }
}

void toWorksheetTextWorker::process(QString text)
{
    toDictionary translationMap;
    try
    {
        std::unique_ptr <SQLParser::Statement> stat = StatementFactTwoParmSing::Instance().create("OracleDML", text, "");
		stat->scanTree();

        TLOG(5, toDecorator, __HERE__)
        << "Parsing ok:" << std::endl
        << stat->root()->toStringRecursive().toStdString() << std::endl;

        std::function<void(Statement &source, Token const& n)> table_ref = [&](Statement &source, Token const&node)
        {
            if (node.getTokenType() != Token::L_TABLEALIAS)
                return;

            Token const *translation = source.translateAlias(node.toString(), &node);
            if (TokenTable const *tokenTable = dynamic_cast<TokenTable const*>(translation))
            {
                TLOG(5, toNoDecorator, __HERE__) << tokenTable->tableName() << std::endl;
                translationMap.insert(node.toString().toUpper(), tokenTable->tableName().toUpper());
            }
        };

        toASTWalkFilter(*stat, table_ref);
    }
    catch ( SQLParser::ParseException const &e)
    {
        TLOG(5, toDecorator, __HERE__) << "Exc:" << e.what() << std::endl;
    }
    catch (...)
    {
        TLOG(5, toDecorator, __HERE__) << "Exc:"  << std::endl;
    }
    emit processed(translationMap);
}

toEditorTypeButton::toEditorTypeButton(QWidget *parent, const char *name)
    : toToggleButton(ENUM_REF(toWorksheetText, EditorTypeEnum), parent, name)
{
}

toEditorTypeButton::toEditorTypeButton()
    : toToggleButton(ENUM_REF(toWorksheetText, EditorTypeEnum), NULL)
{
}
