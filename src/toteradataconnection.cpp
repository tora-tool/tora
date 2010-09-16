
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

/*
 * Some parts:

COPYRIGHT AND LICENSE

This module is placed in the public domain. It can be freely copied and
redistributed under the same terms as Perl itself.

Copyright © 2005 Geoffrey Rommel

 *
 */

#ifdef Q_OS_WIN32
#  include "windows/cregistry.h"
#include <Windows.h>
#endif

#include "toconf.h"
#include "toconfiguration.h"
#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

#include "todefaultkeywords.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>

#include <QDebug>
#include <QList>
#include <QHostInfo>
#include <QHostAddress>

// teradata defines
#undef VERSION

extern "C" {
#include <coptypes.h>       /* Teradata include files */
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>
#include <dbchqep.h>
}

// from http://cpansearch.perl.org/src/GROMMEL/Teradata-SQL-0.09/tdsql.c
/* coptypes really should have this typedef, but it doesn't.
   WGR, 2008-03-19. */
typedef signed char  ByteInt;

/* Data types */
#define BIGINT_NN          600
#define BIGINT_N           601
#define BLOB               400
#define BLOB_DEFERRED      404
#define BLOB_LOCATOR       408
#define BYTEINT_NN         756
#define BYTEINT_N          757
#define BYTE_NN            692
#define BYTE_N             693
 /* CHAR, TIME, and TIMESTAMP are the same */
#define CHAR_NN            452
#define CHAR_N             453
#define CLOB               416
#define CLOB_DEFERRED      420
#define CLOB_LOCATOR       424
#define DATE_NN            752
#define DATE_N             753
#define DECIMAL_NN         484
#define DECIMAL_N          485
#define FLOAT_NN           480
#define FLOAT_N            481
#define GRAPHIC_NN         468
#define GRAPHIC_N          469
#define INTEGER_NN         496
#define INTEGER_N          497
#define LONG_VARBYTE_NN    696
#define LONG_VARBYTE_N     697
#define LONG_VARCHAR_NN    456
#define LONG_VARCHAR_N     457
#define LONG_VARGRAPHIC_NN 472
#define LONG_VARGRAPHIC_N  473
#define SMALLINT_NN        500
#define SMALLINT_N         501
#define VARBYTE_NN         688
#define VARBYTE_N          689
#define VARCHAR_NN         448
#define VARCHAR_N          449
#define VARGRAPHIC_NN      464
#define VARGRAPHIC_N       465

 /* Maximum number of fields per request */
#define MAX_FIELDS      520
 /* Maximum length of returned data */
#define MAX_RDA_LEN     65400


typedef struct
{
    UInt16 FieldCount;
    UInt16 Datatype;
    UInt16 Length;
} DATAINFO;

 /* The following is the entire structure needed for the IRQ extension
    (passing values to parameterized SQL). We use two parcels:
    DataInfo and IndicData.
    Header: IRX8. Level: 1. Element Type: 0 (pointers). */

struct irq_ext
{
    struct D8CAIRX   irqx_header;
    struct D8XIELEM  irqx_DataInfo_elem;
    struct D8XIEP    irqx_DataInfo_body;
    struct D8XIELEM  irqx_IndicData_elem;
    struct D8XIEP    irqx_IndicData_body;
};

static double powers10[19] = {
  1.0, 10.0, 100.0, 1000.0, 10000.0,
  1.0E5,  1.0E6,  1.0E7,  1.0E8,  1.0E9,  1.0E10, 1.0E11,
  1.0E12, 1.0E13, 1.0E14, 1.0E15, 1.0E16, 1.0E17, 1.0E18 };

static toSQL SQLListDatabases(
    "toTeradataConnection:ListDatabases",
    "SELECT DISTINCT ( trim ( db.databasename ) )\n"
    "  FROM dbc.AccessRights ar,\n"
    "       dbc.dbase db\n"
    " WHERE ar.userid IN ( SELECT  userid\n"
    "                         FROM dbc.sessiontbl\n"
    "                        WHERE sessionno = SESSION )\n"
    "   AND ar.databaseid = db.databaseid",
    "List all databases with access rights (used for object cache)",
    "",
    "Teradata");

static toSQL SQLListTablesDatabases(
    "toTeradataConnection:ListTablesDatabase",
    "SELECT trim ( tablename ),\n"
    "       commentstring\n"
    "  FROM dbc.tables\n"
    " WHERE trim ( databasename ) = trim ( :f1<char[128]> )",
    "List tables for a database (used for the object cache)",
    "",
    "Teradata");

static toSQL SQLListColumns(
    "toTeradataConnection:ListColumns",
    "SELECT trim ( columnname ),\n"
    "       columntype,\n"
    "       commentstring\n"
    "  FROM dbc.columns\n"
    " WHERE trim ( databasename ) = trim ( :f1<char[128]> )\n"
    "   AND trim ( tablename ) = trim ( :f2<char[128]> )",
    "List columns for a table (used for the object cache)",
    "",
    "Teradata");

/*--------------------------------------------------------------------
 *  Insert a decimal point into a "decimal" field.
 *------------------------------------------------------------------*/
void _insert_dp (char *target,
                 char *source,
                 int ndec)
{
    int   i, j, workstart;
    char  work[25];

    if (ndec <= 0)
    {
        strcpy(target, source);
        return;
    }

    work[24] = '\0';  /* End of string */

    /* Build the string in the work area from right to left. */

    i = strlen(source) - 1;
    for (j = 23; j > 23 - ndec; j--)
    {
        if (i < 0 || source[i] == '-' )
        {
            work[j] = '0';
            continue;
        }
        /* Otherwise, it should be a digit. */
        work[j] = source[i];
        i--;
    }

    work[j] = '.';
    j--;

    for (; j >= 0; j--) {
        if (i < 0) {
            workstart = j+1;
            break;
        }
        if ( source[i] == '-' ) {
            work[j] = '-';
            workstart = j;
            break;
        }
        /* Otherwise, it should be a digit. */
        work[j] = source[i];
        i--;
    }

    strcpy(target, work+workstart);
    return;
}

