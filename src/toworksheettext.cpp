/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2008 Quest Software, Inc
 * Portions Copyright (C) 2008 Other Contributors
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#include "config.h"

#include "toworksheettext.h"

#include "utils.h"
#include "toworksheet.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>


toWorksheetText::toWorksheetText(toWorksheet *worksheet,
                                 QWidget *parent,
                                 const char *name)
        : toHighlightedText(parent, name), Worksheet(worksheet) { }


bool toWorksheetText::editSave(bool askfile)
{
    bool ret = toHighlightedText::editSave(askfile);
    Worksheet->setCaption();
    return ret;
}


bool toWorksheetText::editOpen(QString suggestedFile)
{
    int ret = 1;
    if (isModified())
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
    {
        QFileInfo file(filename());
        fname = toOpenFilename(file.dir().path(), QString::null, this);
    }
    if (fname.isEmpty())
        return false;
    try
    {
        if (ret == 2)
            toWorksheet::fileWorksheet(fname);
        else
        {
            openFilename(fname);
            Worksheet->setCaption();
        }

        emit fileOpened();
        return true;
    }
    TOCATCH;

    return false;
}

