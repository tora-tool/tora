
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
#include "tohtml.h"
#include "toextract.h"
#include "toreport.h"

#include <qapplication.h>
#include <qdatetime.h>
#ifdef Q_OS_WIN32
#include <winsock2.h>
#endif


static bool SameContext(const QString &str, const QString &str2)
{
    if (str.startsWith(str2))
    {
        if (str.length() == str2.length())
            return true;
        if (str.length() > str2.length() &&
                str.at(str2.length()) == QChar('\001'))
            return true;
    }
    return false;
}

static bool HasChildren(std::list<QString> &desc,
                        std::list<QString>::iterator i,
                        const QString &context)
{
    if ((*i).length() > context.length())
        return true;
    i++;
    if (i == desc.end())
        return false;
    return SameContext(*i, context);
}

static std::list<QString>::iterator FindItem(std::list<QString> &desc,
        std::list<QString>::iterator i,
        int level,
        const QString &context,
        const QString &search)
{
    while (i != desc.end())
    {
        if (!SameContext(*i, context) && !context.isEmpty())
            return desc.end();
        if (toExtract::partDescribe(*i, level) == search)
            return i;
        i++;
    }
    return desc.end();
}

static QString GenerateConstraint(const QString &name,
                                  const QString &def,
                                  const QString &status)
{
    return QString::fromLatin1("<TR><TD VALIGN=top WIDTH=25%>%1</TD><TD VALIGN=top WIDTH=50%>%2</TD><TD VALIGN=top WIDTH=25%>%3</TD></TR>\n").
           arg(toHtml::escape(name)).
           arg(toHtml::escape(def)).
           arg(toHtml::escape(status));
}

static QString DescribeConstraints(std::list<QString> &desc,
                                   std::list<QString>::iterator i,
                                   int level,
                                   const QString &context)
{
    QString ret = QString::fromLatin1("<H4>CONSTRAINTS</H4>\n"
                                      "<TABLE BORDER=0 WIDTH=100%><TR><TH ALIGN=left WIDTH=25%>Name</TH>"
                                      "<TH ALIGN=left WIDTH=50%>Constraint</TH><TH ALIGN=left WIDTH=25%>Status</TH></TR>\n");

    QString lastName;
    QString status;
    QString definition;

    do
    {
        if (!SameContext(*i, context))
            break;

        QString name = toExtract::partDescribe(*i, level);
        QString extra = toExtract::partDescribe(*i, level + 1);

        if (lastName != name)
        {
            if (!lastName.isNull())
                ret += GenerateConstraint(lastName, definition, status);
            definition = QString::null;
            status = QString::null;
            lastName = name;
        }

        if (extra == QString::fromLatin1("DEFINITION"))
            definition += toExtract::partDescribe(*i, level + 2) + " ";
        else if (extra == QString::fromLatin1("STATUS"))
        {
            QString t = toExtract::partDescribe(*i, level + 2);
            if (t.startsWith(QString::fromLatin1("ENABLE")) || t.startsWith(QString::fromLatin1("DISABLE")))
                status += t + QString::fromLatin1(" ");
        }

        i++;
    }
    while (i != desc.end());
    ret += GenerateConstraint(lastName, definition, status);
    ret += QString::fromLatin1("</TABLE>\n");
    return ret;
}

struct toReportColumn
{
    QString Order;
    QString Datatype;
    QString Comment;
};

