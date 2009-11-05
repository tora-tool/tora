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
    std::list<QString> table_columns;
    QString column_name; // extract data to these variables before ...
    QString column_type; // ... combining and pushing to table_columns list

    QString tmp;
    QString tmp1;
    bool first_column = true;

    // Part I - go through given rows and extract table information into local structures
    std::list<QString>::iterator i = destin.begin();

    while (i != destin.end()) {
        tmp = toExtract::partDescribe(*i, 3);
        if (tmp.isNull()) {
            // part 3 (starting from 0) is null only for table name description
            if (!table_name.isNull()) {
                // if table_name already filled in - error in data or this code
                throw qApp->translate("toExtract", "Too many table definitions");
            }
            schema_name = toExtract::partDescribe(*i, 0);
            tmp         = toExtract::partDescribe(*i, 1);
            if (tmp != "TABLE") {
                throw qApp->translate("toExtract", "Unkown string %s").arg(*i);
            }
            table_name  = toExtract::partDescribe(*i, 2);
        } else {
            if (!SameContext(*i, context)) {
                if (!context.isNull()) {
                    table_columns.insert(table_columns.end(), column_name + " " + column_type);
                    column_name == "";
                }
                context = toExtract::contextDescribe(*i, 1);
            }
            if (tmp == "COLUMN") {
                tmp1 = toExtract::partDescribe(*i, 5);
                if (tmp1.isNull()) {
                    column_name = toExtract::partDescribe(*i, 4);
                } else if (tmp1 == "ORDER") {
                    // nothing to do with ORDER
                } else {
                    column_type = tmp1;
                }
            } else if (tmp == "TABLESPACE") {
                tablespace = toExtract::partDescribe(*i, 4);
            } else {
                throw qApp->translate("toExtract", "Unkown string %s").arg(*i);
            }
        } // if context.isNull
        i++;
    } // while
    if (!column_name.isNull()) {
        table_columns.insert(table_columns.end(), column_name + " " + column_type);
    }

    // Part II - generate oracle create table statement from data in local structures
    // table header
    tmp = "create table " + schema_name + "." + table_name + "(";

    // table body
    for (std::list<QString>::iterator i = table_columns.begin(); i != table_columns.end(); i++) {
        if (!first_column) {
            tmp += ",";
            first_column = false;
        }
        tmp += ("\n" + *i);
    } // for

    // table footer
    tmp += ")";
    if (!tablespace.isNull())
        tmp += ("\ntablespace " + tablespace);

    return tmp;
} // createTable

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
    bool add_column = true;

    // drop statements
    for (std::list<QString>::iterator i = drop.begin(); i != drop.end(); i++) {
printf(">>%s\n", (*i).toAscii().constData());
        table_name = toExtract::partDescribe(*i, 2);
        tmp = toExtract::partDescribe(*i, 3);
        if (!SameContext(*i, context)) {
            if (context != "") {
                if (column_name != "")
                    statements += ("alter table " + table_name + " drop column " + column_name + ";\n");
                column_name = "";
            }
            context = toExtract::contextDescribe(*i, 1);
        }

        if (tmp == "COLUMN") {
            tmp1 = toExtract::partDescribe(*i, 5);
            if (tmp1.isNull()) {
printf("qqqqq\n");
                column_name = toExtract::partDescribe(*i, 4);
            } else if (tmp1 == "ORDER") {
                // nothing to do with ORDER
            }
        } else {
            throw qApp->translate("toExtract", "Unkown string %s").arg(tmp);
        }
    }
    if (column_name != "") {
        statements += ("alter table " + table_name + " drop column " + column_name + ";\n");
    }

    // add/modify/rename statements
    column_name = "";
    context = "";
    for (std::list<QString>::iterator i = create.begin();i != create.end();i++) {

        table_name = toExtract::partDescribe(*i, 2);
        tmp = toExtract::partDescribe(*i, 3);
        if (!SameContext(*i, context)) {
            if (context != "") {
                if (add_column)
                    statements += ("alter table " + table_name + " add (" + column_name + " " + column_type + ");\n");
                column_name == "";
                add_column = true;
            }
            context = toExtract::contextDescribe(*i, 1);
        }

        if (tmp == "COLUMN") {
            tmp1 = toExtract::partDescribe(*i, 5);
            if (tmp1.isNull()) {
                column_name = toExtract::partDescribe(*i, 4);
            } else if (tmp1 == "MODIFY") {
                add_column = false;
                tmp = toExtract::partDescribe(*i, 6);
                column_name = toExtract::partDescribe(*i, 4);
                statements += ("alter table " + table_name + " modify (" + column_name + " " + tmp + ");\n");
            } else if (tmp1 == "RENAME") {
                add_column = false;
                tmp = toExtract::partDescribe(*i, 6);
                column_name = toExtract::partDescribe(*i, 4);
                statements += ("alter table " + table_name + " rename column " + column_name + " to " + tmp + ";\n");
            } else if (tmp1 == "ORDER") {
                // nothing to do with ORDER
            } else {
                column_type = tmp1;
            }
        } else if (tmp == "RENAME") {
            new_table_name = toExtract::partDescribe(*i, 4);
            add_column = false;
        } else {
            throw qApp->translate("toExtract", "Unkown string %s").arg(tmp);
        }
    }
    if (column_name != "") {
        if (add_column)
            statements += ("alter table " + table_name + " add (" + column_name + " " + column_type + ");\n");
    }

    if (!new_table_name.isNull())
        statements += ("rename table " + table_name + " to " + new_table_name + ";");

    return statements;
} // alterTable

/***********************************************************************************
   Main function for table creation/altering called from tobrowsertable.cpp
   It will call either createTable either alterTable
************************************************************************************/
QString toOracleExtract::migrateTable(toExtract &ext, std::list<QString> &source,
                                        std::list<QString> &destin) const
{
    printf("source=\n");
    for (std::list<QString>::iterator i = source.begin(); i != source.end(); i++) {
        printf("%s\n", i->toAscii().constData());
    }
    printf("destin=\n");
    for (std::list<QString>::iterator i = destin.begin();i != destin.end();i++) {
        printf("%s\n", i->toAscii().constData());
    }

    if (source.empty()) {
        printf("New table has to be created.\n");
        return createTable(destin);
    } else {
        printf("Existing table is to be modified.\n");
        return alterTable(source, destin);
    }
} // migrateTable
