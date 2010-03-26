
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

#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "utils.h"
#include "toconnection.h"
#include "totreewidget.h"
#include "toplsqltext.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "toresultview.h"

#define PLSQL_ERROR "ERROR"
#define PLSQL_WARNING "WARNING"
#define PLSQL_STATIC "STATIC"
#define PLSQL_RESULT ""


static struct TypeMapType
{
    const char *Type;
    const char *Description;
    const char *Icon;
    bool WantName;
    bool Declaration;
}
TypeMap[] = { { "FUNCTION", "Fc", ":/icons/function.png", true , true },    // Must be first in list
    { "PROCEDURE", "Pr", ":/icons/procedure.png", true , true },
    { "PACKAGE", "Pkg", ":/icons/package.png", true , true },
    { "DECLARE", "Anon", NULL, false, true },
    { "TYPE", "Type", ":/icons/type.png", true , false},
    { "CURSOR", "Cursor", NULL, true , false},
    { "IF", "Condition", NULL, false, false},
    { "LOOP", "Loop", NULL, false, false},
    { "WHILE", "Loop", NULL, false, false},
    { "FOR", "Loop", NULL, false, false},
    { NULL, NULL, false, false}
};


static toTreeWidgetItem *toLastItem(toTreeWidgetItem *parent)
{
    toTreeWidgetItem *lastItem = NULL;
    for (toTreeWidgetItem *item = parent->firstChild(); item; item = item->nextSibling())
        lastItem = item;
    return lastItem;
}

// TODO/FIXME: well, icon handling and the "tree item" stuff
// is really ugly. It should be simplified one day (model-view) to
// allow searching, sorting, etc. Of course it should force me to
// understand parser output...
class toContentsItem : public toTreeWidgetItem
{
public:
    int Line;

    toContentsItem(toTreeWidgetItem *parent, const QString & icon, const QString &name, int line)
            : toTreeWidgetItem(parent, toLastItem(parent), name)
    {
        setup(icon, line);
    }
    toContentsItem(toTreeWidget *parent, const QString & icon, const QString &name, const QString &id, int line)
            : toTreeWidgetItem(parent, name, id)
    {
        setup(icon, line);
    }
    
private:
    void setup(const QString & icon, int line)
    {
        Line = line;
        if (!icon.isNull())
           setIcon(0, QIcon(icon));
        int space = text(0).indexOf(" ");
        if (space != -1)
           setText(0, text(0).mid(space));
    }

};

static bool FindKeyword(toSQLParse::statement &statements, bool onlyNames,
                        bool &declaration, int &line, QString &name,
                        QString & icon, QString & itemtype)
{
    if (statements.Type == toSQLParse::statement::Keyword ||
            statements.Type == toSQLParse::statement::Token)
    {
        line = statements.Line;
        if (name.isEmpty())
        {
            name = statements.String.toUpper();

            int j;
            for (j = 0; TypeMap[j].Type && TypeMap[j].Type != name; j++)
                ;
            if (TypeMap[j].Type)
            {
                name = TypeMap[j].Description;
                icon = TypeMap[j].Icon;
                itemtype = TypeMap[j].Type;
            }
            else
                name = "Anonymous";

            declaration = TypeMap[j].Declaration;

            if (onlyNames && !TypeMap[j].WantName)
            {
                name = QString::null;
                return true;
            }

            return !TypeMap[j].WantName;
        }
        else if (statements.String.toUpper() != "BODY")
        {
            name += " " + statements.String;
            return true;
        }
        else
            name += " body";
    }
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin(); i != statements.subTokens().end(); i++)
    {
        bool ret = FindKeyword(*i, onlyNames, declaration, line, name, icon, itemtype);
        if (ret)
            return ret;
    }
    return false;
}

