
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

#ifndef TOTEXTVIEW_H
#define TOTEXTVIEW_H

#include "core/toeditwidget.h"
#include "core/utils.h"
#include "editor/toeditglobals.h"

class QTextBrowser;
class toSearchReplace;


/** A tora editwidget version of the @ref QTextEdit widget.
 */

class toTextView : public QWidget, public toEditWidget
{
        Q_OBJECT;
    public:

        toTextView(QWidget *parent = 0, const char *name = 0);

        void setFontFamily(const QString &fontFamily);
        void setReadOnly(bool ro);
        void setText(const QString &t);
        void setFilename(const QString &f);

        /** Reimplemented for internal reasons.
         */
        virtual void editCopy(void);
        /** Reimplemented for internal reasons.
         */
        virtual void editSelectAll(void);
        /** Reimplemented for internal reasons.
         */
        virtual bool editSave(bool);
        /** Reimplemented for internal reasons.
         */
        virtual void focusInEvent (QFocusEvent *e);

        virtual bool editOpen(const QString&)
        {
            return false;
        }
        virtual void editPrint() {}
        virtual void editUndo() {}
        virtual void editRedo() {}
        virtual void editCut() {}
        virtual void editPaste() {}
        virtual void editReadAll() {}
        virtual QString editText();


        virtual bool searchNext();
        virtual void searchReplace() {};

    private:
        QTextBrowser *m_view;
        toSearchReplace *m_search;
        QString m_filename;

    private slots:
        void setEditorFocus();
        void handleSearching(Search::SearchFlags flags);
};

#endif