/*--------------------------------------------------------------------
**  Convert a decimal field to a double.  This works only on
**  fields of 9 digits or less.
**------------------------------------------------------------------*/
double _dec_to_double (Byte *dec_data,
                       int   decp,     // precision
                       int   decs )    // scale
{
    long    wlong;
    double  wdouble;

    if (decp >= 5)
        wlong = *((long *) dec_data);
    else if (decp >= 3)
        wlong = *((short *) dec_data) + 0;
    else
        wlong = *((ByteInt *) dec_data) + 0; /* Precision is less than 3. */

    wdouble = (double) wlong;
    if (decs > 0)
        wdouble /= powers10[decs];
    return wdouble;
}

/*--------------------------------------------------------------------
**  Convert a decimal field (10 or more digits) to a string.
**------------------------------------------------------------------*/
#ifdef _MSC_VER
/*---------------------- Microsoft Visual C++ */
void _dec_to_string (char *res_string,
                     Byte *dec_data,
                     int   decs )
{
    __int64  wlonglong;
    char   wstring[24];

    wlonglong = *((_int64 *) dec_data);
    sprintf(wstring, "%I64d", wlonglong);
    _insert_dp(res_string, wstring, decs);
}
#else
/*---------------------- Others */
void _dec_to_string (
    char *  res_string,
    Byte * dec_data,
    int     decs )
{
    long long int  wlonglong;
    char   wstring[24];

    wlonglong = *((long long *) dec_data);
    sprintf(wstring, "%lld", wlonglong);
    _insert_dp(res_string, wstring, decs);
}
#endif

static QString QueryParam(const QString &in, toQList &params);

// private type for describing columns
struct _columnDesc {
    QString name;
    QString typeName;
    int tdType;
    unsigned short dataLen;

    // decimal only
    int scale;
    unsigned short dlb;
};
typedef struct _columnDesc columnDesc;

/* used for tracking query state across a shared DBCAREA */
class tdRequest
{
public:

    unsigned int activityCount;
    Int32        requestId;
    Int32        result;
    // used for creating exceptions, execution
    QString      message;
    QString      sql;

    QList<columnDesc> columns;

    // if empty you might just need to fetch more
    QList<toQValue> data;

    bool eof;

    struct DBCAREA *dbcp;

    tdRequest()
        : activityCount(0),
          requestId(0),
          result(0),
          message(),
          eof(false),
          dbcp(0)
    {
        dbcp = (DBCAREA *) malloc(sizeof(struct DBCAREA));
        if(dbcp == 0)
            throw QString::fromLatin1("Error allocating memory");
    }

    ~tdRequest()
     {
         qDebug() << "delete request" << ((long) this);
         if(!dbcp)
             return;

         free(dbcp);
         dbcp = 0;
     }

private:
    tdRequest(const tdRequest &other);
};

class toTeradataProvider : public toConnectionProvider
{
public:
    class teradataSub : public toConnectionSub
    {
        Int32 SessionId;
        char *Cnta;

    public:
        QString Version;
        
        teradataSub(toConnection &conn)
            : SessionId(0)
        {
            QString   user = conn.user();
            QString   pass = conn.password();
            QString   host = conn.host();
            int       port = 1025;

            int pos = host.indexOf(QString(":"));
            if (pos > 0)
            {
                host = host.mid(0, pos);
                port = host.mid(pos + 1).toInt();
                if(port < 1)
                    port = 1025; // for some reason this is 0
            }

            // library doesn't work with names, for some reason
            QString ip = host;
            QList<QHostAddress> addresses = QHostInfo::fromName(host).addresses();
            if(addresses.isEmpty())
                fprintf(stderr, "toTeradataConnection WARNING: no addresses found for hostname\n");
            else {
                if(addresses.length() > 1)
                    fprintf(stderr, "toTeradataConnection WARNING: found multiple addresses, using first\n");
                ip = addresses.at(0).toString();
            }

            // build standard TD logon string
            // [host[:port]]/user[,pass] [charset] [transaction mode] 
            QString logon = QString("%0:%1/%3,%4 UTF-8")
                .arg(ip)
                .arg(port)
                .arg(user)
                .arg(pass);

            qDebug() << "logon" << logon;

            // this must be set for threading to work with cli
            toSetEnv(QString::fromLatin1("THREADONOFF"), QString::fromLatin1("1"));

            tdRequest request;
            request.message = "Login";
            initialize(&request);

            request.dbcp->logon_ptr = logon.toUtf8().data();
             // is qstring length equivalent to strlen for utf?
            request.dbcp->logon_len = strlen(logon.toUtf8().constData());
            if(callServer(&request, DBFCON) != 0)
                throw QString::fromLatin1("Unknown login error.");

            // login successful, set sessionid
            SessionId = request.dbcp->o_sess_id;

            fetchAllParcels(&request);

            Version = getVersion(&request, ip);
        }

        ~teradataSub()
        {
        }


        void close()
        {
            try {
                tdRequest request;
                request.message = "DISCONNECT";

                request.dbcp->extension_pointer = 0;
                request.dbcp->change_opts = 'Y';
                request.dbcp->i_req_id = 0;
                request.dbcp->o_req_id = 0;

                request.columns.clear();
                request.data.clear();

                initialize(&request);

                callServer(&request, DBFDSC);
            }
            catch(...)
            {
                qDebug() << "unhandled exception caught in teradataSub::close()";
            }
        }


