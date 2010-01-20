
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"

#include <stdio.h>

#include <qapplication.h>
#include <qlabel.h>
#include <QProgressDialog>
#include <qdatetime.h>

#include <QString>

#ifdef Q_OS_WIN32
#include <winsock2.h>
#endif


std::map<QString, toExtract::extractor *> *toExtract::Extractors;

void toExtract::extractor::registerExtract(const QString &db,
        const QString &oper,
        const QString &type)
{
    toExtract::allocExtract();

    QString name = toExtract::extractorName(db, oper, type);

    if ((*toExtract::Extractors).find(name) != (*toExtract::Extractors).end())
        fprintf(stderr, "Extractor %s multiply defined\n", name.toAscii().constData());

    (*toExtract::Extractors)[name] = this;
}

void toExtract::extractor::unregisterExtract(const QString &db,
        const QString &oper,
        const QString &type)
{
    toExtract::allocExtract();

    QString name = toExtract::extractorName(db, oper, type);
    std::map<QString, extractor *>::iterator i = (*toExtract::Extractors).find(name);
    if (i == (*toExtract::Extractors).end())
        fprintf(stderr, "Extractor %s not defined on unregistering\n", name.toAscii().constData());
    else
        (*toExtract::Extractors).erase(i);
}

void toExtract::extractor::initialize(toExtract &) const
{}

void toExtract::extractor::create(toExtract &,
                                  QTextStream &,
                                  const QString &,
                                  const QString &,
                                  const QString &,
                                  const QString &) const
{}

void toExtract::extractor::describe(toExtract &,
                                    std::list<QString> &,
                                    const QString &,
                                    const QString &,
                                    const QString &,
                                    const QString &) const
{}

void toExtract::extractor::migrate(toExtract &,
                                   QTextStream &,
                                   const QString &,
                                   std::list<QString> &,
                                   std::list<QString> &) const
{}

void toExtract::extractor::drop(toExtract &,
                                QTextStream &,
                                const QString &,
                                const QString &,
                                const QString &,
                                const QString &) const
{}

std::list<toExtract::datatype> toExtract::extractor::datatypes() const
{
    std::list<toExtract::datatype> ret;
    return ret;
}

toExtract::extractor::extractor()
{}

toExtract::extractor::~extractor()
{}

void toExtract::allocExtract(void)
{
    if (!Extractors)
        Extractors = new std::map<QString, extractor *>;
}

QString toExtract::extractorName(const QString &db, const QString &oper, const QString &type)
{
    QString ret(db);
    if (!oper.isEmpty())
        ret += ":" + oper;
    if (!type.isEmpty())
        ret += ":" + type;
    return ret;
}

toExtract::extractor *toExtract::findExtractor(toConnection &conn,
        const QString &oper,
        const QString &type)
{
    allocExtract();

    QString name = extractorName(conn.provider(), oper, type);
    std::map<QString, extractor *>::iterator i = (*Extractors).find(name);
    if (i != (*Extractors).end())
        return (*i).second;
    return NULL;
}

void toExtract::initialize(void)
{
    if (!Initialized)
    {
        Initialized = true;
        extractor *ext = findExtractor(QString::null, QString::null);
        if (ext)
            ext->initialize(*this);
    }
}

