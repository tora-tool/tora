
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toeditextensions.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tosqlparse.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qmenubar.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <QInputDialog>

#include <QPixmap>
#include <QMenu>
#include <QAction>

#include "icons/deindent.xpm"
#include "icons/indent.xpm"

// #define CONF_COMMA_BEFORE "CommaBefore"
// #define CONF_BLOCK_OPEN_LINE "BlockOpenLine"
// #define CONF_OPERATOR_SPACE "OperatorSpace"
// #define CONF_KEYWORD_UPPER "KeywordUpper"
// #define CONF_RIGHT_SEPARATOR "RightSeparator"
// #define CONF_END_BLOCK_NEWLINE "EndBlockNewline"
// #define CONF_COMMENT_COLUMN "CommentColumn"
// #define DEFAULT_COMMENT_COLUMN "60"


QMenu   *IncMenu           = NULL;
QAction *IncrementalSearch = NULL;
QAction *ReverseSearch     = NULL;

QMenu   *IndentMenu   = NULL;
QAction *IndentBlock  = NULL;
QAction *IndentBuffer = NULL;
QAction *ObsBlock     = NULL;
QAction *ObsBuffer    = NULL;

QMenu   *CaseMenu  = NULL;
QAction *UpperCase = NULL;
QAction *LowerCase = NULL;

QAction *Indent       = NULL;
QAction *Deindent     = NULL;
QAction *Quote        = NULL;
QAction *UnQuote      = NULL;
QAction *GotoLine     = NULL;
QAction *AutoComplete = NULL;

toEditExtensions::toEditExtensions() : toEditWidget::editHandler()
{
}

void toEditExtensions::receivedFocus(toEditWidget *widget)
{
    if (widget)
        Current = dynamic_cast<toMarkedText *>(widget);
    else
        Current = NULL;

    bool enable = Current && !Current->isReadOnly();

    IncMenu->setEnabled(enable);
    IndentMenu->setEnabled(enable);
    CaseMenu->setEnabled(enable);

    Indent->setEnabled(enable);
    Deindent->setEnabled(enable);
    Quote->setEnabled(enable);
    UnQuote->setEnabled(enable);
    GotoLine->setEnabled(enable);

    toHighlightedText * cur = dynamic_cast<toHighlightedText *>(widget);
    AutoComplete->setEnabled(cur);
}

void toEditExtensions::autoComplete()
{
    toHighlightedText *cur = dynamic_cast<toHighlightedText *>(Current);
    if (cur)
        cur->autoCompleteFromAPIs();
}

void toEditExtensions::lostFocus(toEditWidget *widget)
{
    if (widget)
    {
        toMarkedText *current = dynamic_cast<toMarkedText *>(widget);
        if (current && Current == current)
            receivedFocus(NULL);
    }

}

void toEditExtensions::editEnabled(bool enable)
{
    IncMenu->setEnabled(enable);
    IndentMenu->setEnabled(enable);
    CaseMenu->setEnabled(enable);

    Indent->setEnabled(enable);
    Deindent->setEnabled(enable);
    Quote->setEnabled(enable);
    UnQuote->setEnabled(enable);
    GotoLine->setEnabled(enable);

    AutoComplete->setEnabled(enable);
}

void toEditExtensions::gotoLine()
{
    if (Current)
    {
//         toEditExtensionGoto dialog(Current);
//         if (dialog.exec())
//             dialog.gotoLine();
        bool ok;
        int curline, curcol;
        Current->getCursorPosition(&curline, &curcol);
        int line = QInputDialog::getInteger(
                        Current,
                        tr("Go to line"),
                        tr("Line Number"),
                        curline,
                        1,
                        Current->lines(),
                        1,
                        &ok);
        if (ok)
            Current->setCursorPosition(line-1, 0);
    }
}

void toEditExtensions::intIndent(int delta)
{
    int line1, col1, line2, col2;

    if (Current)
    {
        if (!Current->getSelection(&line1, &col1, &line2, &col2))
        {
            Current->getCursorPosition(&line1, &col1);
            line2 = line1;
        }
        else if (col2 == 0)
        {
            line2--;
        }

        QString res;
        for (int i = line1;i <= line2;i++)
        {
            QString t = Current->text(i);
            int chars = 0;
            int level = toSQLParse::countIndent(t, chars);

            // trim newline, if present.  we have to do this because
            // qscintilla returns newlines, the old code didn't.
            t.remove('\n');

            res += toSQLParse::indentString(std::max(0, level + delta));
            if (i < line2)
                res += t.mid(chars) + "\n";
            else
                res += t.mid(chars);
        }

        Current->setSelection(line1, 0, line2, Current->text(line2).length() - 1);
        Current->insert(res, true);
    }
}

