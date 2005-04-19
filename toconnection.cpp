/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

#include <string>
#include <time.h>

#include <qapplication.h>
#include <qdir.h>
#include <qprogressdialog.h>
#include <qtextstream.h>
#include <qwidget.h>

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

#define TO_DEBUGOUT(x) fprintf(stderr,(const char *)x);

// Connection provider implementation

std::map<QCString, toConnectionProvider *> *toConnectionProvider::Providers;
std::map<QCString, toConnectionProvider *> *toConnectionProvider::Types;

static int NumberFormat;
static int NumberDecimals;

void toConnectionProvider::checkAlloc(void)
{
    if (!Providers)
        Providers = new std::map<QCString, toConnectionProvider *>;
}

void toConnectionProvider::addProvider(const QCString &provider)
{
    checkAlloc();
    Provider = provider;
    (*Providers)[Provider] = this;
}

toConnectionProvider::toConnectionProvider(const QCString &provider, bool add
                                              )
{
    Provider = provider;
    if (add
       )
        addProvider(provider);
    if (!Types)
        Types = new std::map<QCString, toConnectionProvider *>;
    (*Types)[provider] = this;
}

std::list<QString> toConnectionProvider::providedOptions(const QCString &)
{
    std::list<QString> ret;
    return ret;
}

void toConnectionProvider::removeProvider(const QCString &provider)
{
    std::map<QCString, toConnectionProvider *>::iterator i = Providers->find(provider);
    if (i != Providers->end())
        Providers->erase(i);
}

toConnectionProvider::~toConnectionProvider()
{
    try
    {
        if (!Provider.isEmpty())
            removeProvider(Provider);
        std::map<QCString, toConnectionProvider *>::iterator i = Types->find(Provider);
        if (i != Types->end())
            Types->erase(i);
    }
    catch (...)
    {}
}

std::list<QString> toConnectionProvider::providedHosts(const QCString &)
{
    std::list<QString> ret;
    return ret;
}

std::list<QCString> toConnectionProvider::providers()
{
    std::list<QCString> ret;
    if (!Providers)
        return ret;
    for (std::map<QCString, toConnectionProvider *>::iterator i = Providers->begin();i != Providers->end();i++)
        ret.insert(ret.end(), (*i).first);
    return ret;
}

void toConnectionProvider::initializeAll(void)
{
    if (Types)
        for (std::map<QCString, toConnectionProvider *>::iterator i = Types->begin();
                i != Types->end();i++)
            (*i).second->initialize();
}

toConnectionProvider &toConnectionProvider::fetchProvider(const QCString &provider)
{
    checkAlloc();
    std::map<QCString, toConnectionProvider *>::iterator i = Providers->find(provider);
    if (i == Providers->end())
        throw QT_TRANSLATE_NOOP("toConnectionProvider", "Tried to fetch unknown provider %1").arg(provider);
    return *((*i).second);
}

std::list<QString> toConnectionProvider::options(const QCString &provider)
{
    return fetchProvider(provider).providedOptions(provider);
}

QWidget *toConnectionProvider::configurationTab(const QCString &provider, QWidget *parent)
{
    return fetchProvider(provider).providerConfigurationTab(provider, parent);
}

toConnection::connectionImpl *toConnectionProvider::connection(const QCString &provider,
        toConnection *conn)
{
    return fetchProvider(provider).provideConnection(provider, conn);
}

std::list<QString> toConnectionProvider::hosts(const QCString &provider)
{
    return fetchProvider(provider).providedHosts(provider);
}

std::list<QString> toConnectionProvider::databases(const QCString &provider, const QString &host,
        const QString &user, const QString &pwd)
{
    return fetchProvider(provider).providedDatabases(provider, host, user, pwd);
}

const QString &toConnectionProvider::config(const QCString &tag, const QCString &def)
{
    QCString str = Provider;
    str.append(":");
    str.append(tag);
    return toTool::globalConfig(str, def);
}

void toConnectionProvider::setConfig(const QCString &tag, const QCString &def)
{
    QCString str = Provider;
    str.append(":");
    str.append(tag);
    toTool::globalSetConfig(str, def);
}

QWidget *toConnectionProvider::providerConfigurationTab(const QCString &, QWidget *)
{
    return NULL;
}

// Query value implementation

toQValue::toQValue(int i)
{
    Type = intType;
    Value.Int = i;
}

toQValue::toQValue(double i)
{
    Type = doubleType;
    Value.Double = i;
}

toQValue::toQValue(const toQValue &copy)
{
    Type = copy.Type;
    switch (Type)
    {
    case intType:
        Value.Int = copy.Value.Int;
        break;
    case doubleType:
        Value.Double = copy.Value.Double;
        break;
    case stringType:
        Value.String = new QString(*copy.Value.String);
        break;
    case binaryType:
        Value.Array = new QByteArray(*copy.Value.Array);
        break;
    case nullType:
        break;
    }
}

