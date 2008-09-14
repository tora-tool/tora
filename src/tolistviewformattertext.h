/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERTEXT_H
#define TOLISTVIEWFORMATTERTEXT_H

#include "config.h"
#include "tolistviewformatter.h"

class toListViewFormatterText : public toListViewFormatter
{
public:
    toListViewFormatterText();
    virtual ~toListViewFormatterText();
    virtual QString getFormattedString(toListView& tListView);
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model);
};

#endif
