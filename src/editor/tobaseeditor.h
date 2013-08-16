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

#ifndef TOBASEEDITOR_H
#define TOBASEEDITOR_H

#include "core/toeditwidget.h"
#include "editor/tosearchreplace.h"

#include <QtGui/QWidget>

class toMarkedText;
class QFileSystemWatcher;

class toBaseEditor : public QWidget, public toEditWidget
{
	Q_OBJECT;

public:
	toBaseEditor(toMarkedText* editor, QWidget *parent = 0);

    /** Reimplemented from toEditWidget
     */
    virtual QString editText();
    virtual void editUndo(void);
    virtual void editRedo(void);
    virtual void editCut(void);
    virtual void editCopy(void);
    virtual void editPaste(void);
    virtual void editSelectAll(void);
    virtual void editPrint(void);
    virtual bool editOpen(const QString &suggestedFile = QString::null);
    virtual bool editSave(bool askfile);
	virtual bool searchNext();

	virtual void editReadAll();

    /** Get filename of current file in editor.
     * @return Filename of editor.
     */
    QString const& filename(void) const
    {
        return Filename;
    }

    /** Open a file for editing.
     * @param file File to open for editing.
     */
    void openFilename(const QString &file);

    /** Set the current filename of the file in editor.
     * @param str String containing filename.
     */
    void setFilename(const QString &str)
    {
        Filename = str;
    }

    /** Insert text and optionallly mark inserted text.
     * @param str String to insert.
     * @param mark True if mark inserted as selected.
     */
    virtual void insert(const QString &str, bool select = false);


    void findPosition(int index, int &line, int &col);

	// QsciScintilla wrapper
	QString text() const;
	QString text (int line) const;
	void setText(const QString &text);
	void append(const QString &text);

	void getCursorPosition(int *line, int *index) const;
	void setCursorPosition(int line, int index);

	int lines() const;
	int lineLength(int line) const;

	bool isReadOnly () const;
	void setReadOnly(bool ro);

	bool isModified () const;
	void setModified (bool m);

	void ensureLineVisible (int line);

    void setSelection(int lineFrom, int indexFrom, int lineTo, int indexTo);
    QString selectedText() const;
    bool hasSelectedText() const;

public slots:
	void setWordWrap(bool wrap);
	void setXMLWrap(bool wrap);
	void clear();

    virtual void searchReplace();

signals:
	// TODO/FIXME: simplify signals
	void displayMenu(QMenu *);
	// emitted when a new file is opened
	void fileOpened(void);
    void fileOpened(QString file);
    void fileSaved(QString file);

protected:
    toMarkedText *m_editor;

private:
    //! Filename of the file in this buffer.
    QString Filename;

    //! Watch for file (if any) changes from external apps
    QFileSystemWatcher * m_fsWatcher;

    toSearchReplace *m_search;

    void fsWatcherClear();

private slots:
	//! \brief Handle file external changes (3rd party modifications)
	void m_fsWatcher_fileChanged(const QString & filename);

    /** Update user interface with availability of copy/paste etc.
     */
    void setEditFlags(void);

    void handleSearching(Search::SearchFlags flags);
    void setCoordinates(int line, int column);

    void setEditorFocus();

    void gotFocus();
    void lostFocus();
};

#endif
