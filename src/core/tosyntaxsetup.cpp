
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

#include "core/tosyntaxsetup.h"
#include "shortcuteditor/shortcuteditordialog.h"
#include "core/totreewidget.h"
#include "core/utils.h"
#include "parsing/tosyntaxanalyzer.h"
#include "editor/todebugtext.h"
#include "editor/toworksheettext.h"
#include "core/toconfiguration_new.h"
#include "core/toeditorsetting.h"

#include <QtGui/QFontDialog>
#include <QtGui/QColorDialog>

#include <Qsci/qscilexersql.h>

using namespace ToConfiguration;

toSyntaxSetup::toSyntaxSetup(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent)
    , toSettingTab("fonts.html")
	, ColorsEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum))
	, Current(NULL)
	, Styles(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::EditStyleMap).value<toStylesMap>())
{
	using namespace ToConfiguration;

    if (name)
        setObjectName(name);

    setupUi(this);

    connect(EditorShortcutsEdit, SIGNAL(clicked()),
            this, SLOT(openEditorShortcutsDialog()));

    // Crete/Adjust additioal complex widgets
    Analyzer = new toSyntaxAnalyzerNL(Example->editor());
    //Example->setAnalyzer(Analyzer);
    Example->sciEditor()->setReadOnly(true);

    Example->sciEditor()->setText(QString::fromLatin1("create procedure CheckObvious as\n"
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

    Example->setCurrentDebugLine(4);
    QMap<int, QString> Errors;
    Errors[2] = tr("Unknown variable");
    Example->setErrors(Errors);

    for (int idx = 0; idx < ColorsEnum.keyCount(); idx++)
    {
    	QString colorName = ColorsEnum.key(idx);
    	SyntaxComponent->addItem(colorName);
    }

    // load values from toConfigurationNewSingle into Widgets (if widget name == Config Option Name)
    toSettingTab::loadSettings(this);

    {
        QFont font(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
        checkFixedWidth(font);
        CodeExampleFont->setFont(font);
    }

    TextExampleFont->setFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfTextFont).toString()));

    {
        QString str(toConfigurationNewSingle::Instance().option(Editor::ListTextFont).toString());
        QFont font;
        if (str.isEmpty())
        {
            QWidget *wid = new toTreeWidget(this);
            font = qApp->font(wid);
            delete wid;
        }
        else
        {
            font = Utils::toStringToFont(str);
        }
        ListFontName = Utils::toFontToString(font);
        ResultExampleFont->setFont(font);
    }
}

void toSyntaxSetup::checkFixedWidth(const QFont &fnt)
{
    QFontMetrics mtr(fnt);
    if (mtr.width(QString::fromLatin1("iiiiiiii")) == mtr.width(QString::fromLatin1("MMMMMMMM")))
        KeywordUpperBool->setEnabled(true);
    else
    {
        KeywordUpperBool->setChecked(false);
        KeywordUpperBool->setEnabled(false);
    }
}

void toSyntaxSetup::selectFont(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, CodeExampleFont->font(), this);

    if (ok)
    {
        CodeExampleFont->setFont(font);
        Example->setFont(font);
        checkFixedWidth(font);
    }
}

void toSyntaxSetup::selectText(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, TextExampleFont->font(), this);

    if (ok)
        TextExampleFont->setFont(font);
}

void toSyntaxSetup::selectResultFont(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, Utils::toStringToFont(ListFontName), this);

    if (ok)
    {
        ListFontName = Utils::toFontToString(font);
        ResultExampleFont->setFont(font);
    }
}

void toSyntaxSetup::openEditorShortcutsDialog()
{
    ShortcutEditorDialog dia(this);
    dia.exec();
}

int toSyntaxSetup::wordClass() const
{
    QString t = Current->text();
    int e = ColorsEnum.keyToValue(t.toStdString().c_str());
    if (e == -1)
    	throw tr("Unknown color name %1").arg(t);

    return e;
}

void toSyntaxSetup::changeLine(QListWidgetItem *item)
{
    Current = item;
    if (Current)
    {
    	toSyntaxAnalyzer::WordClassEnum wc = (toSyntaxAnalyzer::WordClassEnum) wordClass();
    	QPalette palette = FontSample->palette();
        palette.setColor(QPalette::Background, Styles.value(wc).BGColor);
        palette.setColor(QPalette::Foreground, Styles.value(wc).FGColor);
        //ExampleColor->setPalette(palette);
        //ExampleColor->setAutoFillBackground(true);
        //ExampleColor->setText("What ever text");

        palette = FGSample->palette();
        palette.setColor(QPalette::Background, Styles.value(wc).FGColor);
        FGSample->setPalette(palette);

        palette = BGSample->palette();
        palette.setColor(QPalette::Background, Styles.value(wc).BGColor);
        BGSample->setPalette(palette);

        palette = FontSample->palette();
        palette.setColor(QPalette::Background, Styles.value(wc).BGColor);
        palette.setColor(QPalette::Foreground, Styles.value(wc).FGColor);
        FontSample->setFont(Styles.value(wc).Font);
        FontSample->setPalette(palette);
    }
}

void toSyntaxSetup::selectColor(void)
{
    try
    {
        if (Current)
        {
            int coleng = wordClass();
            QColor col = QColorDialog::getColor(Styles.value(coleng).FGColor);
            if (col.isValid())
            {
                Styles[coleng].FGColor = col;

               QPalette palette = FontSample->palette();
               palette.setColor(QPalette::Foreground, col);
               FontSample->setPalette(palette);
               FontSample->update();

               palette.setColor(QPalette::Background, col);
               FGSample->setPalette(palette);
               FGSample->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::saveSetting(void)
{
    toSettingTab::saveSettings(this);
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Editor::ConfTextFont, Utils::toFontToString(TextExampleFont->font()));
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Editor::ConfCodeFont, Utils::toFontToString(CodeExampleFont->font()));
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Editor::ListTextFont, Utils::toFontToString(ResultExampleFont->font()));

    // for ToConfiguration::Editor::EditStyleMap see ShortcutModel::saveValues
}

ToConfiguration::Editor toSyntaxSetup::s_editorConfig;