static QString DescribeColumns(std::list<QString> &desc,
                               std::list<QString>::iterator i,
                               int level,
                               const QString &context)
{
    bool hasComments = false;
    bool hasDatatype = false;

    int maxCol = 1;

    std::map<QString, toReportColumn> cols;
    do
    {
        if (!SameContext(*i, context))
            break;

        QString col = toExtract::partDescribe(*i, level + 0);
        QString extra = toExtract::partDescribe(*i, level + 1);

        if (extra == QString::fromLatin1("ORDER"))
        {
            cols[col].Order = toExtract::partDescribe(*i, level + 2);
            maxCol = qMax(maxCol, cols[col].Order.toInt());
        }
        else if (extra == QString::fromLatin1("COMMENT"))
        {
            cols[col].Comment = toExtract::partDescribe(*i, level + 2);
            hasComments = true;
        }
        else if (!extra.isEmpty())
        {
            cols[col].Datatype += extra + QString::fromLatin1(" ");
            hasDatatype = true;
        }

        i++;
    }
    while (i != desc.end());

    QString ret = qApp->translate("toReport",
                                  "<H4>COLUMNS</H4>\n"
                                  "<TABLE WIDTH=100% BORDER=0><TR><TH ALIGN=left WIDTH=25%>Name</TH>");
    if (hasDatatype)
        ret += qApp->translate("toReport", "<TH ALIGN=left WIDTH=25%>Definition</TH>");
    if (hasComments)
        ret += qApp->translate("toReport", "<TH ALIGN=left WIDTH=50%>Description</TH>");
    ret += QString::fromLatin1("</TR>\n");

    for (int j = 1;j <= maxCol;j++)
    {
        for (std::map<QString, toReportColumn>::iterator k = cols.begin();k != cols.end();k++)
        {
            if ((*k).second.Order.toInt() == j)
            {
                ret += QString::fromLatin1("<TR><TD VALIGN=top WIDTH=25%>");
                ret += toHtml::escape((*k).first);
                ret += QString::fromLatin1("</TD>");
                if (hasDatatype)
                {
                    ret += QString::fromLatin1("<TD VALIGN=top WIDTH=25%>");
                    if (!(*k).second.Datatype.isEmpty())
                        ret += toHtml::escape((*k).second.Datatype);
                    else
                        ret += QString::fromLatin1("<BR>");
                    ret += QString::fromLatin1("</TD>");
                }
                if (hasComments)
                {
                    ret += QString::fromLatin1("<TD VALIGN=top WIDTH=50%>");
                    if (!(*k).second.Comment.isEmpty())
                        ret += toHtml::escape((*k).second.Comment);
                    else
                        ret += QString::fromLatin1("&nbsp;");
                    ret += QString::fromLatin1("</TD>");
                }
                ret += QString::fromLatin1("</TR>\n");
                break;
            }
        }
    }
    ret += QString::fromLatin1("</TABLE>\n");
    return ret;
}

static QString ValidPart(std::list<QString> &desc,
                         std::list<QString>::iterator &i,
                         int level)
{
    QString part = toExtract::partDescribe(*i, level);
    while (part.isNull() && i != desc.end())
    {
        i++;
        part = toExtract::partDescribe(*i, level);
    }
    return part;
}

