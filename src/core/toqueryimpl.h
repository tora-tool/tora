
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOQUERYIMPL_H
#define TOQUERYIMPL_H

#include "core/tocache.h"
#include "core/toqvalue.h"

class toQuery;

/** Abstract parent of implementations of a query for a database provider
 * (See @ref toConnection::connectionImpl and @ref toConnectionProvider)
 */
class queryImpl
{
    toQuery *Parent;
public:
    ///typedef toCacheNew::ColumnDescription ColumnDescription;

    /** Get the parent query object. All the parameters of the query must be read from here.
     * nothing is passed to the functions.
     */
    toQuery *query()
    {
        return Parent;
    }

    /** Create a query implementation. The constructor must not perform any actions with the
     * database that could block for a noticable time (Like execute or parse a query). The
     * data for the query may not be available when this object created.
     * @param query Parent query object.
     */
    queryImpl(toQuery *query)
        : Parent(query)
    { }
    /** Destroy query implementation.
     */
    virtual ~queryImpl()
    { }
    /** Execute a query. Parameters can be gotten from the @ref toQuery object.
     */
    virtual void execute(void) = 0;
    /** Read the next value from the stream.
     * @return The value read from the query.
     */
    virtual toQValue readValue(void) = 0;
    /** Check if the end of the query has been reached.
     * @return True if all values have been read.
     */
    virtual bool eof(void) = 0;
    /** Get the number of rows processed in the last executed query.
     */
    virtual unsigned long rowsProcessed(void) = 0;
    /** Describe the currently running query.
     * @return A list of column descriptions of the query.
     */
    virtual toQColumnDescriptionList describe(void) = 0;
    /** Get number of columns in the resultset.
     * @return Column number.
     */
    virtual unsigned columns(void) = 0;
    /** Cancel the current execution of a query. This will usually be called from another
     * thread than is executing the query.
     */
    virtual void cancel(void) = 0;

protected:
};


#endif
