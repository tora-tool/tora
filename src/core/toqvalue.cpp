
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

#include "core/toqvalue.h"
#include "core/utils.h"
#include "core/toconfiguration_new.h"
#include "core/todatabasesetting.h"

#include <QtCore/QVariant>
#include <QApplication>

//#include <cstdio>

static int NumberFormat;
static int NumberDecimals;


toQValue::toQValue(int i)
{
    Value = i;
}

toQValue::toQValue(double i)
{
    Value = i;
}

toQValue::toQValue(qlonglong d)
{
    Value = d;
}

toQValue::toQValue(qulonglong d)
{
    Value = d;
}
toQValue::toQValue(toRowDesc d)
{
    Value.setValue(d);
}

toQValue::toQValue(const toQValue &copy): Value(copy.Value)
{
    /** Be destructive only if complexType is held
     *  There should be no copying of data read from a query,
     *  but toQValue is also used for query parameters(toQList and others)
     *  and these are copied often (toNoBlockQuery.Params => toQuery.Params)
     */
    if (isComplexType())
        const_cast<toQValue&>(copy).Value = "deleted value(clone)";
}

const toQValue &toQValue::operator = (const toQValue & copy)
{
    Value = copy.Value;
    /** Be destructive only if complexType is held
     *  There should be no copying of data read from a query,
     *  but toQValue is also used for query parameters(toQList and others)
     *  and these are copied often (toNoBlockQuery.Params => toQuery.Params)
     */
    if (isComplexType())
        const_cast<toQValue&>(copy).Value = "deleted value(assign)";
    return *this;
}

toQValue::toQValue(const QString &str)
{
    Value = str;
}

toQValue::toQValue()
{
}

toQValue::~toQValue()
{
    if (isComplexType())
    {
        complexType *i = Value.value<toQValue::complexType*>();
        if (i)
            delete i;
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
        return Value.toByteArray() < other.Value.toByteArray();

    // otherwise, try to convert to double for comparison
    bool ok;
    double d1 = Value.toString().toDouble(&ok);
    if (ok)
    {
        double d2 = other.Value.toString().toDouble(&ok);
        if (ok)
            return d1 < d2;
    }

    return Value.toString() < other.Value.toString();
}


bool toQValue::operator>(const toQValue &other) const
{
    return !operator<(other);
}


bool toQValue::operator<=(const toQValue &other) const
{
    if (isInt() && other.isInt())
        return toInt() <= other.toInt();
    if (isDouble() && other.isDouble())
        return toDouble() <= other.toDouble();
    if (isLong() && other.isLong())
        return toLong() <= other.toLong();
    if (isuLong() && other.isuLong())
        return touLong() <= other.touLong();
    if (isBinary() && other.isBinary())
        return Value.toByteArray() <= other.Value.toByteArray();

    // otherwise, try to convert to double for comparison
    bool ok;
    double d1 = Value.toString().toDouble(&ok);
    if (ok)
    {
        double d2 = other.Value.toString().toDouble(&ok);
        if (ok)
            return d1 <= d2;
    }

    return Value.toString() <= other.Value.toString();
}


bool toQValue::operator>=(const toQValue &other) const
{
    return !operator<=(other);
}

bool toQValue::isNumber() const
{
    return isInt() || isDouble() || isLong() || isuLong();
}

bool toQValue::operator == (const toQValue &val) const
{
    return Value == val.Value;
}

QVariant const& toQValue::toQVariant() const
{
    return Value;
}

bool toQValue::isInt() const
{
    return Value.type() == QVariant::Int;
}

bool toQValue::isDouble() const
{
    return Value.type() == QVariant::Double;
}

bool toQValue::isuLong() const
{
    return Value.type() == QVariant::ULongLong;
}

bool toQValue::isLong() const
{
    return Value.type() == QVariant::LongLong;
}

bool toQValue::isString() const
{
    return Value.type() == QVariant::String;
}

bool toQValue::isBinary() const
{
    return Value.type() == QVariant::ByteArray;
}

bool toQValue::isComplexType(void) const
{
    //toRowDesc is special
    if (Value.canConvert<toRowDesc>())
        return false;
    return Value.type() == QVariant::UserType;
}

bool toQValue::isNull() const
{
    return Value.isNull();
}

const QByteArray toQValue::toByteArray() const
{
    return Value.toByteArray();
}

QString toQValue::displayData() const
{
    if ( isNull() && toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmptyBool).toBool())
    {
        return QString::fromLatin1("{null}");
    }

    if ( isBinary())
    {
        QByteArray const &raw = Value.toByteArray();
        return raw.toHex();
    }

    return Value.toString();
}

QString toQValue::editData() const
{
    if ( isComplexType())
    {
        complexType *i = Value.value<toQValue::complexType*>();
        return i->editData();
    }

    return Value.toString();
}

QString toQValue::userData() const
{
    if (isNull() )
        return QString::fromLatin1("NULL");

    if ( isComplexType())
    {
        complexType *i = Value.value<toQValue::complexType*>();
        return i->userData();
    }

    return Value.toString();
}

int toQValue::toInt() const
{
    return Value.toInt();
}

double toQValue::toDouble() const
{
    return Value.toDouble();
}
toRowDesc toQValue::getRowDesc() const
{
    Q_ASSERT(Value.canConvert<toRowDesc>());
    return Value.value<toRowDesc>();
}

qlonglong toQValue::toLong() const
{
    return Value.toLongLong();
}

qulonglong toQValue::touLong() const
{
    return Value.toULongLong();
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

int toQValue::numberFormat()
{
    return NumberFormat;
}

int toQValue::numberDecimals()
{
    return NumberDecimals;
}

toQValue toQValue::fromVariant(const QVariant &val)
{
    toQValue ret;
    ret.Value = val;
    return ret;
}

toQValue toQValue::createBinary(const QByteArray &arr)
{
    toQValue ret;
    ret.Value = arr;
    return ret;
}

toQValue toQValue::createFromHex(const QByteArray &hex)
{
    QByteArray arr;
    for (int i = 0; i < hex.length(); i += 2)
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
    for (int i = 0; i < hex.length(); i += 2)
    {
        int num;
// qt4        char c = hex.at(i);
        char c = hex.at(i).toLatin1();
        if (c >= 'a')
            num = c - 'a';
        else if (c >= 'A')
            num = c - 'A';
        else
            num = c - '0';
        num <<= 4;
// qt4         c = hex.at(i + 1);
        c = hex.at(i + 1).toLatin1();
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
    return Value.toString();
}


QString toQValue::toSIsize() const
{
    if (this->isNull())
        return NULL;

    double size = toDouble();
    QString s = "%1";
    int i = 0;

    while (size / 1024 >= 10)
    {
        i++;
        size = size / 1024;
    }

    switch (i)
    {
        case 0:
            break;
        case 1:
            s.append("K");
            break;
        case 2:
            s.append("M");
            break;
        case 3:
            s.append("G");
            break;
        case 4:
            s.append("T");
            break;
        case 5:
            s.append("P");
            break;
        default:
            s.append("E");
            break;
    }

    return s.arg(size, 0, 'f', 0);
}

bool toQValue::updateNewValue(toQValue value)
{
    if (Value.type() == QVariant::UserType)
        return false;
    if (value.isComplexType())
        return false;
    Value = value.Value;
    return true;
}
