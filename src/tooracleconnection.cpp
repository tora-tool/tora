
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifndef TO_NO_ORACLE

#include "utils.h"

#ifdef Q_OS_WIN32
#  include "windows/cregistry.h"
#include <Windows.h>
#endif

/**
 * mrj: disabled stream pooling. it doesn't seem to be fully thread
 * safe. if you're hitting the database hard, say the tuning overview
 * tab with a refresh of 2 seconds, you get consistent crashes.
 */
//#define OTL_STREAM_POOLING_ON

#define OTL_STL
#define OTL_EXCEPTION_ENABLE_ERROR_OFFSET
#define OTL_ORA_UTF8
#define OTL_ORA_UNICODE
#define OTL_ORA_TIMESTAMP
#define OTL_ANSI_CPP
// mrj: disabled, otl_stream::eof sometimes throws an unexpected
// exception which causes a crash
// #define OTL_FUNC_THROW_SPEC_ON

#if 1
#if defined(OTL_ORA10G) || defined(OTL_ORA10G_R2) || defined(OTL_ORA11G)
#define OTL_ORA_OCI_ENV_CREATE
#define OTL_ORA_OCI_ENV_CREATE_MODE OCI_THREADED|OCI_OBJECT|OCI_EVENTS
#endif
#endif

#if 0 /* OTL tracing */
#define OTL_TRACE_LEVEL 0xff
#define OTL_TRACE_STREAM cerr
#include <iostream>
using namespace std;
#endif

#include "otlv4.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"
#include "tooraclesetting.h"

#include <stdio.h>
#include <iostream>

#include <qcheckbox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qvalidator.h>

#include <QString>
#include <QTextStream>

static int toMaxLong = toConfigurationSingle::Instance().maxLong();

static toSQL SQLComment("toOracleConnection:Comments",
                        "SELECT Column_name,Comments FROM sys.All_Col_Comments\n"
                        " WHERE Owner = :f1<char[100]>\n"
                        "   AND Table_Name = :f2<char[100]>",
                        "Display column comments");

static toSQL SQLMembers("toOracleConnection:Members",
                        "SELECT object_name,overload,argument_name,data_type\n"
                        "  FROM sys.All_Arguments\n"
                        " WHERE Owner = :f1<char[100]>\n"
                        "   AND Package_Name = :f2<char[100]>\n"
                        " ORDER BY object_name,overload,DECODE(argument_name,NULL,9999,sequence)",
                        "Get list of package members");

static toSQL SQLListObjects("toOracleConnection:ListObjects",
                            "select a.owner,a.object_name,a.object_type,b.comments\n"
                            "  from sys.all_objects a,\n"
                            "       sys.all_tab_comments b\n"
                            " where a.owner = b.owner(+) and a.object_name = b.table_name(+)\n"
                            "   and a.object_type = b.table_type(+) and a.object_type != 'SYNONYM'",
                            "List the objects to cache for a connection, should have same "
                            "columns and binds");

static toSQL SQLListSynonyms("toOracleConnection:ListSynonyms",
                             "select synonym_name,table_owner,table_name\n"
                             "  from sys.all_synonyms\n"
                             " where owner = :usr<char[101]> or owner = 'PUBLIC'\n"
                             " order by table_owner,table_name",
                             "List the synonyms available to a user, should have same columns and binds");

static void ThrowException(const otl_exception &exc)
{
    if (exc.code == 24344)
        throw toConnection::exception(QString::fromLatin1("ORA-24344 success with compilation error"));
    else
    {
        toConnection::exception ret(QString::fromUtf8((const char *)exc.msg));

        if (ret.isEmpty())
        {
            if (exc.code != 0)
            {
                ret = QString::fromLatin1("ORA-") +
                      QString::number(exc.code) +
                      QString::fromLatin1(" missing error description");
            }
            else if (ret.isEmpty())
            {
                ret = QString::fromLatin1(
                    "Missing error description. This may be caused by a library "
                    "version mismatch. Check that your ORACLE_HOME and library path is correct.");
            }
        }

        if (exc.stm_text && strlen(exc.stm_text))
        {
            ret += QString::fromLatin1("\n");
            QString sql = QString::fromUtf8((const char *)exc.stm_text);
            if (exc.code >= 0)
            {
                QString t = QString::fromUtf8((const char *)exc.stm_text, exc.error_offset);
                ret.setOffset(t.length());
                sql.insert(t.length(), QString::fromLatin1("<ERROR>"));
            }
            ret += sql;
        }
        throw ret;
    }
}

