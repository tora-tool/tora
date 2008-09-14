/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOLISTVIEWFORMATTERSQL_H
#define TOLISTVIEWFORMATTERSQL_H

#include <map>
#include <list>

#include <qstring.h>

#include "tolistviewformatter.h"

typedef std::map<QString, int> SQLTypeMap;

class toListViewFormatterSQL : public toListViewFormatter
{

private:
    enum{SQL_STRING, SQL_NUMBER, SQL_DATE};
    SQLTypeMap getSQLTypes(toListView& tListView);
    std::list<QString> headerFields;
public:
    toListViewFormatterSQL();
    virtual ~toListViewFormatterSQL();
    virtual QString getFormattedString(toListView& tListView);
    virtual QString getFormattedString(toExportSettings &settings,
                                       const toResultModel *model);
};



#endif
