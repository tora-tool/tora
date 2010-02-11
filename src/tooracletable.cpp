#include <QDebug>
#include "tooracleextract.h"

// the same function as in tooracleextract.cpp should probably be moved to toextract.cpp?
static bool SameContext(const QString &str, const QString &str2)
{
    if (str.startsWith(str2))
    {
        if (str.length() == str2.length())
            return true;
        if (str.length() > str2.length() &&
                str.at(str2.length()) == QChar('\001'))
            return true;
    }
    return false;
} // SameContext

/***********************************************************************************
Purpose: Create new table
TODO: Maybe toExtract::parseColumnDescription should be used to extract columns?
************************************************************************************/
QString toOracleExtract::createTable(std::list<QString> &destin) const
{
    QString context; // grouping properties of one entity (for example one column)

    QString schema_name;
    QString table_name;
    QString tablespace;
    std::list<toExtract::columnInfo> table_columns;

    QString tmp;
    QString end_statements; // statements going after create table one (f.e. comments)
    bool first_column = true;

    // Part I - go through given rows and extract table information into local structures
    std::list<QString>::iterator i = destin.begin();

    while (i != destin.end())
    {
        tmp = toExtract::partDescribe(*i, 3);
        if (tmp.isNull())
        {
            // part 3 (starting from 0) is null only for table name description
            if (!table_name.isNull())
            {
                // if table_name already filled in - error in data or this code
                throw qApp->translate("toExtract", "Too many table definitions");
            }
            schema_name = toExtract::partDescribe(*i, 0);
            tmp         = toExtract::partDescribe(*i, 1);
            if (tmp != "TABLE")
            {
                throw qApp->translate("toExtract", "Unkown string %1").arg(*i);
            }
            table_name  = toExtract::partDescribe(*i, 2);
        }
        else
        {
            if (tmp == "COLUMN")
            {
            }
            else if (tmp == "TABLESPACE")
            {
                tablespace = toExtract::partDescribe(*i, 4);
            }
            else if (tmp == "COMMENT" )
            {
                end_statements += ("comment on table " + table_name +
                                   " is '" + toExtract::partDescribe(*i, 4) + "';\n");
            }
            else
            {
                throw qApp->translate("toExtract", "Unkown string %1").arg(*i);
            }
        } // if context.isNull
        i++;
    } // while

    table_columns = toExtract::parseColumnDescription(destin);

    // Part II - generate oracle create table statement from data in local structures
    // table header
    tmp = "create table " + schema_name + "." + table_name + "(";

    // table body - loop through columns
    for (std::list<toExtract::columnInfo>::iterator i = table_columns.begin(); i != table_columns.end(); i++)
    {
        if (!first_column)
            tmp += ",";
        else
            first_column = false;
        tmp += ("\n" + (*i).Name + " " + (*i).Definition);

        if (!(*i).DefaultValue.isEmpty())
            tmp += (" DEFAULT '" + (*i).DefaultValue + "'");

        if ((*i).bNotNull)
            tmp += " NOT NULL";

        if (!(*i).Comment.isEmpty())
            end_statements +=
                ("comment on column " + table_name + "." + (*i).Name + " is '" + (*i).Comment + "';\n");
    } // for loop through columns

    // table footer
    tmp += ")";
    if (!tablespace.isNull())
        tmp += ("\ntablespace " + tablespace + ";");
    else
        tmp += ";";

    if (!end_statements.isEmpty())
        tmp += ("\n" + end_statements);

    return tmp;
} // createTable

/***********************************************************************************
Purpose: Checks if specified property(-ies) is mentioned in given list.
  This function is used when altering tables. It can for example say that column
mentioned in drop list should not actually be dropped because it is being renamed.
************************************************************************************/
bool propertyMentioned(std::list<QString> &list,
                       const QString property1,
                       const QString property2)
{
    QStringList split;
    bool bProp1;

    std::list<QString>::iterator i = list.begin();
    while (i != list.end())
    {
        split = (*i).split("\01");
        bProp1 = false;
        for (int j = 0; j < split.count(); j++)
        {
            if (!bProp1)
            {
                if (split[j] == property1)
                {
                    bProp1 = true;
                }
            }
            else if (split[j] == property2)
            {
                return true;
            }
        }
        i++;
    }
    return false;
} // columnMentioned