class toOracleProvider : public toConnectionProvider
{
public:
    class oracleSub : public toConnectionSub
    {
    public:
        otl_connect *Connection;
        oracleSub(otl_connect *conn)
        {
            Connection = conn;
        }
        ~oracleSub()
        {
            try {
                delete Connection;
            }
            catch(...) {
            }

            Connection = 0;
        }
        virtual void cancel(void)
        {
            Connection->cancel();
        }
        virtual void throwExtendedException(toConnection &conn, const otl_exception &exc)
        {
            if (conn.version() < "0800" && exc.code == 0)
            {
                // Serious OCI voodoo to get the Parse error location on Oracle7 servers

                Lda_Def lda;
                Cda_Def cda;
                if (OCISvcCtxToLda(Connection->connect_struct.svchp,
                                   Connection->connect_struct.errhp,
                                   &lda) == OCI_SUCCESS)
                {
                    if (oopen(&cda,
                              &lda,
                              (OraText *)0, -1,
                              -1,
                              (OraText *)0, -1) == OCI_SUCCESS)
                    {
                        cda.peo = 0;
                        oparse(&cda, (OraText *)exc.stm_text, -1, FALSE, 1);
                        ((otl_exception &)exc).code = cda.peo;
                        oclose(&cda);
                    }
                    OCILdaToSvcCtx(&Connection->connect_struct.svchp,
                                   Connection->connect_struct.errhp,
                                   &lda);
                }
            }
            ThrowException(exc);
        }
    };

class oracleQuery : public toQuery::queryImpl
    {
        bool Cancel;
        bool Running;
        bool SaveInPool;
        otl_stream *Query;
    public:
        oracleQuery(toQuery *query, oracleSub *)
                : toQuery::queryImpl(query)
        {
            Running = Cancel = false;
            SaveInPool = false;
            Query = NULL;
        }
        virtual ~oracleQuery()
        {
            if (!Query)
                return;

            Query->close();
            delete Query;
        }
        virtual void execute(void);