const toQValue &toQValue::operator = (const toQValue &copy)
{
    if (Type == stringType)
        delete Value.String;
    else if (Type == binaryType)
        delete Value.Array;

    Type = copy.Type;
    switch (Type)
    {
    case intType:
        Value.Int = copy.Value.Int;
        break;
    case doubleType:
        Value.Double = copy.Value.Double;
        break;
    case stringType:
        Value.String = new QString(*copy.Value.String);
        break;
    case binaryType:
        Value.Array = new QByteArray(*copy.Value.Array);
        break;
    case nullType:
        break;
    }
    return *this;
}

bool toQValue::operator == (const toQValue &val) const
{
    if (isNull() && val.isNull())
        return true;
    if (val.Type != Type)
        return false;
    switch (Type)
    {
    case intType:
        return (val.Value.Int == Value.Int);
    case doubleType:
        return (val.Value.Double == Value.Double);
    case stringType:
        return (*val.Value.String) == (*Value.String);
    case binaryType:
        return (*val.Value.Array) == (*Value.Array);
    case nullType:
        break;
    }
    return false;  // Should never get here
}

toQValue::toQValue(const QString &str)
{
    Type = stringType;
    Value.String = new QString(str);
}

toQValue::toQValue(void)
{
    Type = nullType;
}

toQValue::~toQValue()
{
    if (Type == stringType)
        delete Value.String;
    else if (Type == binaryType)
        delete Value.Array;
}

bool toQValue::isInt(void) const
{
    return Type == intType;
}

bool toQValue::isDouble(void) const
{
    return Type == doubleType;
}

bool toQValue::isString(void) const
{
    return Type == stringType;
}

bool toQValue::isBinary(void) const
{
    return Type == binaryType;
}

bool toQValue::isNull(void) const
{
    if (Type == nullType)
        return true;
    if (Type == stringType && Value.String->isNull())
        return true;
    return false;
}

const QByteArray &toQValue::toByteArray() const
{
    if (Type != binaryType)
        throw qApp->translate("toQValue", "Tried to convert non binary value to binary");
    return *Value.Array;
}

static char HexString[] = "0123456789ABCDEF";