/***********************************************************************************
Purpose: Checks if column_type has default value. If it does not - it checks if
         default value was present before. If so - it returns true - meaning that
         altering operation should also remove default value
************************************************************************************/
bool defaultValueRemoved(std::list<QString> &list,
                         const QString column_name,
                         const QString column_type)
{
    QStringList split;

    if (column_type.indexOf("default", 0, Qt::CaseInsensitive) > -1)
        // there is default value specified, nothing else to check
        return false;

    std::list<QString>::iterator i = list.begin();
    while (i != list.end())
    {
        split = (*i).split("\01");
        if (split.count() == 6)
            if ((split[3] == "COLUMN") &&
                    (split[4] == column_name) &&
                    (split[5].indexOf(" default '", 0, Qt::CaseInsensitive) > -1))
                return true; // yes, default value WAS removed
        i++;
    }
    return false;
} // defaultValueRemoved
/***********************************************************************************
Purpose: Alter existing table
************************************************************************************/
QString toOracleExtract::alterTable(std::list<QString> &source,
                                    std::list<QString> &destin) const
{
    std::list<QString> drop; // will hold table properties being removed
    std::list<QString> create; // will hold table properties being added
    toExtract::srcDst2DropCreate(source, destin, drop, create);

    QString statements;
    QString context = "";
    QString table_name;
    QString new_table_name;
    QString column_name = "", column_type;
    QString tmp, tmp1;
    QString end_statements; // statements to be added to the end
    QString operation; // used to differentiate between add and modify column
    QString not_null;
    bool add_modify_column = false;

    // drop statements
    for (std::list<QString>::iterator i = drop.begin(); i != drop.end(); i++)
    {
#ifdef DEBUG
        qDebug() << "drop>>" + *i;
#endif
        table_name = toExtract::partDescribe(*i, 2);
        if (!SameContext(*i, context))
        {
            if (context != "")
            {
                if (column_name != "")
                    // check if property is not mentioned in create list,
                    // if it is, this is not dropping of column but rather renaming
                    if (!propertyMentioned(destin, "COLUMN", column_name))
                        statements += ("alter table " + table_name + " drop column " + column_name + ";\n");
                column_name = "";
            }
            context = toExtract::contextDescribe(*i, 5);
        }

        std::list<QString> row = toExtract::splitDescribe(*i);
        // read through "context" - skip general tablename information
        for (int j = 0; j < 3; j++)
            toShift(row);
        tmp = toShift(row);
        if (tmp == "COLUMN")
        {
            column_name = toShift(row);
            tmp = toShift(row);
            if (tmp.isNull())
            {
                // nothing to be done here, column name was defined
            }
            else if (tmp == "ORDER")
            {
                // nothing to do with ORDER
            }
            else if (tmp == "COMMENT")
            {
                if (propertyMentioned(destin, "COLUMN", column_name)) // if column is not dropped...
                    if (!propertyMentioned(destin, column_name, "COMMENT")) // ...and comment is not changed
                        // then drop comment
                        statements += ("comment on column " + table_name + "." + column_name + " is '';\n");
            }
            else if (tmp == "EXTRA")
            {
                if (propertyMentioned(destin, "COLUMN", column_name))
                    // if column does not have to be dropped - just remove "not null" flag
                    statements += ("alter table " + table_name + " modify (" + column_name + " null);\n");
            }
            else
            {
                // nothing to do with column type
            }
        }
        else if (tmp == "PARAMETERS")
        {
            // TODO Check if anything should/could be done with parameters being dropped
            tmp = toExtract::partDescribe(*i, 4);
#ifdef DEBUG
            qDebug() << "Dropping parameters is NOT currently supported (" + tmp + ").";
#endif
        }
        else if (tmp == "STORAGE")
        {
            // TODO Check if anything should/could be done with sotrage attributes being dropped
            tmp = toExtract::partDescribe(*i, 4);
#ifdef DEBUG
            qDebug() << "Dropping storage attributes is NOT currently supported (" + tmp + ").";
#endif
        }
        else
        {
            throw qApp->translate("toExtract", "Unkown string %1 for drop statements").arg(tmp);
        }
    }
    if (column_name != "")
        if (!propertyMentioned(destin, "COLUMN", column_name))
            statements += ("alter table " + table_name + " drop column " + column_name + ";\n");

    // add/modify/rename statements
    column_name = "";
    context = "";
    for (std::list<QString>::iterator i = create.begin(); i != create.end(); i++)
    {
#ifdef DEBUG
        qDebug() << "add>>" + *i;
#endif

        table_name = toExtract::partDescribe(*i, 2);
        tmp = toExtract::partDescribe(*i, 5);
        if (!SameContext(*i, context))
        {
            if (context != "")
            {
                if (add_modify_column)
                {
                    if (propertyMentioned(source, "COLUMN", column_name))
                    {
                        operation = "modify";
                        if (defaultValueRemoved(drop, column_name, column_type))
                            column_type += " default null"; // Note! It is not possible to remove default value in Oracle!!!
                    }
                    else
                    {
                        operation = "add";
                    }
                    statements += ("alter table " + table_name + " " + operation +
                                   " (" + column_name + " " + column_type + not_null + ");\n");
                    add_modify_column = false;
                }
                column_name == "";
            }
            context = toExtract::contextDescribe(*i, 5);
            not_null = "";
        }

        std::list<QString> row = toExtract::splitDescribe(*i);
        // read through "context" - skip general tablename information
        for (int j = 0; j < 3; j++)
            toShift(row);
        tmp = toShift(row);
        if (tmp == "COLUMN")
        {
            column_name = toShift(row);
            tmp = toShift(row);  // value AFTER column name
            if (tmp.isNull())
            {
                column_name = tmp1;
            }
            else if (tmp == "RENAME")
            {
                tmp = toShift(row);
                statements += ("alter table " + table_name + " rename column " + column_name + " to " + tmp + ";\n");
            }
            else if (tmp == "ORDER")
            {
                // nothing to do with ORDER
            }
            else if (tmp == "COMMENT")
            {
                tmp = toShift(row);
                end_statements += ("comment on column " + table_name + "." + column_name + " is '" + tmp + "';\n");
            }
            else if (tmp == "EXTRA")
            {
                not_null = " NOT NULL"; // set "not null" flag on column
                add_modify_column = true;
            }
            else
            {
                column_type = tmp;
                add_modify_column = true;
            }
        }
        else if (tmp == "RENAME")   // rename table
        {
            new_table_name = toShift(row);
        }
        else
        {
            throw qApp->translate("toExtract", "Unkown string %1 for alter/add statements.").arg(tmp);
        }
    }
    if (add_modify_column)
    {
        if (propertyMentioned(source, "COLUMN", column_name))
        {
            operation = "modify";
            if (defaultValueRemoved(drop, column_name, column_type))
                column_type += " default null"; // Note! It is not possible to remove default value in Oracle!!!
        }
        else
        {
            operation = "add";
        }
        statements += ("alter table " + table_name + " " + operation +
                       " (" + column_name + " " + column_type + not_null + ");\n");
    }

    if (!new_table_name.isNull())
        statements += ("alter table " + table_name + " rename to " + new_table_name + ";");

    return statements + end_statements;
} // alterTable

/***********************************************************************************
   Main function for table creation/altering called from tobrowsertable.cpp
   It will call either createTable either alterTable
************************************************************************************/
QString toOracleExtract::migrateTable(toExtract &ext, std::list<QString> &source,
                                      std::list<QString> &destin) const
{
#ifdef DEBUG
    qDebug() << "toOracleExtract::migrateTable source=";
    for (std::list<QString>::iterator i = source.begin(); i != source.end(); i++)
    {
        qDebug() << *i;
    }
    qDebug() << "toOracleExtract::migrateTable destin=";
    for (std::list<QString>::iterator i = destin.begin(); i != destin.end(); i++)
    {
        qDebug() << *i;
    }
#endif


    if (source.empty())
    {
#ifdef DEBUG
        qDebug() << "New table has to be created.";
#endif
        return createTable(destin);
    }
    else
    {
#ifdef DEBUG
        qDebug() << "Existing table is to be modified.";
#endif
        return alterTable(source, destin);
    }
} // migrateTable
