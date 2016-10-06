
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

#include "core/toextract.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectiontraits.h"
#include "core/toraversion.h"
#include "core/toconf.h"

#include <QApplication>
#include <QProgressDialog>
#include <QtCore/QString>
#include <QtNetwork/QHostInfo>

std::list<toExtract::datatype> toExtract::extractor::datatypes() const
{
    std::list<toExtract::datatype> ret;
    return ret;
}

toExtract::extractor::extractor(toExtract &parent)
    : ext(parent)
{
}

toExtract::extractor::~extractor()
{}

toExtract::toExtract(toConnection &conn, QWidget *parent)
    : Connection(conn)
    , Parent(parent)
    , Heading(true)
    , Prompt(true)
    , Constraints(true)
    , Indexes(true)
    , Grants(true)
    , Storage(true)
    , Parallel(true)
    , Contents(true)
    , Comments(true)
    , Partition(true)
    , Code(true)
    , Replace(false)
    , Schema("1")
    , Initialized(false)
    , BlockSize(8192)
    , CommitDistance(0)
{
    ext = ExtractorFactorySing::Instance().create(Connection.provider().toStdString(), *this);
}

void toExtract::setState(const QString &name, const QVariant &val)
{
    Context[name] = val;
}

QVariant toExtract::state(const QString &name)
{
    return Context[name];
}

void toExtract::srcDst2DropCreate(std::list<QString> &source, std::list<QString> &destination,
                                  std::list<QString> &drop, std::list<QString> &create)
{
    drop.clear();
    create.clear();

    std::list<QString>::iterator i = source.begin();
    std::list<QString>::iterator j = destination.begin();
    while (i != source.end() && j != destination.end())
    {
        if (*i != *j)
        {
            if (*i < *j)
            {
                drop.insert(drop.end(), *i);
                i++;
            }
            else
            {
                create.insert(create.end(), *j);
                j++;
            }
        }
        else
        {
            i++;
            j++;
        }
    }
    while (i != source.end())
    {
        drop.insert(drop.end(), *i);
        i++;
    }
    while (j != destination.end())
    {
        create.insert(create.end(), *j);
        j++;
    }
}

void toExtract::create(QTextStream &ret, const toExtract::ObjectList &objects)
{
    ret << generateHeading(qApp->translate("toExtract", "CREATE"), objects);

    QProgressDialog *progress = NULL;

    if (Parent)
    {
        progress = new QProgressDialog(
            qApp->translate("toExtract",
                            "Creating create script"),
            qApp->translate("toExtract", "Cancel"),
            0,
            objects.size(),
            Parent);
        progress->setWindowTitle(qApp->translate("toExtract", "Creating script"));
    }

    try
    {
        Utils::toBusy busy;
        int num = 1;
        foreach(auto i, objects)
        {
            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(i.second.toString());
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Creating script was canceled");
            }
            num++;

            QString type = i.first;
            QString owner = Connection.getTraits().unQuote(i.second.owner());
            QString name  = Connection.getTraits().unQuote(i.second.name());
            ObjectType typeEnum = objectTypeFromString(type.toUpper());
            QString schema = intSchema(owner, false);
            try
            {
                try
                {
                    if (ext)
                    {
                        if(!Initialized)
                        {
                            ext->initialize();
                            Initialized = true;
                        }
                        ext->create(ret,
                                    typeEnum,
                                    schema,
                                    owner,
                                    name);
                    }
                    else
                        throw qApp->translate("toExtract", "Invalid type %1 to create").arg(type);
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Create"), i.second.toString(), exc);
                }
            }
            catch (const QString &exc)
            {
                Utils::toStatusMessage(exc);
            }
        }
    }
    catch (...)
    {
        delete progress;
        throw;
    }
    delete progress;
}

std::list<QString> toExtract::describe(const toExtract::ObjectList &objects)
{
    std::list<QString> ret;
    QProgressDialog *progress = NULL;

    if (Parent)
    {
        progress = new QProgressDialog(
            qApp->translate("toExtract", "Creating description"),
            qApp->translate("toExtract", "Cancel"),
            0,
            objects.size(),
            Parent);
        progress->setWindowTitle(qApp->translate("toExtract", "Creating description"));
    }

    try
    {
        Utils::toBusy busy;
        int num = 1;
        foreach(auto i, objects)
        {
            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(i.second.toString());
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Describe was canceled");
            }
            num++;

            QString type = i.first;
            QString owner = Connection.getTraits().unQuote(i.second.owner());
            QString name  = Connection.getTraits().unQuote(i.second.name());
            ObjectType typeEnum = objectTypeFromString(type.toUpper());
            QString schema = intSchema(owner, true);

            std::list<QString> cur;

            try
            {
                try
                {
                    if (ext)
                    {
                        if(!Initialized)
                        {
                            ext->initialize();
                            Initialized = true;
                        }
                        ext->describe(cur,
                                      typeEnum,
                                      schema,
                                      owner,
                                      name);
                    }
                    else
                    {
                        throw qApp->translate("toExtract", "Invalid type %1 to describe").arg(type);
                    }
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Describe"), i.second.toString(), exc);
                }
                cur.sort();
                ret.merge(cur);
            }
            catch (const QString &exc)
            {
                Utils::toStatusMessage(exc);
            }
        }
    }
    catch (...)
    {
        delete progress;
        throw;
    }
    delete progress;
    return ret;
}

