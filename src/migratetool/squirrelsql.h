/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef SQURRELSQL_H
#define SQURRELSQL_H

#include <QList>

#include "tonewconnection.h"

namespace MigrateTool
{

/*! \brief Parse SquirrelSQL connection files.
Version 2.6.x or later is supported only.
OK, this code looks ugly but it's enough for "one time runners".
\author Petr Vanek <petr@scribus.info>
*/
QMap<int,toConnectionOptions> squirrelSql(QWidget * parent = 0);

} // namespace

#endif
