/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
        void errorsChanged(const QString & type, const QMap<int,QString> & values);
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
        void applyResult(const QString &, const QMap<int,QString>&);
        void updateContent()
        {
            updateContent(m_editor);
        };
        void changeContent(toTreeWidgetItem *);

};

#endif
