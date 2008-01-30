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

#include <qfontdialog.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <totreewidget.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QString>

toSyntaxSetup::toSyntaxSetup(QWidget *parent, const char *name, Qt::WFlags fl)
        : QWidget(parent), toSettingTab("fonts.html"),
        Analyzer(toSyntaxAnalyzer::defaultAnalyzer())
{
    setupUi(this);
    KeywordUpper->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_KEYWORD_UPPER, DEFAULT_KEYWORD_UPPER).isEmpty());
    SyntaxHighlighting->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_HIGHLIGHT, "Yes").isEmpty());
    CodeCompletion->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_CODE_COMPLETION, "Yes").isEmpty());
    CompletionSort->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_COMPLETION_SORT, "Yes").isEmpty());
    AutoIndent->setChecked(!toConfigurationSingle::Instance().globalConfig(CONF_AUTO_INDENT, "Yes").isEmpty());
    Extensions->setText(toConfigurationSingle::Instance().globalConfig(CONF_EXTENSIONS, DEFAULT_EXTENSIONS));
    TabStop->setValue(toMarkedText::defaultTabWidth());

    {
        QFont font(toStringToFont(toConfigurationSingle::Instance().globalConfig(CONF_CODE, "")));
        checkFixedWidth(font);
        CodeExample->setFont(font);
    }

    TextExample->setFont(toStringToFont(toConfigurationSingle::Instance().globalConfig(CONF_TEXT, "")));

    {
        QString str = toConfigurationSingle::Instance().globalConfig(CONF_LIST, "");
        QFont font;
        if (str.isEmpty())
        {
            QWidget *wid = new toTreeWidget;
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
#define INIT_COL(c) { \
 Colors[Analyzer.typeString(c)] = Analyzer.getColor(c); \
 SyntaxComponent->insertItem(tr(Analyzer.typeString(c))); \
 }
        INIT_COL(toSyntaxAnalyzer::Default);
        INIT_COL(toSyntaxAnalyzer::Comment);
        INIT_COL(toSyntaxAnalyzer::Number);
        INIT_COL(toSyntaxAnalyzer::Keyword);
        INIT_COL(toSyntaxAnalyzer::String);
        INIT_COL(toSyntaxAnalyzer::DefaultBg);
        INIT_COL(toSyntaxAnalyzer::ErrorBg);
        INIT_COL(toSyntaxAnalyzer::DebugBg);
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
                                         "  HugeNumber := -12345678.90;\n"
                                         "/*\n"
                                         " * multi line comment\n"
                                         " */\n"
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
    QString conf(CONF_COLOR ":");
    conf += typeString(typ);
    QString res = toConfigurationSingle::Instance().globalConfig(conf, "");

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

toSyntaxAnalyzer::infoType toSyntaxAnalyzer::typeString(const QString &str)
{
    if (str == "Default")
        return Default;
    if (str == "Comment")
        return Comment;
    if (str == "Number")
        return Number;
    if (str == "Keyword")
        return Keyword;
    if (str == "String")
        return String;
    if (str == "Background")
        return DefaultBg;
    if (str == "Error background")
        return ErrorBg;
    if (str == "Debug background")
        return DebugBg;
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

QString toSyntaxAnalyzer::typeString(infoType typ)
{
    switch (typ)
    {
    case Default:
        return "Default";
    case Comment:
        return "Comment";
    case Number:
        return "Number";
    case Keyword:
        return "Keyword";
    case String:
        return "String";
    case DefaultBg:
        return "Background";
    case ErrorBg:
        return "Error background";
    case DebugBg:
        return "Debug background";
    }
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

void toSyntaxAnalyzer::updateSettings(void)
{
    try
    {
        const QColorGroup &cg = qApp->palette().active();
        readColor(cg.text(), Default);
//         readColor(Qt::green, Comment);
		readColor(QColor(160, 160, 160), Comment);
//         readColor(Qt::cyan, Number);
		readColor(QColor(0, 160, 13), Number);
//         readColor(Qt::blue, Keyword);
		readColor(QColor(13, 0, 160), Keyword);
        readColor(Qt::red, String);
        readColor(cg.base(), DefaultBg);
        readColor(Qt::darkRed, ErrorBg);
        readColor(Qt::darkGreen, DebugBg);
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
    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, CodeExample->font(), this);

    if (ok)
    {
        CodeExample->setFont(font);
        Example->setFont(font);
        checkFixedWidth(font);
    }
}

void toSyntaxSetup::selectText(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, TextExample->font(), this);

    if (ok)
        TextExample->setFont(font);
}

void toSyntaxSetup::selectResultFont(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont (&ok, toStringToFont(List), this);

    if (ok)
    {
        List = toFontToString(font);
        ResultExample->setFont(font);
    }
}

QString toSyntaxSetup::color()
{
    QString t = Current->text();
    for (std::map<QString, QColor>::iterator i = Colors.begin();i != Colors.end();i++)
        if (qApp->translate("toSyntaxSetup", (*i).first) == t)
            return (*i).first;
    throw tr("Unknown color name %1").arg(t);
}

void toSyntaxSetup::changeLine(Q3ListBoxItem *item)
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
            QString coleng = color();
            QColor col = QColorDialog::getColor(Colors[coleng]);
            if (col.isValid())
            {
                Colors[coleng] = col;
                ExampleColor->setBackgroundColor(col);
                Example->analyzer().Colors[toSyntaxAnalyzer::typeString(coleng)] = col;
                Example->updateSyntaxColor(toSyntaxAnalyzer::typeString(coleng));
                Example->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::saveSetting(void)
{
    toConfigurationSingle::Instance().globalSetConfig(CONF_TEXT, toFontToString(TextExample->font()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_CODE, toFontToString(CodeExample->font()));
    toConfigurationSingle::Instance().globalSetConfig(CONF_LIST, List);
    bool highlight = SyntaxHighlighting->isChecked();
    toConfigurationSingle::Instance().globalSetConfig(CONF_HIGHLIGHT, highlight ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_KEYWORD_UPPER, KeywordUpper->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_CODE_COMPLETION, highlight && CodeCompletion->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_COMPLETION_SORT, CompletionSort->isChecked() ? "Yes" : "");
    toConfigurationSingle::Instance().globalSetConfig(CONF_AUTO_INDENT, AutoIndent->isChecked() ? "Yes" : "");
    toMarkedText::setDefaultTabWidth(TabStop->value());
    toConfigurationSingle::Instance().globalSetConfig(CONF_TAB_STOP, QString::number(toMarkedText::defaultTabWidth()));
    for (std::map<QString, QColor>::iterator i = Colors.begin();i != Colors.end();i++)
    {
        QString str(CONF_COLOR);
        str += ":";
        str += (*i).first;
        QString res;
        res.sprintf("%d,%d,%d",
                    (*i).second.red(),
                    (*i).second.green(),
                    (*i).second.blue());
        toConfigurationSingle::Instance().globalSetConfig(str, res);
    }
    toSyntaxAnalyzer::defaultAnalyzer().updateSettings();
    toConfigurationSingle::Instance().globalSetConfig(CONF_EXTENSIONS, Extensions->text());
}
