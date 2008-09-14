
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

#include "config.h"
#include "toconf.h"
#include "utils.h"

#include "toresultlistformat.h"
#include "toconfiguration.h"


toResultListFormat::toResultListFormat(QWidget *parent, const char *name)
        : QDialog(parent)
{

    setupUi(this);
    setModal(true);
    Format->addItem(tr("Text"));
    Format->addItem(tr("Tab delimited"));
    Format->addItem(tr("CSV"));
    Format->addItem(tr("HTML"));
    Format->addItem(tr("SQL"));
    int num = toConfigurationSingle::Instance().defaultFormat();
    Format->setCurrentIndex(num);
    formatChanged(num);

    Delimiter->setText(toConfigurationSingle::Instance().csvDelimiter());
    Separator->setText(toConfigurationSingle::Instance().csvSeparator());
}


void toResultListFormat::formatChanged(int pos)
{
    Separator->setEnabled(pos == 2);
    Delimiter->setEnabled(pos == 2);
}


void toResultListFormat::saveDefault(void)
{
// NOTE: unused in the whole app at all
//     toConfigurationSingle::Instance().globalSetConfig(
//         CONF_CSV_DELIMITER,
//         Delimiter->text());
//     toConfigurationSingle::Instance().globalSetConfig(
//         CONF_CSV_SEPARATOR,
//         Separator->text());
    toConfigurationSingle::Instance().setDefaultFormat(Format->currentIndex());
}