        virtual toQValue readValue(void)
        {
            char *buffer = NULL;
            otl_var_desc *dsc = Query->describe_next_out_var();
            if (!dsc)
                throw QString::fromLatin1("Couldn't get description of next column to read");

            oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
            if (!conn)
                throw QString::fromLatin1("Internal error, not oracle sub connection");
            if (Cancel)
                throw QString::fromLatin1("Cancelled while waiting to read value");
            Running = true;
            SaveInPool = true;
            try
            {
                toQValue null;
                switch (dsc->ftype)
                {
                case otl_var_double:
                case otl_var_float:
                {
                    double d = 0;
                    (*Query) >> d;
                    Running = false;
                    if (Query->is_null())
                        return null;
                    return toQValue(d);
                }
                break;
                case otl_var_int:
                case otl_var_unsigned_int:
                case otl_var_short:
                case otl_var_long_int:
                {
                    int i = 0;
                    (*Query) >> i;
                    Running = false;
                    if (Query->is_null())
                        return null;
                    return toQValue(i);
                }
                break;
                case otl_var_varchar_long:
                case otl_var_raw:
                case otl_var_raw_long:
                {
                    int len = toMaxLong;
                    if (toMaxLong < 0)
                        len = DEFAULT_MAX_LONG;
                    buffer = (char *)malloc(len + 1);
                    buffer[len] = 0;
                    otl_long_string str(buffer, len);
                    (*Query) >> str;
                    Running = false;

                    if (!str.len()) {
                        free(buffer);
                        return null;
                    }

                    QString buf;
                    if (dsc->ftype == otl_var_varchar_long)
                    {
                        buf = (QString::fromUtf8(buffer));
                        free(buffer);
                        return buf;
                    }
                    else
                    {
                        QByteArray ret(buffer, str.len());
                        free(buffer);
                        return toQValue::createBinary(ret);
                    }
                }
                case otl_var_clob:
                case otl_var_blob:
                {
                    otl_lob_stream lob;
                    (*Query) >> lob;
                    if (lob.len() == 0)
                    {
                        Running = false;
                        return null;
                    }
                    int len = lob.len();
                    if (toMaxLong >= 0 && len > toMaxLong)
                        len = toMaxLong;
                    if (dsc->ftype == otl_var_clob)
                        len *= 5;
                    else
                        len *= 2;
                    buffer = (char *)malloc(len + 1);
                    buffer[0] = 0;
                    otl_long_string data(buffer, len);
                    lob >> data;
                    if (!lob.eof())
                    {
                        otl_long_string sink(10000);
                        do
                        {
                            lob >> sink;
                        }
                        while (!lob.eof() && sink.len() > 0);

                        toStatusMessage(QString::fromLatin1("Data exists past length of LOB"));
                    }
                    buffer[data.len()] = 0;
                    Running = false;

                    if (dsc->ftype == otl_var_clob)
                    {
                        QString buf = QString::fromUtf8(buffer);
                        free(buffer);
                        return buf;
                    }
                    else
                    {
                        QByteArray ret(buffer, data.len());
                        free(buffer);
                        return toQValue::createBinary(ret);
                    }
                }
                break;
                default:       // Try using char if all else fails
                {
                    // The *5 is for raw columns or UTF expanded data, also dates and numbers
                    // are a bit tricky but if someone specifies a dateformat longer than 100 bytes he
                    // deserves everything he gets!
                    buffer = new char[std::max(dsc->elem_size * 5 + 1, 100)];
                    buffer[0] = 0;
                    (*Query) >> buffer;
                    QString buf = QString::fromUtf8(buffer);
                    delete[] buffer;

                    Running = false;

                    if (Query->is_null())
                        return null;
                    return buf;
                }
                break;
                }
            }
            catch (const otl_exception &exc)
            {
                Running = false;
                delete[] buffer;
                if(conn && query())
                    conn->throwExtendedException(query()->connection(), exc);
            }
            catch (...)
            {
                Running = false;
                delete[] buffer;
                throw;
            }
            // Never get here
            return QString(); //qt4 ::null;
        }
        virtual void cancel(void);
        virtual bool eof(void)
        {
            if (!Query || Cancel)
                return true;
            try {
                return Query->eof();
            }
            catch (const otl_exception &exc)
            {
                if(query())
                {
                    oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
                    if(conn)
                        conn->throwExtendedException(query()->connection(), exc);
                }

                return true;
            }
        }
        virtual int rowsProcessed(void)
        {
            if (!Query)
                return 0;
            return Query->get_rpc();
        }
        virtual int columns(void)
        {
            int descriptionLen;
            Query->describe_select(descriptionLen);
            return descriptionLen;
        }
        virtual std::list<toQuery::queryDescribe> describe(void)
        {
            std::list<toQuery::queryDescribe> ret;
            int descriptionLen;
            int datatypearg1 = 0;
            int datatypearg2 = 0;
            otl_column_desc *description = Query->describe_select(descriptionLen);

            for (int i = 0;i < descriptionLen;i++)
            {
                toQuery::queryDescribe desc;
                desc.AlignRight = false;
                desc.Name = QString::fromUtf8(description[i].name);
                /*
                 * http://www.stanford.edu/dept/itss/docs/oracle/10g/server.101/b10758/sqlqr06.htm
                 * include ocidfn.h
                 */
                switch (description[i].dbtype)
                {
                case 1: /* VARCHAR2, NVARCHAR2 */
                    desc.Datatype = QString::fromLatin1("VARCHAR2(%i)");
                    datatypearg1 = description[i].char_size;
#ifdef OTL_ORA_UNICODE
                    if (description[i].charset_form == 2)
                    {
                        desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
                    }
#endif
                    break;

                case 2: /* NUMBER */
                    desc.Datatype = QString::fromLatin1("NUMBER");

                    if (description[i].prec)
                    {
                        desc.Datatype = QString::fromLatin1("NUMBER(%i)");
                        datatypearg1 = description[i].prec;
                        if (description[i].scale != 0)
                        {
                            desc.Datatype = QString::fromLatin1("NUMBER(%i,%i)");
                            datatypearg2 = description[i].scale;
                        }
                    }
                    desc.AlignRight = true;
                    break;

                case 8: /* LONG */
                    desc.Datatype = QString::fromLatin1("LONG");
                    break;

                case 12: /* DATE */
                    desc.Datatype = QString::fromLatin1("DATE");
                    desc.AlignRight = true;
                    break;

                case 23: /* RAW */
                    desc.Datatype = QString::fromLatin1("RAW(%i)");
                    datatypearg1 = description[i].dbsize;
                    break;

                case 24: /* LONG RAW */
                    desc.Datatype = QString::fromLatin1("LONG RAW");
                    break;

                case 104: /* ROWID, docu: 69, ocidfn.h: 104  */
                case 208: /* UROWID */
                    desc.Datatype = QString::fromLatin1("ROWID");
                    break;

                case 96: /* CHAR, NCHAR */
                    desc.Datatype = QString::fromLatin1("CHAR(%i)");
                    datatypearg1 = description[i].char_size;
#ifdef OTL_ORA_UNICODE
                    if (description[i].charset_form == 2)
                    {
                        desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
                    }
#endif
                    break;
#ifdef OTL_ORA_NATIVE_TYPES
                case 100: /* BINARY_FLOAT */
                    desc.Datatype = QString::fromLatin1("BINARY_FLOAT");
                    break;

                case 101: /* BINARY_DOUBLE */
                    desc.Datatype = QString::fromLatin1("BINARY_DOUBLE");
                    break;
#endif
                case 112: /* CLOB, NCLOB */
                    desc.Datatype = QString::fromLatin1("CLOB");
#ifdef OTL_ORA_UNICODE
                    if (description[i].charset_form == 2)
                    {
                        desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
                    }
#endif
                    break;

                case 113: /* BLOB */
                    desc.Datatype = QString::fromLatin1("BLOB");
                    break;
#ifdef OTL_ORA_TIMESTAMP
                case 187: /* TIMESTAMP, docu: 180, ocidfn.h: 187 */
                    desc.Datatype = QString::fromLatin1("TIMESTAMP(%i)");
                    datatypearg1 = description[i].scale;
                    break;

                case 188: /* TIMESTAMP WITH TIME ZONE, docu: 181, ocidfn.h: 188 */
                    desc.Datatype = QString::fromLatin1("TIMESTAMP(%i) WITH TIME ZONE");
                    datatypearg1 = description[i].scale;
                    break;

                case 189: /* INTERVAL YEAR TO MONTH, docu: 182, ocidfn.h: 189 */
                    desc.Datatype = QString::fromLatin1("INTERVAL YEAR(%i) TO MONTH");
                    datatypearg1 = description[i].prec;
                    break;

                case 190: /* INTERVAL DAY TO SECOND, docu: 183, ocidfn.h: 190  */
                    desc.Datatype = QString::fromLatin1("INTERVAL DAY(%i) TO SECOND(%i)");
                    datatypearg1 = description[i].prec;
                    datatypearg2 = description[i].scale;
                    break;

                case 232: /* TIMESTAMP WITH LOCAL TIME ZONE docu: 231, ocidfn.h: 232 */
                    desc.Datatype = QString::fromLatin1("TIMESTAMP(%i) WITH LOCAL TIME ZONE");
                    datatypearg1 = description[i].scale;
                    break;
#endif
                default:
                    desc.Datatype = QString::fromLatin1("UNKNOWN");

                    /* report unmatched datatypes for adding later */
                    printf("File a bug report and include the table layout and the following data:\n");
                    printf("  type=%i, otl_type=%i, size=%i, nullok=%i, charset_form=%i, charset_size=%i, prec=%i, scale=%i\n",
                           description[i].dbtype,
                           description[i].otl_var_dbtype,
                           description[i].dbsize,
                           description[i].nullok,
                           description[i].charset_form,
                           description[i].char_size,
                           description[i].prec,
                           description[i].scale);
                }
                desc.Datatype.sprintf(desc.Datatype.toUtf8().constData(), datatypearg1, datatypearg2);

                desc.Null = description[i].nullok;

                ret.insert(ret.end(), desc);
            }
            return ret;
        }
    };

class oracleConnection : public toConnection::connectionImpl
    {
        QString connectString(void)
        {
            QString ret;
            ret = connection().user().toUtf8();
            ret += QString::fromLatin1("/");
            ret += connection().password().toUtf8();
            if (!connection().host().isEmpty())
            {
                ret += QString::fromLatin1("@");
                ret += connection().database().toUtf8();
            }
            return ret;
        }
        oracleSub *oracleConv(toConnectionSub *sub)
        {
            oracleSub *conn = dynamic_cast<oracleSub *>(sub);
            if (!conn)
                throw QString::fromLatin1("Internal error, not oracle sub connection");
            return conn;
        }
    public:
        oracleConnection(toConnection *conn)
                : toConnection::connectionImpl(conn)
        { }

