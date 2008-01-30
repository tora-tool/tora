#include "tobackuptool.h"

#include "icons/tobackup.xpm"
#include "tobackup.h"

const char** toBackupTool::pictureXPM(void)
{
    return const_cast<const char**>(tobackup_xpm);
}

toBackupTool::toBackupTool() : toTool(240, "Backup Manager")
{ }

const char* toBackupTool::menuItem()
{
    return "Backup Manager";
}

QWidget* toBackupTool::toolWindow(QWidget *parent, toConnection &connection)
{
    std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
    if (i != Windows.end())
    {
        (*i).second->setFocus();
        return NULL;
    }
    else
    {
        QWidget *window = new toBackup(this, parent, connection);
        Windows[&connection] = window;
        return window;
    }
}

void toBackupTool::closeWindow(toConnection &connection)
{
    std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
    if (i != Windows.end())
        Windows.erase(i);
}