        void execute(tdRequest *request, QString sql, toQList &params)
        {
            qDebug() << "execute! session: " << SessionId << "request id" << (long) request << request->message;
            qDebug() << "sql" << sql;

            // clean up from previous dbcarea uses
            request->dbcp->extension_pointer = 0;
            request->dbcp->change_opts = 'Y';
            request->dbcp->i_req_id = 0;
            request->dbcp->o_req_id = 0;

            request->columns.clear();
            request->data.clear();

            request->sql = sql;

            // cli will crash the app
            if(sql.isEmpty())
                return;

            initialize(request);

            if(params.size() > 0)
            {
                // prepare(request, params);
                sql = QueryParam(sql, params);
            }

            // td barfs on newlines
            sql = sql.replace('\n', ' ');
            sql = sql.replace('\r', ' ');
            sql = sql.trimmed();

            qDebug() << "stripped sql" << sql;

            // set sql statement
            QByteArray buf = sql.toUtf8();
            char *str = buf.data();
            request->dbcp->req_ptr = str;
            request->dbcp->req_len = strlen(str);

            if(callServer(request, DBFIRQ) != 0)
                throw QString::fromLatin1("Failed to initiate request");

            // fetch up to first column (or until eof)
            while(fetchParcel(request) && request->data.size() < 1)
                ;

            if(request->data.size() < 1)
                request->eof = true;
        }


        virtual void cancel(void)
        {
            // todo this is supposed to cancel anything running on
            // this session but i don't know if cli can.
        }

        void abort(tdRequest *request)
        {
            // abort a request
            if(!request->dbcp || request->dbcp->i_req_id == 0)
                return;

            try
            {
                if(callServer(request, DBFABT) != EM_OK)
                    throw QString::fromLatin1("Error aborting request");
            }
            catch(QString err)
            {
                qDebug() << "Expected error in abort" << err;
            }
        }

        void endRequest(tdRequest *request)
        {
            if(request->dbcp->i_req_id == 0)
                return;

            qDebug() << "End request" << (long) request;
            if(callServer(request, DBFERQ) != EM_OK)
                throw QString::fromLatin1("Error in end request");

            request->dbcp->i_req_id = 0;
        }

        toQValue readValue(tdRequest *request)
        {
            if(request->eof)
                throw QString::fromLatin1("Internal error: exhausted results");

            toQValue ret = request->data.takeFirst();

            // check eof for next readValue()
            if(request->data.size() < 1)
            {
                if(!fetchParcel(request) || request->data.size() < 1)
                    request->eof = true;
            }

            return ret;
        }

    private:

        QString getVersion(tdRequest *request, QString host)
        {
            Int32 result;
            DBCHQEP QEPParms;
            char release[63];

            const char *hostdata = host.toUtf8().data();
            memset(&QEPParms, 0, sizeof(QEPParms));
            QEPParms.qepLevel = QEPLEVEL;
            QEPParms.qepItem = QEPIDBR;
            QEPParms.qepRALen = sizeof(release);
            QEPParms.qepRArea = &release;

#ifdef CLI_64BIT
            QEPParms.qepTDP = (char *) hostdata;
#else
            QEPParms.qepTDP = (Int32) hostdata;
#endif

            QEPParms.qepTLen = strlen(hostdata);
            DBCHQE(&result, Cnta, &QEPParms);

            if(result != EM_OK) {
                fprintf(stderr, "toTeradataConnection WARNING: getVersion failed\n");
                return QString("");
            }

            release[62] = 0;
            QString ret(release);

            QStringList split = ret.split(" ", QString::SkipEmptyParts);
            if(split.size() == 2)
                ret = split[1].trimmed();
            qDebug() << "version" << ret;
            return ret;
        }

        // initialize a new tdRequest (dbcarea) object
        void initialize(tdRequest *request)
        {
            qDebug() << "initialize";

            request->dbcp->total_len = sizeof(struct DBCAREA);
            DBCHINI(&(request->result), Cnta, request->dbcp);
            checkError(request);

            request->dbcp->change_opts       = 'Y';
            request->dbcp->resp_mode         = 'I'; /* Field mode */
            request->dbcp->use_presence_bits = 'Y'; /* Indicator bits on USING parcels */
            request->dbcp->req_proc_opt      = 'B'; /* Include Info parcels */
            request->dbcp->keep_resp         = 'N';
            request->dbcp->wait_across_crash = 'N';
            request->dbcp->tell_about_crash  = 'Y';
            request->dbcp->loc_mode          = 'Y';
            request->dbcp->var_len_req       = 'N';
            request->dbcp->var_len_fetch     = 'N';
            request->dbcp->save_resp_buf     = 'N';
            request->dbcp->two_resp_bufs     = 'N';
            request->dbcp->ret_time          = 'N';
            request->dbcp->parcel_mode       = 'Y';
            request->dbcp->wait_for_resp     = 'Y';
            request->dbcp->maximum_parcel    = 'H'; /* Allow large parcels */
            request->dbcp->req_buf_len       = 65536;
            request->dbcp->resp_buf_len      = 65473;

            request->dbcp->max_decimal_returned = 18;

            request->dbcp->token     = 0;
            request->dbcp->req_ptr   = 0;
            request->dbcp->req_len   = 0;
            request->dbcp->i_sess_id = SessionId;
        }

        void checkError(tdRequest *request)
        {
            if(request->result == EM_OK || request->result == REQEXHAUST)
                return;

            if(request->dbcp->msg_len > 0)
            {
                QString err(QByteArray(request->dbcp->msg_text, request->dbcp->msg_len));
                qDebug() << "Request failed: " << request->message << err;

                throw request->message + QString::fromLatin1(" ") + err;
            }
            else
                throw QString::fromLatin1("Request failed: ") + request->message;
        }

        Int32 callServer(tdRequest *request, Int32 func)
        {
            // set the connection sessionid.  see the docs on
            // multi-threaded cli. it's not super clear, but the
            // dbcarea struct is not thread safe. every query instance
            // must have it's own, they just pass the session value.
            request->dbcp->i_sess_id = SessionId;

            // set the function we're calling
            request->dbcp->func = func;

            Int32 ret = DBCHCL(&(request->result), Cnta, request->dbcp);

            if(func != DBFDSC)  // logoff function
                checkError(request);
            return ret;
        }

