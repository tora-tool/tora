
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

#include "toconf.h"


#include "tomain.h"
#include "tohtml.h"

#include <qapplication.h>
#include <qregexp.h>

#include <QString>
#include <QChar>

toHtml::toHtml(const QString &data)
{
    Length = data.length();
    Data = data;
    Position = 0;
    LastChar = 0;
}

toHtml::~toHtml()
{
}

void toHtml::skipSpace(void)
{
    if (Position >= Length)
        return;
    QChar c = LastChar;
    if (c == 0)
        c = Data[Position];
    if (c.isSpace())
    {
        Position++;
        LastChar = 0;
        while (Position < Length && Data[Position].isSpace())
            Position++;
    }
}

bool toHtml::eof(void)
{
    if (Position > Length)
        throw qApp->translate("toHtml", "Invalidly went beyond end of file");
    return Position == Length;
}

void toHtml::nextToken(void)
{
    if (eof())
        throw qApp->translate("toHtml", "Reading HTML after eof");
    QualifierNum = 0;
    QChar c = LastChar;
    if (c == 0)
        c = Data[Position];
    if (c == '<')
    {
        IsTag = true;
        Position++;
        LastChar = 0;
        skipSpace();
        if (Position >= Length)
            throw qApp->translate("toHtml", "Lone < at end");
        if (Data[Position] != '/')
        {
            Open = true;
        }
        else
        {
            Open = false;
            Position++;
        }
        skipSpace();
        {
            int start = Position;
            while (Position < Length && !(Data[Position].isSpace()) && Data[Position] != '>')
            {
                Data[Position] = Data[Position].toLower();
                Position++;
            }
            Tag = mid(start, Position - start);
        }
        for (;;)
        {
            skipSpace();
            if (Position >= Length)
                throw qApp->translate("toHtml", "Unended tag at end");

            c = LastChar;
            if (c == 0)
                c = Data[Position];
            if (c == '>')
            {
                LastChar = 0;
                Position++;
                break;
            }

            // Must always be an empty char here, so LastChar not needed to be checked.

            {
                int start = Position;

                while (Position < Length &&
                        !(Data[Position].isSpace()) &&
                        Data[Position] != '=' &&
                        Data[Position] != '>')
                {
                    Data[Position] = Data[Position].toLower();
                    Position++;
                }
                Qualifiers[QualifierNum].Name = mid(start, Position - start);
            }
            skipSpace();
            if (Position >= Length)
                throw qApp->translate("toHtml", "Unended tag qualifier at end");
            c = LastChar;
            if (c == 0)
                c = Data[Position];
            if (c == '=')
            {
                LastChar = 0;
                Position++;
                skipSpace();
                if (Position >= Length)
                    throw qApp->translate("toHtml", "Unended tag qualifier data at end");
                c = Data[Position];
                if (c == '\'' || c == '\"')
                {
                    Position++;
                    int start = Position;
                    while (Data[Position] != c)
                    {
                        Position++;
                        if (Position >= Length)
                            throw qApp->translate("toHtml", "Unended quoted string at end");
                    }
                    Qualifiers[QualifierNum].Value = mid(start, Position - start);
                    Position++;
                    LastChar = 0;
                }
                else
                {
                    int start = Position;
                    while (!(Data[Position].isSpace()) && Data[Position] != '>')
                    {
                        Position++;
                        if (Position >= Length)
                            throw qApp->translate("toHtml", "Unended qualifier data at end");
                    }
                    Qualifiers[QualifierNum].Value = mid(start, Position - start);
                }
            }
            QualifierNum++;
            if (QualifierNum >= TO_HTML_MAX_QUAL)
                throw qApp->translate("toHtml", "Exceded qualifier max in toHtml");
        }
    }
    else
    {
        IsTag = false;
        int start = Position;
        Position++;
        LastChar = 0;
        while (Position < Length)
        {
            if (Data[Position] == '<')
                break;
            Position++;
        }
        Text = mid(start, Position - start);
    }
}

QString toHtml::value(const QString &q)
{
    for (int i = 0;i < QualifierNum;i++)
    {
        if (q == Qualifiers[i].Name)
            return Qualifiers[i].Value;
    }
    return NULL;
}

QString toHtml::text()
{
    QString ret;
    for (int pos = 0; pos < Text.length(); pos++)
    {
        if (Text[pos] == '&')
        {
            int start = pos + 1;
            while (Text[pos] != 0 && Text[pos] != ';')
                pos++;
            QString tmp(QByteArray(start, pos - start));
            if (tmp[0] == '#')
            {
                tmp = tmp.right(tmp.length() - 1);
                ret += char(tmp.toInt());
            }
            else if (tmp == "auml")
                ret += "�";
            // The rest of the & codes...
        }
        else
            ret += Text[pos];
    }
    return ret;
}

QString toHtml::mid(int start, int size)
{
    if (size == 0)
        return "";
    if (start >= Length)
        throw qApp->translate("toHtml", "Tried to access string out of bounds in mid (start=%1)").arg(start);
    if (size > Length)
        throw qApp->translate("toHtml", "Tried to access string out of bounds in mid (size=%1)").arg(size);
    if (start + size > Length)
        throw qApp->translate("toHtml", "Tried to access string out of bounds in mid (total=%1+%2>%3)").
        arg(start).
        arg(size).
        arg(Length);

    LastChar = Data[start + size];
    Data[start + size] = 0;
    return Data.mid(start, size);
}

bool toHtml::search(const QString &all, const QString &str)
{
    QString data(str.toLower().toLatin1());
    enum
    {
        beginning,
        inTag,
        inString,
        inWord
    } lastState = beginning, state = beginning;
    int pos = 0;
    QChar endString = 0;
    for (int i = 0;i < all.length();i++)
    {
        QChar c = all.at(i).toLower();
        if (c == '\'' || c == '\"')
        {
            endString = c;
            state = inString;
        }
        else if (c == '<')
        {
            state = inTag;
        }
        else
        {
            switch (state)
            {
            case inString:
                if (c == endString)
                    state = lastState;
                break;
            case beginning:
                if (data.at(pos) != c)
                {
                    pos = 0;
                    state = inWord;
                }
                else
                {
                    pos++;
                    if (pos >= data.length())
                    {
                        if (i + 1 >= all.length() || !all.at(i + 1).isLetterOrNumber())
                            return true;
                        pos = 0;
                    }
                    break;
                }
                // Intentionally no break here
            case inWord:
                if (!c.isLetterOrNumber())
                    state = beginning;
                break;
            case inTag:
                if (c == '>')
                    state = beginning;
                break;
            }
        }
    }
    return false;
}

QString toHtml::escape(const QString &html)
{
    QString ret = html;

    static QRegExp amp(QString::fromLatin1("\\&"));
    static QRegExp lt(QString::fromLatin1("\\<"));
    static QRegExp gt(QString::fromLatin1("\\>"));

    return ret;
}
