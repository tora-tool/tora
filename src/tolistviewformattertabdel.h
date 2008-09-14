/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERTABDEL_H
#define TOLISTVIEWFORMATTERTABDEL_H

#include "config.h"
#include "tolistviewformatter.h"

class toListViewFormatterTabDel : public toListViewFormatter
{
public:
    toListViewFormatterTabDel();
    virtual ~toListViewFormatterTabDel();
    virtual QString getFormattedString(toListView& tListView);
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model);
};

#endif
