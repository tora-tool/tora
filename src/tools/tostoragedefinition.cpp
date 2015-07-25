
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

#include "tools/tostoragedefinition.h"
#include "tools/tofilesize.h"

#include <QToolTip>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

toStorageDefinition::toStorageDefinition(QWidget* parent, const char* name, toWFlags fl)
    : QWidget(parent)
{
    setupUi(this);

    InitialSize->setTitle(tr("&Initial size"));
    InitialSize->setValue(50);

    NextSize->setTitle(tr("&Next size"));
    NextSize->setValue(50);

    OptimalSize->setTitle(tr("&Size"));
    OptimalSize->setEnabled(false);
}

std::list<QString> toStorageDefinition::sql(void)
{
    QString str(QString::fromLatin1("STORAGE (INITIAL "));
    str.append(InitialSize->sizeString());
    str.append(QString::fromLatin1(" NEXT "));
    str.append(NextSize->sizeString());
    str.append(QString::fromLatin1(" MINEXTENTS "));
    str.append(InitialExtent->text());
    str.append(QString::fromLatin1(" MAXEXTENTS "));
    if (UnlimitedExtent->isChecked())
        str.append(QString::fromLatin1("UNLIMITED"));
    else
        str.append(MaximumExtent->text());
    if (!Optimal->isEnabled())
    {
        str.append(QString::fromLatin1(" PCTINCREASE "));
        str.append(PCTIncrease->text());
    }

    if (!OptimalNull->isChecked())
    {
        str.append(QString::fromLatin1(" OPTIMAL "));
        str.append(OptimalSize->sizeString());
    }
    str.append(QString::fromLatin1(")"));
    std::list<QString> ret;
    ret.insert(ret.end(), str);
    return ret;
}

void toStorageDefinition::forRollback(bool val)
{
    Optimal->setEnabled(val);
    PCTIncrease->setEnabled(!val);
    InitialExtent->setMinimum(2);
    InitialExtent->setValue(4);
}
