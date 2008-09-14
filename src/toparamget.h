/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOPARAMGET_H
#define TOPARAMGET_H

#include "config.h"
#include "toconnection.h"
#include "tohelp.h"

#include <list>
#include <map>

#include <qdialog.h>
#include <qpushbutton.h>

#include <QResizeEvent>
#include <QScrollArea>
#include <QGridLayout>

class QComboBox;

class toParamGetButton : public QPushButton
{
    Q_OBJECT
    int Row;

public:
    toParamGetButton(int row, QWidget *parent = 0, const char *name = 0)
            : QPushButton(name, parent), Row(row)
    {

        connect(this, SIGNAL(clicked()), this, SLOT(internalClicked()));
    }

private slots:
    void internalClicked(void)
    {
        emit clicked(Row);
    }

signals:
    void clicked(int);
};


/** A help class to parse SQL for input/output bindings and if
 * available also pop up a dialog and ask for values. Also maintains
 * an internal cache of old values.
 */
class toParamGet : public QDialog, public toHelpContext
{
    Q_OBJECT;

    /** Default values cache
     */
    static std::map<QString, std::list<QString> > DefaultCache;

    /** Specified values cache
     */
    static std::map<QString, std::list<QString> > Cache;

    QScrollArea *View;
    QGridLayout *Container;

    std::list<QComboBox *> Value;
    toParamGet(QWidget *parent = 0, const char *name = 0);

public:
    /** Get parameters for specified SQL string.
     * @param conn Connection to get binds for.
     * @param parent Parent widget if dialog is needed.
     * @param str SQL to parse for bindings. Observe that this string can be changed to further
     *            declare the binds.
     * @param interactive If not interactive simply rewrite the query and pass on the defaults.
     * @return Returns a list of values to feed into the query.
     */
    static toQList getParam(toConnection &conn,
                            QWidget *parent,
                            QString &str,
                            bool interactive = true);

    /** Specify a default value for the cache. This can not overwrite a manually
     * specified value by the user.
     * @param conn Connection to get binds for.
     * @param name Name of the bind variable.
     * @param val Value of the bind variable.
     */
    static void setDefault(toConnection &conn,
                           const QString &name,
                           const QString &val);

private slots:
    virtual void showMemo(int row);
};

#endif
