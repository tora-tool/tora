
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tofilesize.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultstorage.h"
#include "tosql.h"
#include "tostorage.h"
#include "tostoragedefinition.h"
#include "ui_tostorageprefsui.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QMdiArea>
#include <QTableView>
#include <QHeaderView>

#include <QPixmap>
#include <QFileDialog>
#include <QMenu>
#include <QSortFilterProxyModel>

#include "icons/addfile.xpm"
#include "icons/addtablespace.xpm"
#include "icons/droptablespace.xpm"
#include "icons/coalesce.xpm"
#include "icons/eraselog.xpm"
#include "icons/logging.xpm"
#include "icons/modfile.xpm"
#include "icons/modtablespace.xpm"
#include "icons/movefile.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/readtablespace.xpm"
#include "icons/refresh.xpm"
#include "icons/storageextents.xpm"
#include "icons/tostorage.xpm"
#include "icons/writetablespace.xpm"

// #define CONF_DISP_TABLESPACES "DispTablespaces"
// #define CONF_DISP_COALESCED "DispCoalesced"
// #define CONF_DISP_EXTENTS "DispExtents"
// #define CONF_DISP_AVAILABLEGRAPH "AvailableGraph"

class toStoragePrefs : public QWidget, public Ui::toStoragePrefsUI, public toSettingTab
{
    toTool *Tool;

public:
    toStoragePrefs(toTool *tool, QWidget* parent = 0, const char* name = 0);
    virtual void saveSetting(void);
};

toStoragePrefs::toStoragePrefs(toTool *tool, QWidget* parent, const char* name)
        : QWidget(parent), toSettingTab("storage.html"), Tool(tool)
{
    setupUi(this);

    DispCoalesced->setChecked(toConfigurationSingle::Instance().dispCoalesced());
    DispExtents->setChecked(toConfigurationSingle::Instance().dispExtents());
    DispTablespaces->setChecked(toConfigurationSingle::Instance().dispTablespaces());
    DispAvailableGraph->setChecked(toConfigurationSingle::Instance().dispAvailableGraph());
}

void toStoragePrefs::saveSetting(void)
{
    toConfigurationSingle::Instance().setDispCoalesced(DispCoalesced->isChecked());
    toConfigurationSingle::Instance().setDispExtents(DispExtents->isChecked());
    toConfigurationSingle::Instance().setDispTablespaces(DispTablespaces->isChecked());
    toConfigurationSingle::Instance().setDispAvailableGraph(DispAvailableGraph->isChecked());
}

class toStorageTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(tostorage_xpm);
    }
public:
    toStorageTool()
            : toTool(50, "Storage Manager")
    { }
    virtual const char *menuItem()
    {
        return "Storage Manager";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toStorage(parent, connection);
    }
    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toStoragePrefs(this, parent);
    }
    virtual void closeWindow(toConnection &connection){};
};

toDropTablespace::toDropTablespace(QWidget* parent, const char* name, Qt::WFlags fl)
        : QWidget(parent, fl)
{
    setupUi(this);
    if (!name)
        setObjectName("toDropTablespace");
    else
        setObjectName(name);
}

std::list<QString> toDropTablespace::sql()
{
    std::list<QString> ret;
    if (IncludingObjectsCheck->isChecked())
    {
        toPush(ret, QString::fromLatin1("INCLUDING CONTENTS "));
        if (dataFileCheck->isChecked())
            toPush(ret, QString::fromLatin1("AND DATAFILES "));
        if (CascadeCheck->isChecked())
            toPush(ret, QString::fromLatin1("CASCADE CONSTRAINTS "));
    }
    else
        toPush(ret, QString::fromLatin1(""));
    return ret;
}



toStorageTablespace::toStorageTablespace(QWidget* parent, const char* name, Qt::WFlags fl)
        : QWidget(parent, fl)
{
    setupUi(this);
    Modify = false;

    if (!name)
        setObjectName("toStorageTablespace");
    else
        setObjectName(name);

    MinimumExtent->setTitle(tr("&Minimum Extent"));
    LocalUniform->setTitle(tr("Extent &Size"));
    LocalUniform->setValue(1024);
}

void toStorageTablespace::permanentToggle(bool val)
{
    try
    {
        if (toCurrentConnection(this).version() >= "08")
        {
            emit tempFile(!val && !Dictionary->isChecked());
            return ;
        }
        if (!val)
            Dictionary->setChecked(true);
        LocalAuto->setEnabled(val);
        LocalSelect->setEnabled(val);
    }
    TOCATCH
}

void toStorageTablespace::dictionaryToggle(bool val)
{
    try
    {
        if (toCurrentConnection(this).version() < "08")
            Permanent->setEnabled(val);
        else
            emit tempFile(!Permanent->isChecked() && !val);
        DefaultStorage->setEnabled(val);
        emit allowStorage(val);
    }
    TOCATCH
}

void toStorageTablespace::allowDefault(bool val)
{
    LocalAuto->setEnabled(!val);
    LocalSelect->setEnabled(!val);
    emit allowStorage(val);
}

