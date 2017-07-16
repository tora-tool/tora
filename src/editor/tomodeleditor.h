
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

#pragma once

#include "editor/tomarkededitor.h"
#include "editor/tohighlightededitor.h"
#include "core/utils.h"

#include <QDialog>
#include <QLabel>
#include <QToolBar>
#include <QtCore/QModelIndex>

class QCheckBox;
class QToolBar;
class toBaseEditor;
class QAbstractItemModel;

#if TORA3_MEMOEDITOR
/**
 * A dialog for displaying and editing a row and column of a model
 */
class toModelEditor : public QDialog
{
        Q_OBJECT;

        typedef TMemoWithExec<toMarkedEditor, toModelEditor> toModelText;
        typedef TMemoWithExec<toHighlightedEditor, toModelEditor> toModelSQL;

    public:
        /**
         * Create this editor.
         *
         * @param parent Parent widget.
         * @param model data source
         * @param row Optional location specifier, pass on in @ref changeData call.
         * @param col Optional location specifier, pass on in @ref changeData call.
         * @param sql Use SQL syntax highlighting of widget.
         * @param modal Display in modal dialog
         */
        toModelEditor(QWidget *parent,
                      QAbstractItemModel *model,
                      QModelIndex current,
                      bool sql = false,
                      bool modal = true); // changed to true to prevent random crashes on tora exits with memo opened - petr vanek

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
        void openFile(void);
        void saveFile(void);
        void readSettings(void);
        void writeSettings(void) const;
        virtual void setNull(bool);

#ifdef EDITOR_REFACTORING
    protected:
        QToolBar *toolbar()
        {
            return Toolbar;
        }
        toMarkedEditor *editor()
        {
            return Editor;
        }
        QLabel *label()
        {
            return Label;
        }
#endif

    public slots:
        /** Save changes and close.
         */
        void store(void);

        /** Goto first column.
         */
        void firstColumn();

        /** Goto next column.
         */
        void nextColumn();

        /** Goto previous column.
         */
        void previousColumn();

        /** Goto last column.
         */
        void lastColumn();

        /** Change position in whatever it is your displaying.
         */
        void changePosition(QModelIndex index);

        /** Change value of editor.
         */
        void setText(const QString &);

        /** Retun next chunk of data(for write into a file
         */
        QByteArray nextData() const;
    private:
        // Editor of widget
        toBaseEditor *Editor;

        QToolBar  *Toolbar;
        QLabel    *Label;
        QCheckBox *NullCheck;

        bool                Editable;
        QModelIndex         Current;
        QAbstractItemModel *Model;

        mutable unsigned offset;
};

#endif