QCString toQValue::utf8(void) const
{
    switch (Type)
    {
    case nullType:
        {
            QCString ret;
            return ret;
        }
    case intType:
        {
            QCString ret;
            ret.setNum(Value.Int);
            return ret;
        }
    case doubleType:
        {
            QCString ret;
            if (Value.Double != int(Value.Double))
            {
                ret.setNum(Value.Double);
                return ret;
            }
            char buf[100];
            switch (NumberFormat)
            {
            default:
                ret.setNum(Value.Double);
                break;
            case 1:
                sprintf(buf, "%E", Value.Double);
                ret = buf;
                break;
            case 2:
                sprintf(buf, "%0.*f", NumberDecimals, Value.Double);
                ret = buf;
                break;
            }
            return ret;
        }
    case stringType:
        return Value.String->utf8();
    case binaryType:
        {
            QCString ret(Value.Array->size()*2 + 1);
            for (unsigned int i = 0;i < Value.Array->size();i++)
            {
                unsigned char c = (unsigned char)Value.Array->at(i);
                ret.at(i*2) = HexString[(c / 16) % 16];
                ret.at(i*2 + 1) = HexString[c % 16];
            }
            ret.at(Value.Array->size()*2) = 0;
            return ret;
        }
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

int toQValue::toInt(void) const
{
    switch (Type)
    {
    case nullType:
        return 0;
    case intType:
        return Value.Int;
    case doubleType:
        return int(Value.Double);
    case stringType:
        return Value.String->toInt();
    case binaryType:
        throw qApp->translate("toQValue", "Can't transform binary value to int");
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

double toQValue::toDouble(void) const
{
    switch (Type)
    {
    case nullType:
        return 0;
    case intType:
        return double(Value.Int);
    case doubleType:
        return Value.Double;
    case stringType:
        return Value.String->toDouble();
    case binaryType:
        throw qApp->translate("toQValue", "Can't transform binary value to double");
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

void toQValue::setNumberFormat(int format, int decimals)
{
    NumberFormat = format;
    NumberDecimals = decimals;
}

QString toQValue::formatNumber(double number)
{
    if (number == int(number))
        return QString::number(number);
    switch (NumberFormat)
    {
    case 1:
        {
            char buf[100];
            sprintf(buf, "%E", number);
            return buf;
        }
    case 2:
        {
            char buf[100];
            sprintf(buf, "%0.*f", NumberDecimals, number);
            return buf;
        }
    default:
        return QString::number(number);
    }
}

int toQValue::numberFormat(void)
{
    return NumberFormat;
}

int toQValue::numberDecimals(void)
{
    return NumberDecimals;
}

toQValue toQValue::createBinary(const QByteArray &arr)
{
    toQValue ret;
    ret.Type = binaryType;
    ret.Value.Array = new QByteArray(arr);
    return ret;
}

toQValue toQValue::createFromHex(const QCString &hex)
{
    QByteArray arr((hex.length() + 1) / 2);
    for (unsigned int i = 0;i < hex.length();i += 2)
    {
        int num;
        char c = hex.at(i);
        if (c >= 'a')
            num = c - 'a';
        else if (c >= 'A')
            num = c - 'A';
        else
            num = c - '0';
        num <<= 4;
        c = hex.at(i + 1);
        if (c >= 'a')
            num += c - 'a';
        else if (c >= 'A')
            num += c - 'A';
        else
            num += c - '0';
        arr.at(i / 2) = num;
    }
    return createBinary(arr);
}

toQValue toQValue::createFromHex(const QString &hex)
{
    QByteArray arr((hex.length() + 1) / 2);
    for (unsigned int i = 0;i < hex.length();i += 2)
    {
        int num;
        char c = hex.at(i);
        if (c >= 'a')
            num = c - 'a';
        else if (c >= 'A')
            num = c - 'A';
        else
            num = c - '0';
        num <<= 4;
        c = hex.at(i + 1);
        if (c >= 'a')
            num += c - 'a';
        else if (c >= 'A')
            num += c - 'A';
        else
            num += c - '0';
        arr.at(i / 2) = num;
    }
    return createBinary(arr);
}

toQValue::operator QString() const
{
    switch (Type)
    {
    case nullType:
        return QString::null;
    case doubleType:
        return formatNumber(Value.Double);
    case intType:
        return QString::number(Value.Int);
    case stringType:
        return *Value.String;
    case binaryType:
        {
            QString ret;
            for (unsigned int i = 0;i < Value.Array->size();i++)
            {
                unsigned char c = (unsigned char)Value.Array->at(i);
                ret += HexString[(c / 16) % 16];
                ret += HexString[c % 16];
            }
            return ret;
        }
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

// toQuery implementation

toQuery::toQuery(toConnection &conn, const toSQL &sql,
                 const QString &arg1, const QString &arg2,
                 const QString &arg3, const QString &arg4,
                 const QString &arg5, const QString &arg6,
                 const QString &arg7, const QString &arg8,
                 const QString &arg9)
        : Connection(conn), ConnectionSub(conn.mainConnection()), SQL(sql(Connection))
{
    Mode = Normal;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const QString &sql,
                 const QString &arg1, const QString &arg2,
                 const QString &arg3, const QString &arg4,
                 const QString &arg5, const QString &arg6,
                 const QString &arg7, const QString &arg8,
                 const QString &arg9)
        : Connection(conn), ConnectionSub(conn.mainConnection()), SQL(sql.utf8())
{
    Mode = Normal;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const toSQL &sql, const toQList &params)
        : Connection(conn), ConnectionSub(conn.mainConnection()), Params(params), SQL(sql(conn))
{
    Mode = Normal;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const QString &sql, const toQList &params)
        : Connection(conn), ConnectionSub(conn.mainConnection()), Params(params), SQL(sql.utf8())
{
    Mode = Normal;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, queryMode mode, const toSQL &sql, const toQList &params)
        : Connection(conn),
        Params(params),
        SQL(sql(conn))
{
    Mode = mode;

    switch (Mode)
    {
    case Normal:
    case All:
        ConnectionSub = conn.mainConnection();
        break;
    case Background:
        ConnectionSub = conn.backgroundConnection();
        break;
    case Long:
        ConnectionSub = conn.longConnection();
        break;
    }

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, queryMode mode, const QString &sql, const toQList &params)
        : Connection(conn),
        Params(params),
        SQL(sql.utf8())
{
    Mode = mode;

    switch (Mode)
    {
    case Normal:
    case All:
        ConnectionSub = conn.mainConnection();
        break;
    case Background:
        ConnectionSub = conn.backgroundConnection();
        break;
    case Long:
        ConnectionSub = conn.longConnection();
        break;
    }

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Connection.freeConnection(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, queryMode mode)
        : Connection(conn)
{
    Mode = mode;

    switch (Mode)
    {
    case Normal:
    case All:
        ConnectionSub = conn.mainConnection();
        break;
    case Background:
        ConnectionSub = conn.backgroundConnection();
        break;
    case Long:
        ConnectionSub = conn.longConnection();
        break;
    }

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        throw;
    }
    ConnectionSub->setQuery(this);
}

void toQuery::execute(const toSQL &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql(Connection);
    Params = params;
    Query->execute();
}

void toQuery::execute(const QString &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql.utf8();
    Params = params;
    Query->execute();
}

toQuery::~toQuery()
{
    toBusy busy;
    delete Query;
    try
    {
        if (ConnectionSub->query() == this)
            ConnectionSub->setQuery(NULL);
        Connection.freeConnection(ConnectionSub);
    }
    catch (...)
    {}
}

bool toQuery::eof(void)
{
    if (Mode == All)
    {
        if (Query->eof())
        {
            Connection.Lock.lock();
            bool found = false;
            try
            {
                std::list<toConnectionSub *> &cons = Connection.connections();
                for (std::list<toConnectionSub *>::iterator i = cons.begin();i != cons.end();i++)
                {
                    if (*i == ConnectionSub)
                    {
                        i++;
                        if (i != cons.end())
                        {
                            ConnectionSub = *i;
                            Connection.Lock.unlock();
                            found = true;
                            delete Query;
                            Query = NULL;
                            Query = connection().Connection->createQuery(this, ConnectionSub);
                            Query->execute();
                            Connection.Lock.lock();
                        }
                        break;
                    }
                }
                Connection.Lock.unlock();
            }
            catch (...)
            {}
        }
    }
    return Query->eof();
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const toSQL &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const QString &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const toSQL &sql,
                               const QString &arg1, const QString &arg2,
                               const QString &arg3, const QString &arg4,
                               const QString &arg5, const QString &arg6,
                               const QString &arg7, const QString &arg8,
                               const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const QString &sql,
                               const QString &arg1, const QString &arg2,
                               const QString &arg3, const QString &arg4,
                               const QString &arg5, const QString &arg6,
                               const QString &arg7, const QString &arg8,
                               const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQValue toQuery::readValue(void)
{
    toBusy busy;
    if (Connection.Abort)
        throw qApp->translate("toQuery", "Query aborted");
    if (Mode == All)
        eof();
    return toNull(Query->readValue());
}

toQValue toQuery::readValueNull(void)
{
    toBusy busy;
    if (Connection.Abort)
        throw qApp->translate("toQuery", "Query aborted");
    if (Mode == All)
        eof();
    return Query->readValue();
}

void toQuery::cancel(void)
{
    Query->cancel();
}

// toConnection implementation

void toConnection::addConnection(void)
{
    toBusy busy;
    toConnectionSub *sub = Connection->createConnection();
    toLocker lock (Lock)
        ;
    Connections.insert(Connections.end(), sub);
    toQList params;
    for (std::list<QString>::iterator i = InitStrings.begin();i != InitStrings.end();i++)
    {
        try
        {
            Connection->execute(sub, (*i).utf8(), params);
        }
        TOCATCH
    }
}

toConnection::toConnection(const QCString &provider,
                           const QString &user, const QString &password,
                           const QString &host, const QString &database,
                           const std::set
                               <QString> &options, bool cache)
            : Provider(provider), User(user), Password(password), Host(host), Database(database), Options(options)
{
    BackgroundConnection = NULL;
    BackgroundCount = 0;
    Connection = toConnectionProvider::connection(Provider, this);
    addConnection();
    Version = Connection->version(mainConnection());
    NeedCommit = Abort = false;
    ReadingCache = false;
    if (cache)
    {
        if (toTool::globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt() == 1)
            readObjects();
    }
    else
    {
        ReadingValues.up();
        ReadingValues.up();
    }
}

toConnection::toConnection(const toConnection &conn)
        : Provider(conn.Provider),
        User(conn.User),
        Password(conn.Password),
        Host(conn.Host),
        Database(conn.Database),
        Options(conn.Options)
{
    BackgroundConnection = NULL;
    BackgroundCount = 0;
    Connection = toConnectionProvider::connection(Provider, this);
    addConnection();
    Version = Connection->version(mainConnection());
    ReadingValues.up();
    ReadingValues.up();
    ReadingCache = false;
    NeedCommit = Abort = false;
}

std::list<QString> toConnection::running(void)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    std::list<QString> ret;
    toConnectionSub *sub = (*(Connections.begin()));
    if (sub && sub->query())
        ret.insert(ret.end(), sub->query()->sql());
    if (BackgroundConnection && BackgroundConnection->query())
        ret.insert(ret.end(), BackgroundConnection->query()->sql());
    for (std::list<toConnectionSub *>::const_iterator i = Running.begin();i != Running.end();i++)
    {
        sub = *i;
        if (sub && sub->query())
            ret.insert(ret.end(), sub->query()->sql());
    }
    return ret;
}

void toConnection::cancelAll(void)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Running.begin();i != Running.end();i++)
        (*i)->cancel();
}

toConnection::~toConnection()
{
    toBusy busy;

    {
        toLocker lock (Lock)
            ;
        {
            for (std::list<QObject *>::iterator i = Widgets.begin();i != Widgets.end();i = Widgets.begin())
            {
                delete (*i);
            }
        }
        {
            for (std::list<toConnectionSub *>::iterator i = Running.begin();i != Running.end();i++)
                try
                {
                    (*i)->cancel();
                }
                catch (...)
                {}
        }
    }
    Abort = true;
    if (ReadingCache)
    {
        ReadingValues.down();
        ReadingValues.down();
    }
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        try
        {
            Connection->closeConnection(*i);
        }
        catch (...)
        {}
    }
    delete Connection;
}

toConnectionSub *toConnection::mainConnection()
{
    if (Connection->handleMultipleQueries())
    {
        toLocker lock (Lock)
            ;
        return (*(Connections.begin()));
    }
    else
    {
        return longConnection();
    }
}

toConnectionSub *toConnection::backgroundConnection()
{
    if (!Connection->handleMultipleQueries())
        return longConnection();
    if (toTool::globalConfig(CONF_BKGND_CONNECT, "").isEmpty())
        return mainConnection();
    Lock.lock();
    if (!BackgroundConnection)
    {
        Lock.unlock();
        toConnectionSub *tmp = longConnection();
        Lock.lock();

        BackgroundConnection = tmp;
        BackgroundCount = 0;
    }
    BackgroundCount++;
    Lock.unlock();
    return BackgroundConnection;
}

toConnectionSub *toConnection::longConnection()
{
    Lock.lock();
    bool multiple = Connection->handleMultipleQueries();
    if ((multiple && Connections.size() == 1) ||
            (!multiple && Connections.size() == 0))
    {
        Lock.unlock();
        addConnection();
    }
    else
        Lock.unlock();
    toLocker lock (Lock)
        ;
    std::list<toConnectionSub *>::iterator i = Connections.begin();
    if (multiple)
        i++;
    toConnectionSub *ret = (*i);
    Connections.erase(i);
    Running.insert(Running.end(), ret);
    return ret;
}

void toConnection::freeConnection(toConnectionSub *sub)
{
    toLocker lock (Lock)
        ;
    if (sub == BackgroundConnection)
    {
        BackgroundCount--;
        if (BackgroundCount > 0)
            return ;
        BackgroundConnection = NULL;
    }
    {
        for (std::list<toConnectionSub *>::iterator i = Running.begin();i != Running.end();i++)
        {
            if (*i == sub)
            {
                Running.erase(i);
                break;
            }
        }
    }
    {
        for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
        {
            if (*i == sub)
                return ;
        }
    }
    Connections.insert(Connections.end(), sub);
}

void toConnection::commit(void)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
        Connection->commit(*i);
    while (Connections.size() > 2)
    {
        std::list<toConnectionSub *>::iterator i = Connections.begin();
        i++;
        delete (*i);
        Connections.erase(i);
    }
    setNeedCommit(false);
}

void toConnection::rollback(void)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        Connection->rollback(*i);
    }
    while (Connections.size() > 2)
    {
        std::list<toConnectionSub *>::iterator i = Connections.begin();
        i++;
        delete (*i);
        Connections.erase(i);
    }
    setNeedCommit(false);
}

void toConnection::delWidget(QObject *widget)
{
    for (std::list<QObject *>::iterator i = Widgets.begin();i != Widgets.end();i++)
    {
        if ((*i) == widget)
        {
            Widgets.erase(i);
            break;
        }
    }
}

bool toConnection::closeWidgets(void)
{
    for (std::list<QObject *>::iterator i = Widgets.begin();i != Widgets.end();i = Widgets.begin())
    {
        if ((*i)->inherits("QWidget"))
        {
            QWidget * widget = static_cast<QWidget *>(*i);
            if (!widget->close(true))
                return false;
        }
        else
            delete *i;
        std::list<QObject *>::iterator nextI = Widgets.begin();
        if (i == nextI)
            throw qApp->translate("toConnection", "All tool widgets need to have autodelete flag set");
    }
    return true;
}

QString toConnection::description(bool version) const
{
    QString ret(User);
    ret += QString::fromLatin1("@");
    ret += Database;
    if (!Host.isEmpty() && Host != "SQL*Net")
    {
        ret += QString::fromLatin1(".");
        ret += Host;
    }

    if (version)
    {
        if (!Version.isEmpty())
        {
            ret += QString::fromLatin1(" [");
            ret += QString::fromLatin1(Version);
            ret += QString::fromLatin1("]");
        }
    }
    return ret;
}

void toConnection::addInit(const QString &sql)
{
    delInit(sql);
    InitStrings.insert(InitStrings.end(), sql);
}

void toConnection::delInit(const QString &sql)
{
    std::list<QString>::iterator i = InitStrings.begin();
    while (i != InitStrings.end())
    {
        if ((*i) == sql)
            i = InitStrings.erase(i);
        else
            i++;
    }
}

const std::list<QString> toConnection::initStrings() const
{
    return InitStrings;
}

void toConnection::parse(const QString &sql)
{
    toBusy busy;
    Connection->parse(mainConnection(), sql.utf8());
}

void toConnection::parse(const toSQL &sql)
{
    toBusy busy;
    Connection->parse(mainConnection(), toSQL::sql(sql, *this));
}

void toConnection::execute(const toSQL &sql, toQList &params)
{
    toBusy busy;
    Connection->execute(mainConnection(), toSQL::sql(sql, *this), params);
}

void toConnection::execute(const QString &sql, toQList &params)
{
    toBusy busy;
    Connection->execute(mainConnection(), sql.utf8(), params);
}

void toConnection::execute(const toSQL &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;
    Connection->execute(mainConnection(), toSQL::sql(sql, *this), args);
}

void toConnection::execute(const QString &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;
    Connection->execute(mainConnection(), sql.utf8(), args);
}

void toConnection::allExecute(const toSQL &sql, toQList &params)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        try
        {
            Connection->execute(*i, toSQL::sql(sql, *this), params);
        }
        TOCATCH
    }
}