void toEditExtensions::deindentBlock(void)
{
    intIndent(-toSQLParse::getSetting().IndentLevel);
}

void toEditExtensions::indentBlock(void)
{
    intIndent(toSQLParse::getSetting().IndentLevel);
}

void toEditExtensions::autoIndentBlock(void)
{
    if (Current)
    {
        try
        {
            int line1, col1, line2, col2;
            if (Current->getSelection(&line1, &col1, &line2, &col2))
            {
                QString t = Current->text(line1).mid(0, col1);
                t += QString::fromLatin1("a");
                int chars = 0;
                QString ind = toSQLParse::indentString(toSQLParse::countIndent(t, chars));
                QString mrk = Current->selectedText();
                QString res;
                try
                {
                    res = toSQLParse::indent(ind + mrk, toCurrentConnection(Current));
                }
                catch (...)
                {
                    res = toSQLParse::indent(ind + mrk);
                }
                t = Current->text(line2);
                unsigned int l = res.length() - ind.length();
                if (col2 == int(t.length()) && t.length() > 0) // Strip last newline if on last col of line
                    l--;
                res = res.mid(ind.length(), l); // Strip indent.
                Current->insert(res, true);
            }
        }
        TOCATCH;
    }
}

void toEditExtensions::autoIndentBuffer(void)
{
    if (Current)
    {
        QString text = Current->text();
        int pos = 0;
        while (pos < text.length() && text.at(pos).isSpace())
        {
            pos++;
        }
        Current->selectAll();
        try
        {
            try
            {
                Current->insert(toSQLParse::indent(text.mid(pos), toCurrentConnection(Current)));
            }
            catch (...)
            {
                Current->insert(toSQLParse::indent(text.mid(pos)));
            }
        }
        TOCATCH;
    }
}


void toEditExtensions::quoteBlock(void)
{
    if (!Current)
        return;

    QString text = Current->selectedText();
    if (!text.isEmpty())
    {
        bool ends = text.right(1) == "\n";
        text = text.replace("\"", "\\\"");
        text = text.replace("\n", "\\n\"\n\"");

        if (ends)
            text = text.left(text.length() - 5) + "\"\n";
        else
            text += "\"";

        Current->insert("\"" + text, true);
    }
}


void toEditExtensions::unquoteBlock(void)
{
    if (!Current)
        return;

    QString text = Current->selectedText();
    if (!text.isEmpty())
    {
        // this is to replace quoting at the end of the line
        text = text.replace("\\n\"\n", "\n");
        // er... text could start with whitespace. a regex would be good here.
        text = text.replace("\n\"", "\n");

        text = text.replace("\\\"", "\"");

        if (text.left(1) == "\"")
            text = text.right(text.length() - 1);

        if (text.right(1) == "\"")
            text = text.left(text.length() - 1);
        else if (text.right(1) == "\"\n")
            text = text.left(text.length() - 2) + "\n";

        Current->insert(text, true);
    }
}


void toEditExtensions::upperCase(void)
{
    if (Current)
    {
        QString text = Current->selectedText().toUpper();
        if (!text.isEmpty())
            Current->insert(text, true);
    }
}

void toEditExtensions::lowerCase(void)
{
    if (Current)
    {
        QString text = Current->selectedText().toLower();
        if (!text.isEmpty())
            Current->insert(text, true);
    }
}

static int CountLine(const QString &str)
{
    int found = str.lastIndexOf(QString::fromLatin1("\n"));
    if (found < 0)
        return str.length();
    else
        return str.length() - found + 1;
}

