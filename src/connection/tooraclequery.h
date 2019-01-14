
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#undef  TORA_MAKE_DLL
#ifndef TORA_DLL
#define TORA_DLL
#endif
#ifndef TROTL_DLL
#define TROTL_DLL
#endif

#include "core/toquery.h"
#include "core/toqueryimpl.h"
#include "connection/tooracleconnection.h"
#include "connection/tooracledatatype.h"

#include "trotl.h"
#include "trotl_convertor.h"

class oracleQuery : public queryImpl
{
        bool Cancel;
        bool Running;
        bool SaveInPool;

    public:
        class trotlQuery : public ::trotl::SqlStatement
        {
            public:
                typedef ::trotl::SqlStatement super;
                trotlQuery(::trotl::OciConnection &conn, const ::trotl::tstring &stmt, ub4 lang = OCI_NTV_SYNTAX, int bulk_rows =::trotl::g_OCIPL_BULK_ROWS);

                void readValue(toQValue &value);
        };
        trotlQuery * Query;

        oracleQuery(toQueryAbstr *query, toOracleConnectionSub *);

        virtual ~oracleQuery();

        virtual void execute(void);

        virtual void execute(QString const&);

        virtual toQValue readValue(void);

        virtual void cancel(void);

        virtual bool eof(void);

        virtual unsigned long rowsProcessed(void)
        {
            if (!Query)
            {
                TLOG(6, toDecorator, __HERE__) << "rowsProcessed() - non-query" << std::endl;
                return 0;
            }
            int retval;

            if ( Query->get_stmt_type() == ::trotl::SqlStatement::STMT_SELECT )
            {
                retval = Query->get_last_row();
            }
            else
            {
                retval = Query->row_count();
            }
            TLOG(6, toDecorator, __HERE__) << "rowsProcessed(" << retval << ")" << std::endl;
            return retval;
        }

        virtual unsigned columns(void)
        {
            //int descriptionLen;
            //Query->describe_select(descriptionLen);
            TLOG(6, toDecorator, __HERE__) << "columns(" << Query->get_column_count() << ")" << std::endl;
            return Query->get_column_count();
        }

        virtual toQColumnDescriptionList describe(void)
        {
            TLOG(6, toDecorator, __HERE__) << std::endl;
            toQColumnDescriptionList ret;

            int datatypearg1 = 0;
            int datatypearg2 = 0;
            const std::vector<trotl::DescribeColumn*> &col = Query->get_columns();
            if (col.empty()) return ret;
            std::vector<trotl::DescribeColumn*>::const_iterator it = col.begin();
            ++it; // starts with 1st column
            for (; it != col.end(); ++it)
            {
                toCache::ColumnDescription desc;
                desc.Name = QString::fromUtf8( (*it)->_name.c_str() );
                desc.Datatype = QString::fromUtf8( (*it)->typeName().c_str() );

                if (desc.Datatype == "NUMBER")
                    desc.AlignRight = true;
                else
                    desc.AlignRight = false;

                //datatypearg1 = description[i].char_size;
                desc.Datatype.sprintf(desc.Datatype.toUtf8().constData(), datatypearg1, datatypearg2);
                //desc.Null = description[i].nullok;

                ret.insert(ret.end(), desc);
            }

            return ret;
        }
}; //   class oracleQuery : public toQueryAbstr::queryImpl

