
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

#include <QSettings>

#include "config.h"
#include "toconf.h"
#include "utils.h"

#include "tolistviewformatter.h"
#include "toresultlistformat.h"
#include "toconfiguration.h"


toResultListFormat::toResultListFormat(QWidget *parent, DialogType type, const char *name)
        : QDialog(parent)
{

    setupUi(this);
    setModal(true);
    formatCombo->addItem(tr("Text"));
    formatCombo->addItem(tr("Tab delimited"));
    formatCombo->addItem(tr("CSV"));
    formatCombo->addItem(tr("HTML"));
    formatCombo->addItem(tr("SQL"));

    int num = toConfigurationSingle::Instance().defaultFormat();
    formatCombo->setCurrentIndex(num);
    formatChanged(num);

    delimiterEdit->setText(toConfigurationSingle::Instance().csvDelimiter());
    separatorEdit->setText(toConfigurationSingle::Instance().csvSeparator());

    selectedRowsRadio->setChecked(type == TypeCopy);
    selectedColumnsRadio->setChecked(type == TypeCopy);

    allRowsRadio->setChecked(type == TypeExport);
    allColumnsRadio->setChecked(type == TypeExport);
}

toExportSettings toResultListFormat::exportSettings()
{
    toExportSettings::RowExport r;
    toExportSettings::ColumnExport c;

    if (selectedRowsRadio->isChecked()) r = toExportSettings::RowsSelected;
    else if (displayedRowsRadio->isChecked()) r = toExportSettings::RowsDisplayed;
    else r = toExportSettings::RowsAll;

    if (selectedColumnsRadio->isChecked()) c = toExportSettings::ColumnsSelected;
    else c = toExportSettings::ColumnsAll;

    return toExportSettings(r,
                     c,
                     formatCombo->currentIndex(),
                     includeRowHeaderCheck->isChecked(),
                     includeColumnHeaderCheck->isChecked(),
                     separatorEdit->text(),
                     delimiterEdit->text());
}

toExportSettings toResultListFormat::plaintextCopySettings()
{
    return toExportSettings(toExportSettings::RowsSelected,
                     toExportSettings::ColumnsSelected,
                     0,
                     false,
                     false,
                     "",
                     "");
}

void toResultListFormat::formatChanged(int pos)
{
    separatorEdit->setEnabled(pos == 2);
    delimiterEdit->setEnabled(pos == 2);
}


void toResultListFormat::accept()
{
    toConfigurationSingle::Instance().setDefaultFormat(formatCombo->currentIndex());
    QDialog::accept();
}