toExtract::toExtract(toConnection &conn, QWidget *parent)
        : Connection(conn), Parent(parent)
{
    Heading = true;
    Prompt = true;
    Constraints = true;
    Indexes = true;
    Grants = true;
    Storage = true;
    Parallel = true;
    Contents = true;
    Comments = true;
    Partition = true;
    Replace = false;
    Schema = "1";
    Initialized = false;
    BlockSize = 8192;
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

void toExtract::parseObject(const QString &object,
                            QString &owner,
                            QString &name)
{
    try
    {
        unsigned int search = 0;
        if (object[0] == '\"')
        {
            int pos = object.indexOf('\"', 1);
            if (pos < 0)
                throw 1;
            owner = object.left(pos);
            owner = owner.right(owner.length() - 1);
            search = pos + 1;
        }
        else
        {
            int pos = object.indexOf('.');
            if (pos < 0)
                pos = object.length();
            owner = object.left(pos);
            search = pos;
        }
        if (search >= (unsigned int) object.length())
        {
            name = owner;
            owner = Connection.user();
            return;
        }
        else if (object.at(search) != '.')
            throw 2;
        search++;
        if (object.at(search) == '\"')
        {
            int pos = object.indexOf('\"', search + 1);
            if (pos < 0)
                throw 3;
            name = object.left(pos);
            name = owner.right(pos - search - 1);
            search = pos + 1;
            if (search < (unsigned int) object.length())
                throw 4;
        }
        else
        {
            name = object.right(object.length() - search);
        }
    }
    catch (int i)
    {
        throw qApp->translate("toExtract", "Failed to extract owner and name from (%1), internal error %2").arg(object).arg(i);
    }
}

bool toExtract::canHandle(toConnection &conn)
{
    return bool(findExtractor(conn, QString::null, QString::null));
}

std::list<toExtract::datatype> toExtract::datatypes()
{
    std::list<toExtract::datatype> ret;
    try
    {
        extractor *ext = findExtractor(connection(), QString::null, QString::null);
        if (ext)
            ret = ext->datatypes();
        ret.sort();
    }
    TOCATCH
    return ret;
}

void toExtract::create(QTextStream &ret, std::list<QString> &objects)
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
    initialize();

    try
    {
        toBusy busy;
        int num = 1;
        for (std::list<QString>::iterator i = objects.begin();i != objects.end();i++)
        {
            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(*i);
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Creating script was canceled");
            }
            num++;

            QString type = *i;
            QString owner;
            QString name;
            int pos = type.indexOf(QString::fromLatin1(":"));
            if (pos < 0)
                throw qApp->translate("toExtract", "Internal error, missing : in object description");
            parseObject(type.right(type.length() - pos - 1), owner, name);
            type.truncate(pos);
            QString utype = type.toUpper();
            QString schema = intSchema(owner, false);
            try
            {
                try
                {
                    extractor *ext = findExtractor(QString::fromLatin1("CREATE"), utype);
                    if (ext)
                        ext->create(*this,
                                    ret,
                                    utype,
                                    schema,
                                    owner,
                                    name);
                    else
                        throw qApp->translate("toExtract", "Invalid type %1 to create").arg(type);
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Create"), *i, exc);
                }
            }
            catch (const QString &exc)
            {
                toStatusMessage(exc);
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

std::list<QString> toExtract::describe(std::list<QString> &objects)
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
    initialize();

    try
    {
        toBusy busy;
        int num = 1;
        for (std::list<QString>::iterator i = objects.begin();i != objects.end();i++)
        {
            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(*i);
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Describe was canceled");
            }
            num++;

            QString type = *i;
            QString owner;
            QString name;
            int pos = type.indexOf(QString::fromLatin1(":"));
            if (pos < 0)
                throw qApp->translate("toExtract", "Internal error, missing : in object description");
            parseObject(type.right(type.length() - pos - 1), owner, name);
            type.truncate(pos);
            QString utype = type.toUpper();
            QString schema = intSchema(owner, true);

            std::list<QString> cur;

            try
            {
                try
                {
                    extractor *ext = findExtractor(QString::fromLatin1("DESCRIBE"), utype);
                    if (ext)
                        ext->describe(*this,
                                      cur,
                                      utype,
                                      schema,
                                      owner,
                                      name);
                    else
                    {
                        throw qApp->translate("toExtract", "Invalid type %1 to describe").arg(type);
                    }
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Describe"), *i, exc);
                }
                cur.sort();
                ret.merge(cur);
            }
            catch (const QString &exc)
            {
                toStatusMessage(exc);
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

void toExtract::drop(QTextStream &ret, std::list<QString> &objects)
{
    ret << generateHeading(qApp->translate("toExtract", "DROP"), objects);

    QProgressDialog *progress = NULL;
    if (Parent)
    {
        progress = new QProgressDialog(
            qApp->translate("toExtract", "Creating drop script"),
            qApp->translate("toExtract", "Cancel"),
            0,
            objects.size(), Parent);
        progress->setWindowTitle(qApp->translate("toExtract", "Creating drop script"));
    }
    initialize();

    try
    {
        toBusy busy;
        int num = 1;
        for (std::list<QString>::iterator i = objects.begin();i != objects.end();i++)
        {
            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(*i);
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Creating drop script was canceled");
                num++;
            }

            QString type = *i;
            QString owner;
            QString name;
            int pos = type.indexOf(QString::fromLatin1(":"));
            if (pos < 0)
                throw qApp->translate("toExtract", "Internal error, missing : in object description");
            parseObject(type.right(type.length() - pos - 1), owner, name);
            type.truncate(pos);
            QString utype = type.toUpper();
            QString schema = intSchema(owner, false);

            try
            {
                try
                {
                    extractor *ext = findExtractor(QString::fromLatin1("DROP"), utype);
                    if (ext)
                        ext->drop(*this,
                                  ret,
                                  utype,
                                  schema,
                                  owner,
                                  name);
                    else
                    {
                        throw qApp->translate("toExtract", "Invalid type %1 to drop");
                        ;
                    }
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Drop script"), *i, exc);
                }
            }
            catch (const QString &exc)
            {
                toStatusMessage(exc);
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

std::map<QString, std::list<QString> > toExtract::migrateGroup(std::list<QString> &grpLst)
{
    std::map<QString, std::list<QString> > ret;
    for (std::list<QString>::iterator i = grpLst.begin();i != grpLst.end();i++)
    {
        std::list<QString> ctx = splitDescribe(*i);
        if (ctx.size() < 3)
            toStatusMessage(qApp->translate("toExtract", "Invalid describe context (<3 parts) \"%1\")").arg(*i));
        else
        {
            QString t = toShift(ctx);
            t += QString::fromLatin1("\01");
            t += toShift(ctx);
            t += QString::fromLatin1("\01");
            t += toShift(ctx);
            toPush(ret[t], *i);
        }
    }
    return ret;
}

/* This function can take properties of more than one object as parameters.
   They are later separated by the call to migrateGroup. Migrate function
   for specific object is called separately for each identified object.
*/
void toExtract::migrate(QTextStream &ret, std::list<QString> &drpLst, std::list<QString> &crtLst)
{
    std::list<QString> t;
    t.insert(t.end(), qApp->translate("toExtract", "Object list not available in migration"));
    ret << generateHeading(qApp->translate("toExtract", "MIGRATE"), t);

    QProgressDialog *progress = NULL;

    std::map<QString, std::list<QString> > objDrp;
    std::map<QString, std::list<QString> > objCrt;

    objDrp = migrateGroup(drpLst);
    objCrt = migrateGroup(crtLst);

    for (std::map<QString, std::list<QString> >::iterator j = objCrt.begin();j != objCrt.end();j++)
        objDrp[(*j).first]; // Make sure all objects in the createlist also exists in the droplist

    if (Parent)
    {
        progress = new QProgressDialog(
            qApp->translate("toExtract", "Creating migration script"),
            qApp->translate("toExtract", "Cancel"),
            0,
            objDrp.size());
        progress->setWindowTitle(qApp->translate("toExtract", "Creating migration script"));
    }
    initialize();

    try
    {
        toBusy busy;
        int num = 1;

        for (std::map<QString, std::list<QString> >::iterator i = objDrp.begin();i != objDrp.end();i++)
        {
            QString t = (*i).first;

            if (progress)
            {
                progress->setValue(num);
                progress->setLabelText(t);
                qApp->processEvents();
                if (progress->wasCanceled())
                    throw qApp->translate("toExtract", "Creating drop script was canceled");
            }
            num++;

            std::list<QString> &crt = objCrt[t];
            std::list<QString> &drp = objDrp[t];
            crt.sort();
            drp.sort();
            std::list<QString> ctx = splitDescribe(t);
            QString schema = toShift(ctx);
            QString utype = toShift(ctx).toUpper();
            QString name = toShift(ctx);

            try
            {
                try
                {
                    extractor *ext = findExtractor(QString::fromLatin1("MIGRATE"), utype);
                    if (ext)
                        ext->migrate(*this,
                                     ret,
                                     utype,
                                     drp,
                                     crt);
                    else
                    {
                        throw qApp->translate("toExtract", "Invalid type %1 to migrate").arg(utype);
                    }
                }
                catch (const QString &exc)
                {
                    rethrow(qApp->translate("toExtract", "Migration script"), (*i).first, exc);
                }
            }
            catch (const QString &exc)
            {
                toStatusMessage(exc);
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

QString toExtract::generateHeading(const QString &action,
                                   std::list<QString> &lst)
{
    if (!Heading)
        return QString::null;
    char host[1024];
    gethostname(host, 1024);

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
                  arg(QString::fromLatin1(TOVERSION)).
                  arg(QString::fromLatin1(host)).
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
    for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
    {
        if (!(*i).startsWith(QString::fromLatin1("TABLE REFERENCES")))
        {
            str += QString::fromLatin1("-- ");
            str += (*i);
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
                          "Error:          %3").
    arg(what).
    arg(object).
    arg(exc);
}

void toExtract::setSizes(void)
{
    Initial.clear();
    Next.clear();
    Limit.clear();
    if (Resize == QString::fromLatin1("1"))
    {
        for (int i = 1;i < 10000;i *= 10)
        {
            QString str;
            if (i < 1000)
            {
                str = QString::number(40 * i + 1);
                str += QString::fromLatin1(" K");
            }
            else
                str = QString::fromLatin1("UNLIMITED");
            toPush(Initial, str);
            str = QString::number(5 * BlockSize * i);
            str += QString::fromLatin1(" K");
            toPush(Next, str);
            toPush(Limit, str);
        }
    }
    else if (!Resize.isEmpty())
    {
        QStringList lst = Resize.split(":");
        if (lst.count() % 3 != 0)
            throw qApp->translate("toExtract", "Malformed resize string (Should contain multiple of 3 number of ':')");
        for (int i = 0;i < lst.count();i += 3)
        {
            if (i + 3 < lst.count())
                toPush(Limit, lst[i]);
            else
                toPush(Limit, QString::fromLatin1("UNLIMITED"));
            toPush(Initial, lst[i + 1]);
            toPush(Next, lst[i + 2]);
        }
    }
}

QString toExtract::intSchema(const QString &owner, bool desc)
{
    if (owner.toUpper() == "PUBLIC")
        return QString("PUBLIC");
    if (Schema == "1")
    {
        QString ret = Connection.quote(owner);
        if (!desc)
            ret += ".";
        return ret;
    }
    else if (Schema.isEmpty())
        return QString::null;
    QString ret = Connection.quote(Schema);
    if (!desc)
        ret += ".";
    return ret;
}

void toExtract::initialNext(const QString &blocks, QString &initial, QString &next)
{
    std::list<QString>::iterator iinit = Initial.begin();
    std::list<QString>::iterator inext = Next.begin();
    std::list<QString>::iterator ilimit = Limit.begin();
    while (ilimit != Initial.end())
    {
        if (*ilimit == QString::fromLatin1("UNLIMITED") ||
                (*ilimit).toFloat() > blocks.toFloat())
        {
            initial = *iinit;
            next = *inext;
            return ;
        }
        iinit++;
        inext++;
        ilimit++;
    }
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
    for (std::list<QString>::iterator i = args.begin();i != args.end();i++)
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
    for (int i = 0;i < ctx.count();i++)
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

QString toExtract::createFromParse(std::list<toSQLParse::statement>::iterator start,
                                   std::list<toSQLParse::statement>::iterator end)
{
    QString ret;
    toSQLParse::statement newstat(toSQLParse::statement::Statement);
    while (start != end)
    {
        newstat.subTokens().insert(newstat.subTokens().end(), *start);
        start++;
    }
    return toSQLParse::indentStatement(newstat, connection()).trimmed();
}

std::list<toExtract::columnInfo> toExtract::parseColumnDescription(std::list<QString>::const_iterator begin,
        std::list<QString>::const_iterator end,
        int level)
{
    std::list<columnInfo> ret;

    while (begin != end)
    {
        std::list<QString> row = toExtract::splitDescribe(*begin);
        for (int i = 0;i < level;i++)
            toShift(row);
        if (toShift(row) == "COLUMN")
        {
            QString name = toShift(row);
            columnInfo *current = NULL;
            for (std::list<toExtract::columnInfo>::iterator j = ret.begin();j != ret.end();j++)
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
            QString extra = toShift(row);
            if (extra == "ORDER")
                current->Order = toShift(row).toInt();
            else if (extra == "COMMENT")
                current->Comment = toShift(row);
            else if (extra == "EXTRA") {
                extra = toShift(row);
                if (extra == "NOT NULL")
                    current->bNotNull = true;
                else
                    printf("Error! Found unknown extra data for column %s -> %s\n",
                           name.toAscii().constData(),
                           extra.toAscii().constData());
            } else if (!extra.isEmpty())
            {
                QString data = toShift(row);
                if (data.isEmpty())
                {
// TS 2009-11-01 This can be called twice on the same column, for say VARCHAR2 and for EXTRA
//               with data variable being empty both times.
//                    if (!current->Definition.isEmpty())
//                        throw qApp->translate("toExtract", "More than one definition for column %1").arg(name);
                    int iDefaultPos = // position of string "DEFAULT" in QString
                        extra.indexOf("DEFAULT", 0, Qt::CaseInsensitive);
                    if (iDefaultPos > -1) {
                        // in the following code default value is calculated from a string like
                        // VARCHAR2(7) DEFAULT 'something' or VARCHAR2(7) DEFAULT null.
                        // iDefaultPos will point to start of "DEFAULT"
                        // therefore we have to also remove word "DEFAULT" as well as space and
                        // opening quote (unless default is null). Ending quote is removed in
                        // folowing truncate statement if default is not null (without quotes).
                        if (extra[8 + iDefaultPos] == '\'') {
                            current->DefaultValue = extra.right(extra.size() - iDefaultPos - 9);
                            current->DefaultValue.truncate((int)current->DefaultValue.size() - 1);
                        } else {
                            // "DEFAULT null" is represented as empty string in "edit table" dialog
                            current->DefaultValue = "";
                        }
                        extra.truncate(iDefaultPos-1);
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