void toConnection::allExecute(const QString &sql, toQList &params)
{
    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        try
        {
            Connection->execute(*i, sql.utf8(), params);
        }
        TOCATCH
    }
}

void toConnection::allExecute(const toSQL &sql,
                              const QString &arg1, const QString &arg2,
                              const QString &arg3, const QString &arg4,
                              const QString &arg5, const QString &arg6,
                              const QString &arg7, const QString &arg8,
                              const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        try
        {
            Connection->execute(*i, toSQL::sql(sql, *this), args);
        }
        TOCATCH
    }
}

void toConnection::allExecute(const QString &sql,
                              const QString &arg1, const QString &arg2,
                              const QString &arg3, const QString &arg4,
                              const QString &arg5, const QString &arg6,
                              const QString &arg7, const QString &arg8,
                              const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;
    toLocker lock (Lock)
        ;
    for (std::list<toConnectionSub *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        try
        {
            Connection->execute(*i, sql.utf8(), args);
        }
        TOCATCH
    }
}

const QCString &toConnection::provider(void) const
{
    return Provider;
}

QString toConnection::cacheDir()
{
    QString home = QDir::homeDirPath();
    QString dirname = toTool::globalConfig(CONF_CACHE_DIR, "");

    if (dirname.isEmpty())
    {
#ifdef WIN32
        if (getenv("TEMP"))
            dirname = QString(getenv("TEMP"));
        else
#endif

            dirname = QString(home);
#ifdef TOAD

        dirname += "/.toad_cache";
#else

        dirname += "/.tora_cache";
#endif

    }
    return dirname;
}