/* Parameter t: toPLSQLText::Production - for production compiling (default)
                toPLSQLText::Warning - to compile with warnings enabled
*/
bool toPLSQLText::compile(CompilationType t)
{
    QString str = text();
    bool ret = true; // indicates if compilation action was successfull
    if (str.isEmpty())
        return true;

    bool body = false;

    toSQLParse::stringTokenizer tokens(str);

    QString token = tokens.getToken();
    if (token.toUpper() == "CREATE")
    {
        token = tokens.getToken();
        if (token.toUpper() == "OR")
        {
            token = tokens.getToken();
            if (token.toUpper() == "REPLACE")
                token = tokens.getToken();
        }
    }

    QString type = token.toUpper();
    if (type != QString::fromLatin1("PROCEDURE") &&
            type != QString::fromLatin1("TYPE") &&
            type != QString::fromLatin1("FUNCTION") &&
            type != QString::fromLatin1("PACKAGE"))
    {
        toStatusMessage(tr("Invalid start of code"));
        return false;
    }

    token = tokens.getToken();
    if (token.toUpper() == "BODY")
    {
        body = true;
        token = tokens.getToken();
    }
    QString object = token;
    QString schema = Schema;

    int offset = tokens.offset();
    token = tokens.getToken();
    if (token == ".")
    {
        schema = object;
        object = tokens.getToken();
        offset = tokens.offset();
    }

    QString sql = QString::fromLatin1("CREATE OR REPLACE ");
    sql.append(type);
    if (body)
        sql.append(QString::fromLatin1(" BODY "));
    else
        sql.append(QString::fromLatin1(" "));
    sql.append(schema);
    sql.append(QString::fromLatin1("."));
    sql.append(object);
    sql.append(QString::fromLatin1(" "));
    sql.append(str.mid(offset));

    // Remove empty space and the slash at the end of statement.
    // Note: slash is valid in oracle sqlplus only
    int i = sql.length() - 1;
    while ((i > 0) &&
           ((sql.at(i) == ' ') ||
            (sql.at(i) == '\r') ||
            (sql.at(i) == '\n') ||
            (sql.at(i) == '\t') ||
            (sql.at(i) == '/')))
      i--;
    sql.truncate(i + 1);

    compilation_error = "";
    try
    {
        toQList nopar;
        Schema = schema.toUpper();
        Type = type.toUpper();
        if (body)
            Type += QString::fromLatin1(" BODY");
        toQuery q(toMainWidget()->currentConnection(), sql, nopar);
        setModified(false);
    }
    catch (const QString &exc)
    {
        // save compilation execution error to be added to error list later by readErrors()
        compilation_error = exc;
//         toStatusMessage(exc);
        ret = false;
    }

    try
    {
        if (t == toPLSQLText::Warning) // TODO: TS 2009-12-21 is it actually compiling if
                                       // button "compile (without warnings) was pressed?
        {
            QString alter("ALTER %1 \"%2\".\"%3\" COMPILE ");
            if (body)
                alter += "BODY ";
            alter += "PLSQL_WARNINGS='ENABLE:ALL'";
            toQuery w(toMainWidget()->currentConnection(),
                      alter.arg(type).arg(schema).arg(object.toUpper()),
                      toQList());
        }
    }
    catch (const QString &exc)
    {
//         toStatusMessage(exc);
        ret = false;
    }

    readErrors(toMainWidget()->currentConnection());
    emit contentChanged();
    parent_widget->resizeResults();

    return ret;
}

static toSQL SQLReadSource("toPLSQLEditor:ReadSource",
                           "SELECT Text FROM SYS.All_Source\n"
                           " WHERE OWNER = :f1<char[101]>\n"
                           "   AND NAME = :f2<char[101]>\n"
                           "   AND TYPE = :f3<char[101]>\n"
                           " ORDER BY Type,Line",
                           "Read sourcecode for object");

static toSQL SQLReadErrors("toPLSQLEditor:ReadErrors",
                           "SELECT attribute, Line,Text FROM SYS.All_Errors\n"
                           " WHERE OWNER = :f1<char[101]>\n"
                           "   AND NAME = :f2<char[101]>\n"
                           "   AND TYPE = :f3<char[101]>\n"
                           " ORDER BY Attribute, Type, Line",
                           "Get lines with errors in object (Observe first line 0)",
                           "1000",
                           "Oracle");