static void ObfuscateStat(toSQLParse::statement &stat, QString &ret)
{
    if (ret.length() > 0 &&
            stat.String.length() > 0 &&
            toIsIdent(ret.at(ret.length() - 1)) &&
            toIsIdent(stat.String.at(0)))
    {
        if (CountLine(ret) < 60)
            ret += QString::fromLatin1(" ");
        else
            ret += QString::fromLatin1("\n");
    }
    ret += stat.String;
    if (!stat.Comment.isEmpty())
    {
        ret += stat.Comment;
        ret += QString::fromLatin1("\n");
    }
    for (std::list<toSQLParse::statement>::iterator i = stat.subTokens().begin();
            i != stat.subTokens().end();
            i++)
    {
        ObfuscateStat(*i, ret);
    }
}

void toEditExtensions::obfuscateBlock(void)
{
    if (Current)
    {
        QString str = Current->selectedText();
        if (!str.isEmpty())
        {
            toSQLParse::statement stat;
            stat.subTokens() = toSQLParse::parse(str);
            QString res;
            ObfuscateStat(stat, res);
            Current->insert(res, true);
        }
    }
}

void toEditExtensions::obfuscateBuffer(void)
{
    if (Current)
    {
        QString str = Current->text();
        if (!str.isEmpty())
        {
            toSQLParse::statement stat;
            stat.subTokens() = toSQLParse::parse(str);
            Current->selectAll();
            QString res;
            ObfuscateStat(stat, res);
            Current->insert(res);
        }
    }
}

static toEditExtensions EditExtensions;


toEditExtensionSetup::toEditExtensionSetup(
    toEditExtensionTool *tool,
    QWidget *parent,
    const char *name) : QWidget(parent),
        toSettingTab("editextension.html"),
        Tool(tool)
{
    setupUi(this);

    Current = toSQLParse::getSetting();
    Started = false;
    CommaBefore->setChecked(Current.CommaBefore);
    BlockOpenLine->setChecked(Current.BlockOpenLine);
    OperatorSpace->setChecked(Current.OperatorSpace);
    KeywordUpper->setChecked(Current.KeywordUpper);
    RightSeparator->setChecked(Current.RightSeparator);
    EndBlockNewline->setChecked(Current.EndBlockNewline);
    CommentColumn->setValue(Current.CommentColumn);
    AutoIndent->setChecked(toConfigurationSingle::Instance().autoIndentRo());
    DragDrop->setChecked(toConfigurationSingle::Instance().editDragDrop());
    Ok = false;
    try
    {
        Example->setAnalyzer(toMainWidget()->currentConnection().analyzer());
    }
    TOCATCH;

    try
    {
#ifdef TO_NO_ORACLE
        Example->setText(toSQLParse::indent(
                             "CREATE PROCEDURE COUNT_EMPS_IN_DEPTS (OUT V_TOTAL INT)\n"
                             "BEGIN\n"
                             "  DECLARE V_DEPTNO INT DEFAULT 10;\n"
                             "  DECLARE V_COUNT INT DEFAULT 0;\n"
                             "  SET V_TOTAL = 0;\n"
                             "  WHILE V_DEPTNO < 100 DO\n"
                             "    SELECT COUNT(*)\n"
                             "      INTO V_COUNT\n"
                             "      FROM TEST.EMP\n"
                             "      WHERE DEPTNO = V_DEPTNO;\n"
                             "    SET V_TOTAL = V_TOTAL + V_COUNT;\n"
                             "    SET V_DEPTNO = V_DEPTNO + 10;\n"
                             "  END WHILE;\n"
                             "END",
                             Example->analyzer()));
#else
        Example->setText(toSQLParse::indent(
                             "CREATE OR REPLACE procedure spTuxGetAccData (oRet OUT  NUMBER)\n"
                             "AS\n"
                             "  vYear  CHAR(4);\n"
                             "BEGIN\n"
                             "select a.TskCod TskCod, -- A Comment\n"
                             "       count(1) Tot\n"
                             "  from (select * from EssTsk where PrsID >= '1940');\n"
                             "having count(a.TspActOprID) > 0;\n"
                             "    DECLARE\n"
                             "      oTrdStt NUMBER;\n"
                             "    BEGIN\n"
                             "      oTrdStt := 0;\n"
                             "    END;\n"
                             "    EXCEPTION\n"
                             "        WHEN VALUE_ERROR THEN\n"
                             "     oRet := 3;\n"
                             "END;",
                             Example->analyzer()));
#endif

    }
    TOCATCH;
    Started = true;
}

toEditExtensionSetup::~toEditExtensionSetup()
{
    if (!Ok)
        toSQLParse::setSetting(Current);
}


