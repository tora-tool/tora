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
#include "tohighlightedtext.h"
#include "tosyntaxsetup.h"

#include <stdio.h>

#ifdef TO_KDE
#  include <kfontdialog.h>
#else
#  include <qfontdialog.h>
#endif

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qspinbox.h>

#include "tosyntaxsetup.moc"
#include "tosyntaxsetupui.moc"

toSyntaxSetup::toSyntaxSetup(QWidget *parent, const char *name, WFlags fl)
        : toSyntaxSetupUI(parent, name, fl), toSettingTab("fonts.html"),
        Analyzer(toSyntaxAnalyzer::defaultAnalyzer())
{
    KeywordUpper->setChecked(!toTool::globalConfig(CONF_KEYWORD_UPPER, DEFAULT_KEYWORD_UPPER).isEmpty());
    SyntaxHighlighting->setChecked(!toTool::globalConfig(CONF_HIGHLIGHT, "Yes").isEmpty());
    CodeCompletion->setChecked(!toTool::globalConfig(CONF_CODE_COMPLETION, "Yes").isEmpty());
    CompletionSort->setChecked(!toTool::globalConfig(CONF_COMPLETION_SORT, "Yes").isEmpty());
    AutoIndent->setChecked(!toTool::globalConfig(CONF_AUTO_INDENT, "Yes").isEmpty());
    Extensions->setText(toTool::globalConfig(CONF_EXTENSIONS, DEFAULT_EXTENSIONS));
    TabStop->setValue(toMarkedText::defaultTabWidth());

    {
        QFont font(toStringToFont(toTool::globalConfig(CONF_CODE, "")));
        checkFixedWidth(font);
        CodeExample->setFont(font);
    }

    TextExample->setFont(toStringToFont(toTool::globalConfig(CONF_TEXT, "")));

    {
        QString str = toTool::globalConfig(CONF_LIST, "");
        QFont font;
        if (str.isEmpty())
        {
            QWidget *wid = new QListView;
            font = qApp->font(wid);
        }
        else
        {
            font = toStringToFont(str);
        }
        List = toFontToString(font);
        ResultExample->setFont(font);
    }
    try
    {
        Colors[Analyzer.typeString(toSyntaxAnalyzer::NormalBkg)] = Analyzer.getColor(toSyntaxAnalyzer::NormalBkg);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::ErrorBkg)] = Analyzer.getColor(toSyntaxAnalyzer::ErrorBkg);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::CurrentBkg)] = Analyzer.getColor(toSyntaxAnalyzer::CurrentBkg);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::Keyword)] = Analyzer.getColor(toSyntaxAnalyzer::Keyword);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::Comment)] = Analyzer.getColor(toSyntaxAnalyzer::Comment);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::Normal)] = Analyzer.getColor(toSyntaxAnalyzer::Normal);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::String)] = Analyzer.getColor(toSyntaxAnalyzer::String);
        Colors[Analyzer.typeString(toSyntaxAnalyzer::Error)] = Analyzer.getColor(toSyntaxAnalyzer::Error);

        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::NormalBkg)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::Comment)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::CurrentBkg)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::ErrorBkg)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::Keyword)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::Normal)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::String)));
        SyntaxComponent->insertItem(tr(Analyzer.typeString(toSyntaxAnalyzer::Error)));
    }
    TOCATCH

    Example->setAnalyzer(Analyzer);
    Example->setReadOnly(true);
#ifdef TO_NO_ORACLE

    Example->setText(QString::fromLatin1("create procedure CheckObvious\n"
                                         "begin\n"
                                         "  set Quest = 'Great'; -- This variable doesn't exist\n"
                                         "  if Quest = 'Great' then\n"
                                         "    call Obvious(true);\n"
                                         "  end if;\n"
                                         "end"));
#else

    Example->setText(QString::fromLatin1("create procedure CheckObvious as\n"
                                         "begin\n"
                                         "  Quest:='Great'; -- This variable doesn't exist\n"
                                         "  if Quest = 'Great' then\n"
                                         "    Obvious(true);\n"
                                         "  end if;\n"
                                         "end;"));
#endif

    Example->setCurrent(4);
    std::map<int, QString> Errors;
    Errors[2] = tr("Unknown variable");
    Example->setErrors(Errors);

    Current = NULL;
}

void toSyntaxAnalyzer::readColor(const QColor &def, infoType typ)
{
    QCString conf(CONF_COLOR ":");
    conf += typeString(typ);
    QString res = toTool::globalConfig(conf, "");
    if (res.isEmpty())
        Colors[typ] = def;
    else
    {
        int r, g, b;
        if (sscanf(res, "%d,%d,%d", &r, &g, &b) != 3)
            throw qApp->translate("toSyntaxAnalyzer", "Wrong format of color in setings");
        QColor col(r, g, b);
        Colors[typ] = col;
    }
}

