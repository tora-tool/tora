/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERHTML_H
#define TOLISTVIEWFORMATTERHTML_H

#include "config.h"
#include "tolistviewformatter.h"

class toListViewFormatterHTML : public toListViewFormatter
{
public:
    toListViewFormatterHTML();
    virtual ~toListViewFormatterHTML();
    virtual QString getFormattedString(toListView& tListView);
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model);
};

#endif
