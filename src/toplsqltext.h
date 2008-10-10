
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

#ifndef TOPLSQLTEXT_H
#define TOPLSQLTEXT_H


#include "totool.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"

class toTreeWidgetItem;
class toTreeWidget;


/*! \brief An editor widget for PL/SQL Editor.
*/
class toPLSQLText : public toHighlightedText
{
    Q_OBJECT
    QString Schema;
    QString Object;
    QString Type;

    static int ID;

    public:
        toPLSQLText(QWidget *parent = 0);

        enum CompilationType {Production,
                            Warning};

        void setData(const QString &schema, const QString &type, const QString &data);

        const QString &schema(void) const
        {
            return Schema;
        }
        const QString &object(void) const
        {
            return Object;
        }
        void setType(const QString &type)
        {
            setData(Schema, type, Object);
        }
        void setSchema(const QString &schema)
        {
            setData(schema, Type, Object);
        }
        const QString &type(void) const
        {
            return Type;
        }
        void clear(void);

        bool readData(toConnection &connection/*, toTreeWidget **/);
        bool readErrors(toConnection &connection);
        bool compile(CompilationType t);

    signals:
        void errorsChanged(const QString & type, const QMultiMap<int,QString> & values);
        void warningsChanged(const QMap<int,QString> values);
        void contentChanged();

};


/*! \brief A main widget for PL/SQL Editor.
It handles all Content and Messages cooperation with Editor.
*/
class toPLSQLWidget : public QWidget
{
    Q_OBJECT

    public:
        toPLSQLWidget(QWidget * parent = 0);
        ~toPLSQLWidget();

        toPLSQLText * editor() { return m_editor; };

    private:
        toTreeWidget * m_contents;
        QTreeWidget * m_result;
        toPLSQLText * m_editor;

        QSplitter * m_splitter;
        QSplitter * m_contentSplitter;

        QTreeWidgetItem * m_errItem;
        QTreeWidgetItem * m_warnItem;

        void updateArguments(toSQLParse::statement &statements,
                            toTreeWidgetItem *parent);
        void updateContent(toSQLParse::statement &statements,
                        toTreeWidgetItem *parent,
                        const QString &id = QString::null);
        void updateContent(toPLSQLText *editor);

    private slots:
        void goToError(QTreeWidgetItem *, QTreeWidgetItem *);
        void applyResult(const QString &, const QMultiMap<int,QString>&);
        void updateContent()
        {
            updateContent(m_editor);
        };
        void changeContent(toTreeWidgetItem *);

};

#endif
