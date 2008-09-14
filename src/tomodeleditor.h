
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

#ifndef TOMEMOEDITOR_H
#define TOMEMOEDITOR_H

#include "config.h"

#include <qdialog.h>
#include <QLabel>
#include <QToolBar>
#include <QModelIndex>

class QCheckBox;
class QToolBar;
class toMarkedText;
class QAbstractItemModel;


/**
 * A dialog for displaying and editing a row and column of a model.
 *
 */
class toModelEditor : public QDialog
{
    Q_OBJECT;

    // Editor of widget
    toMarkedText *Editor;

    QToolBar  *Toolbar;
    QLabel    *Label;
    QCheckBox *NullCheck;

    bool                Editable;
    QModelIndex         Current;
    QAbstractItemModel *Model;

private slots:
    void openFile(void);
    void saveFile(void);
    void readSettings(void);
    void writeSettings(void) const;
    virtual void setNull(bool);

protected:
    QToolBar *toolbar()
    {
        return Toolbar;
    }
    toMarkedText *editor()
    {
        return Editor;
    }
    QLabel *label()
    {
        return Label;
    }

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
                  bool modal = false);

public slots:
    /** Save changes and close.
     */
    virtual void store(void);

    /** Goto first column.
     */
    virtual void firstColumn();

    /** Goto next column.
     */
    virtual void nextColumn();

    /** Goto previous column.
     */
    virtual void previousColumn();

    /** Goto last column.
     */
    virtual void lastColumn();

    /** Change position in whatever it is your displaying.
     */
    virtual void changePosition(QModelIndex index);

    /** Change value of editor.
     */
    virtual void setText(const QString &);
};

#endif
