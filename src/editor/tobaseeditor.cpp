
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

#include "editor/tobaseeditor.h"
#include "editor/toscintilla.h"
#include "core/toeditmenu.h"
#include "core/toglobalevent.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "ts_log/ts_log_utils.h"

#include <Qsci/qsciscintilla.h>

#include <QtCore/QFileSystemWatcher>
#include <QtCore/QtDebug>
#include <QtCore/QDir>
#include <QVBoxLayout>

toBaseEditor::toBaseEditor(toScintilla *editor, QWidget *parent)
	: QWidget(parent)
	, m_editor(editor)
{
	setObjectName("toBaseEditor");
#pragma message WARN("TODO/FIXME: fix this ugly types based init")
	m_editor->setParent(this);

	Q_ASSERT_X(m_editor, "toBaseEditor init", "editor must be existing instance of toMarkedTextEditor");

	QVBoxLayout *l = new QVBoxLayout();
	l->setSpacing(0);
	l->setContentsMargins(0, 0, 0, 0);

	l->addWidget(m_editor);
	m_editor->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	m_search = new toSearchReplace(this);
	m_search->hide();
//	m_editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	l->addWidget(m_search);

	setLayout(l);

    setEditFlags();

    m_fsWatcher = new QFileSystemWatcher(this);
    connect(m_fsWatcher, SIGNAL(fileChanged(const QString&)),
            this, SLOT(m_fsWatcher_fileChanged(const QString &)));

    connect(m_editor, SIGNAL(textChanged()), this, SLOT(setEditFlags()));
    connect(m_editor, SIGNAL(copyAvailable(bool)), this, SLOT(setEditFlags()));

    connect(m_editor, SIGNAL(gotFocus()), this, SLOT(gotFocus()));
    connect(m_editor, SIGNAL(lostFocus()), this, SLOT(lostFocus()));
    connect(m_editor, SIGNAL(cursorPositionChanged(int, int)),
            this, SLOT(setCoordinates(int, int)));

    connect(m_search, SIGNAL(searchNext(Search::SearchFlags)),
    		this, SLOT(handleSearching(Search::SearchFlags)));
    connect(m_search, SIGNAL(windowClosed()),
            this, SLOT(setEditorFocus()));
    setFocusProxy(m_editor);
}

QString toBaseEditor::editText()
{
	return m_editor->text();
}

void toBaseEditor::editUndo(void)
{
	m_editor->undo();
}

void toBaseEditor::editRedo(void)
{
	m_editor->redo();
}

void toBaseEditor::editCut(void)
{
	m_editor->cut();
}

void toBaseEditor::editCopy(void)
{
	m_editor->copy();
}

void toBaseEditor::editPaste(void)
{
	m_editor->paste();
}

void toBaseEditor::editSelectAll(void)
{
	m_editor->selectAll(false);
}

void toBaseEditor::editPrint(void)
{
	m_editor->print(Filename);
}

bool toBaseEditor::editOpen(const QString &suggestedFile)
{
    if (m_editor->isModified())
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
    if (!suggestedFile.isEmpty())
        fname = suggestedFile;
    else
        fname = Utils::toOpenFilename(QString::null, this);

    if (!fname.isEmpty())
    {
        try
        {
            openFilename(fname);
            emit fileOpened();
            emit fileOpened(fname);
            printf("emit file opened\n");
            return true;
        }
        TOCATCH
    }
    return false;
}

bool toBaseEditor::editSave(bool askfile)
{
    fsWatcherClear();
    bool ret = false;

    QString fn;
    QFileInfo file(filename());
    if(!filename().isEmpty() && file.exists() && file.isWritable())
      fn = file.absoluteFilePath();

    if(!filename().isEmpty() && fn.isEmpty() && file.dir().exists())
      fn = file.absoluteFilePath();

    if (askfile || fn.isEmpty())
      fn = Utils::toSaveFilename(fn, QString::null, this);

    if (!fn.isEmpty() && Utils::toWriteFile(fn, m_editor->text()))
    {
        toGlobalEventSingle::Instance().addRecentFile(fn);
        setFilename(fn);
        m_editor->setModified(false);
        emit fileSaved(fn);

        m_fsWatcher->addPath(fn);
        ret = true;
    }
    return ret;
}