static QString DescribePart(std::list<QString> &desc,
                            std::list<QString>::iterator &i,
                            int level,
                            const QString &parentContext)
{
    QString ret;
    QString text;

    QString lastPart;

    std::list<QString>::iterator start = i;

    do
    {
        if (!SameContext(*i, parentContext))
            break;
        QString part = ValidPart(desc, i, level);

        do
        {
            QString part = toExtract::partDescribe(*i, level);
        }
        while (part.isNull());
        if (lastPart != part)
        {
            if (lastPart.isNull())
            {
                i = FindItem(desc, start, level, parentContext, QString::fromLatin1("TABLE"));
                if (i == desc.end())
                {
                    i = start;
                    part = ValidPart(desc, i, level);
                }
                else
                    part = toExtract::partDescribe(*i, level);
            }
            else
            {
                while (part == QString::fromLatin1("TABLE"))
                {
                    i++;
                    if (i == desc.end())
                        return ret;
                    part = toExtract::partDescribe(*i, level);
                }
            }
            if (lastPart == QString::fromLatin1("TABLE"))
            {
                i = start;
                part = ValidPart(desc, i, level);
            }
            lastPart = part;
        }
        QString child = parentContext + QString::fromLatin1("\001") + part;

        if (part == QString::fromLatin1("COLUMN") ||
                part == QString::fromLatin1("COMMENT") ||
                part == QString::fromLatin1("CONSTRAINT"))
        {
            i++;
        }
        else if (HasChildren(desc, i, child))
        {
            ret += QString::fromLatin1("<P><H%1>%2</H%3></P>\n").
                   arg(level + 1).
                   arg(toHtml::escape(part)).
                   arg(level + 1);
            std::list<QString>::iterator com = FindItem(desc,
                                               i,
                                               level + 1,
                                               child,
                                               QString::fromLatin1("COMMENT"));
            if (com != desc.end())
                ret += "<P>" + toHtml::escape(toExtract::partDescribe(*com, level + 2)) + "</P>";

            std::list<QString>::iterator col = FindItem(desc,
                                               i,
                                               level + 1,
                                               child,
                                               QString::fromLatin1("COLUMN"));

            if (col != desc.end())
                ret += DescribeColumns(desc, col, level + 2, child + QString::fromLatin1("\001COLUMN"));

            std::list<QString>::iterator con = FindItem(desc,
                                               i,
                                               level + 1,
                                               child,
                                               QString::fromLatin1("CONSTRAINT"));

            if (con != desc.end())
                ret += DescribeConstraints(desc, con, level + 2, child + QString::fromLatin1("\001CONSTRAINT"));

            ret += DescribePart(desc, i, level + 1, child);
        }
        else
        {
            if (!part.isEmpty())
                text += QString::fromLatin1("<P>") + toHtml::escape(part) + QString::fromLatin1("</P>\n");
            i++;
        }
    }
    while (i != desc.end());
    return text + ret;
}

QString toGenerateReport(toConnection &conn, std::list<QString> &desc)
{
    char host[1024];
    gethostname(host, 1024);

    QString db = conn.host();
    if (db.length() && db != QString::fromLatin1("*"))
        db += QString::fromLatin1(":");
    else
        db = QString::null;
    db += conn.database();

    QString ret = qApp->translate("toReport", "<HTML><HEAD><TITLE>Report on database %7</TITLE></HEAD>\n"
                                  "<BODY><H1>Report on database %8</H1>\n"
                                  "<TABLE BORDER=0>\n"
                                  "<TR><TD VALIGN=top>Generated by:</TD><TD VALIGN=top>TOra, Version %1</TD></TR>\n"
                                  "<TR><TD VALIGN=top>At:</TD><TD VALIGN=top>%2</TD></TR>\n"
                                  "<TR><TD VALIGN=top>From:</TD><TD VALIGN=top>%3, an %4 %5 database</TD></TR>\n"
                                  "<TR><TD VALIGN=top>On:</TD><TD VALIGN=top>%6</TD></TR>\n"
                                  "</TABLE>\n").
                  arg(QString::fromLatin1(TOVERSION)).
                  arg(QString::fromLatin1(host)).
                  arg(db).
                  arg(QString(conn.provider())).
                  arg(QString(conn.version())).
                  arg(QDateTime::currentDateTime().toString()).
                  arg(db).
                  arg(db);

    std::list<QString>::iterator i;
    i = FindItem(desc, desc.begin(), 0, QString::null, QString::fromLatin1("NONE"));
    if (i != desc.end())
    {
        ret += qApp->translate("toReport", "<H1>Global Objects</H1>\n");
        ret += DescribePart(desc, i, 1, QString::fromLatin1("NONE"));
    }

    i = desc.begin();
    QString lastContext;
    while (i != desc.end())
    {
        QString context = toExtract::partDescribe(*i, 0);
        if (context != QString::fromLatin1("NONE"))
        {
            if (context != lastContext)
            {
                if (context == QString::fromLatin1("public"))
                    ret += qApp->translate("toReport", "<H1>Public</H1>");
                else
                    ret += qApp->translate("toReport", "<H1>Schema %1</H1>\n").arg(toHtml::escape(context));
            }
            ret += DescribePart(desc, i, 1, context);
            lastContext = context;
        }
        else
            i++;
    }
    ret += QString::fromLatin1("</BODY>\n</HTML>");

    return ret;

}