void toEditExtensionSetup::saveCurrent(void)
{
    Current.ExpandSpaces = toConfigurationSingle::Instance().tabSpaces();
    Current.CommaBefore = CommaBefore->isChecked();
    Current.BlockOpenLine = BlockOpenLine->isChecked();
    Current.OperatorSpace = OperatorSpace->isChecked();
    Current.KeywordUpper = KeywordUpper->isChecked();
    Current.RightSeparator = RightSeparator->isChecked();
    Current.EndBlockNewline = EndBlockNewline->isChecked();
    Current.IndentLevel = toConfigurationSingle::Instance().tabStop();
    Current.CommentColumn = CommentColumn->value();
    toSQLParse::setSetting(Current);
}


void toEditExtensionSetup::changed(void)
{
    if (Started)
    {
        saveCurrent();
        try
        {
            Example->setText(toSQLParse::indent(Example->text(), Example->analyzer()));
        }
        TOCATCH;
    }
}


void toEditExtensions::searchForward(void)
{
    if (Current)
        Current->incrementalSearch(true);
}

void toEditExtensions::searchBackward(void)
{
    if (Current)
        Current->incrementalSearch(false);
}

class toEditExtensionTool : public toTool
{

public:
    toEditExtensionTool() : toTool(910, "Editor Extensions")
    {
        toSQLParse::settings cur;
        cur.ExpandSpaces = toConfigurationSingle::Instance().tabSpaces();
        cur.CommaBefore = toConfigurationSingle::Instance().commaBefore();
        cur.BlockOpenLine = toConfigurationSingle::Instance().blockOpenLine();
        cur.OperatorSpace = toConfigurationSingle::Instance().operatorSpace();
        cur.KeywordUpper = toConfigurationSingle::Instance().keywordUpper();
        cur.RightSeparator = toConfigurationSingle::Instance().rightSeparator();
        cur.EndBlockNewline = toConfigurationSingle::Instance().endBlockNewline();
        cur.IndentLevel = toConfigurationSingle::Instance().tabStop();
        cur.CommentColumn = toConfigurationSingle::Instance().commentColumn();
        toSQLParse::setSetting(cur);
    }

    virtual QWidget *toolWindow(QWidget *, toConnection &)
    {
        return NULL; // Has no tool window
    }

    virtual void closeWindow(toConnection &connection) {};

    virtual void customSetup(void)
    {
        QMenu *edit = toMainWidget()->getEditMenu();

        edit->addSeparator();

        IncMenu = edit->addMenu(
                      qApp->translate("toEditExtensionTool", "Incremental Search"));

        IncrementalSearch = IncMenu->addAction(qApp->translate("toEditExtensionTool",
                                               "Forward"),
                                               &EditExtensions,
                                               SLOT(searchForward()));
        IncrementalSearch->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);