        /** Return a string representation to address an object.
         * @param name The name to be quoted.
         * @return String addressing table.
         */
        virtual QString quote(const QString &name)
        {
            bool ok = true;
            for (int i = 0;i < name.length();i++)
            {
                if (name.at(i).toUpper() != name.at(i) || !toIsIdent(name.at(i)))
                    ok = false;
            }
            if (ok)
                return name.toLower();
            else
                return QString::fromLatin1("\"") + name + QString::fromLatin1("\"");
        }
        virtual QString unQuote(const QString &str)
        {
            if (str.at(0).toLatin1() == '\"' && str.at(str.length() - 1).toLatin1() == '\"')
                return str.left(str.length() - 1).right(str.length() - 2);
            return str.toUpper();
        }

        virtual std::list<toConnection::objectName> objectNames(void)
        {
            std::list<toConnection::objectName> ret;

            std::list<toQValue> par;
            toQuery objects(connection(), toQuery::Long,
                            SQLListObjects, par);
            toConnection::objectName cur;
            while (!objects.eof())
            {
                cur.Owner = objects.readValueNull();
                cur.Name = objects.readValueNull();
                cur.Type = objects.readValueNull();
                cur.Comment = objects.readValueNull();
                ret.insert(ret.end(), cur);
            }

            return ret;
        }
        virtual std::map<QString, toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
        {
            std::map<QString, toConnection::objectName> ret;

            toConnection::objectName cur;
            cur.Type = QString::fromLatin1("A");
            std::list<toQValue> par;
            par.insert(par.end(), toQValue(connection().user().toUpper()));
            toQuery synonyms(connection(), toQuery::Long,
                             SQLListSynonyms, par);
            std::list<toConnection::objectName>::iterator i = objects.begin();
            while (!synonyms.eof())
            {
                QString synonym = synonyms.readValueNull();
                cur.Owner = synonyms.readValueNull();
                cur.Name = synonyms.readValueNull();
                while (i != objects.end() && (*i) < cur)
                    i++;
                if (i == objects.end())
                    break;
                if (cur.Name == (*i).Name && cur.Owner == (*i).Owner)
                {
                    ret[synonym] = (*i);
                    (*i).Synonyms.insert((*i).Synonyms.end(), synonym);
                }
            }

            return ret;
        }
        virtual toQDescList columnDesc(const toConnection::objectName &table)
        {
            toBusy busy;
            if (table.Type == QString::fromLatin1("PACKAGE"))
            {
                toQDescList ret;
                try
                {
                    toQuery::queryDescribe desc;
                    desc.Datatype = ("MEMBER");
                    desc.Null = false;
                    QString lastName;
                    QString lastOver;
                    toQuery member(connection(), SQLMembers, table.Owner, table.Name);
                    bool hasArgs = false;
                    while (!member.eof())
                    {
                        QString name = member.readValue();
                        QString overld = member.readValue();
                        QString arg = member.readValueNull();
                        QString type = member.readValueNull();
                        if (lastName != name || overld != lastOver)
                        {
                            if (hasArgs)
                                desc.Name += ")";
                            if (!desc.Name.isEmpty())
                                ret.insert(ret.end(), desc);
                            desc.Name = name;
                            lastName = name;
                            lastOver = overld;
                            hasArgs = false;
                        }
                        if (arg.isEmpty())
                        {
                            if (hasArgs)
                            {
                                desc.Name += ")";
                                hasArgs = false;
                            }
                            desc.Name += " RETURNING ";
                        }
                        else
                        {
                            if (hasArgs)
                                desc.Name += ", ";
                            else
                            {
                                desc.Name += "(";
                                hasArgs = true;
                            }
                            desc.Name += arg;
                            desc.Name += " ";
                        }
                        desc.Name += type;
                    }
                    if (desc.Name.contains("("))
                        desc.Name += ")";
                    if (!desc.Name.isEmpty())
                        ret.insert(ret.end(), desc);
                }
                catch (...)
                    {}
                return ret;
            }

            std::map<QString, QString> comments;
            try
            {
                toQuery comment(connection(), SQLComment, table.Owner, table.Name);
                while (!comment.eof())
                {
                    QString col = comment.readValue();
                    comments[col] = comment.readValueNull();
                }
            }
            catch (...)
                {}

            try
            {
                toQuery query(connection());
#ifdef OTL_STREAM_POOLING_ON
                // Need to clear the stream cache first.
                oracleSub *sub = dynamic_cast<oracleSub *>(query.connectionSub());
                sub->Lock.down();
                sub->Connection->set_stream_pool_size(std::max(toConfigurationSingle::Instance().openCursors(), 1));
                sub->Lock.up();
#endif

                QString SQL = QString::fromLatin1("SELECT * FROM \"");
                SQL += table.Owner;
                SQL += QString::fromLatin1("\".\"");
                SQL += table.Name;
                SQL += QString::fromLatin1("\" WHERE NULL=NULL");
                toQList par;
                query.execute(SQL, par);
                toQDescList desc = query.describe();
                for (toQDescList::iterator j = desc.begin();j != desc.end();j++)
                    (*j).Comment = comments[(*j).Name];

                return desc;
            }
            catch ( ... )
            {
                throw;
            }

            toQDescList ret;
            return ret;
        }

