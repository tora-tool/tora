
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

#include "editor/todifftext.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"

#include <dtl/dtl.hpp>

#include <QListWidget>
#include <QVBoxLayout>
#include <QApplication>

#include <Qsci/qscilexerdiff.h>

class Line
{
public:
    Line(QString _txt, unsigned _number) : number(_number), txt(_txt) {};
    Line(const Line& other) : number(other.number) , txt(other.txt) {};

    Line& operator=(const Line& other)
    {
        number = other.number;
        txt = other.txt;
        return *this;
    }

    unsigned number;
    QString txt;
};
inline bool operator==(const Line& lhs, const Line& rhs)
{
    return lhs.txt == rhs.txt;
}
inline bool operator!=(const Line& lhs, const Line& rhs){ return !(lhs == rhs); }

#define declareStyle(style,color, paper, font) styleNames[style] = tr(#style); \
    setColor(color, style); \
    setPaper(paper, style); \
    setFont(font, style);

class DiffLexer: public QsciLexerCustom
{
public:
    typedef QsciLexerCustom super;
    enum Style
    {
        Default = 0,
        LineRemoved = QsciLexerDiff::LineRemoved,
        LineAdded = QsciLexerDiff::LineAdded,
        LineChanged =QsciLexerDiff::LineChanged
    };

    DiffLexer(QObject *parent) : QsciLexerCustom(parent)
    {
        using namespace ToConfiguration;
        QFont mono = QFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
        declareStyle(Default,
                     QColor(Qt::black),
                     QColor("white"),
                     mono);
        declareStyle(LineRemoved,
                     QColor(Qt::black),
                     QColor(toDiffText::lightCyan),
                     mono);
        declareStyle(LineAdded,
                     QColor(Qt::black),
                     QColor(toDiffText::lightMagenta),
                     mono);
        declareStyle(LineChanged,
                     QColor(0x0, 0x80, 0x0),
                     QColor(Qt::cyan),
                     mono);
    }

    QString description(int style) const override
    {
        if (styleNames.contains(style))
        {
            return styleNames[style];
        }
        else
        {
            return QString("");
        }
    }

    const char* language() const override
    {
        return "ToraDiff";
    }

    void styleText(int,int) override
    {
    }

    QMap<int,QString> styleNames;
}
;

toDiffText::toDiffText(QWidget *parent, const char *name)
    : toScintilla(parent)
{
    using namespace ToConfiguration;
    if (name)
        setObjectName(name);
    QFont mono = QFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
    toScintilla::setFont(mono);

    QsciLexer *lexer = new DiffLexer(this);
    toScintilla::setLexer(lexer);
}

void toDiffText::setText (const QString& oldTxt, const QString& newTxt)
{
    using dtl::Diff;
    typedef Line elem;
    typedef std::vector<Line> sequence;

    QRegExp newline("\n|\r\n|\r");
    QStringList oldLines = oldTxt.split(newline);
    QStringList newLines = newTxt.split(newline);

    sequence Old, New;
    unsigned i = 0;
    foreach(QString line, oldLines)
    {
        Old.push_back(Line(line, i++));
    }
    i = 0;
    foreach(QString line, newLines)
    {
        New.push_back(Line(line, i++));
    }

    Diff< elem, sequence > d(Old, New);
    d.compose();

    auto seq = d.getSes().getSequence();
    int lineNumber = 0;
    foreach(auto &e, seq)
    {
        Line line = e.first;
        auto info  = e.second;
        long long beforeIdx = info.beforeIdx;
        long long afterIdx  = info.afterIdx;
        dtl::edit_t type    = info.type;

        long pos = toScintilla::positionFromLineIndex(lineNumber, 0);
        toScintilla::append(line.txt);
        toScintilla::append("\n");

        toScintilla::SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, pos, 0x1f);
        switch(type)
        {
            case dtl::SES_DELETE:
                toScintilla::SendScintilla(QsciScintillaBase::SCI_SETSTYLING, line.txt.length(), QsciLexerDiff::LineRemoved);
                break;
            case dtl::SES_COMMON:
                toScintilla::SendScintilla(QsciScintillaBase::SCI_SETSTYLING, line.txt.length(), QsciLexerDiff::Default);
                break;
            case dtl::SES_ADD:
                toScintilla::SendScintilla(QsciScintillaBase::SCI_SETSTYLING, line.txt.length(), QsciLexerDiff::LineAdded);
                break;
            default:
                toScintilla::SendScintilla(QsciScintillaBase::SCI_SETSTYLING, line.txt.length(), QsciLexerDiff::Default);
        }
        lineNumber++;
    }
}

QColor toDiffText::lightCyan =  QColor(Qt::cyan).light(180);
QColor toDiffText::lightMagenta = QColor (Qt::magenta).light (180);