bool toBaseEditor::searchNext()
{
	if (!m_search->isVisible())
	{
		m_search->show();
		m_search->setReadOnly(m_editor->isReadOnly());
	}
	return true;
}

void toBaseEditor::searchReplace()
{
    m_search->setVisible(!m_search->isVisible());
    m_search->setReadOnly(m_editor->isReadOnly());
}

void toBaseEditor::editReadAll()
{
	qDebug() << "toBaseEditor::editReadAll() not implemented";
}

void toBaseEditor::setEditFlags()
{
    if (m_editor->isReadOnly())
    {
    	FlagSet.Save = true;
    	FlagSet.Print = true;
    	FlagSet.Copy = m_editor->hasSelectedText();
    	FlagSet.Search = true;
    	FlagSet.SelectAll = true;
    }
    else
    {
        FlagSet.Open = true;
        FlagSet.Save = true;
        FlagSet.Print = true;
        FlagSet.Undo = m_editor->isUndoAvailable();
		FlagSet.Redo = m_editor->isRedoAvailable();
        FlagSet.Cut = m_editor->hasSelectedText();
        FlagSet.Copy = m_editor->hasSelectedText();
        FlagSet.Paste = true;
        FlagSet.Search = true;
        FlagSet.SelectAll = true;
    }
}

void toBaseEditor::m_fsWatcher_fileChanged(const QString & filename)
{
    m_fsWatcher->blockSignals(true);
    setFocus(Qt::OtherFocusReason);
    if (QMessageBox::question(this, tr("External File Modification"),
                              tr("File %1 was modified by an external application. Reload (your changes will be lost)?").arg(filename),
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No)
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

void toBaseEditor::fsWatcherClear()
{
    QStringList l(m_fsWatcher->files());
    if (!l.empty())
        m_fsWatcher->removePaths(l);
}

void toBaseEditor::gotFocus()
{
	TLOG(9, toDecorator, __HERE__) << this << std::endl;
}

void toBaseEditor::lostFocus()
{
	TLOG(9, toDecorator, __HERE__) << this << std::endl;
}

void toBaseEditor::clear(void)
{
    Filename = "";
    fsWatcherClear();
    setEditFlags();
    m_editor->clear();
    m_editor->setModified(false);
}

void toBaseEditor::openFilename(const QString &file)
{
#pragma message WARN("TODO/FIXME: markers!")
    fsWatcherClear();

    QString data = Utils::toReadFile(file);
    m_editor->setText(data);
    setFilename(file);
    m_editor->setModified(false);
    toGlobalEventSingle::Instance().addRecentFile(file);

    m_fsWatcher->addPath(file);

    Utils::toStatusMessage(tr("File opened successfully"), false, false);
}

void toBaseEditor::handleSearching(Search::SearchFlags flags)
{
	qDebug() << "toBaseEditor::handleSearchin" << m_search->searchText() << m_search->replaceText() << flags;
	bool ret = m_editor->findText(m_search->searchText(),
								  m_search->replaceText(),
								  flags);
	if (!ret)
		qWarning() << "TODO/FIXME" << "toBaseEditor::handleSearching return value";
}

void toBaseEditor::setCoordinates(int line, int column)
{
	toGlobalEventSingle::Instance().setCoordinates(line + 1, column + 1);
}

void toBaseEditor::setEditorFocus()
{
	m_editor->findStop();
    m_editor->setFocus(Qt::OtherFocusReason);
}

void toBaseEditor::setWordWrap(bool wrap)
{ m_editor->setWordWrap(wrap); };