        virtual void commit(toConnectionSub *sub)
        {
            oracleSub *conn = oracleConv(sub);
            try
            {
                conn->Connection->commit();
            }
            catch (const otl_exception &exc)
            {
                ThrowException(exc);
            }
        }
        virtual void rollback(toConnectionSub *sub)
        {
            oracleSub *conn = oracleConv(sub);
            try
            {
                conn->Connection->rollback();
            }
            catch (const otl_exception &exc)
            {
                ThrowException(exc);
            }
        }

        virtual toConnectionSub *createConnection(void);

        void closeConnection(toConnectionSub *conn)
        {
            delete conn;
        }

        virtual QString version(toConnectionSub *sub)
        {
            oracleSub *conn = oracleConv(sub);
            try
            {
                otl_stream version(1,
                                   "SELECT version FROM product_component_version where product like 'Oracle%'",
                                   *(conn->Connection));
                if (!version.eof())
                {
                    char buffer[1024];
                    version >> buffer;
                    QStringList vl = QString(buffer).split('.');
                    QString ve;
                    QString verrj;
                    for ( QStringList::iterator vi = vl.begin(); vi != vl.end(); ++vi )
                    {
                        ve = *vi;
                        verrj += ve.rightJustified(2, '0');
                    }
                    return verrj;
                }
            }
            catch (...)
            {
                // Ignore any errors here
            }
            return QString::QString();
        }

        virtual toQuery::queryImpl *createQuery(toQuery *query, toConnectionSub *sub)
        {
            return new oracleQuery(query, oracleConv(sub));
        }

        virtual void execute(toConnectionSub *sub, const QString &sql, toQList &params)
        {
            oracleSub *conn = oracleConv(sub);

            if ( params.empty() )
            {
                try
                {
                    otl_cursor::direct_exec(*(conn->Connection), sql.toUtf8().constData());
                }
                catch (const otl_exception &exc)
                {
                    conn->throwExtendedException(connection(), exc);
                }
            }
            else
                toQuery query(connection(), sql, params);
        }

        virtual void parse(toConnectionSub *sub, const QString &sql)
        {
            oracleSub *conn = oracleConv(sub);
            try
            {
                conn->Connection->reset_throw_count();
                conn->Connection->syntax_check(sql.toUtf8().constData());

            }
            catch (const otl_exception &exc)
            {
                conn->Connection->throw_count++;
                ThrowException(exc);
            }
        }
    };

    toOracleProvider(void)
            : toConnectionProvider("Oracle", false)
    {}

    virtual void initialize(void)
    {
        toMaxLong = toConfigurationSingle::Instance().maxLong();
        if (otl_connect::otl_initialize(1))
            addProvider("Oracle");
        else
            fprintf(stderr, "otl_initialize failed!\n");
    }

