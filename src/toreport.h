/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOREPORT_H
#define TOREPORT_H

#include "config.h"

#include <list>
#include <algorithm>

#include <qstring.h>

/** Generate an HTML based report on a description of database objects from @ref toExtract.
 * @param conn Connection description list was generated from.
 * @param desc Description list to create report on.
 * @return An HTML formatted report on the contents of @ref desc.
 */
QString toGenerateReport(toConnection &conn, std::list<QString> &desc);

#endif