        void fetchRecord(tdRequest *request)
        {
            struct CliDataInfoType *data = (struct CliDataInfoType *) request->dbcp->fet_data_ptr;
            if(!data)
                return;

            Byte *indic_ptr   = (Byte *) data;
            Byte  indic_mask  = 0x80;
            Byte *data_ptr    = ((Byte *) request->dbcp->fet_data_ptr) + ((request->columns.size() + 7) / 8);

            Q_FOREACH(columnDesc desc, request->columns)
            {
                switch (desc.tdType) {
                case SMALLINT_NN:
                case SMALLINT_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        short val = *((short *) data_ptr);
                        request->data.append(val);
                    }
                    data_ptr += 2;
                    break;

                case VARBYTE_NN:
                case VARBYTE_N:
                case LONG_VARBYTE_NN:
                case LONG_VARBYTE_N: {
                    // var gets a length short
                    short len = *((short *) data_ptr);

                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        char *sptr = (char *) (data_ptr + 2);
                        request->data.append(
                            QString("0x") + QString(
                                QByteArray::fromRawData(sptr, len).toHex()));
                    }
                    data_ptr += len + 2;
                    break;
                }

                case VARCHAR_NN:
                case VARCHAR_N:
                case VARGRAPHIC_NN:
                case VARGRAPHIC_N:
                case LONG_VARCHAR_NN:
                case LONG_VARCHAR_N:
                case LONG_VARGRAPHIC_NN:
                case LONG_VARGRAPHIC_N: {
                    // var gets a length short
                    short len = *((short *) data_ptr);

                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        char *sptr = (char *) (data_ptr + 2);
                        request->data.append(
                            QString::fromUtf8(sptr, len));
                    }
                    data_ptr += len + 2;
                    break;
                }

                case BYTE_NN:
                case BYTE_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        char *sptr = (char *) (data_ptr);
                        request->data.append(
                            QString("0x") + QString(
                                QByteArray::fromRawData(sptr, desc.dataLen).toHex()));
                    }
                    data_ptr += desc.dataLen;
                    break;
                    break;

