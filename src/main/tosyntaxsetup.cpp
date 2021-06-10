
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

#include "main/tosyntaxsetup.h"

#include "shortcuteditor/shortcuteditordialog.h"
#include "widgets/totreewidget.h"
#include "core/utils.h"
#include "editor/todebugtext.h"
#include "editor/toworksheettext.h"
#include "core/toconfiguration.h"
#include "ts_log/ts_log_utils.h"
#include "core/tosyntaxanalyzer.h"
#include "editor/tosyntaxanalyzernl.h"
#include "editor/tosyntaxanalyzeroracle.h"
#include "parsing/toindent.h"
#include "core/toeditorconfiguration.h"

#include <QFontDialog>
#include <QColorDialog>

#include <Qsci/qscilexersql.h>

using namespace ToConfiguration;

toSyntaxSetup::toSyntaxSetup(QWidget *parent, const char *name)
    : QWidget(parent)
    , toSettingTab("fonts.html")
    , Current(NULL)
    , WordClassEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum))
    , Styles(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::EditStyleMap).value<toStylesMap>())
    , indentInst(indentParams)
{
    using namespace ToConfiguration;

    if (name)
        setObjectName(name);

    setupUi(this);

    connect(ResultExampleChoose, &QPushButton::clicked, this, [=]() { this->selectResultFont(); });
    connect(CodeExampleChoose, &QPushButton::clicked, this, [=]() { this->selectFont(); });
    connect(PickFontButton, &QPushButton::clicked, this, [=]() { this->selectFont(); });
    connect(KeywordUpperBool, &QCheckBox::toggled, this, [=](bool checked) { this->changeUpper(checked); });
    connect(PickFGButton, &QPushButton::clicked, this, [=]() { this->selectFGColor(); });
    connect(PickBGButton, &QPushButton::clicked, this, [=]() { this->selectBGColor(); });
    connect(SyntaxComponent, &QListWidget::currentItemChanged, this, [=](QListWidgetItem* current) { this->changeLine(current); });
    connect(TextExampleChoose, &QPushButton::clicked, this, [=]() { this->selectText(); });

    connect(EditorShortcutsEdit, SIGNAL(clicked()),
            this, SLOT(openEditorShortcutsDialog()));

    // Crete/Adjust additional complex widgets
    Analyzer = new toSyntaxAnalyzerNL(Example);
    Example->setReadOnly(true);

    Example->setText(QString::fromLatin1(
                                  "PROMPT Create procedure\n"
                                  "create procedure CheckObvious as\n"
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
    Example->SendScintilla(QsciScintilla::SCI_SETCARETLINEVISIBLEALWAYS, true);

    connect(CaretLineBool, SIGNAL(stateChanged(int)), this, SLOT(setCaretAlpha()));
    connect(CaretLineAlphaInt, SIGNAL(valueChanged(int)), this, SLOT(setCaretAlpha()));

#pragma message WARN("TODO: Error line style & Debug line style")
#if 0
    Example->setCurrentDebugLine(4);
    QMap<int, QString> Errors;
    Errors[2] = tr("Unknown variable");
    Example->setErrors(Errors);
#endif

    for (int idx = 0; idx < WordClassEnum.keyCount(); idx++)
    {
        QString colorName = WordClassEnum.key(idx);
        SyntaxComponent->addItem(colorName);
    }

    // load values from toConfigurationNewSingle into Widgets (if widget name == Config Option Name)
    toSettingTab::loadSettings(this);

    if (SyntaxHighlightingInt->currentText() == "QsciSQL")
        Example->setHighlighter(toSqlText::QsciSql);
    else
        Example->setHighlighter(toSqlText::Oracle);

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

    // 3rd TAB Indent
    if (SyntaxHighlightingInt->currentText() == "QsciSQL")
        IndentExample->setHighlighter(toSqlText::QsciSql);
    else
        IndentExample->setHighlighter(toSqlText::Oracle);
    QFile f(":/editor/complex.sql");
    f.open(QFile::ReadOnly);
    QString SqlExample = QString::fromUtf8(f.readAll());
    IndentExample->setText(indentInst.indent(SqlExample));

    connect(IndentLineWidthInt, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),  [=](int newValue) { indentInst.setProperty("IndentLineWidthInt", QVariant((int)newValue)); reIndent(); });
    connect(IndentWidthInt,     static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),  [=](int newValue) { indentInst.setProperty("IndentWidthInt",     QVariant((int)newValue)); reIndent(); });
    connect(ReUseNewlinesBool,  &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("ReUseNewlinesBool", QVariant((bool)newValue)); reIndent(); });

    connect(BreakOnSelectBool,  &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnSelectBool", QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnFromBool,    &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnFromBool",   QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnWhereBool,   &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnWhereBool",  QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnGroupBool,   &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnGroupBool",  QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnOrderBool,   &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnOrderBool",  QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnModelBool,   &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnModelBool",  QVariant((bool)newValue)); reIndent(); });
    connect(BreakOnPivotBool,   &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnPivotBool",  QVariant((bool)newValue)); reIndent(); });
    //connect(BreakOnLimitBool,  &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("BreakOnLimitBool", QVariant((bool)newValue)); reIndent(); });
    connect(WidthModeBool,     &QCheckBox::stateChanged, [=](int newValue) { indentInst.setProperty("WidthModeBool",     QVariant((bool)newValue));  reIndent(); });
}

