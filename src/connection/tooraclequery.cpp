
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

#include "connection/tooraclequery.h"

oracleQuery::oracleQuery(toQueryAbstr *query, toOracleConnectionSub *) : queryImpl(query)
{
    TLOG(6, toDecorator, __HERE__) << std::endl;
    Running = Cancel = false;
    SaveInPool = false;
    Query = NULL;
}

oracleQuery::~oracleQuery()
{
    if (!Query)
        return;

    Query->close();
    delete Query;
}

void oracleQuery::execute(void)
{
    toOracleConnectionSub *conn = dynamic_cast<toOracleConnectionSub*>(query()->connectionSubPtr());
    if (!conn)
        throw QString::fromLatin1("Internal error, not an Oracle sub connection");
    try
    {
        if (Query) delete Query;
        Query = NULL;

        if (Cancel)
            throw QString::fromLatin1("Query aborted before started");
        Running = true;

        QRegExp stripnl("\r");
        QString sql = this->query()->sql();
        sql.replace(stripnl, "");

        Query = new oracleQuery::trotlQuery(*conn->_conn, ::std::string(sql.toUtf8().constData()));
        TLOG(0, toDecorator, __HERE__) << "SQL(conn=" << conn->_conn << ", this=" << Query << "): " << ::std::string(sql.toUtf8().constData()) << std::endl;
        conn->_hasTransaction = toOracleConnectionSub::DIRTY_FLAG;
        // TODO autocommit ??
        // Query->set_commit(0);
    }
    catch (const ::trotl::OciException &exc)
    {
        delete Query;
        Query = NULL;
        Running = false;
        if (exc.is_critical())
            conn->Broken = true;
        ReThrowException(exc);
    }
    try
    {
        for (toQueryParams::const_iterator i = query()->params().begin(); i != query()->params().end(); i++)
        {
            const ::trotl::BindPar& bp = (*Query).get_curr_in_bindpar();

            if ( (bp._bind_type & ( ::trotl::BindPar::BIND_IN | ::trotl::BindPar::BIND_OUT )) == 0)
                throw toConnection::exception(QString::fromLatin1("Fatal pruser error - invalid BindPar"));

            if ( bp._bind_typename == "int" /*&& (*i).isInt()*/ )
            {
                (*Query) << (*i).toInt();
                TLOG(0, toDecorator, __HERE__)
                        << "<<(conn=" << conn->_conn << ", this=" << Query << ")"
                        << "::operator<<(" << bp._type_name << " ftype=" << bp.dty
                        << ", placeholder=" << bp._bind_name
                        << ", value=" << (*i).toInt() << ");"
                        << "\t of:" << query()->params().size() << std::endl;
            }
            else if ( (bp._bind_typename == "char" || bp._bind_typename == "varchar")
                      && ((*i).isString() || (*i).isNumber() || (*i).isNull() ))
            {
                std::string param((const char*)(((QString)*i).toUtf8().constData()));
                (*Query) << param;
                TLOG(0, toDecorator, __HERE__)
                        << "<<(conn=" << conn->_conn << ", this=" << Query << ")"
                        << "::operator<<(" << bp._type_name << " ftype=" << bp.dty
                        << ", placeholder=" << bp._bind_name
                        << ", value=" << ::std::string((const char*)(((QString)*i).toUtf8().constData())) << ");"
                        << "\t of:" << query()->params().size() << std::endl;
            }
            else
            {
                TLOG(0, toDecorator, __HERE__)
                        << "Fatal pruser error - unsupported BindPar: " << bp._bind_typename << std::endl
                        << " For SQL: \n" << query()->sql() << std::endl
                        << "<<(conn=" << conn->_conn << ", this=" << Query << ")"
                        << "::operator<<(" << bp._type_name << " ftype=" << bp.dty
                        << ", placeholder=" << bp._bind_name
                        << ", value=" << ::std::string((const char*)(((QString)*i).toUtf8().constData())) << ");"
                        << "\t of:" << query()->params().size() << std::endl;
                throw toConnection::exception(
                    QString::fromLatin1("Fatal pruser error - unsupported BindPar:%1\nFor SQL:\n%2\n")
                    .arg(bp._bind_typename.c_str())
                    .arg(query()->sql()));
                exit(-1);
            }
        }

        //Running = false;
    }
    catch (const ::trotl::OciException &exc)
    {
        TLOG(0, toDecorator, __HERE__)
                << "What:" << exc.what() << std::endl
                << exc.get_sql() << std::endl
                << "--------------------------------------------------------------------------------"
                << std::endl;
        try
        {
            conn->_conn->reset();
        }
        catch (const ::trotl::OciException &e2)
        {
            TLOG(0, toDecorator, __HERE__)
                    << "Exception in exception What:\n" << e2.what() << std::endl
                    << e2.get_sql() << std::endl
                    << "--------------------------------------------------------------------------------"
                    << std::endl;
        }
        try
        {
            conn->_conn->reset();
        }
        catch (const ::trotl::OciException &e2)
        {
            TLOG(0, toDecorator, __HERE__)
                    << "Exception in exception What:\n" << e2.what() << std::endl
                    << e2.get_sql() << std::endl
                    << "--------------------------------------------------------------------------------"
                    << std::endl;
        }
        delete Query;
        Query = NULL;
        Running = false;
        if (exc.is_critical())
            conn->Broken = true;
        ReThrowException(exc);
    }
}