QString toConnection::cacheFile()
{
    QString dbname (description(false).stripWhiteSpace());

    return (cacheDir() + "/" + dbname).simplifyWhiteSpace();
}

bool toConnection::loadDiskCache()
{
    if (toTool::globalConfig(CONF_CACHE_DISK, DEFAULT_CACHE_DISK).isEmpty())
        return false;

    toConnection::objectName *cur = 0;
    int objCounter = 0;
    int synCounter = 0;

    QString filename = cacheFile();

    QFile file(filename);

    if (!QFile::exists(filename))
        return false;

    QFileInfo fi(file);
    QDateTime today;
    if (fi.lastModified().addDays(toTool::globalConfig(CONF_CACHE_TIMEOUT, DEFAULT_CACHE_TIMEOUT).toInt()) < today)
        return false;

    /** read in all data
     */

    if (!file.open(IO_ReadOnly))
        return false;

    QString data = file.readAll();

    /** build cache lists
     */

    QStringList records = QStringList::split("\x1D", data, true);
    for ( QStringList::Iterator i = records.begin(); i != records.end(); i++)
    {
        objCounter++;
        QStringList record = QStringList::split("\x1E", (*i), true);
        QStringList::Iterator rec = record.begin();
        cur = new objectName;
        (*cur).Owner = (*rec);
        rec++;
        (*cur).Name = (*rec);
        rec++;
        (*cur).Type = (*rec);
        rec++;
        (*cur).Comment = (*rec);
        rec++;
        QStringList slist = QStringList::split("\x1F", (*rec), false);
        for (QStringList::Iterator s = slist.begin(); s != slist.end(); s++)
        {
            SynonymMap[(*s)] = (*cur);
            (*cur).Synonyms.insert((*cur).Synonyms.end(), (*s));
            synCounter++;
        }
        ObjectNames.insert(ObjectNames.end(), (*cur));
        delete cur;
        cur = 0;
    }
    return true;
}