    virtual toConnection::connectionImpl *provideConnection(const QString &, toConnection *conn)
    {
        return new oracleConnection(conn);
    }
    virtual std::list<QString> providedOptions(const QString &)
    {
        std::list<QString> ret;
        ret.insert(ret.end(), "*SQL*Net");
        ret.insert(ret.end(), "-");
        ret.insert(ret.end(), "SYS_OPER");
        ret.insert(ret.end(), "SYS_DBA");
        return ret;
    }
    virtual std::list<QString> providedHosts(const QString &)
    {
        std::list<QString> ret;
        ret.insert(ret.end(), QString::null);
        ret.insert(ret.end(), QString::fromLatin1("SQL*Net"));
        return ret;
    }
    virtual std::list<QString> providedDatabases(const QString &,
            const QString &host,
            const QString &,
            const QString &)
    {
        QString str;
#ifdef Q_OS_WIN32

        CRegistry registry;
        DWORD siz = 1024;
        char buffer[1024];
        try
        {
            if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
                                        "SOFTWARE\\ORACLE\\HOME0",
                                        "TNS_ADMIN",
                                        buffer, siz))
            {
                if (siz > 0)
                    str = buffer;
                else
                    throw 0;
            }
            else
                throw 0;
        }
        catch (...)
        {
            try
            {
                if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
                                            "SOFTWARE\\ORACLE\\HOME0",
                                            "ORACLE_HOME",
                                            buffer, siz))
                {
                    if (siz > 0)
                    {
                        str = buffer;
                        str += "\\network\\admin";
                    }
                    else
                        throw 0;
                }
                else
                    throw 0;
            }
            catch (...)
            {
                if (GetEnvironmentVariableA("TNS_ADMIN", buffer, siz) > 0)
                    str = buffer;
            }
        }
#else
        if (getenv("TNS_ADMIN"))
        {
            str = getenv("TNS_ADMIN");
        }
        else
        {
            str = getenv("ORACLE_HOME");
            str.append(QString::fromLatin1("/network/admin"));
        }
#endif
        str.append(QString::fromLatin1("/tnsnames.ora"));


        std::list<QString> ret;

        QFile file(str);
        if (!file.open(QIODevice::ReadOnly))
            return ret;

        QTextStream in(&file);
        QByteArray barray = in.readAll().toUtf8();
        const char *buf = barray.constData();

        int begname = -1;
        int parambeg = -1;
        int pos = 0;
        int param = 0;
        while (pos < barray.size())
        {
            if (buf[pos] == '#')
            {
                while (pos < barray.size() && buf[pos] != '\n')
                    pos++;
            }
            else if (buf[pos] == '=')
            {
                if (param == 0)
                {
                    if (begname >= 0 && !host.isEmpty())
                        ret.insert(ret.end(), QString::fromLatin1(buf + begname, pos - begname));
                }
            }
            else if (buf[pos] == '(')
            {
                begname = -1;
                parambeg = pos + 1;
                param++;
            }
            else if (buf[pos] == ')')
            {
                if (parambeg >= 0 && host.isEmpty())
                {
                    QString tmp = QString::fromLatin1(buf + parambeg, pos - parambeg);
                    tmp.replace(QRegExp(QString::fromLatin1("\\s+")), QString::null);
                    if (tmp.toLower().startsWith(QString::fromLatin1("sid=")))
                        ret.insert(ret.end(), tmp.mid(4));
                }
                begname = -1;
                parambeg = -1;
                param--;
            }
            else if (!isspace(buf[pos]) && begname < 0)
            {
                begname = pos;
            }
            pos++;
        }
        return ret;
    }
    virtual QWidget *providerConfigurationTab(const QString &provider, QWidget *parent);
};

static toOracleProvider OracleProvider;

void toOracleProvider::oracleQuery::execute(void)
{
    oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
    if (!conn)
        throw QString::fromLatin1("Internal error, not oracle sub connection");
    try
    {
        delete Query;
        Query = NULL;

        if (Cancel)
            throw QString::fromLatin1("Query aborted before started");
        Running = true;

        QRegExp stripnl("\r");
        Query = new otl_stream;
        Query->set_commit(0);
        if (toQValue::numberFormat() == 0)
            Query->set_all_column_types(otl_all_num2str | otl_all_date2str);
        else
            Query->set_all_column_types(otl_all_date2str);

        QString sql = query()->sql();
        sql.replace(stripnl, "");
        Query->open(1, sql.toUtf8().constData(), *(conn->Connection));
    }
    catch (const otl_exception &exc)
    {
        delete Query;
        Query = NULL;

        Running = false;
        conn->throwExtendedException(query()->connection(), exc);
    }
    try
    {
        otl_null null;
        for (toQList::iterator i = query()->params().begin();i != query()->params().end();i++)
        {
            if ((*i).isNull())
                (*Query) << null;
            else
            {
                otl_var_desc *next = Query->describe_next_in_var();
                if (next)
                {
                    switch (next->ftype)
                    {
                    case otl_var_double:
                    case otl_var_float:
                        (*Query) << (*i).toDouble();
                        break;
                    case otl_var_int:
                    case otl_var_unsigned_int:
                    case otl_var_short:
                    case otl_var_long_int:
                        (*Query) << (*i).toInt();
                        break;
                    case otl_var_raw_long:
                    case otl_var_blob:
                        if ((*i).isBinary())
                        {
                            QByteArray arr = (*i).toByteArray();
                            otl_long_string str(arr, arr.size(), arr.size());
                            (*Query) << str;
                            break;
                        }
                        // Intentially left out break
                    case otl_var_varchar_long:
                    case otl_var_clob:
                    {
                        QString buf = (*i).toUtf8();
                        otl_long_string str(buf.toUtf8().constData(), buf.length(), buf.length());
                        (*Query) << str;
                    }
                    break;
                    default:
                        (*Query) << (*i).toUtf8().toUtf8().constData();
                        break;
                    }
                }
                else
                {
                    printf("ERROR: More parameters than binds\n");
                }
            }
        }
        Running = false;
    }
    catch (const otl_exception &exc)
    {
        delete Query;
        Query = NULL;

        Running = false;
        conn->throwExtendedException(query()->connection(), exc);
    }
}