void toSyntaxSetup::checkFixedWidth(const QFont &fnt)
{
    QFontMetrics mtr(fnt);
    if (mtr.boundingRect(QString::fromLatin1("iiiiiiii")).width() == mtr.boundingRect(QString::fromLatin1("MMMMMMMM")).width())
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

        FontSample->setFont(font);
        FontSample->update();

        // One font for all work classes
        for (int idx = 0; idx < WordClassEnum.keyCount(); idx++)
        {
            QString colorName = WordClassEnum.key(idx);
            toSyntaxAnalyzer::WordClassEnum key = (toSyntaxAnalyzer::WordClassEnum)WordClassEnum.value(idx);
            Styles[key].Font = font;
        }
        Example->lexer()->setFont(font, -1); // -1 => all styles
        Example->recolor(0, -1);
        Example->update();
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

void toSyntaxSetup::setCaretAlpha()
{
    // highlight caret line
    if (CaretLineBool->isChecked())
    {
        Example->setCaretLineVisible(true);
        // This is only required until transparency fixes in QScintilla go into stable release
        //QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, QsciScintilla::SC_ALPHA_NOALPHA);
        Example->SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, CaretLineAlphaInt->value());
    } else {
        Example->setCaretLineVisible(false);
    }
    CaretLineAlphaInt->setEnabled(CaretLineBool->isChecked());
}

int toSyntaxSetup::wordClass() const
{
    QString t = Current->text();
    int e = WordClassEnum.keyToValue(t.toStdString().c_str());
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
        palette.setColor(QPalette::Window, Styles.value(wc).BGColor);
        palette.setColor(QPalette::WindowText, Styles.value(wc).FGColor);
        //ExampleColor->setPalette(palette);
        //ExampleColor->setAutoFillBackground(true);
        //ExampleColor->setText("What ever text");

        palette = FGSample->palette();
        palette.setColor(QPalette::Window, Styles.value(wc).FGColor);
        FGSample->setPalette(palette);

        palette = BGSample->palette();
        palette.setColor(QPalette::Window, Styles.value(wc).BGColor);
        BGSample->setPalette(palette);

        palette = FontSample->palette();
        palette.setColor(QPalette::Window, Styles.value(wc).BGColor);
        palette.setColor(QPalette::WindowText, Styles.value(wc).FGColor);
        FontSample->setFont(Styles.value(wc).Font);
        FontSample->setPalette(palette);
        FontSample->setAutoFillBackground(true);
        FontSample->update();

        //Example->setSt
    }
}

void toSyntaxSetup::selectFGColor(void)
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
                palette.setColor(QPalette::WindowText, col);
                FontSample->setPalette(palette);
                FontSample->update();

                palette.setColor(QPalette::Window, col);
                FGSample->setPalette(palette);
                FGSample->update();

                Example->lexer()->setColor(col, coleng);
                Example->recolor(0, -1);
                Example->update();
            }
        }
    }
    TOCATCH
}

void toSyntaxSetup::selectBGColor(void)
{
    try
    {
        if (Current)
        {
            int coleng = wordClass();
            QColor col = QColorDialog::getColor(Styles.value(coleng).BGColor);
            if (col.isValid())
            {
                Styles[coleng].BGColor = col;

                QPalette palette = FontSample->palette();
                palette.setColor(QPalette::Window, col);
                FontSample->setPalette(palette);
                FontSample->update();

                palette.setColor(QPalette::Window, col);
                BGSample->setPalette(palette);
                BGSample->update();

                Example->lexer()->setPaper(col, coleng);
                Example->recolor(0, -1);
                Example->update();
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

    // for ShortcutModel see ShortcutModel::saveValues

    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Editor::EditStyleMap, QVariant::fromValue(Styles));
}

void toSyntaxSetup::reIndent()
{
	Utils::toBusy busy;
	QString oldSQL = IndentExample->text();
	QString newSQL = indentInst.indent(oldSQL);
	IndentExample->beginUndoAction();
	IndentExample->setText(newSQL);
	IndentExample->endUndoAction();
}