void oracleQuery::execute(QString const& sql)
{
    toOracleConnectionSub *conn = dynamic_cast<toOracleConnectionSub*>(query()->connectionSubPtr());
    if (!conn)
        throw QString::fromLatin1("Internal error, not an Oracle sub connection");
    try
    {
        if (Query) delete Query;
        Query = NULL;

        if (Cancel)
            throw QString::fromLatin1("Query aborted before started");
        Running = true;

        Query = new oracleQuery::trotlQuery(*conn->_conn, ::std::string(sql.toUtf8().constData()));
        TLOG(0, toDecorator, __HERE__) << "SQL(conn=" << conn->_conn << ", this=" << Query << "): " << ::std::string(sql.toUtf8().constData()) << std::endl;
        // TODO autocommit ??
        // Query->set_commit(0);
        Query->execute_internal(::trotl::g_OCIPL_BULK_ROWS, OCI_DEFAULT);
        Running = false;
    }
    catch (const ::trotl::OciException &exc)
    {
        delete Query;
        Query = NULL;
        Running = false;
        if (exc.is_critical())
            conn->Broken = true;
        ReThrowException(exc);
    }
}

toQValue oracleQuery::readValue(void)
{
    toOracleConnectionSub *conn = dynamic_cast<toOracleConnectionSub*>(query()->connectionSubPtr());
    try
    {
        toQValue retval;
        Query->readValue(retval);
        return retval;
    }
    catch (const ::trotl::OciException &exc)
    {
        delete Query;
        Query = NULL;
        Running = false;
        if (exc.is_critical())
            conn->Broken = true;
        ReThrowException(exc);
    }
}

void oracleQuery::cancel(void)
{
    toOracleConnectionSub *conn = dynamic_cast<toOracleConnectionSub*>(query()->connectionSubPtr());
    if (!Running || Cancel)
    {
        TLOG(0, toDecorator, __HERE__) << ":oracleQuery::cancel(conn=" << conn->_conn << ", this=" << Query << ") on non-running query" << std::endl;
        return;
    }

    if (!conn)
        throw QString::fromLatin1("Internal error, not an Oracle sub connection");

    conn->_conn->cancel();
    conn->_conn->reset();
    Cancel = true;
    Running = false;
    TLOG(0, toDecorator, __HERE__) << ":oracleQuery::cancel(conn=" << conn->_conn << ", this=" << Query << ")" << std::endl;
}

bool oracleQuery::eof(void)
{
    if (!Query || Cancel)
    {
        TLOG(6, toDecorator, __HERE__) << "eof - on canceled query" << std::endl;
        return true;
    }
    try
    {
        bool e = Query->eof();
        if (e)
        {
            Running = false;
            TLOG(6, toDecorator, __HERE__) << "eof(" << Query->row_count() << ')' << std::endl;
        }
        return e; //Query->eof();
    }
    catch (const ::trotl::OciException &exc)
    {
        TLOG(6, toDecorator, __HERE__) << "eof(e) - " << exc.what() << std::endl;
        if (query())
        {
            toOracleConnectionSub *conn = dynamic_cast<toOracleConnectionSub *>(query()->connectionSubPtr());
            if (conn)
            {
                if (exc.is_critical())
                    conn->Broken = true;
                ReThrowException(exc);
            }
        }
        return true;
    }
    return true; // never reached
}


