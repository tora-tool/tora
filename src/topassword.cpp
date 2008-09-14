/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

#include <qapplication.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

static toSQL SQLChangePassword("toPassword:ChangePassword",
                               "ALTER USER %1 IDENTIFIED BY \"%2\"",
                               "Change password of a user, must have same %");

class toPasswordTool : public toTool
{
protected:
public:
    toPasswordTool()
            : toTool(599, "Password Change")
    { }
    virtual const char *menuItem()
    {
        return "Password Change";
    }
    virtual QWidget *toolWindow(QWidget *, toConnection &connection)
    {
        QString np = QInputDialog::getText(toMainWidget(),
                                           qApp->translate("toPasswordTool", "Enter new password"),
                                           qApp->translate("toPasswordTool", "Enter new password"),
                                           QLineEdit::Password,
                                           QString::null,
                                           NULL);
        if (!np.isNull())
        {
            QString np2 = QInputDialog::getText(toMainWidget(),
                                                qApp->translate("toPasswordTool", "Enter new password again for confirmation"),
                                                qApp->translate("toPasswordTool", "Enter new password again for confirmation"),
                                                QLineEdit::Password,
                                                QString::null,
                                                NULL);
            if (!np2.isNull())
            {
                if (np != np2)
                    TOMessageBox::warning(toMainWidget(),
                                          qApp->translate("toPasswordTool", "Passwords doesn't match"),
                                          qApp->translate("toPasswordTool", "The two givens passwords doesn't match, please try again"));
                else
                {
                    try
                    {
                        connection.execute(toSQL::string(SQLChangePassword, connection).
                                           arg(connection.user()).arg(np));
                    }
                    catch (const QString &exc)
                    {
                        toStatusMessage(exc);
                    }
                }
            }
        }
        return NULL;
    }
    virtual void closeWindow(toConnection &connection){};
};

static toPasswordTool PasswordTool;
