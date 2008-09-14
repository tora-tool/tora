/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORA3_H
#define TORA3_H

#include <QList>

#include "tonewconnection.h"

namespace MigrateTool
{

/*! \brief Parse Tora 1.3.x preferences file
 *
 * Probably works for older versions, too. But does anybody care?
 *
 * \author Mike Johnson
 */
QMap<int,toConnectionOptions> tora3(QWidget *parent = 0);

} // namespace

#endif