                case CHAR_NN:
                case CHAR_N:
                case GRAPHIC_NN:
                case GRAPHIC_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        char *sptr = (char *) (data_ptr);
                        request->data.append(
                            QString::fromUtf8(sptr, desc.dataLen));
                    }
                    data_ptr += desc.dataLen;
                    break;

                case DATE_NN:
                case DATE_N:
                case INTEGER_NN:
                case INTEGER_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        int val = *((int *) data_ptr) + 0;
                        request->data.append((int) val);
                    }
                    data_ptr += 4;
                    break;

                case DECIMAL_NN:
                case DECIMAL_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                        data_ptr += desc.dlb;
                    }
                    else if(desc.dataLen <= 9)
                    {
                        double wdouble = _dec_to_double(data_ptr,
                                                        desc.dataLen,
                                                        desc.scale);
                        request->data.append(toQValue(wdouble));
                        data_ptr += desc.dlb;
                    }
                    else if(desc.dataLen <= 18)
                    {
                        char wstring[24];
                        _dec_to_string(wstring, data_ptr, desc.scale);
                        int slen = strlen(wstring);
                        wstring[slen] = 0;
                        request->data.append(toQValue(wstring));
                        data_ptr += 8;
                    }
                    else
                    {
                        fprintf(stderr, "Warning: decimal value is too large.\n");
                        request->data.append(toQValue(0.0));
                        data_ptr += 16;
                    }

                    break;
                case BIGINT_NN:
                case BIGINT_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        long val = *((long *) data_ptr) + 0;
                        request->data.append((qlonglong) val);
                    }
                    data_ptr += 8;
                    break;

                case BYTEINT_NN:
                case BYTEINT_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        ByteInt val = *((ByteInt *) data_ptr) + 0;
                        request->data.append(val);
                    }
                    data_ptr++;
                    break;

                case FLOAT_NN:
                case FLOAT_N:
                    if((*indic_ptr & indic_mask) > 0)
                    {
                        // add null value
                        request->data.append(toQValue());
                    }
                    else
                    {
                        double val = *((double *) data_ptr) + 0;
                        request->data.append(val);
                    }
                    data_ptr += 8;
                    break;

                case BLOB:      // shouldn't happen, anyhow
                case BLOB_DEFERRED:
                case BLOB_LOCATOR:
                case CLOB:
                case CLOB_DEFERRED:
                case CLOB_LOCATOR:
                default:
                    fprintf(stderr, "Unsupported type\n");
                    request->data.append(toQValue());
                }

                /* Point to the next indicator bit. */
                if(indic_mask != 0x01)
                    indic_mask >>= 1;
                else
                {
                    indic_mask = 0x80;
                    indic_ptr++;
                }
            } // all columns
        }

        void fetchColumns(tdRequest *request)
        {
            char colident[32];

            qDebug() << "fetchColumns";
            request->columns.clear();
            struct CliPrepInfoType *parsel = (struct CliPrepInfoType *) request->dbcp->fet_data_ptr;

            char *col_byte_ptr = ((char *) request->dbcp->fet_data_ptr) + sizeof(struct CliPrepInfoType);
            struct CliPrepColInfoType *col_info = (struct CliPrepColInfoType *) col_byte_ptr;

            // setup request->columns
            int count = parsel->ColumnCount;
            qDebug() << "column count" << count;
            for(int current = 0; current < count; current++)
            {
                columnDesc desc;
                desc.tdType = col_info->DataType;

                switch (col_info->DataType) {
                case SMALLINT_NN:
                case SMALLINT_N:
                    desc.typeName = "SMALLINT";
                    break;

                case LONG_VARBYTE_NN:
                case LONG_VARBYTE_N:
                case VARBYTE_NN:
                case VARBYTE_N:
                    desc.dataLen = col_info->DataLen;
                    desc.typeName = QString::fromLatin1("VARBYTE (") +
                        QString::number(col_info->DataLen) +
                        QString::fromLatin1(")");
                    break;

                case VARCHAR_NN:
                case VARCHAR_N:
                case VARGRAPHIC_NN:
                case VARGRAPHIC_N:
                case LONG_VARCHAR_NN:
                case LONG_VARCHAR_N:
                case LONG_VARGRAPHIC_NN:
                case LONG_VARGRAPHIC_N:
                    desc.dataLen = col_info->DataLen;
                    desc.typeName = QString::fromLatin1("VARCHAR (") +
                        QString::number(col_info->DataLen) +
                        QString::fromLatin1(")");
                    break;

                case BYTE_NN:
                case BYTE_N:
                    desc.dataLen = col_info->DataLen;
                    desc.typeName = QString::fromLatin1("BYTE (") +
                        QString::number(col_info->DataLen) +
                        QString::fromLatin1(")");
                    break;
                case CHAR_NN:
                case CHAR_N:
                case GRAPHIC_NN:
                case GRAPHIC_N:
                    desc.dataLen = col_info->DataLen;
                    desc.typeName = QString::fromLatin1("CHAR (") +
                        QString::number(col_info->DataLen) +
                        QString::fromLatin1(")");
                    break;
                case DATE_NN:
                case DATE_N:
                case INTEGER_NN:
                case INTEGER_N:
                    desc.typeName = "INTEGER";
                    break;
                case DECIMAL_NN:
                case DECIMAL_N:
                    // first byte is len, second byte is scale?
                    desc.dataLen = col_info->DataLen / 256;
                    if (desc.dataLen >= 19)
                        desc.dlb = 16;
                    else if (desc.dataLen >= 10)
                        desc.dlb = 8;
                    else if (desc.dataLen >= 5)
                        desc.dlb = 4;
                    else if (desc.dataLen >= 3)
                        desc.dlb = 2;
                    else
                        desc.dlb = 1;

                    desc.scale = col_info->DataLen % 256;

                    desc.typeName = QString::fromLatin1("DECIMAL (") +
                        QString::number(col_info->DataLen / 256) +
                        QString::fromLatin1(", ") +
                        QString::number(col_info->DataLen % 256) +
                        QString::fromLatin1(")");
                    break;
                case BIGINT_NN:
                case BIGINT_N:
                    desc.typeName = "BIGINT";
                    break;
                case BLOB:
                case BLOB_DEFERRED:
                case BLOB_LOCATOR:
                case CLOB:
                case CLOB_DEFERRED:
                case CLOB_LOCATOR:
                    desc.typeName = "BLOB";
                    break;
                case BYTEINT_NN:
                case BYTEINT_N:
                    desc.typeName = "BYTEINT";
                    break;
                case FLOAT_NN:
                case FLOAT_N:
                    desc.typeName = "FLOAT";
                    break;
                default:
                    desc.typeName = QString::fromLatin1("Unknown (") +
                        QString::number(col_info->DataType) +
                        QString::fromLatin1(")");
                }

                /* Get the name of the column (not the title). */
                memcpy(colident, col_info->Name, col_info->NameLen);
                colident[col_info->NameLen] = '\0';

                desc.name = colident;
                qDebug() << "name: " << desc.name << ", type: " << desc.typeName << ", id: " << desc.tdType;

                /* Point to the next set of column info. This is a pain. */
                col_byte_ptr += 4;
                col_byte_ptr += *((short *)col_byte_ptr) + 2; /* Name */
                col_byte_ptr += *((short *)col_byte_ptr) + 2; /* Format */
                col_byte_ptr += *((short *)col_byte_ptr) + 2; /* Title */
                col_info = (struct CliPrepColInfoType *) col_byte_ptr;

                request->columns.append(desc);
            }
        }

        

        void prepare(tdRequest *request, toQList &params)
        {
            // mrj:
            // doesn't freaking work. i tried everything. i followed
            // the manual. i did what the perl module did.  i thought
            // to try running the sample from the manual and guess
            // what, that doesn't work, either.

            // plus, teradata's site is a ghost town. no docs. few
            // users. nothing on google.

            // damn it, teradata connections will have to rely on
            // tora's option parsing. if you care, complain to
            // teradata about their crappy documentation. i've got the
            // express database 13 and the most recent manual i could
            // find on cliv2 is 04.08.02 from September 2006.

            // at the time of writing, that's 4 years old.

            // all attempts at the below are rewarded with:
            // CLI2: BADELMTYPE(328): DBCAREA extension element must be 1 to 4096.

            // most documentation referencing this error simply says:
            //   BADELMTYPE(328): DBCAREA extension element must be 1 to 4096.

            // one place (after much searching) finally mentioned to
            // check the parcel flavor. i was doing the indicator
            // mode, ostensibly what was working for the perl guy, but
            // i switched to the datainfo method used in the
            // manual. still no joy.

            // so yeah, no parametrized queries for teradata.
#if 0
            qDebug() << "prepare";

            // see example in:
            /*
              Teradata Call-Level Interface
              Version 2
              Reference for Network-Attached Systems
              Release 04.08.02
              B035-2418-096A
              September 2006

              better:
              http://cpansearch.perl.org/src/GROMMEL/Teradata-SQL-0.09/SQL.xs
            */

            struct irq_ext  ExtArea;
            struct irq_ext *pExtArea;
            int             status = 0;
            int             Elen   = 0;

            /* Set up the Extension area. */
            request->dbcp->extension_pointer = &ExtArea;
            pExtArea = (struct irq_ext *) request->dbcp->extension_pointer;
            Elen = sizeof(struct irq_ext);
            memset(pExtArea, 0x00, Elen);

            memcpy(pExtArea->irqx_header.d8xiId, "IRX8", 4);
            pExtArea->irqx_header.d8xiSize = Elen;
            pExtArea->irqx_header.d8xiLvl = 1;

            /* Store the Perl variables in an IndicData array. */
            /* First, reserve the indicator bytes. */
            Byte                    hv_data[MAX_RDA_LEN]; /* "host variables" */
            Byte                   *hvdata_ptr;
            struct CliDataInfoType  hv_datainfo;
            struct CliDInfoType    *hv_datainfo_ptr;
            int                     idlen = 0;

            hvdata_ptr = hv_data;

            hv_datainfo.FieldCount = params.size();
            hv_datainfo_ptr = &(hv_datainfo.InfoVar[0]);

            // set the argument parcels
            Q_FOREACH(toQValue qvalue, params)
            {
                if(qvalue.isNull())
                {
                    hv_datainfo_ptr->SQLType = INTEGER_N;
                    hv_datainfo_ptr->SQLLen = 4;
                    hvdata_ptr = 0;
                    hvdata_ptr += 4;
                    idlen += 4;
                }
                else
                {
                    // todo, uses strings for everything
                    const char *temp = qvalue.toQVariant().toString().toUtf8().constData();
                    qDebug() << "temp" << temp;
                    qDebug() << qvalue;
                    temp = "asdf";
                    int slen = strlen(temp);

                    hv_datainfo_ptr->SQLType = CHAR_N;
                    hv_datainfo_ptr->SQLLen = slen;
                    memcpy(hvdata_ptr, temp, slen);
                    hvdata_ptr += slen;
                    idlen += slen;
                }

                /* Point to the next DataInfo field. */
                hv_datainfo_ptr++;
            }

            /* DataInfo parcel */
            pExtArea->irqx_DataInfo_elem.d8xieLen = sizeof(D8XIELEM) + sizeof(D8XIEP);
            /* The manual says that Element Type 0 = Inline and 1 = Pointer,
               but that seems to be incorrect. We are using pointers and 0. */
            pExtArea->irqx_DataInfo_elem.d8xieTyp = 0;  /* Pointer method */

            pExtArea->irqx_DataInfo_body.d8xiepF = PclDATAINFO; /* Flavor */
            pExtArea->irqx_DataInfo_body.d8xiepLn = sizeof(struct CliDataInfoType);
            pExtArea->irqx_DataInfo_body.d8xiepPt = (char *) &(hv_datainfo);

            /* IndicData parcel */
            pExtArea->irqx_IndicData_elem.d8xieLen = sizeof(D8XIELEM) + sizeof(D8XIEP);
            pExtArea->irqx_IndicData_elem.d8xieTyp = 0;
            pExtArea->irqx_IndicData_body.d8xiepF = PclDATA; /* Flavor */
            pExtArea->irqx_IndicData_body.d8xiepLn = idlen;
            pExtArea->irqx_IndicData_body.d8xiepPt = (char *) &(hv_data);

            // request->dbcp->extension_pointer = &ExtArea;
            qDebug() << "pointer" << (long) request->dbcp->extension_pointer;
            qDebug() << "area" << (long) pExtArea;
            
            request->dbcp->dbriSeg = 'N';
            callServer(request, DBFIRQ);
            request->dbcp->i_req_id = request->dbcp->o_req_id;

            fetchAllParcels(request);

            request->dbcp->i_req_id = 0;
#endif
        }

        bool fetchParcel(tdRequest *request)
        {
            // qDebug() << "fetchParcel";
            request->dbcp->i_req_id  = request->dbcp->o_req_id;

            callServer(request, DBFFET);
            if(request->result == REQEXHAUST)
                return false;
            checkError(request);

            // qDebug() << "parcel flavor" << request->dbcp->fet_parcel_flavor;
            switch (request->dbcp->fet_parcel_flavor)
            {
            case PclFAILURE: /* fallthrough */
            case PclERROR: {
                request->eof = true;

                struct CliFailureType *err_ptr;
                err_ptr = (struct CliFailureType *) request->dbcp->fet_data_ptr;
                if(err_ptr)
                {
                    QString msg(QByteArray(err_ptr->Msg, err_ptr->Length));
                    throw QString("Error %0 Info %1 in:\n%2\nError Message: %3\n\n%4")
                        .arg(err_ptr->Code)
                        .arg(err_ptr->Info)
                        .arg(request->message)
                        .arg(msg)
                        .arg(request->sql);
                }
                break;
            }

            case PclSUCCESS: {
                struct CliSuccessType *pcl_success_ptr;

                pcl_success_ptr = (struct CliSuccessType *) request->dbcp->fet_data_ptr;
                /* Store the ActivityCount in a double. Unfortunately, this
                   is defined as char[4] rather than unsigned int. */
                unsigned int active;
                memcpy(&active, pcl_success_ptr->ActivityCount, 4);
                request->activityCount = (unsigned int) active;
                break;
            }

            case PclPREPINFO:
                fetchColumns(request);
                break;

            case PclRECORD:
                fetchRecord(request);
                break;
            default:
                ;
            } /* end switch */

            return true;
        }

        /*--------------------------------------------------------------------
         *  Fetch all parcels after a query
         *------------------------------------------------------------------*/
        void fetchAllParcels(tdRequest *request)
        {
            qDebug() << "fetch";

            while(fetchParcel(request))
                ;
        }

        void close(tdRequest *request)
        {
            qDebug() << "close";

            if(!request->dbcp)
                return;
            // should be set during execute
//             if(!request->dbcp->i_req_id) // the input request id? there's also o_req_id
//                 return;

            request->dbcp->change_opts = 'N';
            // should be set during open
//             request->dbcp->i_req_id = /*req*/;
            request->dbcp->func = DBFERQ;

            request->dbcp->func = DBFERQ;
            DBCHCL(&(request->result), Cnta, request->dbcp);
        }
    };

    class teradataQuery : public toQuery::queryImpl
    {
        bool Cancel;
        bool Running;
        teradataSub *Connection;
        tdRequest *request;

    public:
        teradataQuery(toQuery *query, teradataSub *conn)
            : toQuery::queryImpl(query),
              Connection(conn),
              request(0)
        {
            Running = Cancel = false;
            request = new tdRequest;
        }

        virtual ~teradataQuery()
        {
            if(request) {
                Connection->endRequest(request);
                delete request;
                request = 0;
            }
        }

        virtual void execute(void)
        {
            request->activityCount = 0;
            request->message = "Execute";
            Connection->execute(request, query()->sql(), query()->params());
        }

        virtual toQValue readValue(void)
        {
            return Connection->readValue(request);
        }

        virtual void cancel(void)
        {
            Connection->abort(request);
        }

        virtual bool eof(void)
        {
            return request->eof;
        }

        virtual int rowsProcessed()
        {
            return request->activityCount;
        }

        virtual int columns()
        {
            return request->columns.size();
        }

        virtual std::list<toQuery::queryDescribe> describe()
        {
            std::list<toQuery::queryDescribe> ret;
            Q_FOREACH(columnDesc desc, request->columns)
            {
                toQuery::queryDescribe qd;
                qd.Name = desc.name;
                qd.Datatype = desc.typeName;
                qd.AlignRight = false;
                ret.push_back(qd);
            }
            return ret;
        }
    };

    class teradataConnection : public toConnection::connectionImpl
    {
        teradataSub *teradataConv(toConnectionSub *sub)
        {
            teradataSub *conn = dynamic_cast<teradataSub *>(sub);
            if (!conn)
                throw QString::fromLatin1("Internal error, not Teradata sub connection");
            return conn;
        }
    public:
        teradataConnection(toConnection *conn)
                : toConnection::connectionImpl(conn)
        { }

        /** Return a string representation to address an object.
         *
         * mrj: todo copied from tooracleconnection
         *
         *
         * Checks if identifier has illegal characters, starts with digit, is a reserved
         * word etc. - if so - returns it enclosed with quotes (otherwise returns the same string).
         * Note that when identifier name returned from oracle data dictionary is in lowercase
         * - it MUST be enclosed with quotes (case insensitive "normal" identifiers are always
         * returned in uppercase).
         * @param name The name to be quoted.
         * @param quoteLowercase Enclose in quotes when identifier has lowercase letters.
         *   When processing data returned by dada dictionary quoteLowercase should be true
         *   When processing data entered by user quoteLowercase should be false
         * @return String addressing table.
         */
        virtual QString quote(const QString &name, const bool quoteLowercase)
        {
            bool ok = true;
            // Identifiers starting with digit should be quoted
            if (name.at(0).isDigit())
                ok = false;
            else
            {
                for (int i = 0; i < name.length(); i++)
                {
                    if ((name.at(i).toUpper() != name.at(i) && quoteLowercase) || !toIsIdent(name.at(i)))
                        ok = false;
                }
            }

            // Check if given identified is a reserved word
            int i = 0;
            while (ok && (DefaultKeywords[i] != NULL))
            {
                if (name.compare(DefaultKeywords[i], Qt::CaseInsensitive) == 0)
                    ok = false;
                i++;
            }
            if (ok)
            {
                if (toConfigurationSingle::Instance().objectNamesUpper())
                    return name.toUpper();
                else
                    return name.toLower();
            }
            else
                return QString::fromLatin1("\"") + name + QString::fromLatin1("\"");
        }
        virtual QString unQuote(const QString &str)
        {
            if (str.at(0).toLatin1() == '\"' && str.at(str.length() - 1).toLatin1() == '\"')
                return str.left(str.length() - 1).right(str.length() - 2);
            return str.toUpper();
        }

        virtual void commit(toConnectionSub *sub)
        {
            // todo
        }
        virtual void rollback(toConnectionSub *sub)
        {
            // todo
        }

        virtual toConnectionSub *createConnection(void)
        {
            toConnection &conn = connection();
            return new teradataSub(conn);
        }

        void closeConnection(toConnectionSub *conn)
        {
            qDebug() << "close connection";
            teradataSub *sub = static_cast<teradataSub *>(conn);
            if(sub)
                sub->close();
            delete conn;
        }

        virtual QString version(toConnectionSub *sub)
        {
            teradataSub *conn = teradataConv(sub);
            try
            {
                QString version = conn->Version;
                QStringList vl = version.split('.');
                QString ve;
                QString verrj;
                for(QStringList::iterator vi = vl.begin(); vi != vl.end(); ++vi )
                {
                    ve = *vi;
                    verrj += ve.rightJustified(2, '0');
                }
                qDebug() << "returning version verrj" << verrj;
                return verrj;
            }
            catch (...)
            {
                // Ignore any errors here
            }
            return QString();
        }

        virtual toQuery::queryImpl *createQuery(toQuery *query, toConnectionSub *sub)
        {
            return new teradataQuery(query, teradataConv(sub));
        }

        virtual void execute(toConnectionSub *sub, const QString &sql, toQList &params)
        {
            toQuery query(connection(), sql, params);
        }

        /** Extract available objects to query for connection. Any access to the
         * database should always be run using a long running query. If something
         * goes wrong should throw exception.
         * @return List of available objects.
         */
        virtual std::list<toConnection::objectName> objectNames(void)
        {
            qDebug() << "objectNames";

            std::list<toConnection::objectName> ret;

            try
            {
                toQuery databases(connection(), SQLListDatabases);
                while(!databases.eof())
                {
                    QString db = databases.readValueNull();
                    qDebug() << "got db" << db;

                    toConnection::objectName cur;
                    cur.Owner = db;
                    cur.Type = "DATABASE";
                    cur.Name = db;
                    ret.insert(ret.end(), cur);

                    try
                    {
                        toQuery tables(connection(), SQLListTablesDatabases, db);
                        while(!tables.eof())
                        {
                            toConnection::objectName table;
                            table.Name = tables.readValueNull();
                            table.Owner = db;
                            table.Type = "TABLE";
                            table.Comment = tables.readValueNull();
                            ret.insert(ret.end(), table);
                        }
                    }
                    catch(...)
                    {
                        qDebug() << "table failed";
                    }
                }
            }
            catch(...)
            {
                qDebug() << "databases failed";
            }

            return ret;
        }

        /** Get synonyms available for connection. Any access to the
         * database should always be run using a long running query. If something
         * goes wrong should throw exception.
         * @param objects Available objects for the connection. Objects
         *                are sorted in owner and name order. Don't modify
         *                this list.
         * @return Map of synonyms to objectnames.
         */
        virtual std::map<QString, toConnection::objectName> synonymMap(
            std::list<toConnection::objectName> &objects)
        {
            qDebug() << "synonymMap";
            std::map<QString, toConnection::objectName> ret;
            return ret;
        }

        /* Extract available columns to query for a table.
         * @param table Table to get column for.
         * @return List of columns for table or view.
         */
        virtual toQDescList columnDesc(const toConnection::objectName &table)
        {
            qDebug() << "columnDesc, table:" << table.Name << ", db:" << table.Owner;
            toQDescList ret;

            try
            {
                toQuery desc(connection(), SQLListColumns, table.Owner, table.Name);
                while(!desc.eof())
                {
                    toQDescribe d;
                    d.Name = desc.readValueNull();
                    d.Datatype = desc.readValueNull();
                    d.Comment = desc.readValueNull();

                    ret.insert(ret.end(), d);
                }
            }
            catch(...)
            {
                qDebug() << "columnDesc failed";
            }

            return ret;
        }
    };

    toTeradataProvider(void)
            : toConnectionProvider("Teradata", false)
    {}

    virtual void initialize(void)
    {
        addProvider("Teradata");
    }

    virtual toConnection::connectionImpl *provideConnection(const QString &, toConnection *conn)
    {
        return new teradataConnection(conn);
    }
    virtual std::list<QString> providedOptions(const QString &)
    {
        std::list<QString> ret;
        // todo
//         ret.insert(ret.end(), "*SQL*Net");
//         ret.insert(ret.end(), "-");
//         ret.insert(ret.end(), "SYS_OPER");
//         ret.insert(ret.end(), "SYS_DBA");
        return ret;
    }
    virtual std::list<QString> providedHosts(const QString &)
    {
        std::list<QString> ret;
        // default port
        ret.insert(ret.end(), ":1025");

        return ret;
    }
    virtual std::list<QString> providedDatabases(const QString &,
            const QString &host,
            const QString &,
            const QString &)
    {
        std::list<QString> ret;
        // todo
        return ret;
    }
    virtual QWidget *providerConfigurationTab(const QString &provider, QWidget *parent)
    {
        return 0;
    }
};