bool toStorageTablespace::allowStorage(void)
{
    return DefaultStorage->isChecked();
}

void toStorageTablespace::uniformToggle(bool val)
{
    LocalUniform->setEnabled(val);
}

std::list<QString> toStorageTablespace::sql()
{
    std::list<QString> ret;
    QString str;
    if (ExtentGroup->isEnabled() && Dictionary->isChecked())
        toPush(ret, QString::fromLatin1("MINIMUM EXTENT ") + MinimumExtent->sizeString());

    if (Logging->isChecked() != LoggingOrig || !Modify)
    {
        if (Logging->isChecked())
            str = QString::fromLatin1("LOGGING");
        else
            str = QString::fromLatin1("NOLOGGING");
        toPush(ret, str);
    }
    if (Online->isChecked() != OnlineOrig || !Modify)
    {
        if (Online->isChecked())
            str = QString::fromLatin1("ONLINE");
        else
            str = QString::fromLatin1("OFFLINE");
        toPush(ret, str);
    }
    if (Permanent->isChecked() != PermanentOrig || !Modify)
    {
        if (Permanent->isChecked())
            str = QString::fromLatin1("PERMANENT");
        else
            str = QString::fromLatin1("TEMPORARY");
        toPush(ret, str);
    }
    if (ExtentGroup->isEnabled())
    {
        str = QString::fromLatin1("EXTENT MANAGEMENT ");
        if (Dictionary->isChecked())
            str.append(QString::fromLatin1("DICTIONARY"));
        else
        {
            str.append(QString::fromLatin1("LOCAL "));
            if (LocalAuto->isChecked())
                str.append(QString::fromLatin1("AUTOALLOCATE"));
            else
            {
                str.append(QString::fromLatin1("UNIFORM SIZE "));
                str.append(LocalUniform->sizeString());
            }
        }
        toPush(ret, str);
    }
    return ret;
}

toStorageDatafile::toStorageDatafile(bool temp, bool dispName, QWidget* parent, const char* name, Qt::WFlags fl)
        : QWidget(parent, fl),
        Tempfile(temp)
{

    setupUi(this);

    Modify = false;
    InitialSizeOrig = NextSizeOrig = MaximumSizeOrig = 0;

    if (!name)
        setObjectName("DataFile");
    else
        setObjectName(name);

    setWindowTitle(tr("Create datafile"));

    if (!dispName)
    {
        Name->hide();
        NameLabel->hide();
        Filename->setFocus();
    }
    else
        Name->setFocus();

    InitialSize->setTitle(tr("&Size"));
    NextSize->setTitle(tr("&Next"));
    MaximumSize->setTitle(tr("&Maximum size"));
}

std::list<QString> toStorageDatafile::sql(void)
{
    std::list<QString> ret;
    if (!Modify)
    {
        QString str;
        if (Tempfile)
            str = QString::fromLatin1("TEMPFILE '");
        else
            str = QString::fromLatin1("DATAFILE '");
        QString filename(Filename->text());
        filename.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("''"));
        str.append(filename);
        str.append(QString::fromLatin1("' SIZE "));
        str.append(InitialSize->sizeString());
        str.append(QString::fromLatin1(" REUSE AUTOEXTEND "));
        if (AutoExtend->isChecked())
        {
            str.append(QString::fromLatin1("ON NEXT "));
            str.append(NextSize->sizeString());
            str.append(QString::fromLatin1(" MAXSIZE "));
            if (UnlimitedMax->isChecked())
                str.append(QString::fromLatin1("UNLIMITED"));
            else
            {
                str.append(MaximumSize->sizeString());
            }
        }
        else
            str.append(QString::fromLatin1("OFF"));
        toPush(ret, str);
    }
    else
    {
        QString str;
        if (Filename->text() != FilenameOrig)
        {
            str = QString::fromLatin1("RENAME FILE '");
            str += FilenameOrig;
            str += QString::fromLatin1("' TO '");
            str += Filename->text();
            str += "'";
            toPush(ret, str);
        }
        if (InitialSize->value() != InitialSizeOrig)
        {
            if (Tempfile)
                str = QString::fromLatin1("TEMPFILE '");
            else
                str = QString::fromLatin1("DATAFILE '");
            str += Filename->text();
            str += QString::fromLatin1("' RESIZE ");
            str += InitialSize->sizeString();
            toPush(ret, str);
        }
        if (Tempfile)
            str = QString::fromLatin1("TEMPFILE '");
        else
            str = QString::fromLatin1("DATAFILE '");
        str += Filename->text();
        str += QString::fromLatin1("' AUTOEXTEND ");
        if (AutoExtend->isChecked())
        {
            str.append(QString::fromLatin1("ON NEXT "));
            str.append(NextSize->sizeString());
            str.append(QString::fromLatin1(" MAXSIZE "));
            if (UnlimitedMax->isChecked())
                str.append(QString::fromLatin1("UNLIMITED"));
            else
            {
                str.append(MaximumSize->sizeString());
            }
        }
        else
            str.append(QString::fromLatin1("OFF"));
        toPush(ret, str);
    }
    return ret;
}

