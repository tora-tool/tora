
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

#ifndef TOBASEEDITOR_H
#define TOBASEEDITOR_H

#include "core/toeditwidget.h"
#include <QWidget>
#include "widgets/tosearchreplace.h"

class toScintilla;
class QFileSystemWatcher;

/** This class implements toEditWidget API (thus connects this widget to TOra's edit menus)
 *	It connects class toScintilla instance with with toSearchReplace.
 */
class toBaseEditor : public QWidget, public toEditWidget
{
        Q_OBJECT;

    public:
        toBaseEditor(toScintilla* editor, QWidget *parent = 0);

        /** Reimplemented from toEditWidget
         */
        virtual FlagSetStruct flagSet();
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

        toScintilla* operator->()
        {
            return m_editor;
        }

        toScintilla const* operator->() const
        {
            return m_editor;
        }

        toScintilla *sciEditor()
        {
            return m_editor;
        }

    public slots:
        void setWordWrap(bool wrap);
        void clear();

        virtual void searchReplace();

    signals:
        // emitted when a new file is opened
        void fileOpened(void);
        void fileOpened(QString file);
        void fileSaved(QString file);

    protected:
        toScintilla *m_editor;

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

    private:
        //! Filename of the file in this buffer.
        QString Filename;

        //! Watch for file (if any) changes from external apps
        QFileSystemWatcher * m_fsWatcher;

        toSearchReplace *m_search;

        void fsWatcherClear();
};

#endif
