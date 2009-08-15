
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
#define PLSQL_RESULT ""


static struct TypeMapType
{
    const char *Type;
    const char *Description;
    bool WantName;
    bool Declaration;
}
TypeMap[] = { { "FUNCTION", "Fc", true , true },    // Must be first in list
    { "PROCEDURE", "Pr", true , true },
    { "PACKAGE", "Pkg", true , true },
    { "DECLARE", "Anon", false, true },
    { "TYPE", "Type", true , false},
    { "CURSOR", "Cur", true , false},
    { "IF", "Cond", false, false},
    { "LOOP", "Loop", false, false},
    { "WHILE", "Loop", false, false},
    { "FOR", "Loop", false, false},
    { NULL, NULL, false, false}
};


static toTreeWidgetItem *toLastItem(toTreeWidgetItem *parent)
{
    toTreeWidgetItem *lastItem = NULL;
    for (toTreeWidgetItem *item = parent->firstChild();item;item = item->nextSibling())
        lastItem = item;
    return lastItem;
}

class toContentsItem : public toTreeWidgetItem
{
public:
    int Line;
    toContentsItem(toTreeWidgetItem *parent, const QString &name, int line)
            : toTreeWidgetItem(parent, toLastItem(parent), name)
    {
        Line = line;
    }
    toContentsItem(toTreeWidget *parent, const QString &name, const QString &id, int line)
            : toTreeWidgetItem(parent, name, id)
    {
        Line = line;
    }
};

static bool FindKeyword(toSQLParse::statement &statements, bool onlyNames, bool &declaration, int &line, QString &name)
{
    if (statements.Type == toSQLParse::statement::Keyword ||
            statements.Type == toSQLParse::statement::Token)
    {
        line = statements.Line;
        if (name.isEmpty())
        {
            name = statements.String.toUpper();

            int j;
            for (j = 0;TypeMap[j].Type && TypeMap[j].Type != name;j++)
                ;
            if (TypeMap[j].Type)
                name = TypeMap[j].Description;
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
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();i != statements.subTokens().end();i++)
    {
        bool ret = FindKeyword(*i, onlyNames, declaration, line, name);
        if (ret)
            return ret;
    }
    return false;
}



bool toPLSQLText::compile(CompilationType t)
{
    QString str = text();
    bool ret = true;
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
//         toStatusMessage(exc);
        ret = false;
    }

    try
    {
        if (t == toPLSQLText::Warning)
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
                           "SELECT attribute, Line-1,Text FROM SYS.All_Errors\n"
                           " WHERE OWNER = :f1<char[101]>\n"
                           "   AND NAME = :f2<char[101]>\n"
                           "   AND TYPE = :f3<char[101]>\n"
                           " ORDER BY Attribute, Type, Line",
                           "Get lines with errors in object (Observe first line 0)");

int toPLSQLText::ID = 0;

toPLSQLText::toPLSQLText(QWidget *parent)
    : toHighlightedText(parent, QString::number(++ID).toLatin1())
{
}

void toPLSQLText::clear(void)
{
    setData(QString::null, QString::null, QString::null);
    toHighlightedText::clear();
}

bool toPLSQLText::readErrors(toConnection &conn)
{
    try
    {
        toQuery errors(conn, SQLReadErrors, Schema, Object, Type);
        QMultiMap<int, QString> Errors;
        QMultiMap<int, QString> Warnings;

        int line;
        QString errType;
        while (!errors.eof())
        {
            errType = errors.readValue(); // "ERROR"/"WARNING" etc.
            line = errors.readValue().toInt();
            if (errType == PLSQL_ERROR)
                Errors.insert(line, errors.readValue());
            else // "WARNING"
                Warnings.insert(line, errors.readValue());
        }
        setErrors(Errors);

        emit errorsChanged(PLSQL_ERROR, Errors);
        emit errorsChanged(PLSQL_WARNING, Warnings);

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

        QString str;
        while (!lines.eof())
            str += lines.readValue();
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
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();i != statements.subTokens().end();i++)
    {
        if ((*i).Type == toSQLParse::statement::List)
        {
            bool first = true;
            for (std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin();j != (*i).subTokens().end();j++)
            {
                if ((*j).String == ",")
                    first = true;
                else if (first)
                {
                    new toContentsItem(parent, "Param " + (*j).String, (*j).Line);
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
    if (!FindKeyword(statements, statements.Type == toSQLParse::statement::Statement, declaration, line, name) || name.isNull())
        return ;

    if (parent)
        item = new toContentsItem(parent, name, line);
    else
    {
        for (item = m_contents->firstChild();item;item = item->nextSibling())
            if (item->text(0) == name && item->text(1) == id)
            {
                item->setText(2, QString::null);
                break;
            }
        if (!item)
            item = new toContentsItem(m_contents, name, id, line);
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
                        new toContentsItem(item, "Var " + (*j).String, (*j).Line);
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

    for (std::list<toSQLParse::statement>::iterator i = statements.begin();i != statements.end();i++)
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
    m_errItem = new QTreeWidgetItem(m_result,
                                    QStringList() << tr("Errors") << "");
    m_warnItem = new QTreeWidgetItem(m_result,
                                     QStringList() << tr("Warnings") << "");

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->insertWidget(0, m_editor);
    m_splitter->insertWidget(1, m_result);

    m_contentSplitter = new QSplitter(Qt::Horizontal, this);
    m_contentSplitter->insertWidget(0, m_contents);
    m_contentSplitter->insertWidget(1, m_splitter);

    if (layout() == 0)
        setLayout(new QVBoxLayout);
    layout()->addWidget(m_contentSplitter);

    connect(m_editor,
            SIGNAL(errorsChanged(const QString &, const QMultiMap<int,QString>&)),
            this,
            SLOT(applyResult(const QString &, const QMultiMap<int,QString>&)));
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

void toPLSQLWidget::applyResult(const QString & type,
                                 const QMultiMap<int,QString> & values)
{
    QTreeWidgetItem * item = (type == PLSQL_ERROR) ? m_errItem : m_warnItem;

    foreach (QTreeWidgetItem * c, item->takeChildren())
        delete c;

    QMap<int,QString>::const_iterator i = values.constBegin();
    while (i != values.constEnd())
    {
        QStringList l(i.value());
        l.append(QString().setNum(i.key()));
        item->addChild(new QTreeWidgetItem(item, l));
        ++i;
    }

    m_result->expandItem(item);
    m_result->resizeColumnToContents(0);
}

void toPLSQLWidget::goToError(QTreeWidgetItem * current, QTreeWidgetItem *)
{
    if (current != m_errItem && current != m_warnItem)
    {
        m_editor->setCursorPosition(current->text(1).toInt(), 0);
        m_editor->setFocus(Qt::OtherFocusReason);
    }
}

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