void toOracleProvider::oracleQuery::cancel(void)
{
    if(!Running || Cancel)
        return;

    oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
    if (!conn)
        throw QString::fromLatin1("Internal error, not oracle sub connection");

    conn->Connection->cancel();
    Cancel = true;
}

toConnectionSub *toOracleProvider::oracleConnection::createConnection(void)
{
    QString oldSid;

    std::set
    <QString> options = connection().options();

    bool sqlNet = (options.find("SQL*Net") != options.end());
    if (!sqlNet)
    {
        oldSid = getenv("ORACLE_SID");
        toSetEnv("ORACLE_SID", connection().database());
    }
    otl_connect *conn = NULL;
    try
    {
        int session_mode = OCI_DEFAULT;
        if (options.find("SYS_OPER") != options.end())
            session_mode = OCI_SYSOPER;
        else if (options.find("SYS_DBA") != options.end())
            session_mode = OCI_SYSDBA;
        do
        {
            conn = new otl_connect;

#ifdef OTL_STREAM_POOLING_ON
            conn->set_stream_pool_size(std::max(toConfigurationSingle::Instance().openCursors(), 1));
#endif

            if (!sqlNet)
                conn->server_attach();
            else
                conn->server_attach(connection().database().toUtf8().constData());
            QString user = connection().user();
            QString pass = connection().password();
            try
            {
                conn->session_begin(user.isEmpty() ? "" : user.toUtf8().constData(),
                                    pass.isEmpty() ? "" : pass.toUtf8().constData(),
                                    0,
                                    session_mode);
            }
            catch (const otl_exception &exc)
            {
                if (toThread::mainThread() && exc.code == 28001)
                {
                    bool ok = false;
                    QString newpass = QInputDialog::getText(
                                          toMainWidget(),
                                          qApp->translate("toOracleConnection", "Password expired"),
                                          qApp->translate("toOracleConnection", "Enter new password"),
                                          QLineEdit::Password,
                                          QString::null,
                                          &ok);
                    if (ok)
                    {
                        QString newpass2 = QInputDialog::getText(
                                               toMainWidget(),
                                               qApp->translate("toOracleConnection", "Password expired"),
                                               qApp->translate("toOracleConnection", "Enter password again for confirmation"),
                                               QLineEdit::Password,
                                               QString::null,
                                               &ok);
                        if (ok)
                        {
                            if (newpass2 != newpass)
                                throw qApp->translate("toOracleConnection", "The two passwords doesn't match");
                            QString nputf = newpass;
                            conn->change_password(user.isEmpty() ? "" : user.toUtf8().constData(),
                                                  pass.isEmpty() ? "" : pass.toUtf8().constData(),
                                                  newpass.isEmpty() ? "" : nputf.toUtf8().constData());
                            connection().setPassword(newpass);
                            delete conn;
                            conn = NULL;
                        }
                        else
                            throw exc;
                    }
                    else
                    {
                        throw exc;
                    }
                }
                else
                    throw exc;
            }
        }
        while (!conn);
    }
    catch (const otl_exception &exc)
    {
        if (!sqlNet)
        {
            if (oldSid.isNull())
                toUnSetEnv("ORACLE_SID");
            else
                toSetEnv("ORACLE_SID", oldSid.toLatin1());
        }
        delete conn;
        ThrowException(exc);
    }
    if (!sqlNet)
    {
        if (oldSid.isNull())
            toUnSetEnv("ORACLE_SID");
        else
        {
            toSetEnv("ORACLE_SID", oldSid.toLatin1());
        }
    }

    try
    {
        QString str = QString::fromLatin1("ALTER SESSION SET NLS_DATE_FORMAT = '");
        str += toConfigurationSingle::Instance().dateFormat();
        str += QString::fromLatin1("'");
        otl_stream date(1, str.toUtf8(), *conn);
    }
    catch (...)
    {
        printf("Failed to set new default date format for session\n");
        toStatusMessage(QObject::tr("Failed to set new default date format for session: %1")
                                  .arg(toConfigurationSingle::Instance().dateFormat()));
    }

    try
    {
        otl_stream info(1,
                        "BEGIN\n"
                        "  SYS.DBMS_APPLICATION_INFO.SET_CLIENT_INFO('" TOAPPNAME
                        " (http://tora.sf.net)"
                        "');\n"
                        "  SYS.DBMS_APPLICATION_INFO.SET_MODULE('" TOAPPNAME "','Access Database');\n"
                        "END;",
                        *conn);
    }
    catch (...)
    {
        printf("Failed to set client info for session\n");
    }

    return new oracleSub(conn);
}