toSyntaxAnalyzer::infoType toSyntaxAnalyzer::typeString(const QCString &str)
{
    if (str == "Normal")
        return Normal;
    if (str == "Keyword")
        return Keyword;
    if (str == "String")
        return String;
    if (str == "Unfinished string")
        return Error;
    if (str == "Comment")
        return Comment;
    if (str == "Error background")
        return ErrorBkg;
    if (str == "Background")
        return NormalBkg;
    if (str == "Current background")
        return CurrentBkg;
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

QCString toSyntaxAnalyzer::typeString(infoType typ)
{
    switch (typ)
    {
    case Normal:
        return "Normal";
    case Keyword:
        return "Keyword";
    case String:
        return "String";
    case Error:
        return "Unfinished string";
    case Comment:
        return "Comment";
    case ErrorBkg:
        return "Error background";
    case NormalBkg:
        return "Background";
    case CurrentBkg:
        return "Current background";
    }
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

void toSyntaxAnalyzer::updateSettings(void)
{
    try
    {
        const QColorGroup &cg = qApp->palette().active();
        readColor(cg.base(), NormalBkg);
        readColor(Qt::darkRed, ErrorBkg);
        readColor(Qt::darkGreen, CurrentBkg);
        readColor(Qt::blue, Keyword);
        readColor(cg.text(), Normal);
        readColor(Qt::red, String);
        readColor(Qt::red, Error);
        readColor(Qt::green, Comment);
    }
    TOCATCH
}

void toSyntaxSetup::checkFixedWidth(const QFont &fnt)
{
    QFontMetrics mtr(fnt);
    if (mtr.width(QString::fromLatin1("iiiiiiii")) == mtr.width(QString::fromLatin1("MMMMMMMM")))
        KeywordUpper->setEnabled(true);
    else
    {
        KeywordUpper->setChecked(false);
        KeywordUpper->setEnabled(false);
    }
}

void toSyntaxSetup::selectFont(void)
{
#ifdef TO_KDE
    QFont font = CodeExample->font();
    bool ok = KFontDialog::getFont(font, false, this);
#else

    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, CodeExample->font(), this);
#endif

    if (ok)
    {
        CodeExample->setFont(font);
        Example->setFont(font);
        checkFixedWidth(font);
    }
}

void toSyntaxSetup::selectText(void)
{
#ifdef TO_KDE
    QFont font = TextExample->font();
    bool ok = KFontDialog::getFont(font, false, this);
#else

    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, TextExample->font(), this);
#endif

    if (ok)
        TextExample->setFont(font);
}

void toSyntaxSetup::selectResultFont(void)
{
#ifdef TO_KDE
    QFont font = toStringToFont(List);
    bool ok = KFontDialog::getFont(font, false, this);
#else

    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, toStringToFont(List), this);
#endif

    if (ok)
    {
        List = toFontToString(font);
        ResultExample->setFont(font);
    }
}

QCString toSyntaxSetup::color()
{
    QString t = Current->text();
    for (std::map<QCString, QColor>::iterator i = Colors.begin();i != Colors.end();i++)
        if (qApp->translate("toSyntaxSetup", (*i).first) == t)
            return (*i).first;
    throw tr("Unknown color name %1").arg(t);
}

void toSyntaxSetup::changeLine(QListBoxItem *item)
{
    Current = item;
    if (Current)
    {
        QColor col = Colors[color()];
        ExampleColor->setBackgroundColor(col);
    }
}

void toSyntaxSetup::selectColor(void)
{
    try
    {
        if (Current)
        {
            QCString coleng = color();
            QColor col = QColorDialog::getColor(Colors[coleng]);
            if (col.isValid())
            {
                Colors[coleng] = col;
                ExampleColor->setBackgroundColor(col);
                Example->analyzer().Colors[toSyntaxAnalyzer::typeString(coleng)] = col;
                Example->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::saveSetting(void)
{
    toTool::globalSetConfig(CONF_TEXT, toFontToString(TextExample->font()));
    toTool::globalSetConfig(CONF_CODE, toFontToString(CodeExample->font()));
    toTool::globalSetConfig(CONF_LIST, List);
    bool highlight = SyntaxHighlighting->isChecked();
    toTool::globalSetConfig(CONF_HIGHLIGHT, highlight ? "Yes" : "");
    toTool::globalSetConfig(CONF_KEYWORD_UPPER, KeywordUpper->isChecked() ? "Yes" : "");
    toTool::globalSetConfig(CONF_CODE_COMPLETION, highlight && CodeCompletion->isChecked() ? "Yes" : "");
    toTool::globalSetConfig(CONF_COMPLETION_SORT, CompletionSort->isChecked() ? "Yes" : "");
    toTool::globalSetConfig(CONF_AUTO_INDENT, AutoIndent->isChecked() ? "Yes" : "");
    toMarkedText::setDefaultTabWidth(TabStop->value());
    toTool::globalSetConfig(CONF_TAB_STOP, QString::number(toMarkedText::defaultTabWidth()));
    for (std::map<QCString, QColor>::iterator i = Colors.begin();i != Colors.end();i++)
    {
        QCString str(CONF_COLOR);
        str += ":";
        str += (*i).first;
        QString res;
        res.sprintf("%d,%d,%d",
                    (*i).second.red(),
                    (*i).second.green(),
                    (*i).second.blue());
        toTool::globalSetConfig(str, res);
    }
    toSyntaxAnalyzer::defaultAnalyzer().updateSettings();
    toTool::globalSetConfig(CONF_EXTENSIONS, Extensions->text());
}
