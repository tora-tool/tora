
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

#include "widgets/toresultlistformat.h"
#include "core/utils.h"
#include "core/tolistviewformatter.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include <QtCore/QSettings>

toResultListFormat::toResultListFormat(QWidget *parent, DialogType type, const char *name)
    : QDialog(parent)
{
    using namespace ToConfiguration;

    setupUi(this);

    connect(formatCombo, &QComboBox::activated, this, &formatChanged);

    setModal(true);
    formatCombo->addItem(tr("Text"));
    formatCombo->addItem(tr("Tab delimited"));
    formatCombo->addItem(tr("CSV"));
    formatCombo->addItem(tr("HTML"));
    formatCombo->addItem(tr("SQL"));

    int num = toConfigurationNewSingle::Instance().option(Global::DefaultListFormatInt).toInt();
    formatCombo->setCurrentIndex(num);
    formatChanged(num);

    bool useColumnHeaders = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ClipboardCHeadersBool).toBool();
    bool useRowHeaders = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ClipboardRHeadersBool).toBool();
    includeColumnHeaderCheck->setChecked(useColumnHeaders);
    includeRowHeaderCheck->setChecked(useRowHeaders);

    delimiterEdit->setText(toConfigurationNewSingle::Instance().option(Exporter::CsvDelimiter).toString());
    separatorEdit->setText(toConfigurationNewSingle::Instance().option(Exporter::CsvSeparator).toString());

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
                            toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ClipboardRHeadersBool).toBool(),
                            toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ClipboardCHeadersBool).toBool(),
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
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::DefaultListFormatInt, formatCombo->currentIndex());
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::ClipboardCHeadersBool, includeColumnHeaderCheck->isChecked());
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::ClipboardRHeadersBool, includeRowHeaderCheck->isChecked());

    QDialog::accept();
}