void toConnection::writeDiskCache()
{
    QString text;
    long objCounter = 0;
    long synCounter = 0;

    if (toTool::globalConfig(CONF_CACHE_DISK, DEFAULT_CACHE_DISK).isEmpty())
        return ;


    QString filename(cacheFile());

    /** check pathnames and create
     */

    QString dirname(cacheDir());
    QDir dir;
    dir.setPath(dirname);

    if (!dir.exists(dirname))
        dir.mkdir(dirname);


    /** build record to write out
     */

    QStringList record;
    QStringList records;
    QStringList recordSynonym;
    for (std::list<objectName>::iterator i = ObjectNames.begin();i != ObjectNames.end();i++)
    {
        record.clear();
        record.append((*i).Owner);
        record.append((*i).Name);
        record.append((*i).Type);
        record.append((*i).Comment);
        for (std::list<QString>::iterator s = (*i).Synonyms.begin();s != (*i).Synonyms.end();s++)
        {
            recordSynonym.append((*s));
            synCounter++;
        }
        record.append(recordSynonym.join("\x1F"));
        recordSynonym.clear();
        objCounter++;
        records.append(record.join("\x1E"));
    }
    /** open file
     */
    QFile file(filename);
    file.open( IO_ReadWrite | IO_Truncate );
    QTextStream t (&file);
    t << records.join("\x1D");
    file.flush();
    file.close();
}

