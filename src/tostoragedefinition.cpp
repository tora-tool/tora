/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "config.h"

#include <qtooltip.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#include "tofilesize.h"
#include "tostoragedefinition.h"


toStorageDefinition::toStorageDefinition(QWidget* parent, const char* name, Qt::WFlags fl)
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
