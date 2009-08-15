
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

#include "utils.h"
#include "toviewdirectory.h"
#include "tomain.h"
#include "toworksheet.h"
#include "tohighlightedtext.h"
#include "totool.h"
#include "toconfiguration.h"

#include <QMdiArea>
#include <QMdiSubWindow>


REGISTER_VIEW("Directory", toViewDirectory);


toViewDirectory::toViewDirectory(QWidget *parent,
                                 Qt::WindowFlags flags)
    : toDocklet(tr("Directory"), parent, flags)
{
    setObjectName("Directory Docklet");

    ListView = new QListView(this);
    setFocusProxy(ListView);

    Model = new QDirModel;
    Model->setSorting(QDir::DirsFirst | QDir::Name);
    Model->setFilter(QDir::AllEntries);
    ListView->setModel(Model);

    connect(ListView,
            SIGNAL(activated(const QModelIndex &)),
            this,
            SLOT(handleActivated(const QModelIndex &)));

    connect(toMainWidget(),
            SIGNAL(addedToolWidget(toToolWidget *)),
            this,
            SLOT(handleToolAdded(toToolWidget *)));

    connect(toMainWidget()->workspace(),
            SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this,
            SLOT(windowActivated(QMdiSubWindow *)));

    setWidget(ListView);

    // default to the default file in worksheet editor if specified
    QString autoload = toConfigurationSingle::Instance().wsAutoLoad();
    if(!autoload.isEmpty())
        showFile(autoload);
    else
        findRoot(QDir::currentPath());
}


QIcon toViewDirectory::icon() const
{
    return style()->standardIcon(QStyle::SP_ComputerIcon);
}


QString toViewDirectory::name() const
{
    return tr("Directory");
}


void toViewDirectory::findRoot(QFileInfo dir) {
    if(!dir.isDir())
        dir = QFileInfo(dir.absoluteDir().absolutePath());

    QModelIndex parent = Model->index(dir.absoluteFilePath());
    ListView->setRootIndex(parent);
    Model->refresh(parent);
}


void toViewDirectory::handleActivated(const QModelIndex &index) {
    if(!index.isValid())
        return;

    QFileInfo info = Model->fileInfo(index);
    if(info.isDir())
        findRoot(info);
    else if(info.isFile())
    {
        toMainWidget()->editOpenFile(info.absoluteFilePath());
        emit activated();
    }
}


void toViewDirectory::handleToolAdded(toToolWidget *tool) {
    toWorksheet *sheet = dynamic_cast<toWorksheet *>(tool);
    if(!sheet)
        return;

    // just in case we already connected to this one
    disconnect(sheet->editor(),
               0,
               this,
               0);

    connect(sheet->editor(),
            SIGNAL(fileOpened(QString)),
            this,
            SLOT(showFile(QString)));

    connect(sheet->editor(),
            SIGNAL(fileSaved(QString)),
            this,
            SLOT(showFile(QString)));
}


void toViewDirectory::showFile(QString file) {
    QFileInfo info(file);

    findRoot(info);
    if(!info.exists())
        return;

    QModelIndex index = Model->index(file);
    if(!index.isValid())
        return;

    ListView->selectionModel()->select(QItemSelection(index, index),
                                       QItemSelectionModel::ClearAndSelect);
    ListView->setCurrentIndex(index);
}


void toViewDirectory::windowActivated(QMdiSubWindow *w)
{
    if(!w || !w->widget())
        return;

    toWorksheet *sheet = dynamic_cast<toWorksheet *>(w->widget());
    if(!sheet)
        return;

    QString file = sheet->editor()->filename();
    if(!file.isEmpty())
        showFile(file);
}