oracleQuery::trotlQuery::trotlQuery(::trotl::OciConnection &conn,
                                    const ::trotl::tstring &stmt,
                                    ub4 lang,
                                    int bulk_rows)
    : ::trotl::SqlStatement(conn, stmt, lang, bulk_rows)
{
    // Be compatible with otl, execute some statements immediately
    if ( get_stmt_type() == STMT_ALTER
            || get_stmt_type() == STMT_OTHER // ANALYZE
            // _in_cnt == 0
       )
        execute_internal(::trotl::g_OCIPL_BULK_ROWS, OCI_DEFAULT);
};

void oracleQuery::trotlQuery::readValue(toQValue &value)
{
    pre_read_value();
    trotl::BindPar const &BP(get_stmt_type() == STMT_SELECT ?
                             get_next_column() :
                             get_next_out_bindpar());

    if (BP.is_null(_last_buff_row) && BP.dty != SQLT_NTY)
    {
        value = toQValue();
        //TLOG(4, toDecorator, __HERE__) << "Just read: NULL" << std::endl;
    }
    else
    {
        switch (BP.dty)
        {
            case SQLT_NUM:
            case SQLT_VNU:
                {
                    OCINumber* vnu = (OCINumber*) & ((char*)BP.valuep)[_last_buff_row * BP.value_sz ];
                    sword res;
                    boolean isint;
                    res = OCINumberIsInt(_errh, vnu, &isint);
                    oci_check_error(__HERE__, _errh, res);
                    try
                    {
                        if (isint)
                        {
                            long long i;
                            res = OCINumberToInt(_errh,
                                                 vnu,
                                                 sizeof(long long),
                                                 OCI_NUMBER_SIGNED,
                                                 &i);
                            oci_check_error(__HERE__, _errh, res);
                            value = toQValue(i);
                            //TLOG(4, toDecorator, __HERE__) << "Just read: '" << i << '\'' << std::endl;
                        }
                        else
                        {
                            double d;
                            sword res = OCINumberToReal(_errh,
                                                        vnu,
                                                        sizeof(double),
                                                        &d);
                            oci_check_error(__HERE__, _errh, res);
                            value = toQValue(d);
                            //TLOG(4, toDecorator, __HERE__) << "Just read: '" << d << '\'' << std::endl;
                        }
                    }
                    catch (const ::trotl::OciException &e)
                    {
                        text str_buf[65];
                        ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);
                        //const char fmt[]="99999999999999999999999999999999999999D00000000000000000000";
                        const char fmt[] = "TM";
                        sword res = OCINumberToText(_errh,
                                                    vnu,
                                                    (const oratext*)fmt,
                                                    sizeof(fmt) - 1,
                                                    0, // CONST OraText *nls_params,
                                                    0, // ub4 nls_p_length,
                                                    (ub4*)&str_len,
                                                    str_buf );
                        oci_check_error(__HERE__, _env._errh, res);
                        str_buf[str_len + 1] = '\0';
                        value = toQValue(QString::fromUtf8((const char*)str_buf));
                    }
                }
                break;
            case SQLT_NTY:
                {
#ifdef ORACLE_HAS_XML
                    if ( ::trotl::BindParXML const *bpx = dynamic_cast<const trotl::BindParXML *>(&BP))
                    {
                        if ((xmlnode*)bpx->_xmlvaluep[_last_buff_row] == NULL)
                        {
                            value = toQValue();
                            //TLOG(4, toDecorator, __HERE__) << "Just read: NULL XML" << std::endl;
                        }
                        else
                        {
                            std::string s(BP.get_string(_last_buff_row));
                            value = toQValue(QString::fromUtf8(s.c_str()));
                            //TLOG(4, toDecorator, __HERE__) << "Just read: \"" << s << "\"" << std::endl;
                        }
                    }
#endif
                    if ( ::trotl::BindParANYDATA const *bpa = dynamic_cast<const trotl::BindParANYDATA *>(&BP))
                    {
                        if ( bpa->_oan_buffer[_last_buff_row] == NULL)
                        {
                            value = toQValue();
                        }
                        else
                        {
                            std::string s(BP.get_string(_last_buff_row));
                            value = toQValue(QString::fromUtf8(s.c_str()));
                        }
                    }
                    else if ( ::trotl::BindParCollectionTabNum const *bpc = dynamic_cast<const trotl::BindParCollectionTabNum *>(&BP))
                    {
                        if ( *(sb2*)(bpc->_collection_indp[_last_buff_row]) == OCI_IND_NULL)
                        {
                            value = toQValue();
                            //TLOG(4, toDecorator, __HERE__) << "Just read: NULL collection" << std::endl;
                        }
                        else
                        {
                            toOracleCollection *i = new toOracleCollection(_conn);
                            trotl::ConvertorForRead c(_last_buff_row);
                            trotl::DispatcherForRead::Go(BP, i->data, c);
                            QVariant v;
                            v.setValue((toQValue::complexType*)i);
                            value = toQValue::fromVariant(v);
                            //TLOG(4, toDecorator, __HERE__) << "Just read: collection:" << (::trotl::tstring)i->data << std::endl;
                        }
                    }
                    else if ( ::trotl::BindParCollectionTabVarchar const *bpc = dynamic_cast<const trotl::BindParCollectionTabVarchar *>(&BP))
                    {
                        if ( *(sb2*)(bpc->_collection_indp[_last_buff_row]) == OCI_IND_NULL)
                        {
                            value = toQValue();
                            //TLOG(4, toDecorator, __HERE__) << "Just read: NULL collection" << std::endl;
                        }
                        else
                        {
                            toOracleCollection *i = new toOracleCollection(_conn);
                            trotl::ConvertorForRead c(_last_buff_row);
                            trotl::DispatcherForRead::Go(BP, i->data, c);
                            QVariant v;
                            v.setValue((toQValue::complexType*)i);
                            value = toQValue::fromVariant(v);
                            //TLOG(4, toDecorator, __HERE__) << "Just read: collection:" << (::trotl::tstring)i->data << std::endl;
                        }
                    }

                }
                break;
            case SQLT_CLOB:
            case SQLT_CFILE:
                {
                    toOracleClob *i = new toOracleClob(_conn);
                    trotl::ConvertorForRead c(_last_buff_row);
                    trotl::DispatcherForRead::Go(BP, i->_data, c);
                    QVariant v;
                    v.setValue((toQValue::complexType*)i);
                    value = toQValue::fromVariant(v);
                    //int id = qMetaTypeId<toQValue::complexType*>();
                    //TLOG(4, toDecorator, __HERE__) << "Just read: \"CLOB\"" << std::endl;
                }
                break;
            case SQLT_BLOB:
            case SQLT_BFILE:
                {
                    toOracleBlob *i = new toOracleBlob(_conn);
                    trotl::ConvertorForRead c(_last_buff_row);
                    trotl::DispatcherForRead::Go(BP, i->data, c);
                    QVariant v;
                    v.setValue((toQValue::complexType*)i);
                    value = toQValue::fromVariant(v);
                    //TLOG(4, toDecorator, __HERE__) << "Just read: \"BLOB\"" << std::endl;
                }
                break;
            case SQLT_RSET:
                {
                    toOracleCursor *i = new toOracleCursor();
                    trotl::ConvertorForRead c(_last_buff_row);
                    trotl::DispatcherForRead::Go(BP, i->data, c);
                    QVariant v;
                    v.setValue((toQValue::complexType*)i);
                    value = toQValue::fromVariant(v);
                    //TLOG(4, toDecorator, __HERE__) << "Just read: \"CURSOR\"" << std::endl;
                }
                break;
            default:
                std::string s(BP.get_string(_last_buff_row));
                value = toQValue(QString::fromUtf8(s.c_str()));
                //TLOG(4, toDecorator, __HERE__) << "Just read: \"" << s << "\"" << std::endl;
                break;
        }
    }

    post_read_value(BP);
}