void toStorageDatafile::browseFile(void)
{
    QString str = TOFileDialog::getSaveFileName(this, tr("Open Storage File"), Filename->text(), QString::fromLatin1("*.dbf"));
    if (!str.isEmpty())
        Filename->setText(str);
}

void toStorageDatafile::autoExtend(bool val)
{
    MaximumSize->setEnabled(!UnlimitedMax->isChecked());
    UnlimitedMax->setEnabled(val);
    NextSize->setEnabled(val);
}

void toStorageDatafile::maximumSize(bool val)
{
    MaximumSize->setEnabled(!val);
}

QString toStorageDatafile::getName(void)
{
    if (!Name)
        throw tr("No name available");
    return Name->text();
}

void toStorageDatafile::valueChanged(const QString &)
{
    if ((Name->isHidden() || !Name->text().isEmpty()) &&
            !Filename->text().isEmpty())
        emit validContent(true);
    else
        emit validContent(false);
}

void toStorageDialog::Setup(void)
{
    DialogTab->removeTab(DialogTab->indexOf(DefaultPage));
    toHelp::connectDialog(this);
}

static toSQL SQLTablespaceInfo("toStorage:TablespaceInfo",
                               "SELECT min_extlen/1024,\n"
                               "       extent_management,\n"
                               "       contents,\n"
                               "       logging,\n"
                               "       status,\n"
                               "       initial_extent/1024,\n"
                               "       next_extent/1024,\n"
                               "       min_extents,\n"
                               "       max_extents,\n"
                               "       pct_increase\n"
                               "  FROM sys.dba_tablespaces\n"
                               " WHERE tablespace_name = :nam<char[70]>",
                               "Get information about a tablespace for the modify dialog, "
                               "must have same columns and bindings");

toStorageDialog::toStorageDialog(const QString &tablespace, QWidget *parent, bool drop)
        : QDialog(parent)//, "Storage Dialog", true)
{
    setupUi(this);

    Setup();
    OkButton->setEnabled(false);

    if (!tablespace.isNull())
    {
        TablespaceOrig = tablespace;
        if (!drop)
        {
            try
            {
                toQList result;
                result = toQuery::readQuery(toCurrentConnection(this),
                                            SQLTablespaceInfo, tablespace);
                if (result.size() != 10)
                    throw tr("Invalid response from query");

                toShift(result);
                QString dict = toShift(result);
                QString temp = toShift(result);

                Mode = NewDatafile;
                setWindowTitle(tr("Add datafile"));
                Tablespace = NULL;
                Default = NULL;
                Drop = NULL;
                Datafile = new toStorageDatafile(dict != QString::fromLatin1("DICTIONARY") &&
                                                 temp != QString::fromLatin1("PERMANENT") &&
                                                 temp != QString::fromLatin1("UNDO"), false, DialogTab);
                DialogTab->addTab(Datafile, tr("Datafile"));
                connect(Datafile, SIGNAL(validContent(bool)), this, SLOT(validContent(bool)));
            }
            TOCATCH
        }
        else
        {
            Mode = DropTablespace;
            Tablespace = NULL;
            Default = NULL;
            Datafile = NULL;
            Drop = new toDropTablespace(DialogTab);
            DialogTab->addTab(Datafile, tr("Drop Tablespace"));
            setWindowTitle(tr("Drop Tablespace"));
            connect(Drop, SIGNAL(validContent(bool)), this, SLOT(validContent(bool)));
            emit validContent(true);
        }
    }
    else
    {
        Mode = NewTablespace;
        setWindowTitle(tr("Add tablespace"));
        Datafile = new toStorageDatafile(false, true, DialogTab);
        DialogTab->addTab(Datafile, tr("Datafile"));
        Tablespace = new toStorageTablespace(DialogTab);
        DialogTab->addTab(Tablespace, tr("Tablespace"));
        Default = new toStorageDefinition(DialogTab);
        DialogTab->addTab(Default, tr("Default Storage"));
        Default->setEnabled(false);
        connect(Tablespace, SIGNAL(allowStorage(bool)), this, SLOT(allowStorage(bool)));
        connect(Tablespace, SIGNAL(tempFile(bool)), Datafile, SLOT(setTempFile(bool)));
        connect(Datafile, SIGNAL(validContent(bool)), this, SLOT(validContent(bool)));
    }

}