static toSQL SQLReadErrors9("toPLSQLEditor:ReadErrors",
                            "SELECT 'ERROR', Line,Text FROM SYS.All_Errors\n"
                            " WHERE OWNER = :f1<char[101]>\n"
                            "   AND NAME = :f2<char[101]>\n"
                            "   AND TYPE = :f3<char[101]>\n"
                            " ORDER BY Type, Line",
                            "",
                            "0900");

int toPLSQLText::ID = 0;

toPLSQLText::toPLSQLText(QWidget *parent)
        : toHighlightedText(parent, QString::number(++ID).toLatin1())
{
    parent_widget = (toPLSQLWidget*)parent;
}

void toPLSQLText::clear(void)
{
    setData(QString::null, QString::null, QString::null);
    toHighlightedText::clear();
}

/* This function is called after compiling the code.
   It's purpose is to read list of warnings/errors.
*/
bool toPLSQLText::readErrors(toConnection &conn)
{
    int errorCount = 0, warningCount = 0;

    try
    {
        toQuery errors(conn, SQLReadErrors, Schema, Object, Type);
        QMultiMap<int, QString> Errors;
        QMultiMap<int, QString> Warnings;
        QMultiMap<int, QString> Static; // dummy empty array used to clear static messages

        int line;
        QString errType;

        // add compilation execution error (if there was one)
        if (!compilation_error.isEmpty())
        {
            Errors.insert(1, compilation_error);
            errorCount++;
        }

        while (!errors.eof())
        {
            errType = errors.readValue(); // "ERROR"/"WARNING" etc.
            line = errors.readValue().toInt();
            if (errType == PLSQL_ERROR)
            {
                Errors.insert(line, errors.readValue());
                errorCount++;
            }
            else // "WARNING"
            {
                Warnings.insert(line, errors.readValue());
                warningCount++;
            }
        }
        setErrors(Errors);

        emit errorsChanged(PLSQL_ERROR, Errors, true);
        emit errorsChanged(PLSQL_WARNING, Warnings);
        emit errorsChanged(PLSQL_STATIC, Static); // clear all static check messages ir result pane
        setErrors(Static, false); // remove static error markers in code

        return true;
    }
    TOCATCH
    return false;
}

bool toPLSQLText::readData(toConnection &conn/*, toTreeWidget *Stack*/)
{
    try
    {
        toQuery lines(conn, SQLReadSource, Schema, Object, Type);

        QString str = "CREATE OR REPLACE ";
        while (!lines.eof())
            str += lines.readValue();
        str += "\n/";
        setText(str);
        setModified(false);
        setCurrent( -1);

        emit contentChanged();

        if (str.isEmpty())
            return false;
        else
            return readErrors(conn);
    }
    TOCATCH
    return false;
}

void toPLSQLWidget::updateArguments(toSQLParse::statement &statements, toTreeWidgetItem *parent)
{
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin(); i != statements.subTokens().end(); i++)
    {
        if ((*i).Type == toSQLParse::statement::List)
        {
            bool first = true;
            for (std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin(); j != (*i).subTokens().end(); j++)
            {
                if ((*j).String == ",")
                    first = true;
                else if (first)
                {
                    // arguments
                    new toContentsItem(parent, ":/icons/type.png", (*j).String, (*j).Line);
                    first = false;
                }
            }
        }
    }
}

