
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

#include "core/tosyntaxsetup.h"
#include "shortcuteditor/shortcuteditordialog.h"
#include "core/totreewidget.h"
#include "core/toconfiguration.h"
#include "core/utils.h"
#include "parsing/tosyntaxanalyzer.h"
#include "editor/todebugtext.h"

#include <QtCore/QMetaEnum>
#include <QtGui/QFontDialog>
#include <QtGui/QColorDialog>

toSyntaxSetup::toSyntaxSetup(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent)
    , toSettingTab("fonts.html")
{

    if (name)
        setObjectName(name);

    setupUi(this);

    Analyzer = new toSyntaxAnalyzerNL(Example->editor());

    KeywordUpper->setChecked(toConfigurationSingle::Instance().keywordUpper());
    ObjectNamesUpper->setChecked(toConfigurationSingle::Instance().objectNamesUpper());
    // TODO SyntaxHighlighting->setChecked(toConfigurationSingle::Instance().highlightType());
    EdgeMarkCheckBox->setChecked(toConfigurationSingle::Instance().useMaxTextWidthMark());
    EdgeSizeSpinBox->setValue(toConfigurationSingle::Instance().maxTextWidthMark());
    CodeCompletion->setChecked(toConfigurationSingle::Instance().codeCompletion());
    EditorShortcuts->setChecked(toConfigurationSingle::Instance().useEditorShortcuts());
    connect(EditorShortcutsEdit, SIGNAL(clicked()),
            this, SLOT(openEditorShortcutsDialog()));
    CompletionSort->setChecked(toConfigurationSingle::Instance().completionSort());
    AutoIndent->setChecked(toConfigurationSingle::Instance().autoIndent());
    Extensions->setText(toConfigurationSingle::Instance().extensions());
    TabStop->setValue(toConfigurationSingle::Instance().tabStop());
    TabSpaces->setChecked(toConfigurationSingle::Instance().tabSpaces());

    {
        QFont font(Utils::toStringToFont(toConfigurationSingle::Instance().codeFont()));
        checkFixedWidth(font);
        CodeExample->setFont(font);
    }

    TextExample->setFont(Utils::toStringToFont(toConfigurationSingle::Instance().textFont()));

    {
        QString str(toConfigurationSingle::Instance().listFont());
        QFont font;
        if (str.isEmpty())
        {
            QWidget *wid = new toTreeWidget(this);
            font = qApp->font(wid);
        }
        else
        {
            font = Utils::toStringToFont(str);
        }
        List = Utils::toFontToString(font);
        ResultExample->setFont(font);
    }

    QMetaEnum const& e = toSyntaxAnalyzer::staticMetaObject.enumerator(toSyntaxAnalyzer::staticMetaObject.indexOfEnumerator("wordClass"));
    for (int idx = 0; idx < e.keyCount(); idx++)
    {
    	QString colorName = e.key(idx);
    	//Colors[colorName] = Analyzer->getColor((toSyntaxAnalyzer::wordClass)e.value(idx));
    	SyntaxComponent->addItem(colorName);
    }
    //    Example->setAnalyzer(Analyzer);
    Example->setReadOnly(true);

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

    Example->setCurrentDebugLine(4);
    QMap<int, QString> Errors;
    Errors[2] = tr("Unknown variable");
    Example->setErrors(Errors);

    Current = NULL;
}

//void toSyntaxAnalyzer::readColor(const QColor &def, infoType typ)
//{
//#define CONF_COLOR  "KeywordColor"
//	QString conf(CONF_COLOR ":");
//	conf += typeString(typ);
//	QString res = toConfigurationSingle::Instance().globalConfig(conf, "");
//
//	if (res.isEmpty())
//		Colors[typ] = def;
//	else {
//		int r, g, b;
//		if (sscanf(res.toAscii().constData(), "%d,%d,%d", &r, &g, &b) != 3)
//			throw qApp->translate("toSyntaxAnalyzer", "Wrong format of color in setings");
//		QColor col(r, g, b);
//		Colors[typ] = col;
//	}
//}

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
    QFont font = QFontDialog::getFont(&ok, Utils::toStringToFont(List), this);

    if (ok)
    {
        List = Utils::toFontToString(font);
        ResultExample->setFont(font);
    }
}

void toSyntaxSetup::openEditorShortcutsDialog()
{
    ShortcutEditorDialog dia(this);
    dia.exec();
}

