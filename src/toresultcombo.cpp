/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultcombo.h"
#include "tosql.h"
#include "totool.h"


toResultCombo::toResultCombo(QWidget *parent, const char *name)
        : QComboBox(parent), Query(0)
{
    setObjectName(name);
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    connect(this, SIGNAL(activated(int)),
            this, SLOT(changeSelected(void)));
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

toResultCombo::~toResultCombo()
{
}

void toResultCombo::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        clear();
        addItems(Additional);
        for (int i = 0;i < Additional.count();i++)
            if (Additional[i] == Selected)
                setCurrentIndex(i);

        Query = new toNoBlockQuery(connection(), toQuery::Background, sql, param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultCombo::changeSelected(void)
{
    Selected = currentText();
}

void toResultCombo::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            while (Query->poll() && !Query->eof())
            {
                QString t = Query->readValue();
                addItem(t);
                if (t == Selected)
                    setCurrentIndex(count() - 1);
            }

            if (Query->eof())
            {
                Poll.stop();
                setFont(font()); // Small hack to invalidate size hint of combobox which should resize to needed size.
                updateGeometry();
            }
        }
    }
    catch (const QString &exc)
    {
        Poll.stop();
        toStatusMessage(exc);
    }
}