toStorageDialog::toStorageDialog(toConnection &conn, const QString &tablespace, QWidget *parent)
        : QDialog(parent)//, "Storage Dialog", true)
{
    setupUi(this);

    Setup();
    try
    {
        Mode = ModifyTablespace;
        Datafile = NULL;
        Tablespace = new toStorageTablespace(DialogTab);
        DialogTab->addTab(Tablespace, tr("Tablespace"));
        Default = new toStorageDefinition(DialogTab);
        DialogTab->addTab(Default, tr("Default Storage"));
        connect(Tablespace, SIGNAL(allowStorage(bool)), this, SLOT(allowStorage(bool)));

        toQList result = toQuery::readQuery(conn, SQLTablespaceInfo, tablespace);
        if (result.size() != 10)
            throw tr("Invalid response from query");
        Tablespace->MinimumExtent->setValue(toShift(result).toInt());

        TablespaceOrig = tablespace;
        Tablespace->Modify = true;
        Default->setEnabled(false);
        if (toShift(result).toString() == "DICTIONARY")
            Tablespace->Dictionary->setChecked(true);
        else
        {
            Tablespace->DefaultStorage->setEnabled(false);
            Tablespace->LocalAuto->setChecked(true);
        }
        Tablespace->ExtentGroup->setEnabled(false);

        Tablespace->Permanent->setChecked(toShift(result).toString() == "PERMANENT");
        Tablespace->Logging->setChecked(toShift(result).toString() == "LOGGING");
        Tablespace->Online->setChecked(toShift(result).toString() == "ONLINE");
        Tablespace->OnlineOrig = Tablespace->Online->isChecked();
        Tablespace->PermanentOrig = Tablespace->Permanent->isChecked();
        Tablespace->LoggingOrig = Tablespace->Logging->isChecked();

        Default->InitialSize->setValue(toShift(result).toInt());
        Default->NextSize->setValue(toShift(result).toInt());
        Default->InitialExtent->setValue(toShift(result).toInt());
        int num = toShift(result).toInt();
        if (num == 0)
            Default->UnlimitedExtent->setChecked(true);
        else
            Default->MaximumExtent->setValue(num);
        Default->PCTIncrease->setValue(toShift(result).toInt());
    }
    catch (const QString &str)
    {
        toStatusMessage(str);
        reject();
    }
}

static toSQL SQLDatafileInfo("toStorage:DatafileInfo",
                             "SELECT bytes/1024,\n"
                             "       autoextensible,\n"
                             "       bytes/blocks*increment_by/1024,\n"
                             "       maxbytes/1024\n"
                             "  FROM (SELECT bytes,autoextensible,blocks,increment_by,maxbytes,file_name,tablespace_name\n"
                             "          FROM sys.dba_data_files\n"
                             "         WHERE tablespace_name = :nam<char[70]>\n"
                             "           AND file_name = :fil<char[1500]>\n"
                             "         UNION\n"
                             "        SELECT bytes,autoextensible,blocks,increment_by,maxbytes,file_name,tablespace_name\n"
                             "          FROM sys.dba_temp_files\n"
                             "         WHERE tablespace_name = :nam<char[70]>\n"
                             "           AND file_name = :fil<char[1500]>)",
                             "Get information about a datafile for the modify dialog, "
                             "must have same columns and bindings",
                             "0801");

static toSQL SQLDatafileInfo8("toStorage:DatafileInfo",
                              "SELECT bytes/1024,\n"
                              "       autoextensible,\n"
                              "       bytes/blocks*increment_by/1024,\n"
                              "       maxbytes/1024\n"
                              "  FROM (SELECT * FROM sys.dba_data_files\n"
                              " WHERE tablespace_name = :nam<char[70]>"
                              "   AND file_name = :fil<char[1500]>)",
                              "",
                              "0800");

toStorageDialog::toStorageDialog(toConnection &conn, const QString &tablespace,
                                 const QString &filename, QWidget *parent)
        : QDialog(parent)//, "Storage Dialog", true)
{
    setupUi(this);

    Setup();
    try
    {
        Mode = ModifyDatafile;

        toQList result;
        result = toQuery::readQuery(toCurrentConnection(this),
                                    SQLTablespaceInfo, tablespace);
        if (result.size() != 10)
            throw tr("Invalid response from query");

        toShift(result);
        QString dict = toShift(result);
        QString temp = toShift(result);

        Datafile = new toStorageDatafile(dict != QString::fromLatin1("DICTIONARY") &&
                                         temp != QString::fromLatin1("PERMANENT") &&
                                         temp != QString::fromLatin1("UNDO"), true, DialogTab);
        DialogTab->addTab(Datafile, tr("Datafile"));
        setWindowTitle(tr("Modify datafile"));
        Tablespace = NULL;
        Default = NULL;

        result = toQuery::readQuery(conn, SQLDatafileInfo, tablespace, filename);

        if (result.size() != 4)
            throw tr("Invalid response from query (Wanted 4, got %1 entries) for %2.%3").
            arg(result.size()).arg(tablespace).arg(filename);
        Datafile->Name->setText(tablespace);
        Datafile->Name->setEnabled(false);
        Datafile->Modify = true;
        Datafile->FilenameOrig = filename;
        Datafile->Filename->setText(filename);
        Datafile->InitialSize->setValue(Datafile->InitialSizeOrig = toShift(result).toInt());
        if (!(toShift(result) == QString::fromLatin1("NO")))
        {
            Datafile->AutoExtend->setChecked(true);
            Datafile->NextSize->setValue(Datafile->NextSizeOrig = toShift(result).toInt());
        }
        else
        {
            Datafile->NextSizeOrig = 0;
        }

        int num = toShift(result).toInt();
        if (num == 0)
        {
            Datafile->UnlimitedMax->setChecked(true);
            Datafile->MaximumSize->setEnabled(false);
        }
        else
        {
            Datafile->UnlimitedMax->setChecked(false);
            Datafile->MaximumSize->setValue(num);
        }
        Datafile->MaximumSizeOrig = num;
    }
    catch (const QString &str)
    {
        toStatusMessage(str);
        reject();
    }
}

