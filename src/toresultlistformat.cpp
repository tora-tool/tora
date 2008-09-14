/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