void toPLSQLWidget::updateContent(toSQLParse::statement &statements,
                                  toTreeWidgetItem *parent,
                                  const QString &id)
{
    toTreeWidgetItem *item = NULL;
    int line;
    QString name;
    bool declaration;
    QString icon;
    QString itemtype;
    if (!FindKeyword(statements, statements.Type == toSQLParse::statement::Statement, declaration, line, name, icon, itemtype) || name.isNull())
        return ;

    if (parent)
        item = new toContentsItem(parent, icon, name, line);
    else
    {
        for (item = m_contents->firstChild(); item; item = item->nextSibling())
            if (item->text(0) == name && item->text(1) == id)
            {
                item->setText(2, QString::null);
                break;
            }
        if (!item) {
            item = new toContentsItem(m_contents, icon, "Item Unit Content", id, line);
        }
        else
        {
            while (item->firstChild())
                delete item->firstChild();
            toContentsItem *ci = dynamic_cast<toContentsItem *>(item);
            if (ci)
                ci->Line = line;
        }
        item->setOpen(true);
    }

    std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();
    if (statements.Type == toSQLParse::statement::Block)
    {
        if (i != statements.subTokens().end())
        {
            updateArguments(*i, item);
            i++;
        }
    }
    else
    {
        updateArguments(statements, item);
        if (i != statements.subTokens().end())
            i++;
    }
    while (i != statements.subTokens().end())
    {
        if ((*i).Type == toSQLParse::statement::Block || (*i).Type == toSQLParse::statement::Statement)
        {
            if (declaration)
            {
                std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin();
                if (j != (*i).subTokens().end())
                {
                    if ((*j).String.toUpper() == "BEGIN")
                        declaration = false;
                    else if ((*j).Type == toSQLParse::statement::Token && (*j).String.toUpper() != "END")
                        // variables
                        new toContentsItem(item, ":/icons/type.png", (*j).String, (*j).Line);
                }
            }
            updateContent(*i, item);
        }
        i++;
    }
    if (!parent && !item->firstChild())
        delete item;
}

void toPLSQLWidget::updateContent(toPLSQLText *ed)
{
    toSQLParse::editorTokenizer tokenizer(ed);
    std::list<toSQLParse::statement> statements = toSQLParse::parse(tokenizer);

    m_contents->clear();

    for (std::list<toSQLParse::statement>::iterator i = statements.begin(); i != statements.end(); i++)
        updateContent(*i, NULL, ed->objectName());
}

void toPLSQLText::setData(const QString &schema, const QString &type, const QString &object)
{
    Schema = schema;
    Type = type;
    Object = object;

    update();
}