QString toSyntaxSetup::color()
{
    QString t = Current->text();
    for (std::map<QString, QColor>::iterator i = Colors.begin(); i != Colors.end(); i++)
        if (qApp->translate("toSyntaxSetup", (*i).first.toAscii().constData()) == t)
            return (*i).first;
    throw tr("Unknown color name %1").arg(t);
}

void toSyntaxSetup::changeLine(QListWidgetItem *item)
{
    Current = item;
    if (Current)
    {
        QColor col = Colors[color()];
        QPalette palette = ExampleColor->palette();
        palette.setColor(QPalette::Background, col);
        ExampleColor->setPalette(palette);
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

                QPalette palette = ExampleColor->palette();
                palette.setColor(QPalette::Background, col);
                ExampleColor->setPalette(palette);

                // Example->analyzer().Colors[toSyntaxAnalyzer::typeString(coleng)] = col;
                // Example->updateSyntaxColor(toSyntaxAnalyzer::typeString(coleng));
                Example->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::saveSetting(void)
{
    toConfigurationSingle::Instance().setTextFont(Utils::toFontToString(TextExample->font()));
    toConfigurationSingle::Instance().setCodeFont(Utils::toFontToString(CodeExample->font()));
    toConfigurationSingle::Instance().setListFont(List);
    // TODO bool highlight = SyntaxHighlighting->isChecked();
    // TODO toConfigurationSingle::Instance().setHighlightType(highlight);
    toConfigurationSingle::Instance().setUseMaxTextWidthMark(EdgeMarkCheckBox->isChecked());
    toConfigurationSingle::Instance().setMaxTextWidthMark(EdgeSizeSpinBox->value());
    toConfigurationSingle::Instance().setKeywordUpper(KeywordUpper->isChecked());
    toConfigurationSingle::Instance().setObjectNamesUpper(ObjectNamesUpper->isChecked());
    // TODO toConfigurationSingle::Instance().setCodeCompletion(highlight && CodeCompletion->isChecked());
    toConfigurationSingle::Instance().setCodeCompletionSort(CompletionSort->isChecked());
    toConfigurationSingle::Instance().setUseEditorShortcuts(EditorShortcuts->isChecked());
    toConfigurationSingle::Instance().setAutoIndent(AutoIndent->isChecked());
    toConfigurationSingle::Instance().setTabStop(TabStop->value());
    toConfigurationSingle::Instance().setTabSpaces(TabSpaces->isChecked());
    for (std::map<QString, QColor>::iterator i = Colors.begin(); i != Colors.end(); i++)
    {
//    	QString str(CONF_COLOR);
//         str += ":";
//         str += (*i).first;
//         QString res;
//         res.sprintf("%d,%d,%d",
//                     (*i).second.red(),
//                     (*i).second.green(),
//                     (*i).second.blue());
//         toConfigurationSingle::Instance().globalSetConfig(str, res);
    }

//#define C2T(c) (Colors[Analyzer.typeString((c))])
//    toConfigurationSingle::Instance().setSyntaxDefault(C2T(toSyntaxAnalyzer::Default));
//    toConfigurationSingle::Instance().setSyntaxComment(C2T(toSyntaxAnalyzer::Comment));
//    toConfigurationSingle::Instance().setSyntaxNumber(C2T(toSyntaxAnalyzer::Number));
//    toConfigurationSingle::Instance().setSyntaxKeyword(C2T(toSyntaxAnalyzer::Keyword));
//    toConfigurationSingle::Instance().setSyntaxString(C2T(toSyntaxAnalyzer::String));
//    toConfigurationSingle::Instance().setSyntaxDefaultBg(C2T(toSyntaxAnalyzer::DefaultBg));
//    toConfigurationSingle::Instance().setSyntaxDebugBg(C2T(toSyntaxAnalyzer::DebugBg));
//    toConfigurationSingle::Instance().setSyntaxErrorBg(C2T(toSyntaxAnalyzer::ErrorBg));
//    toConfigurationSingle::Instance().setSyntaxCurrentLineMarker(C2T(toSyntaxAnalyzer::CurrentLineMarker));
//    toConfigurationSingle::Instance().setSyntaxStaticBg(C2T(toSyntaxAnalyzer::StaticBg));
    //
    //    toSyntaxAnalyzer::defaultAnalyzer().updateSettings();
    toConfigurationSingle::Instance().setExtensions(Extensions->text());
}