static toTeradataProvider TeradataProvider;

static QString QueryParam(const QString &in,
                          toQList &params)
{
    QString ret;
    bool inString = false;
    toQList::iterator cpar = params.begin();
    QString query = QString(in);

    std::map<QString, QString> binds;

    for (int i = 0;i < query.length();i++)
    {
        QChar rc = query.at(i);
        char  c  = rc.toLatin1();

        char nc = 0;
        if (i + 1 < query.length())
            nc = query.at(i + 1).toLatin1();

        switch (c)
        {
        case '\\':
            ret += rc;
            ret += query.at(++i);
            break;
        case '\'':
            inString = !inString;
            ret += rc;
            break;
        case ':':
            // mostly for postgres-style casts, ignore ::
            if (nc == ':')
            {
                ret += rc;
                ret += nc;
                i++;
                break;
            }

            if (!inString)
            {
                QString nam;
                for (i++;i < query.length();i++)
                {
                    rc = query.at(i);
                    if (!rc.isLetterOrNumber())
                        break;
                    nam += rc;
                }
                c = rc.toLatin1();
                QString in;
                if (c == '<')
                {
                    for (i++;i < query.length();i++)
                    {
                        rc = query.at(i);
                        c = rc.toLatin1();
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                        in += rc;
                    }
                }
                i--;

                QString str;
                QString tmp;
                if (nam.isEmpty())
                    break;

                if (binds.find(nam) != binds.end())
                {
                    ret += binds[nam];
                    break;
                }
                if (cpar == params.end())
                    throw toConnection::exception(QString::fromLatin1("Not all bind variables supplied"), i);
                if ((*cpar).isNull())
                {
                    str = QString::fromLatin1("NULL");
                }
                else if ((*cpar).isInt() || (*cpar).isDouble())
                {
                    str = QString(*cpar);
                }
                tmp = (*cpar);
                cpar++;

                if (str.isNull())
                {
                    if (in != QString::fromLatin1("noquote"))
                        str += QString::fromLatin1("'");

                    for (int j = 0;j < tmp.length();j++)
                    {
                        QChar d = tmp.at(j);
                        switch (d.toLatin1())
                        {
                        case 0:
                            str += QString::fromLatin1("\\0");
                            break;
                        case '\n':
                            str += QString::fromLatin1("\\n");
                            break;
                        case '\t':
                            str += QString::fromLatin1("\\t");
                            break;
                        case '\r':
                            str += QString::fromLatin1("\\r");
                            break;
                        case '\'':
                            str += QString::fromLatin1("\\\'");
                            break;
                        case '\"':
                            str += QString::fromLatin1("\\\"");
                            break;
                        case '\\':
                            str += QString::fromLatin1("\\\\");
                            break;
                        default:
                            str += d;
                        }
                    }
                    str += QString::fromLatin1("'");
                }
                binds[nam] = str;
                ret += str;
                break;
            }
        default:
            ret += rc;
        }
    }
    return ret;
}