QString toExtract::generateHeading(const QString &action, const QList<QPair<QString,toCache::ObjectRef> > &objects)
{
    if (!Heading)
        return QString::null;

    QString db = Connection.host();
    if (db.length() && db != QString::fromLatin1("*"))
        db += QString::fromLatin1(":");
    else
        db = QString::null;
    db += Connection.database();

    QString str = qApp->translate("toExtract", "-- This DDL was reverse engineered by\n"
                                  "-- " TOAPPNAME ", Version %1\n"
                                  "--\n"
                                  "-- at:   %2\n"
                                  "-- from: %3, an %4 %5 database\n"
                                  "--\n"
                                  "-- on:   %6\n"
                                  "--\n").
                  arg(QString::fromLatin1(TORAVERSION)).
                  arg(QHostInfo::localHostName()).
                  arg(db).
                  arg(QString(Connection.provider())).
                  arg(QString(Connection.version())).
                  arg(QDateTime::currentDateTime().toString());
    if (action == QString::fromLatin1("FREE SPACE"))
        str += qApp->translate("toExtract", "-- Generating free space report for:\n--");
    else
    {
        str += qApp->translate("toExtract", "-- Generating %1 statement for:\n").arg(action);
    }
    foreach(auto i, objects)
    {
        if (i.first == "TABLE REFERENCES")
        {
            str += QString::fromLatin1("-- ");
            str += i.second.toString();
            str += QString::fromLatin1("\n");
        }
    }
    str += QString::fromLatin1("\n");
    return str;
}

void toExtract::rethrow(const QString &what, const QString &object, const QString &exc)
{
    throw qApp->translate("toExtract", "Error in toExtract\n"
                          "Operation:      %1\n"
                          "Object:         %2\n"
                          "Error:          %3")
    .arg(what)
    .arg(object)
    .arg(exc);
}

QString toExtract::intSchema(const QString &owner, bool desc)
{
    if (owner.toUpper() == "PUBLIC") return QString("PUBLIC");
    if (Schema == "1")
    {
        QString ret = Connection.getTraits().quote(owner);
        if (!desc)
            ret += ".";
        return ret;
    }
    else if (Schema.isEmpty())
        return QString::null;
    QString ret = Connection.getTraits().quote(Schema);
    if (!desc)
        ret += ".";
    return ret;
}

void toExtract::addDescription(std::list<QString> &ret, const std::list<QString> &ctx,
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

    std::list<QString> args = ctx;
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

    QString str;
    bool first = true;
    for (std::list<QString>::iterator i = args.begin(); i != args.end(); i++)
    {
        if (first)
            first = false;
        else
            str += QString::fromLatin1("\01");
        if (!(*i).isEmpty())
            str += *i;
    }
    ret.insert(ret.end(), str);
}

std::list<QString> toExtract::splitDescribe(const QString &str)
{
    QStringList ctx = str.split("\01");
    std::list<QString> ret;
    for (int i = 0; i < ctx.count(); i++)
        ret.insert(ret.end(), ctx[i]);
    return ret;
}

QString toExtract::partDescribe(const QString &str, int level)
{
    QStringList ctx = str.split("\01");
    if (ctx.count() <= level)
        return QString::null;
    return ctx[level];
}

QString toExtract::contextDescribe(const QString &str, int level)
{
    int pos = -1;
    do
    {
        level--;
        pos = str.indexOf("\01", pos + 1);
    }
    while (pos >= 0 && level > 0);

    if (pos < 0)
        pos = str.length();

    if (level == 0)
        return str.mid(0, pos);
    return QString::null;
}