        ReverseSearch = IncMenu->addAction(qApp->translate("toEditExtensionTool",
                                           "Backward"),
                                           &EditExtensions,
                                           SLOT(searchBackward()));
        ReverseSearch->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R);

        // ------------------------------ indentation menu

        IndentMenu = edit->addMenu(
                         qApp->translate("toEditExtensionTool", "Auto Indent"));

        IndentBlock = IndentMenu->addAction(qApp->translate(
                                                "toEditExtensionTool",
                                                "Selection"),
                                            &EditExtensions,
                                            SLOT(autoIndentBlock()));
        IndentBlock->setShortcut(Qt::CTRL + + Qt::ALT + Qt::Key_I);

        IndentBuffer = IndentMenu->addAction(qApp->translate(
                                                 "toEditExtensionTool",
                                                 "Editor"),
                                             &EditExtensions,
                                             SLOT(autoIndentBuffer()));
        IndentBuffer->setShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_I);

        IndentMenu->addSeparator();

        ObsBlock = IndentMenu->addAction(qApp->translate(
                                             "toEditExtensionTool",
                                             "Obfuscate Selection"),
                                         &EditExtensions,
                                         SLOT(obfuscateBlock()));

        ObsBuffer = IndentMenu->addAction(qApp->translate("toEditExtensionTool",
                                          "Obfuscate Editor"),
                                          &EditExtensions,
                                          SLOT(obfuscateBuffer()));

        // ------------------------------ case menu

        CaseMenu = edit->addMenu(
                       qApp->translate("toEditExtensionTool", "Modify Case"));

        UpperCase = CaseMenu->addAction(qApp->translate("toEditExtensionTool",
                                        "Upper"),
                                        &EditExtensions,
                                        SLOT(upperCase()));
        UpperCase->setShortcut(Qt::CTRL + Qt::Key_U);

        LowerCase = CaseMenu->addAction(qApp->translate("toEditExtensionTool",
                                        "Lower"),
                                        &EditExtensions,
                                        SLOT(lowerCase()));
        LowerCase->setShortcut(Qt::CTRL + Qt::Key_L);

        // ------------------------------ etc

        Indent = edit->addAction(
                     QIcon(QPixmap(const_cast<const char**>(indent_xpm))),
                     qApp->translate("toEditExtensionTool", "Indent Block"),
                     &EditExtensions,
                     SLOT(indentBlock()));
        Indent->setShortcut(Qt::ALT + Qt::Key_Right);

        Deindent = edit->addAction(
                       QIcon(QPixmap(const_cast<const char**>(deindent_xpm))),
                       qApp->translate("toEditExtensionTool", "De-indent Block"),
                       &EditExtensions,
                       SLOT(deindentBlock()));
        Deindent->setShortcut(Qt::ALT + Qt::Key_Left);

        Quote = edit->addAction(qApp->translate("toEditExtensionTool",
                                                "Quote Selection"),
                                &EditExtensions,
                                SLOT(quoteBlock()));

        UnQuote = edit->addAction(qApp->translate("toEditExtensionTool",
                                  "UnQuote Selection"),
                                  &EditExtensions,
                                  SLOT(unquoteBlock()));

        GotoLine = edit->addAction(qApp->translate("toEditExtensionTool",
                                   "Goto Line"),
                                   &EditExtensions,
                                   SLOT(gotoLine()));
        GotoLine->setShortcut(Qt::CTRL + Qt::Key_G);

        AutoComplete = edit->addAction(
                           qApp->translate("toEditExtensionTool",
                                           "Complete"),
                           &EditExtensions,
                           SLOT(autoComplete()));
        AutoComplete->setShortcut(Qt::CTRL + Qt::Key_Space);

        // add buttons to main window
        // disabled due the problems in the state of toolbars
//         toMainWidget()->addButtonApplication(Deindent);
//         toMainWidget()->addButtonApplication(Indent);

        EditExtensions.receivedFocus(NULL);

        connect(toMainWidget(),
                SIGNAL(editEnabled(bool)),
                &EditExtensions,
                SLOT(editEnabled(bool)));
    }

    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toEditExtensionSetup(this, parent);
    }
};

void toEditExtensionSetup::saveSetting(void)
{
    Ok = true;
    toConfigurationSingle::Instance().setCommaBefore(CommaBefore->isChecked());
    toConfigurationSingle::Instance().setBlockOpenLine(BlockOpenLine->isChecked());
    toConfigurationSingle::Instance().setOperatorSpace(OperatorSpace->isChecked());
    toConfigurationSingle::Instance().setKeywordUpper(KeywordUpper->isChecked());
    toConfigurationSingle::Instance().setRightSeparator(RightSeparator->isChecked());
    toConfigurationSingle::Instance().setEndBlockNewline(EndBlockNewline->isChecked());
    toConfigurationSingle::Instance().setCommentColumn(CommentColumn->value());
    toConfigurationSingle::Instance().setAutoIndentRo(AutoIndent->isChecked());
    toConfigurationSingle::Instance().setEditDragDrop(DragDrop->isChecked());
    saveCurrent();
}

static toEditExtensionTool EditExtensionTool;

// toEditExtensionGoto::toEditExtensionGoto(toMarkedText *editor)
//         : QDialog(editor),
//         Editor(editor)
// {
// 
//     setupUi(this);
// 
//     toHelp::connectDialog(this);
//     Line->setMaximum(Editor->lines());
//     Line->setMinimum(1);
//     {
//         int curline, curcol;
//         Editor->getCursorPosition(&curline, &curcol);
//         Line->setValue(curline);
//     }
// }

// void toEditExtensionGoto::gotoLine()
// {
//     Editor->setCursorPosition(Line->value() - 1, 0);
// }