void toConnection::cacheObjects::run()
{
    bool diskloaded = false;
    try
    {
        diskloaded = Connection.loadDiskCache();
        if (!diskloaded)
        {
            Connection.ObjectNames = Connection.Connection->objectNames();
        }
        Connection.ObjectNames.sort();
        Connection.ReadingValues.up();
        if (!diskloaded)
        {
            Connection.SynonymMap = Connection.Connection->synonymMap(Connection.ObjectNames);
            Connection.writeDiskCache();
        }
    }
    catch (...)
    {
        if (Connection.ReadingValues.getValue() == 0)
            Connection.ReadingValues.up();
    }
    Connection.ReadingValues.up();
}


void toConnection::readObjects(void)
{
    if (toTool::globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt() == 3)
    {
        ReadingCache = false;
        return ;
    }

    if (!ReadingCache)
    {
        ReadingCache = true;
        try
        {
            (new toThread(new cacheObjects(*this)))->start();
        }
        catch (...)
        {
            ReadingCache = false;
        }
    }
}

void toConnection::rereadCache(void)
{

    if (toTool::globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt() == 3)
    {
        ColumnCache.clear();
        return ;
    }

    if (ReadingValues.getValue() < 2 && ReadingCache)
    {
        toStatusMessage(qApp->translate("toConnection",
                                        "Not done caching objects, can not clear unread cache"));
        return ;
    }


    ReadingCache = false;
    while (ReadingValues.getValue() > 0)
        ReadingValues.down();

    ObjectNames.clear();
    ColumnCache.clear();
    SynonymMap.clear();

    /** delete cache file to force reload
     */

    QString filename(cacheFile());

    if (QFile::exists(filename))
        QFile::remove
            (filename);

    readObjects();
}

QString toConnection::quote(const QString &name)
{
    return Connection->quote(name);
}

QString toConnection::unQuote(const QString &name)
{
    return Connection->unQuote(name);
}

bool toConnection::cacheAvailable(bool synonyms, bool block, bool need)
{
    if (toTool::globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt() == 3)
        return true;

    if (!ReadingCache)
    {
        if (!need)
            return true;
        if (toTool::globalConfig(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt() == 2 && !block)
            return true;
        readObjects();
        toMainWidget()->checkCaching();
    }
    if (ReadingValues.getValue() == 0 || (ReadingValues.getValue() == 1 && synonyms == true))
    {
        if (block)
        {
            toBusy busy;
            if (toThread::mainThread())
            {
                QProgressDialog waiting(qApp->translate("toConnection",
                                                        "Waiting for object caching to be completed.\n"
                                                        "Canceling this dialog will probably leave some list of\n"
                                                        "database objects empty."),
                                        qApp->translate("toConnection", "Cancel"),
                                        10,
                                        toMainWidget(),
                                        "progress",
                                        true);
                waiting.setCaption(qApp->translate("toConnection", "Waiting for object cache"));
                int num = 1;

                int waitVal = (synonyms ? 2 : 1);
                do
                {
                    qApp->processEvents();
                    toThread::msleep(100);
                    waiting.setProgress((++num) % 10);
                    if (waiting.wasCancelled())
                        return false;
                }
                while (ReadingValues.getValue() < waitVal);
            }

            ReadingValues.down();
            if (synonyms)
            {
                ReadingValues.down();
                ReadingValues.up();
            }
            ReadingValues.up();
        }
        else
            return false;
    }
    return true;
}

std::list<toConnection::objectName> &toConnection::objects(bool block)
{
    if (!cacheAvailable(false, block))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        static std::list<objectName> ret;
        return ret;
    }

    return ObjectNames;
}