void toStorageDialog::validContent(bool val)
{
    OkButton->setEnabled(val);
}

void toStorageDialog::allowStorage(bool val)
{
    Default->setEnabled(val);
}

void toStorageDialog::displaySQL(void)
{
    std::list<QString> lines = sql();
    QString res;
    for (std::list<QString>::iterator i = lines.begin();i != lines.end();i++)
    {
        res += *i;
        res += QString::fromLatin1(";\n");
    }
    if (res.length() > 0)
    {
        toMemoEditor memo(this, res, -1, -1, true, true);
        memo.exec();
    }
    else
        toStatusMessage(tr("No changes made"));
}

std::list<QString> toStorageDialog::sql(void)
{
    try
    {
        std::list<QString> ret;
        switch (Mode)
        {
        case DropTablespace:
        {
            QString start = QString::fromLatin1("DROP TABLESPACE \"");
            start += TablespaceOrig;
            start += QString::fromLatin1("\" ");
            if (Drop)
            {
                std::list<QString> lst = Drop->sql();
                for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                {
                    start += QString::fromLatin1(" ");
                    start += *i;
                }
            }
            toPush(ret, start);
        }
        break;
        case ModifyDatafile:
        {
            QString start = QString::fromLatin1("ALTER DATABASE ");
            std::list<QString> lst = Datafile->sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
            {
                QString t = start;
                t += *i;
                toPush(ret, t);
            }
        }
        break;
        case ModifyTablespace:
        {
            QString start = QString::fromLatin1("ALTER TABLESPACE \"");
            start += TablespaceOrig;
            start += QString::fromLatin1("\" ");
            std::list<QString> lst = Tablespace->sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
            {
                QString t = start;
                t += *i;
                toPush(ret, t);
            }
            if (Tablespace->allowStorage())
            {
                start += QString::fromLatin1("DEFAULT ");
                std::list<QString> lst = Default->sql();
                for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                {
                    QString t = start;
                    t += *i;
                    toPush(ret, t);
                }
            }
        }
        break;
        case NewDatafile:
        {
            QString start = QString::fromLatin1("ALTER TABLESPACE \"");
            start += TablespaceOrig;
            start += QString::fromLatin1("\" ADD ");
            std::list<QString> lst = Datafile->sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
            {
                start += QString::fromLatin1(" ");
                start += *i;
            }
            toPush(ret, start);
        }
        break;
        case NewTablespace:
        {
            QString start = QString::fromLatin1("CREATE TABLESPACE \"");
            start += Datafile->getName().toUpper();
            start += QString::fromLatin1("\" ");
            std::list<QString> lst = Datafile->sql();
            {
                for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                {
                    start += QString::fromLatin1(" ");
                    start += *i;
                }
            }
            lst = Tablespace->sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
            {
                start += QString::fromLatin1(" ");
                start += *i;
            }
            if (Tablespace->allowStorage())
            {
                start += QString::fromLatin1(" DEFAULT");
                lst = Default->sql();
                for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                {
                    start += QString::fromLatin1(" ");
                    start += *i;
                }
            }
            toPush(ret, start);
        }
        }
        return ret;
    }
    catch (const QString &str)
    {
        toStatusMessage(str);
        std::list<QString> ret;
        return ret;
    }
}

static toStorageTool StorageTool;