std::list<toExtract::columnInfo> toExtract::parseColumnDescription(std::list<QString>::const_iterator begin,
        std::list<QString>::const_iterator end,
        int level)
{
    std::list<columnInfo> ret;

    while (begin != end)
    {
        std::list<QString> row = toExtract::splitDescribe(*begin);
        for (int i = 0; i < level; i++)
            Utils::toShift(row);
        if (Utils::toShift(row) == "COLUMN")
        {
            QString name = Utils::toShift(row);
            columnInfo *current = NULL;
            for (std::list<toExtract::columnInfo>::iterator j = ret.begin(); j != ret.end(); j++)
            {
                if ((*j).Name == name)
                {
                    current = &(*j);
                    break;
                }
            }
            if (current == NULL)
            {
                ret.insert(ret.end(), columnInfo(name));
                current = &(*(ret.rbegin()));
                current->bNotNull = false; // by default columns are not "not null"
            }
            QString extra = Utils::toShift(row);
            if (extra == "ORDER")
                current->Order = Utils::toShift(row).toInt();
            else if (extra == "COMMENT")
                current->Comment = Utils::toShift(row);
            else if (extra == "EXTRA")
            {
                extra = Utils::toShift(row);
                if (extra == "NOT NULL")
                    current->bNotNull = true;
                else
                    printf("Error! Found unknown extra data for column %s -> %s\n",
                           name.toLatin1().constData(),
                           extra.toLatin1().constData());
            }
            else if (!extra.isEmpty())
            {
                QString data = Utils::toShift(row);
                if (data.isEmpty())
                {
// TS 2009-11-01 This can be called twice on the same column, for say VARCHAR2 and for EXTRA
//               with data variable being empty both times.
//                    if (!current->Definition.isEmpty())
//                        throw qApp->translate("toExtract", "More than one definition for column %1").arg(name);
                    int iDefaultPos = // position of string "DEFAULT" in QString
                        extra.indexOf("DEFAULT", 0, Qt::CaseInsensitive);
                    if (iDefaultPos > -1)
                    {
                        // in the following code default value is calculated from a string like
                        // VARCHAR2(7) DEFAULT 'something' or VARCHAR2(7) DEFAULT null.
                        // iDefaultPos will point to start of "DEFAULT"
                        // therefore we have to also remove word "DEFAULT" as well as space and
                        // opening quote (unless default is null). Ending quote is removed in
                        // folowing truncate statement if default is not null (without quotes).
                        if (extra[8 + iDefaultPos] == '\'')
                        {
                            current->DefaultValue = extra.right(extra.size() - iDefaultPos - 9);
                            current->DefaultValue.truncate((int)current->DefaultValue.size() - 1);
                        }
                        else
                        {
                            // "DEFAULT null" is represented as empty string in "edit table" dialog
                            current->DefaultValue = "";
                        }
                        extra.truncate(iDefaultPos - 1);
                    }
                    current->Definition = extra;
                }
                else
                {
                    if (current->Data.find(extra) != current->Data.end())
                        throw qApp->translate("toExtract", "Same kind of definition existing more than once for column %1").arg(name);
                    current->Data[extra] = data;
                }
            }
        }
        begin++;
    }

    ret.sort();

    return ret;
}

toExtract::ObjectType toExtract::objectTypeFromString(const QString& objType)
{
    if (objType == "TABLE")
        return TABLE;
    else if (objType == "VIEW")
        return VIEW;
    else if (objType == "SYNONYM")
        return SYNONYM;
    else if (objType == "PROCEDURE")
        return PROCEDURE;
    else if (objType == "FUNCTION")
        return FUNCTION;
    else if (objType == "PACKAGE")
        return PACKAGE;
    else if (objType == "PACKAGE BODY")
        return PACKAGE_BODY;
    else if (objType == "PACKAGE_BODY")
        return PACKAGE_BODY;
    else if (objType == "INDEX")
        return INDEX;
    else if (objType == "SEQUENCE")
        return SEQUENCE;
    else if (objType == "TRIGGER")
        return TRIGGER;
    else if (objType == "USER")
        return USER;
    else if (objType == "ROLE")
        return ROLE;
    else if (objType == "PROFILE")
        return PROFILE;
    else if (objType == "DATABASE_LINK")
        return DATABASE_LINK;
    else if (objType == "DATABASE LINK")
        return DATABASE_LINK;
    else if (objType == "TYPE")
        return TYPE;
    else if (objType == "DIRECTORY")
        return DIRECTORY;
    else if (objType == "MATERIALIZED_VIEW")
        return MATERIALIZED_VIEW;
    else if (objType == "MATERIALIZED VIEW")
        return MATERIALIZED_VIEW;
    else if (objType == "TABLE FAMILY")
        return TABLE_FAMILY;
    else if (objType == "TABLE CONTENTS")
        return TABLE_CONTENTS;
    else if (objType == "TABLE REFERENCES")
        return TABLE_REFERENCES;
    else if (objType == "CONSTRAINT")
        return CONSTRAINT;

    throw qApp->translate("toExtract", "Unsupported object type to handle %1").arg(objType);
}

QString toExtract::extractor::quote(QString const& name)
{
    return ext.connection().getTraits().quote(name);
}

