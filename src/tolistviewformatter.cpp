/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "tolistviewformatter.h"

toListViewFormatter::toListViewFormatter()
{
}

toListViewFormatter::~toListViewFormatter()
{
}

void toListViewFormatter::endLine(QString &output)
{
#ifdef Q_OS_WIN32
    output += "\r\n";
#else
    output += "\n";
#endif
}

QVector<int> toListViewFormatter::selectedList(
    const QModelIndexList &selected)
{

    QVector<int> ret;
    for (QList<QModelIndex>::const_iterator it = selected.begin();
            it != selected.end();
            it++)
    {

        ret.append((*it).row());
    }

    return ret;
}