/*
** 11g version, see $ORACLE_HOME/rdbms/admin/utlxplan.sql
*/
static toSQL SQLCreatePlanTable(toSQL::TOSQL_CREATEPLAN,
                                "CREATE TABLE %1 (\n"
                                "STATEMENT_ID        VARCHAR2(30),\n"
                                "PLAN_ID             NUMBER,\n"
                                "TIMESTAMP           DATE,\n"
                                "REMARKS             VARCHAR2(4000),\n"
                                "OPERATION           VARCHAR2(30),\n"
                                "OPTIONS             VARCHAR2(255),\n"
                                "OBJECT_NODE         VARCHAR2(128),\n"
                                "OBJECT_OWNER        VARCHAR2(30),\n"
                                "OBJECT_NAME         VARCHAR2(30),\n"
                                "OBJECT_ALIAS        VARCHAR2(65),\n"
                                "OBJECT_INSTANCE     NUMERIC,\n"
                                "OBJECT_TYPE         VARCHAR2(30),\n"
                                "OPTIMIZER           VARCHAR2(255),\n"
                                "SEARCH_COLUMNS      NUMBER,\n"
                                "ID                  NUMERIC,\n"
                                "PARENT_ID           NUMERIC,\n"
                                "DEPTH               NUMERIC,\n"
                                "POSITION            NUMERIC,\n"
                                "COST                NUMERIC,\n"
                                "CARDINALITY         NUMERIC,\n"
                                "BYTES               NUMERIC,\n"
                                "OTHER_TAG           VARCHAR2(255),\n"
                                "PARTITION_START     VARCHAR2(255),\n"
                                "PARTITION_STOP      VARCHAR2(255),\n"
                                "PARTITION_ID        NUMERIC,\n"
                                "OTHER               LONG,\n"
                                "DISTRIBUTION        VARCHAR2(30),\n"
                                "CPU_COST            NUMERIC,\n"
                                "IO_COST             NUMERIC,\n"
                                "TEMP_SPACE          NUMERIC,\n"
                                "ACCESS_PREDICATES   VARCHAR2(4000),\n"
                                "FILTER_PREDICATES   VARCHAR2(4000),\n"
                                "PROJECTION          VARCHAR2(4000),\n"
                                "TIME                NUMERIC,\n"
                                "QBLOCK_NAME         VARCHAR2(30),\n"
                                "OTHER_XML           CLOB\n"
                                ")",
                                "Create plan table, must have same % signs");


toOracleSetting::toOracleSetting(QWidget *parent)
        : QWidget(parent), toSettingTab("database.html#oracle")
{

    setupUi(this);
    DefaultDate->setText(toConfigurationSingle::Instance().dateFormat());
    CheckPoint->setText(toConfigurationSingle::Instance().planCheckpoint());
    ExplainPlan->setText(toConfigurationSingle::Instance().planTable());
    OpenCursors->setValue(toConfigurationSingle::Instance().openCursors());
    KeepPlans->setChecked(toConfigurationSingle::Instance().keepPlans());
    VsqlPlans->setChecked(toConfigurationSingle::Instance().vsqlPlans());
    int len = toConfigurationSingle::Instance().maxLong();
    if (len >= 0)
    {
        MaxLong->setText(QString::number(len));
        MaxLong->setValidator(new QIntValidator(MaxLong));
        Unlimited->setChecked(false);
    }
    try
    {
        // Check if connection exists
        toMainWidget()->currentConnection();
        CreatePlanTable->setEnabled(true);
    }
    catch (...)
        {}
}


void toOracleSetting::saveSetting()
{
    toConfigurationSingle::Instance().setKeepPlans(KeepPlans->isChecked());
    toConfigurationSingle::Instance().setVsqlPlans(VsqlPlans->isChecked());
    toConfigurationSingle::Instance().setDateFormat(DefaultDate->text());

    // try to change NLS for already running sessions
    QString str("ALTER SESSION SET NLS_DATE_FORMAT = '%1'");
    foreach (QString c, toMainWidget()->connections())
    {
        try
        {
            toConnection &conn = toMainWidget()->connection(c);

            if(toIsOracle(conn))
            {
                conn.allExecute(
                    str.arg(toConfigurationSingle::Instance().dateFormat()));
            }
        }
        catch(...) {
            toStatusMessage(tr("Failed to set new default date format for connection: %1").arg(c));
        }
    }

    toConfigurationSingle::Instance().setPlanCheckpoint(CheckPoint->text());
    toConfigurationSingle::Instance().setPlanTable(ExplainPlan->text());
    toConfigurationSingle::Instance().setOpenCursors(OpenCursors->value());
    if (Unlimited->isChecked())
    {
        toMaxLong = -1;
        toConfigurationSingle::Instance().setMaxLong(-1);
    }
    else
    {
        toConfigurationSingle::Instance().setMaxLong(MaxLong->text().toInt());
        toMaxLong = MaxLong->text().toInt();
    }
}


void toOracleSetting::createPlanTable()
{
    try
    {
        toConnection &conn = toMainWidget()->currentConnection();
        conn.execute(toSQL::string(SQLCreatePlanTable, conn).
                     arg(ExplainPlan->text()));
    }
    TOCATCH;
}


QWidget *toOracleProvider::providerConfigurationTab(const QString &, QWidget *parent)
{
    return new toOracleSetting(parent);
}

#endif
