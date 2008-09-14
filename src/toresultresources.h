/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTRESOURCES_H
#define TORESULTRESOURCES_H

#include "config.h"
#include "toresultitem.h"
#include "tosql.h"

#define TOSQL_RESULTRESOURCE

/** This widget displays information about resources of a query. The statement
 * is identified by the first parameter which should be the address as gotten
 * from the @ref toSQLToAddress function.
 */

class toResultResources : public toResultItem
{
public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultResources(QWidget *parent, const char *name = NULL); // Implemented in toresultitem.cpp
};

#endif