toStorage::toStorage(QWidget *main, toConnection &connection)
        : toToolWidget(StorageTool, "storage.html", main, connection, "toStorage")
{
    QToolBar *toolbar = toAllocBar(this, tr("Storage manager"));
    layout()->addWidget(toolbar);

    UpdateAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                            tr("Update"), this);
    connect(UpdateAct, SIGNAL(triggered()), this, SLOT(refresh()));
    UpdateAct->setShortcut(QKeySequence::Refresh);
    toolbar->addAction(UpdateAct);

    toolbar->addSeparator();

    ExtentAct = new QAction(QPixmap(const_cast<const char**>(storageextents_xpm)),
                            tr("Show extent view."), this);
    ExtentAct->setCheckable(true);
    bool extents = toConfigurationSingle::Instance().dispExtents();
    if (extents)
        ExtentAct->setChecked(true);
    toolbar->addAction(ExtentAct);
    connect(ExtentAct, SIGNAL(toggled(bool)), this, SLOT(showExtent(bool)));
    ExtentAct->setToolTip(tr("Show extent view."));

    TablespaceAct = new QAction(QPixmap(const_cast<const char**>(tostorage_xpm)),
                                tr("Show tablespaces or just datafiles."), this);
    TablespaceAct->setCheckable(true);
    bool tablespaces = toConfigurationSingle::Instance().dispTablespaces();
    if (tablespaces)
        TablespaceAct->setChecked(true);
    toolbar->addAction(TablespaceAct);
    connect(TablespaceAct, SIGNAL(toggled(bool)), this, SLOT(showTablespaces(bool)));
    TablespaceAct->setToolTip(tr("Show tablespaces or just datafiles."));

    toolbar->addSeparator();

    OnlineAct = new QAction(QPixmap(const_cast<const char**>(online_xpm)),
                            tr("Take tablespace online"), this);
    connect(OnlineAct, SIGNAL(triggered()), this, SLOT(online(void)));
    toolbar->addAction(OnlineAct);

    OfflineAct = new QAction(QPixmap(const_cast<const char**>(offline_xpm)),
                             tr("Take tablespace offline"), this);
    connect(OfflineAct, SIGNAL(triggered()), this, SLOT(offline(void)));
    toolbar->addAction(OfflineAct);

    toolbar->addSeparator();

    LoggingAct = new QAction(QPixmap(const_cast<const char**>(logging_xpm)),
                             tr("Set tablespace default to logging"), this);
    connect(LoggingAct, SIGNAL(triggered()), this, SLOT(logging(void)));
    toolbar->addAction(LoggingAct);

    EraseLogAct = new QAction(QPixmap(const_cast<const char**>(eraselog_xpm)),
                              tr("Set tablespace default to no logging"), this);
    connect(EraseLogAct, SIGNAL(triggered()), this, SLOT(noLogging(void)));
    toolbar->addAction(EraseLogAct);

    toolbar->addSeparator();

    ReadWriteAct = new QAction(QPixmap(const_cast<const char**>(writetablespace_xpm)),
                               tr("Allow read write access to tablespace"), this);
    connect(ReadWriteAct, SIGNAL(triggered()), this, SLOT(readWrite(void)));
    toolbar->addAction(ReadWriteAct);

    ReadOnlyAct = new QAction(QPixmap(const_cast<const char**>(readtablespace_xpm)),
                              tr("Set tablespace to read only"), this);
    connect(ReadOnlyAct, SIGNAL(triggered()), this, SLOT(readOnly(void)));
    toolbar->addAction(ReadOnlyAct);

    toolbar->addSeparator();

    ModTablespaceAct = new QAction(QPixmap(const_cast<const char**>(modtablespace_xpm)),
                                   tr("Modify tablespace"), this);
    connect(ModTablespaceAct, SIGNAL(triggered()), this, SLOT(modifyTablespace(void)));
    toolbar->addAction(ModTablespaceAct);

    DropTablespaceAct = new QAction(QPixmap(const_cast<const char**>(droptablespace_xpm)),
                                    tr("Drop tablespace"), this);
    connect(DropTablespaceAct, SIGNAL(triggered()), this, SLOT(dropTablespace(void)));
    toolbar->addAction(DropTablespaceAct);

    ModFileAct = new QAction(QPixmap(const_cast<const char**>(modfile_xpm)),
                             tr("Modify file"), this);
    connect(ModFileAct, SIGNAL(triggered()), this, SLOT(modifyDatafile(void)));
    toolbar->addAction(ModFileAct);

    toolbar->addSeparator();

    CreateTablespaceAct = new QAction(QPixmap(const_cast<const char**>(addtablespace_xpm)),
                                      tr("Create new tablespace"), this);
    connect(CreateTablespaceAct, SIGNAL(triggered()), this, SLOT(newTablespace(void)));
    toolbar->addAction(CreateTablespaceAct);

    NewFileAct = new QAction(QPixmap(const_cast<const char**>(addfile_xpm)),
                             tr("Add datafile to tablespace"), this);
    connect(NewFileAct, SIGNAL(triggered()), this, SLOT(newDatafile(void)));
    toolbar->addAction(NewFileAct);

    toolbar->addSeparator();

    CoalesceAct = new QAction(QPixmap(const_cast<const char**>(coalesce_xpm)),
                              tr("Coalesce tablespace"), this);
    connect(CoalesceAct, SIGNAL(triggered()), this, SLOT(coalesce(void)));
    toolbar->addAction(CoalesceAct);

    MoveFileAct = new QAction(QPixmap(const_cast<const char**>(movefile_xpm)),
                              tr("Move datafile"), this);
    connect(MoveFileAct, SIGNAL(triggered()), this, SLOT(moveFile(void)));
    toolbar->addAction(MoveFileAct);

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    layout()->addWidget(splitter);

    Storage = new toResultStorage(toConfigurationSingle::Instance().dispAvailableGraph(),
                                  splitter);
    ExtentParent = new QSplitter(Qt::Horizontal, splitter);
    Objects = new QTableView(ExtentParent);
    ObjectsModel = new toStorageObjectModel(this);
    QSortFilterProxyModel * proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(ObjectsModel);
    Objects->setModel(proxyModel);
    Objects->setSortingEnabled(true);
    Objects->setSelectionBehavior(QAbstractItemView::SelectRows);
    Objects->setSelectionMode(QAbstractItemView::SingleSelection);

    Extents = new toStorageExtent(ExtentParent);
    Storage->setSelectionMode(toTreeWidget::Single);

    connect(Objects->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(selectObject(const QModelIndex &, const QModelIndex &)));

    if (!extents)
        ExtentParent->hide();

    if (!tablespaces)
        Storage->setOnlyFiles(true);

    connect(Storage, SIGNAL(selectionChanged(void)), this, SLOT(selectionChanged(void)));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    refresh();
    selectionChanged();
    setFocusProxy(Storage);
}