toPLSQLWidget::toPLSQLWidget(QWidget * parent)
        : QWidget(parent)
{
    m_contents = new toTreeWidget(this);
    m_contents->addColumn(tr("Contents"));
    m_contents->setRootIsDecorated(true);
    m_contents->setSorting( -1);
    m_contents->setTreeStepSize(10);
    m_contents->setSelectionMode(toTreeWidget::Single);
    m_contents->setResizeMode(toTreeWidget::AllColumns);
    connect(m_contents, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeContent(toTreeWidgetItem *)));

    m_editor = new toPLSQLText(this);

    m_result = new QTreeWidget(this);
    m_result->setColumnCount(2);
    m_result->setAlternatingRowColors(true);
    m_result->setAllColumnsShowFocus(true);

    m_result->setHeaderLabels(QStringList() << tr("Message") << tr("Line"));
    m_errItem = m_warnItem = m_staticItem = NULL;

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->insertWidget(0, m_editor);
    m_splitter->insertWidget(1, m_result);

    errorCount = warningCount = staticCount = 0;
    resizeResults();

    m_contentSplitter = new QSplitter(Qt::Horizontal, this);
    m_contentSplitter->insertWidget(0, m_contents);
    m_contentSplitter->insertWidget(1, m_splitter);

    if (layout() == 0)
        setLayout(new QVBoxLayout);
    layout()->addWidget(m_contentSplitter);

    connect(m_editor,
            SIGNAL(errorsChanged(const QString &, const QMultiMap<int, QString>&, const bool)),
            this,
            SLOT(applyResult(const QString &, const QMultiMap<int, QString>&, const bool)));
    connect(m_result,
            SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this,
            SLOT(goToError(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(m_editor, SIGNAL(contentChanged()),
            this, SLOT(updateContent()));

    QSettings s;
    s.beginGroup("toPLSQLEditor");
    m_splitter->restoreState(s.value("splitterWidget").toByteArray());
    m_contentSplitter->restoreState(s.value("contentSplitter").toByteArray());
    s.endGroup();
}

toPLSQLWidget::~toPLSQLWidget()
{
    // closeEvent is not called at all
    QSettings s;
    s.beginGroup("toPLSQLEditor");
    s.setValue("splitterWidget", m_splitter->saveState());
    s.setValue("contentSplitter", m_contentSplitter->saveState());
    s.endGroup();
}

/* If NULL is given as type - removes all error, warning and static check messages
   from m_result list. If argument type has a value - only messages of given type
   are removed.
*/
void toPLSQLWidget::cleanupResults(const QString & type)
{
    if ((m_errItem != NULL) && (type == PLSQL_ERROR || type == NULL))
    {
        foreach (QTreeWidgetItem * c, m_errItem->takeChildren())
        delete c;
        delete m_errItem;
        m_errItem = NULL;
    }

    if ((m_warnItem != NULL) && (type == PLSQL_WARNING || type == NULL))
    {
        foreach (QTreeWidgetItem * c, m_warnItem->takeChildren())
        delete c;
        delete m_warnItem;
        m_warnItem = NULL;
    }

    if ((m_staticItem != NULL) && (type == PLSQL_STATIC || type == NULL))
    {
        foreach (QTreeWidgetItem * c, m_staticItem->takeChildren())
        delete c;
        delete m_staticItem;
        m_staticItem = NULL;
    }
} // cleanupResults

/* Resize result pane depending on number of errors, warnings etc. That is if there is anything
   to show there - restore it's size to "visible". If there are no messages to show - collapse it.
*/
int resultSize = 1;
void toPLSQLWidget::resizeResults(void)
{
    QList<int> widget_sizes = m_splitter->sizes();

    if (errorCount == 0 && warningCount == 0 && staticCount == 0 && widget_sizes[1] > 0)
    {
        resultSize = widget_sizes[1];
        widget_sizes[1] = 0;
        m_splitter->setSizes(widget_sizes);
    }
    else if ((errorCount > 0 || warningCount > 0 || staticCount > 0) && (widget_sizes[1] == 0))
    {
        widget_sizes[1] = resultSize;
        m_splitter->setSizes(widget_sizes);
    }
} // resizeRezults

void toPLSQLWidget::setCount(const QString & type, const int count)
{
    if (type == PLSQL_ERROR) {
        errorCount = count;
    } else if (type == PLSQL_WARNING) {
        warningCount = count;
    } else if (type == PLSQL_STATIC) {
        staticCount = count;
    }
} // setCount

/* Applies list of errors, warnings or static check observations to corresponding node
   in results list. This will usually be called after some parts of this list have
   changed: after compiling (twice: once for errors and once for warnings) or after
   runing statick check (once: for static observations only).
   Parameters:
     type - PLSQL_ERROR, PLSQL_WARNING or PLSQL_STATIC
     values - multimap containing list of errors or warnings with
              line number and description
     cleanup - cleanup result pane of any messages (including root elements):
               if true - clears all error, warning and static test errors
               if false - only messages of given type are cleared
*/
void toPLSQLWidget::applyResult(const QString & type,
                                const QMultiMap<int, QString> & values,
                                const bool cleanup)
{
    QTreeWidgetItem * item = 0; // abstract item will be used as list of errors or
                            // warnings or static check observations etc.
    int count = 0;

    if (cleanup) cleanupResults();
    else cleanupResults(type);

    if (values.empty()) 
    {
        setCount(type, 0);
        return;
    }

    if (type == PLSQL_ERROR) {
        m_errItem = new QTreeWidgetItem(m_result, QStringList() << tr("Errors") << "");
        item = m_errItem;
    } else if (type == PLSQL_WARNING) {
        m_warnItem = new QTreeWidgetItem(m_result, QStringList() << tr("Warnings") << "");
        item = m_warnItem;
    } else if (type == PLSQL_STATIC) {
        m_staticItem = new QTreeWidgetItem(m_result, QStringList() << tr("Static check") << "");
        item = m_staticItem;
    } else
        qDebug() << "Unknown type " << type << " in toPLSQLWidget::applyResult";

    // add new items to the list
    QMap<int, QString>::const_iterator i = values.constBegin();
    while (i != values.constEnd())
    {
        QStringList l(i.value());
        l.append(QString().setNum(i.key()));
        item->addChild(new QTreeWidgetItem(item, l));
        ++i;
        count++;
    }

    setCount(type, count);

    m_result->expandItem(item);
    m_result->resizeColumnToContents(0);
} // applyResult

/* Called whenever user selects an error, warning or static observation in results list.
   It should place cursor at he position of corresponding error, warning or static
   observation position.
   Parameters:
     current - newly selected item
     <second parameter> - previously selected item - not used
*/
void toPLSQLWidget::goToError(QTreeWidgetItem * current, QTreeWidgetItem *)
{
    // do not try to move cursor when "parent" item in result list is selected
    if (current != m_errItem && current != m_warnItem && current != m_staticItem)
    {
        m_editor->setCursorPosition(current->text(1).toInt() - 1, 0);
        m_editor->setFocus(Qt::OtherFocusReason);
    }
} // goToError

void toPLSQLWidget::changeContent(toTreeWidgetItem *ci)
{
    toContentsItem *item = dynamic_cast<toContentsItem *>(ci);
    if (item)
    {
        while (ci->parent())
            ci = ci->parent();
    }
    m_editor->setCursorPosition(item->Line, 0);
    m_editor->setFocus(Qt::OtherFocusReason);
#ifdef AUTOEXPAND
    else
        ci->setOpen(true);
#endif
}

/* All plsql edit widgets are inherited from toMarkedText which does implement
   save feature. We need to overload it here in order to handle saving of
   package specification and body into one file.
*/
bool toPLSQLText::editSave(bool askfile)
{

    // Only packages should be handled differently, for all other types
    // call original version of save function
    // TODO: types as well?
    if ((Type != "PACKAGE") && (Type != "PACKAGE BODY"))
        return toMarkedText::editSave(askfile);

    QFileInfo file(filename());
    QString fn = filename();

    if (askfile || fn.isEmpty())
    {
        // get list of default extensions
        QString t;
        t = GetExtensions();
        // append extensions for package spec+body
        // TODO: make this extension configurable
        t.append(";;Spec & Body (*.pls)");
        fn = toSaveFilename(file.dir().path(), t, this);
    }

    if (!fn.isEmpty())
    {
        if (fn.endsWith(".pls"))
        {
            // if .pls was chosen - both spec and body must be saved in one file

            // find another part of package
            toPLSQLText * other_part = (Editor->getAnotherPart(Schema, Object, Type));

            if (other_part == NULL)
            {
                // other part of package could not be found. Panic!
                printf("Other part of package is unknown!!!\n");
                return false;
            }

            QString create_statement("create or replace ");
            // save specification first
            if (Type == "PACKAGE")
            {
                if (!toWriteFile(fn, create_statement + text() + "\n/\n" +
                                 create_statement + other_part->text() + "\n/\n"))
                    return false;
            }
            else
            {
                if (!toWriteFile(fn, create_statement + other_part->text() + "\n/\n" +
                                 create_statement + text() + "\n/\n"))
                    return false;
            }
            toMainWidget()->addRecentFile(fn);
            setFilename(fn);
            other_part->setFilename(fn);
            setModified(false);
            other_part->setModified(false);
            emit fileSaved(fn);
            emit other_part->fileSaved(fn);
            return true;
        }
        else
        {
            // if something else (not .pls) was chosen - then default
            // functionality (save only current tab) must be performed
            // set chosen file
            setFilename(fn);
            // and call default save functionality without choosing a file again
            return toMarkedText::editSave(false);
        }
    }
    return false;
} // editSave
