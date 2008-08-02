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
#include "toqvalue.h"

#include <QVariant>
#include <QApplication>

static int NumberFormat;
static int NumberDecimals;


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

toQValue::toQValue(qlonglong d)
{
    Type = longType;
    Value.Long = d;
}

toQValue::toQValue(qulonglong d)
{
    Type = ulongType;
    Value.uLong = d;
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
    case longType:
        Value.Long = copy.Value.Long;
        break;
    case ulongType:
        Value.uLong = copy.Value.uLong;
    }
}


bool toQValue::operator<(const toQValue &other) const
{
    if (isInt() && other.isInt())
        return toInt() < other.toInt();
    if (isDouble() && other.isDouble())
        return toDouble() < other.toDouble();
    if (isLong() && other.isLong())
        return toLong() < other.toLong();
    if (isuLong() && other.isuLong())
        return touLong() < other.touLong();
    if (isBinary() && other.isBinary())
        return Value.Array < other.Value.Array;

    // otherwise, try to convert to double for comparison
    bool ok;
    double d1 = toString().toDouble(&ok);
    if (ok)
    {
        double d2 = other.toString().toDouble(&ok);
        if (ok)
            return d1 < d2;
    }

    return toString() < other.toString();
}


bool toQValue::operator>(const toQValue &other) const
{
    return !operator<(other);
}


const toQValue &toQValue::operator = (const toQValue & copy)
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
    case longType:
        Value.Long = copy.Value.Long;
        break;
    case ulongType:
        Value.uLong = copy.Value.uLong;
        break;
    }
    return *this;
}

bool toQValue::isNumber() const
{
    switch (Type)
    {
    case intType:
    case doubleType:
    case nullType:              // intentional fall through
    case longType:
    case ulongType:
        return true;

    case stringType:            // intentional
    case binaryType:
        return false;
    }

    return false;
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
    case longType:
        return val.Value.Long == Value.Long;
    case ulongType:
        return val.Value.uLong == Value.uLong;
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

QVariant toQValue::toQVariant(void) const
{
    if (isInt())
        return QVariant(toInt());
    if (isDouble())
        return QVariant(toDouble());
    if (isNull())
        return QVariant();
    if (isString())
        return QVariant(toUtf8());
    if (isBinary())
        return QVariant(toString());
    if (isLong())
        return QVariant(toLong());
    if (isuLong())
        return QVariant(touLong());
    return QVariant(toByteArray());
}

bool toQValue::isInt(void) const
{
    return Type == intType;
}

bool toQValue::isDouble(void) const
{
    return Type == doubleType;
}

bool toQValue::isuLong(void) const
{
    return Type == ulongType;
}

bool toQValue::isLong(void) const
{
    return Type == longType;
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

QString toQValue::toUtf8(void) const
{
    switch (Type)
    {
    case nullType:
    {
        QString ret;
        return ret;
    }
    case intType:
    {
        QString ret;
        ret.setNum(Value.Int);
        return ret;
    }
    case longType:
    {
        QString ret;
        ret.setNum(Value.Long);
        return ret;
    }
    case ulongType:
    {
        QString ret;
        ret.setNum(Value.uLong);
        return ret;
    }
    case doubleType:
    {
        QString ret;
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
        return *(Value.String);
    case binaryType:
    {
        QString ret(Value.Array->size()*2 + 1);
        for (int i = 0;i < Value.Array->size();i++)
        {
            unsigned char c = (unsigned char)Value.Array->at(i);
//                 ret.at(i*2) = HexString[(c / 16) % 16];
//                 ret.at(i*2 + 1) = HexString[c % 16];
            ret[i*2] = HexString[(c / 16) % 16];
            ret[i*2 + 1] = HexString[c % 16];
        }
//             ret.at(Value.Array->size()*2) = 0;
        ret[Value.Array->size()*2] = 0;
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
    case longType:
        return int(Value.Long);
    case ulongType:
        return int(Value.uLong);
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
    case longType:
        return double(Value.Long);
    case ulongType:
        return double(Value.uLong);
    case doubleType:
        return Value.Double;
    case stringType:
        return Value.String->toDouble();
    case binaryType:
        throw qApp->translate("toQValue", "Can't transform binary value to double");
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

qlonglong toQValue::toLong(void) const
{
    switch (Type)
    {
    case longType:
        return Value.Long;
    case ulongType:
        return (qlonglong) Value.uLong;
    case nullType:
        return (qlonglong) 0;
    case intType:
        return (qlonglong) Value.Int;
    case doubleType:
        return (qlonglong) Value.Double;
    case stringType:
        return Value.String->toLongLong();
    case binaryType:
        throw qApp->translate("toQValue", "Can't transform binary value to long");
    }
    throw qApp->translate("toQValue", "Unknown type of query value");
}

qulonglong toQValue::touLong(void) const
{
    switch (Type)
    {
    case longType:
        return (qulonglong) Value.Long;
    case ulongType:
        return Value.uLong;
    case nullType:
        return (qulonglong) 0;
    case intType:
        return (qulonglong) Value.Int;
    case doubleType:
        return (qulonglong) Value.Double;
    case stringType:
        return Value.String->toULongLong();
    case binaryType:
        throw qApp->translate("toQValue", "Can't transform binary value to ulong");
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

toQValue toQValue::fromVariant(const QVariant &val)
{
    switch (val.type())
    {
    case QVariant::Invalid:
        return toQValue();
    case QVariant::Bool:
        return toQValue((int) val.toBool());
    case QVariant::Double:
        return toQValue((double) val.toDouble());
    case QVariant::Int:
        return toQValue((int) val.toInt());
    case QVariant::LongLong:
        return toQValue((qlonglong) val.toLongLong());
    case QVariant::UInt:
        return toQValue((qulonglong) val.toUInt());
    case QVariant::ULongLong:
        return toQValue(val.toULongLong());
    case QVariant::ByteArray:
        return toQValue::createBinary(val.toByteArray());
    default:
        return toQValue(val.toString());
    }
}

toQValue toQValue::createBinary(const QByteArray &arr)
{
    toQValue ret;
    ret.Type = binaryType;
    ret.Value.Array = new QByteArray(arr);
    return ret;
}

toQValue toQValue::createFromHex(const QByteArray &hex)
{
    QByteArray arr;
    for (int i = 0;i < hex.length();i += 2)
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
//         arr.at(i / 2) = num;
        arr[i / 2] = num;
    }
    return createBinary(arr);
}

toQValue toQValue::createFromHex(const QString &hex)
{
    QByteArray arr;
    for (int i = 0;i < hex.length();i += 2)
    {
        int num;
// qt4        char c = hex.at(i);
        char c = hex.at(i).toAscii();
        if (c >= 'a')
            num = c - 'a';
        else if (c >= 'A')
            num = c - 'A';
        else
            num = c - '0';
        num <<= 4;
// qt4         c = hex.at(i + 1);
        c = hex.at(i + 1).toAscii();
        if (c >= 'a')
            num += c - 'a';
        else if (c >= 'A')
            num += c - 'A';
        else
            num += c - '0';
        arr[i / 2] = num;
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
    case longType:
        return QString::number(Value.Long);
    case ulongType:
        return QString::number(Value.uLong);
    case stringType:
        return *Value.String;
    case binaryType:
    {
        QString ret;
        for (int i = 0;i < Value.Array->size();i++)
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
