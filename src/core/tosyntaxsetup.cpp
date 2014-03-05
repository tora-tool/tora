
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

    Analyzer = new toSyntaxAnalyzerNL(Example->editor());

    //SyntaxHighlighting->setCurrentIndex(toConfigurationNewSingle::Instance().syntaxHighlighting());
    //EditorType->setCurrentIndex(toConfigurationNewSingle::Instance().editorType());
    //KeywordUpper->setChecked(toConfigurationNewSingle::Instance().keywordUpper());
    //ObjectNamesUpper->setChecked(toConfigurationNewSingle::Instance().objectNamesUpper());
    //CompletionSort->setChecked(toConfigurationNewSingle::Instance().completionSort());

    //UseMaxTextWidthMarkBool->setChecked(toConfigurationNewSingle::Instance().useMaxTextWidthMark());
    //MaxTextWidthMark->setValue(toConfigurationNewSingle::Instance().maxTextWidthMark());
    //CodeCompletionBool->setChecked(toConfigurationNewSingle::Instance().codeCompletion());
    //EditorShortcuts->setChecked(toConfigurationNewSingle::Instance().option(Editor::UseEditorShortcutsBool).toBool());
    connect(EditorShortcutsEdit, SIGNAL(clicked()),
            this, SLOT(openEditorShortcutsDialog()));

    //AutoIndentBool->setChecked(toConfigurationNewSingle::Instance().autoIndent());
    //Extensions->setText(toConfigurationNewSingle::Instance().extensions());
    //TabStopInt->setValue(toConfigurationNewSingle::Instance().tabStop());
    //UseSpacesForIndentBool->setChecked(toConfigurationNewSingle::Instance().useSpacesForIndent());

    {
        QFont font(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
        checkFixedWidth(font);
        CodeExample->setFont(font);
    }

    TextExample->setFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfTextFont).toString()));

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
        ResultExample->setFont(font);
    }

    for (int idx = 0; idx < ColorsEnum.keyCount(); idx++)
    {
    	QString colorName = ColorsEnum.key(idx);
    	SyntaxComponent->addItem(colorName);
    }
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

    toSettingTab::loadSettings(this);
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
    QFont font = QFontDialog::getFont(&ok, CodeExample->font(), this);

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
    QFont font = QFontDialog::getFont(&ok, TextExample->font(), this);

    if (ok)
        TextExample->setFont(font);
}

void toSyntaxSetup::selectResultFont(void)
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, Utils::toStringToFont(ListFontName), this);

    if (ok)
    {
        ListFontName = Utils::toFontToString(font);
        ResultExample->setFont(font);
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
    	QPalette palette = ExampleColor->palette();
        palette.setColor(QPalette::Background, Styles.value(wc).BGColor);
        palette.setColor(QPalette::Foreground, Styles.value(wc).FGColor);
        ExampleColor->setPalette(palette);
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

                QPalette palette = ExampleColor->palette();
                palette.setColor(QPalette::Background, col);
                ExampleColor->setPalette(palette);
                Example->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::saveSetting(void)
{
	//toConfigurationNewSingle::Instance().setSyntaxHighlighting(SyntaxHighlighting->currentIndex());
	//toConfigurationNewSingle::Instance().setEditorType(EditorType->currentIndex());

    //toConfigurationNewSingle::Instance().setTextFontName(Utils::toFontToString(TextExample->font()));
    //toConfigurationNewSingle::Instance().setCodeFontName(Utils::toFontToString(CodeExample->font()));
    //toConfigurationNewSingle::Instance().setListFontName(ListFontName);
    // TODO bool highlight = SyntaxHighlighting->isChecked();
    // TODO toConfigurationNewSingle::Instance().setHighlightType(highlight);
    //toConfigurationNewSingle::Instance().setUseMaxTextWidthMark(UseMaxTextWidthMarkBool->isChecked());
    //toConfigurationNewSingle::Instance().setMaxTextWidthMark(MaxTextWidthMark->value());
    //toConfigurationNewSingle::Instance().setKeywordUpper(KeywordUpper->isChecked());
    //toConfigurationNewSingle::Instance().setObjectNamesUpper(ObjectNamesUpper->isChecked());
    // TODO toConfigurationNewSingle::Instance().setCodeCompletion(highlight && CodeCompletion->isChecked());
    //toConfigurationNewSingle::Instance().setCompletionSort(CompletionSort->isChecked());
    //toConfigurationNewSingle::Instance().setUseEditorShortcuts(EditorShortcuts->isChecked());
    //toConfigurationNewSingle::Instance().setAutoIndent(AutoIndentBool->isChecked());
    //toConfigurationNewSingle::Instance().setTabStop(TabStopInt->value());
    //toConfigurationNewSingle::Instance().setUseSpacesForIndent(UseSpacesForIndentBool->isChecked());

    //toConfigurationNewSingle::Instance().setStyles(Styles);

//#define C2T(c) (Colors[Analyzer.typeString((c))])
//    toConfigurationNewSingle::Instance().setSyntaxDefaultBg(C2T(toSyntaxAnalyzer::DefaultBg));
//    toConfigurationNewSingle::Instance().setSyntaxDebugBg(C2T(toSyntaxAnalyzer::DebugBg));
//    toConfigurationNewSingle::Instance().setSyntaxErrorBg(C2T(toSyntaxAnalyzer::ErrorBg));
//    toConfigurationNewSingle::Instance().setSyntaxCurrentLineMarker(C2T(toSyntaxAnalyzer::CurrentLineMarker));
//    toConfigurationNewSingle::Instance().setSyntaxStaticBg(C2T(toSyntaxAnalyzer::StaticBg));
    //toConfigurationNewSingle::Instance().setExtensions(Extensions->text());
    toSettingTab::saveSettings(this);
}

ToConfiguration::Editor toSyntaxSetup::s_editorConfig;
