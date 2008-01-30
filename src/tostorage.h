/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOSTORAGE_H
#define TOSTORAGE_H

#include "config.h"
#include "ui_tostoragedatafileui.h"
#include "ui_tostoragedialogui.h"
#include "ui_tostoragetablespaceui.h"
#include "ui_todroptablespaceui.h"
#include "totool.h"
#include "toresultstorage.h"

#include <list>

#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>

class QCheckBox;
class Q3GroupBox;
class QLabel;
class QLineEdit;
class Q3PopupMenu;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QSplitter;
class QTabWidget;
class toConnection;
class toFilesize;
class toResultStorage;
class toResultView;
class toStorageDefinition;
class toStorageDialog;


class toStorageTablespace : public QWidget, public Ui::toStorageTablespaceUI
{
    Q_OBJECT

    bool Modify;
    bool LoggingOrig;
    bool OnlineOrig;
    bool PermanentOrig;

public:
    toStorageTablespace(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

    std::list<QString> sql(void);
    bool allowStorage(void);

    friend class toStorageDialog;
signals:
    void allowStorage(bool);
    void tempFile(bool);

public slots:
    virtual void permanentToggle(bool);
    virtual void dictionaryToggle(bool);
    virtual void uniformToggle(bool);
    virtual void allowDefault(bool);
};

class toDropTablespace : public QWidget, public Ui::toDropTablespaceUI
{
    Q_OBJECT
public:
    toDropTablespace(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);
    std::list<QString> sql(void);
signals:
    void validContent(bool);
};

class toStorageDatafile : public QWidget, public Ui::toStorageDatafileUI
{
    Q_OBJECT

    bool Modify;
    int InitialSizeOrig;
    int NextSizeOrig;
    int MaximumSizeOrig;
    QString FilenameOrig;
    bool Tempfile;

public:
    toStorageDatafile(bool tempFile, bool dispName,
                      QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

    QString getName(void);
    std::list<QString> sql();

    friend class toStorageDialog;
signals:
    void validContent(bool);

public slots:
    virtual void browseFile(void);
    virtual void autoExtend(bool);
    virtual void setTempFile(bool temp)
    {
        Tempfile = temp;
    }

    virtual void maximumSize(bool);
    virtual void valueChanged(const QString &);
};

class toStorageDialog : public QDialog, public Ui::toStorageDialogUI
{
    Q_OBJECT
    enum
    {
        NewTablespace,
        NewDatafile,
        ModifyTablespace,
        ModifyDatafile,
        DropTablespace
    } Mode;
    QString TablespaceOrig;
    void Setup(void);
public:
    toStorageDatafile *Datafile;
    toStorageTablespace *Tablespace;
    toStorageDefinition *Default;
    toDropTablespace *Drop;

    toStorageDialog(toConnection &conn, const QString &Tablespace, QWidget *parent);
    toStorageDialog(toConnection &conn, const QString &Tablespace,
                    const QString &file, QWidget *parent);
    toStorageDialog(const QString &tablespace, QWidget *parent, bool drop = false);

    std::list<QString> sql(void);
public slots:
    void validContent(bool val);
    void allowStorage(bool val);
    virtual void displaySQL(void);
};

class toStorage : public toToolWidget
{
    Q_OBJECT

    toResultStorage *Storage;

    toListView *Objects;
    toStorageExtent *Extents;
    QSplitter *ExtentParent;

    Q3PopupMenu *ToolMenu;

    QAction * UpdateAct;
    QAction *OnlineAct;
    QAction *OfflineAct;
    QAction *CoalesceAct;
    QAction *LoggingAct;
    QAction *EraseLogAct;
    QAction *ModTablespaceAct;
    QAction *ReadOnlyAct;
    QAction *ReadWriteAct;
    QAction *NewFileAct;
    QAction *MoveFileAct;
    QAction *ModFileAct;
    QAction *ExtentAct;
    QAction *TablespaceAct;
    QAction *DropTablespaceAct;
    QAction * CreateTablespaceAct;

public:
    toStorage(QWidget *parent, toConnection &connection);

public slots:
    void refresh(void);

    void coalesce(void);

    void online(void);
    void offline(void);

    void logging(void);
    void noLogging(void);

    void readWrite(void);
    void readOnly(void);

    void newDatafile(void);
    void newTablespace(void);
    void modifyTablespace(void);
    void modifyDatafile(void);
    void moveFile(void);
    void dropTablespace(void);

    void showExtent(bool);
    void showTablespaces(bool);
    void selectionChanged(void);
    void selectObject(void);
    void windowActivated(QWidget *widget);
};

#endif
