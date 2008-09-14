/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERCSV_H
#define TOLISTVIEWFORMATTERCSV_H

#include "config.h"
#include "tolistviewformatter.h"

class toListViewFormatterCSV : public toListViewFormatter
{
private:
    QString QuoteString(const QString &str);

public:
    toListViewFormatterCSV();
    virtual ~toListViewFormatterCSV();
    virtual QString getFormattedString(toListView& tListView);
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model);
};

#endif
