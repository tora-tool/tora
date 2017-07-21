
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

#include "tools/toworksheeteditor.h"
#include "tools/toworksheet.h"
#include "editor/toworksheettext.h"

#include "core/utils.h"

#include <QMessageBox>
#include <QtCore/QtDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

toWorksheetEditor::toWorksheetEditor(toWorksheet *worksheet,
                                     QWidget *parent,
                                     const char *name)
    : toBaseEditor(new toWorksheetText(NULL), parent)
    , Worksheet(worksheet)
{ }

toSqlText* toWorksheetEditor::editor()
{
    return qobject_cast<toSqlText*>(m_editor);
}

bool toWorksheetEditor::editSave(bool askfile)
{
    bool ret = super::editSave(askfile);
    Worksheet->slotSetCaption();
    return ret;
}

bool toWorksheetEditor::editOpen(const QString &suggestedFile)
{
    int ret = 1;
    if (sciEditor()->isModified())
    {
        // grab focus so user can see file and decide to save
        setFocus(Qt::OtherFocusReason);

        ret = TOMessageBox::information(
                  this,
                  qApp->translate("toWorksheetText", "Save changes?"),
                  qApp->translate(
                      "toWorksheetText",
                      "The editor has been changed. Do you want to save them,\n"
                      "discard changes or open file in new worksheet?"),
                  qApp->translate("toWorksheetText", "&Save"),
                  qApp->translate("toWorksheetText", "&Discard"),
                  qApp->translate("toWorksheetText", "&New worksheet"), 0);

        if (ret < 0)
            return false;
        else if (ret == 0)
        {
            if (!editSave(false))
                return false;
        }
    }

    QString fname;
    if (suggestedFile != QString::null)
        fname = suggestedFile;
    else
        fname = Utils::toOpenFilename(QString::null, this);
    if (fname.isEmpty())
        return false;
    try
    {
        if (ret == 2)
            toWorksheet::fileWorksheet(fname);
        else
        {
            openFilename(fname);
            Worksheet->slotSetCaption();
        }

        emit fileOpened();
        emit fileOpened(fname);
        return true;
    }
    TOCATCH;

    return false;
}

void toWorksheetEditor::focusInEvent(QFocusEvent *e)
{
    qDebug() << ">>> toWorksheetEditor::focusInEvent" << this;
    super::focusInEvent(e);
}

void toWorksheetEditor::focusOutEvent(QFocusEvent *e)
{
    qDebug() << ">>> toWorksheetEditor::focusOutEvent" << this;
    super::focusOutEvent(e);
}