void toStorage::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Storage"), this);

            ToolMenu->addAction(UpdateAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(OnlineAct);
            ToolMenu->addAction(OfflineAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(LoggingAct);
            ToolMenu->addAction(EraseLogAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(ReadWriteAct);
            ToolMenu->addAction(ReadOnlyAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(DropTablespaceAct);
            ToolMenu->addAction(ModTablespaceAct);
            ToolMenu->addAction(ModFileAct);
            ToolMenu->addAction(NewFileAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(CoalesceAct);
            ToolMenu->addAction(MoveFileAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toStorage::refresh(void)
{
    Storage->showCoalesced(toConfigurationSingle::Instance().dispCoalesced());
    Storage->query();
}

void toStorage::coalesce(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" COALESCE"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::online(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" ONLINE"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::logging(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" LOGGING"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::noLogging(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" NOLOGGING"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::readOnly(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        ;
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" READ ONLY"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::readWrite(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" READ WRITE"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::offline(void)
{
    try
    {
        QString reason;
        QString dsc = tr("Select mode to take tablespace %1 offline.").arg(Storage->currentTablespace());
        switch (TOMessageBox::information(this,
                                          tr("Tablespace offline"),
                                          dsc,
                                          tr("Normal"), tr("Temporary"), tr("Cancel")))
        {
        case 0:
            reason = QString::fromLatin1("NORMAL");
            break;
        case 1:
            reason = QString::fromLatin1("TEMPORARY");
            break;
        default:
            return ;
        }
        QString str;
        str = QString::fromLatin1("ALTER TABLESPACE \"");
        str.append(Storage->currentTablespace());
        str.append(QString::fromLatin1("\" OFFLINE "));
        str.append(reason);
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toStorage::selectionChanged(void)
{
    OfflineAct->setEnabled(false);
    OnlineAct->setEnabled(false);
    CoalesceAct->setEnabled(false);
    LoggingAct->setEnabled(false);
    EraseLogAct->setEnabled(false);
    ModTablespaceAct->setEnabled(false);
    DropTablespaceAct->setEnabled(false);
    NewFileAct->setEnabled(false);
    MoveFileAct->setEnabled(false);
    ModFileAct->setEnabled(false);
    ReadOnlyAct->setEnabled(false);
    ReadWriteAct->setEnabled(false);

    toTreeWidgetItem *item = Storage->selectedItem();
    if (item)
    {
        if (item->parent() || Storage->onlyFiles())
        {
            if (!ExtentParent->isHidden())
                Extents->setFile(item->text(12), item->text(13).toInt());
            item = item->parent();
            MoveFileAct->setEnabled(true);
            ModFileAct->setEnabled(true);
        }
        else if (!ExtentParent->isHidden())
            Extents->setTablespace(item->text(0));

        if (!ExtentParent->isHidden())
        {
            ObjectsModel->setValues(Extents->objects());
            Objects->resizeColumnsToContents();
            Objects->resizeRowsToContents();
        }

        if (item)
        {
            toTreeWidgetItem *child = item->firstChild();
            if (!child)
            {
                OnlineAct->setEnabled(true);
                OfflineAct->setEnabled(true);
                ReadWriteAct->setEnabled(true);
                ReadOnlyAct->setEnabled(true);
            }
            else
            {
                if (child->text(1) == QString::fromLatin1("OFFLINE"))
                    OnlineAct->setEnabled(true);
                else if (child->text(1) == QString::fromLatin1("ONLINE"))
                {
                    OfflineAct->setEnabled(true);
                    if (child->text(2) == QString::fromLatin1("READ ONLY"))
                        ReadWriteAct->setEnabled(true);
                    else
                        ReadOnlyAct->setEnabled(true);
                }
            }
            if (item->text(4) == QString::fromLatin1("LOGGING"))
                EraseLogAct->setEnabled(true);
            else
                LoggingAct->setEnabled(true);

            if (item->text(10) != QString::fromLatin1("100%"))
                CoalesceAct->setEnabled(true);
        }
        NewFileAct->setEnabled(true);
        ModTablespaceAct->setEnabled(true);
        DropTablespaceAct->setEnabled(true);
    }
    if (ToolMenu)
    {
#if 0                           // todo
        ToolMenu->setItemEnabled(TO_ID_ONLINE, OnlineButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_OFFLINE, OfflineButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_LOGGING, LoggingButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_NOLOGGING, EraseLogButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_READ_WRITE, ReadWriteButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_READ_ONLY, ReadOnlyButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_MODIFY_TABLESPACE,
                                 ModTablespaceButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_DROP_TABLESPACE,
                                 DropTablespaceButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_MODIFY_DATAFILE,
                                 ModFileButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_ADD_DATAFILE, NewFileButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_COALESCE, CoalesceButton->isEnabled());
        ToolMenu->setItemEnabled(TO_ID_MOVE_FILE, MoveFileButton->isEnabled());
#endif
    }
}

void toStorage::newDatafile(void)
{
    try
    {
        toStorageDialog newFile(Storage->currentTablespace(), this);

        if (newFile.exec())
        {
            std::list<QString> lst = newFile.sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                connection().execute(*i);

            refresh();
        }
    }
    TOCATCH
}

void toStorage::newTablespace(void)
{
    try
    {
        toStorageDialog newSpace(QString::null, this);

        if (newSpace.exec())
        {
            std::list<QString> lst = newSpace.sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                connection().execute(*i);

            refresh();
        }
    }
    TOCATCH
}

void toStorage::dropTablespace(void)
{
    try
    {
        toStorageDialog dropSpace(Storage->currentTablespace(), this, true);

        if (dropSpace.exec())
        {
            std::list<QString> lst = dropSpace.sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                connection().execute(*i);

            refresh();
        }
    }
    TOCATCH
}

void toStorage::modifyTablespace(void)
{
    try
    {
        toStorageDialog modifySpace(connection(), Storage->currentTablespace(), this);

        if (modifySpace.exec())
        {
            std::list<QString> lst = modifySpace.sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                connection().execute(*i);

            refresh();
        }
    }
    TOCATCH
}

void toStorage::modifyDatafile(void)
{
    try
    {
        toStorageDialog modifySpace(connection(), Storage->currentTablespace(),
                                    Storage->currentFilename(), this);

        if (modifySpace.exec())
        {
            std::list<QString> lst = modifySpace.sql();
            for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                connection().execute(*i);

            refresh();
        }
    }
    TOCATCH
}

void toStorage::moveFile(void)
{
    try
    {
        QString orig = Storage->currentFilename();
        QString file = TOFileDialog::getSaveFileName(this, tr("Rename Data File"), orig, QString::fromLatin1("*.dbf"));
        if (!file.isEmpty() && file != orig)
        {
            QString str;
            str = QString::fromLatin1("ALTER TABLESPACE \"");
            str.append(Storage->currentTablespace());
            str.append(QString::fromLatin1("\" RENAME DATAFILE '"));
            orig.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("''"));
            file.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("''"));
            str.append(orig);
            str.append(QString::fromLatin1("' TO '"));
            str.append(file);
            str.append(QString::fromLatin1("'"));
            connection().execute(str);
            refresh();
        }
    }
    TOCATCH
}

void toStorage::selectObject(const QModelIndex & current, const QModelIndex &)
{
    QModelIndex ix = current;
    if (ix.isValid())
    {
//      toStorageExtent::extentTotal ce = ObjectsModel->values().at(ix.row());
        toStorageExtent::extentTotal ce = ObjectsModel->values().at(
                ix.model()->headerData(ix.row(), Qt::Vertical).toInt()-1);
        Extents->highlight(ce.Owner, ce.Table, ce.Partition);
    }
}

void toStorage::showExtent(bool ena)
{
    if (ena)
    {
        ExtentParent->show();
        selectionChanged();
    }
    else
    {
        ExtentParent->hide();
    }
}

void toStorage::showTablespaces(bool tab)
{
    Storage->setOnlyFiles(!tab);
}


toStorageObjectModel::toStorageObjectModel(QObject * parent)
    : QAbstractTableModel(parent)
{
    m_values.clear();
    HeaderData << tr("Owner") << tr("Segment") << tr("Partition") << tr("Extents") << tr("Blocks");
}

toStorageObjectModel::~toStorageObjectModel()
{
}

void toStorageObjectModel::setValues(std::list<toStorageExtent::extentTotal> values)
{
    m_values.clear();
    for (std::list<toStorageExtent::extentTotal>::iterator i = values.begin();i != values.end(); ++i)
        m_values.append((*i));
    reset();
}

int toStorageObjectModel::rowCount(const QModelIndex & parent) const
{
    return m_values.size();
}

int toStorageObjectModel::columnCount(const QModelIndex & parent)  const
{
    return 5;
}

QVariant toStorageObjectModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 3 || index.column() == 4)
            return Qt::AlignRight;
        else
            return Qt::AlignLeft;
    }

    if (role == Qt::DisplayRole)
    {
        toStorageExtent::extentTotal e = m_values.at(index.row());
        switch (index.column())
        {
            case 0: return e.Owner; break;
            case 1: return e.Table; break;
            case 2: return e.Partition; break;
            case 3: return e.Extents; break;
            case 4: return e.Size; break;
            default: return "Never should go here";
        };
    };
    // safe return
    return QVariant();
}

Qt::ItemFlags toStorageObjectModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant toStorageObjectModel::headerData(int section, Qt::Orientation orientation, int role)  const
{
    if (role != Qt::DisplayRole)
         return QVariant();
    if (orientation == Qt::Horizontal)
        return HeaderData.at(section);
    else
        return QString("%1").arg(section + 1);
}
