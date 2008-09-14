/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef SQLDEVELOPER_H
#define SQLDEVELOPER_H

#include <QList>

#include "tonewconnection.h"

namespace MigrateTool
{

/*! \brief Parse Oracle SQL Developer connection export file.
Version 1.5 or later ("new" format) is supported only.
OK, this code looks ugly but it's enough for "one time runners".
\author Petr Vanek <petr@scribus.info>
*/
QMap<int,toConnectionOptions> sqlDeveloper(QWidget * parent = 0);

} // namespace

#endif