void toConnection::addIfNotExists(toConnection::objectName &obj)
{
    if (!cacheAvailable(true, false))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        return ;
    }
    std::list<toConnection::objectName>::iterator i = ObjectNames.begin();
    while (i != ObjectNames.end() && (*i) < obj)
        i++;
    if (i != ObjectNames.end() && *i == obj) // Already exists, don't add
        return ;
    ObjectNames.insert(i, obj);
}

std::map<QString, toConnection::objectName> &toConnection::synonyms(bool block)
{
    if (!cacheAvailable(true, block))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        static std::map<QString, objectName> ret;
        return ret;
    }

    return SynonymMap;
}

const toConnection::objectName &toConnection::realName(const QString &object,
        QString &synonym,
        bool block)
{
    if (!cacheAvailable(true, block))
        throw qApp->translate("toConnection", "Not done caching objects");

    QString name;
    QString owner;

    QString q = QString::fromLatin1("\"");
    QString c = QString::fromLatin1(".");

    bool quote = false;
    for (unsigned int pos = 0;pos < object.length();pos++)
    {
        if (object.at(pos) == q)
        {
            quote = !quote;
        }
        else
        {
            if (!quote && object.at(pos) == c)
            {
                owner = name;
                name = QString::null;
            }
            else
                name += object.at(pos);
        }
    }

    QString uo = owner.upper();
    QString un = name.upper();

    synonym = QString::null;
    for (std::list<objectName>::iterator i = ObjectNames.begin();i != ObjectNames.end();i++)
    {
        if (owner.isEmpty())
        {
            if (((*i).Name == un || (*i).Name == name) &&
                    ((*i).Owner == user().upper() || (*i).Owner == database()))
                return *i;
        }
        else if (((*i).Name == un || (*i).Name == name) &&
                 ((*i).Owner == uo || (*i).Owner == owner))
            return *i;
    }
    if (owner.isEmpty())
    {
        std::map<QString, objectName>::iterator i = SynonymMap.find(name);
        if (i == SynonymMap.end() && un != name)
        {
            i = SynonymMap.find(un);
            synonym = un;
        }
        else
            synonym = name;
        if (i != SynonymMap.end())
        {
            return (*i).second;
        }
    }
    throw qApp->translate("toConnection", "Object %1 not available for %2").arg(object).arg(user());
}

const toConnection::objectName &toConnection::realName(const QString &object, bool block)
{
    QString dummy;
    return realName(object, dummy, block);
}

toQDescList &toConnection::columns(const objectName &object, bool nocache)
{
    std::map<objectName, toQDescList>::iterator i = ColumnCache.find(object);
    if (i == ColumnCache.end() || nocache)
    {
        ColumnCache[object] = Connection->columnDesc(object);
    }

    return ColumnCache[object];
}

bool toConnection::objectName::operator < (const objectName &nam) const
{
    if (Owner < nam.Owner || (Owner.isNull() && !nam.Owner.isNull()))
        return true;
    if (Owner > nam.Owner || (!Owner.isNull() && nam.Owner.isNull()))
        return false;
    if (Name < nam.Name || (Name.isNull() && !nam.Name.isNull()))
        return true;
    if (Name > nam.Name || (!Name.isNull() && nam.Name.isNull()))
        return false;
    if (Type < nam.Type)
        return true;
    return false;
}

bool toConnection::objectName::operator == (const objectName &nam) const
{
    return Owner == nam.Owner && Name == nam.Name && Type == nam.Type;
}

toSyntaxAnalyzer &toConnection::connectionImpl::analyzer()
{
    return toSyntaxAnalyzer::defaultAnalyzer();
}

toSyntaxAnalyzer &toConnection::analyzer()
{
    return Connection->analyzer();
}

std::list<toConnection::objectName> toConnection::connectionImpl::objectNames(void)
{
    std::list<toConnection::objectName> ret;
    return ret;
}

std::map<QString, toConnection::objectName> toConnection::connectionImpl::synonymMap(std::list<toConnection::objectName> &)
{
    std::map<QString, toConnection::objectName> ret;
    return ret;
}

toQDescList toConnection::connectionImpl::columnDesc(const objectName &)
{
    toQDescList ret;
    return ret;
}

void toConnection::connectionImpl::parse(toConnectionSub *, const QCString &)
{
    throw qApp->translate("toConnection", "Parse only not implemented for this type of connection");
}
