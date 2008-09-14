
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

/*
* mrjohnson: this file was a split from toworksheet, which used to
* have a Light boolean that it used to trim down the editor to be
* suitable for adding to other tools. this is better.
*/

#include "config.h"

#include "utils.h"
#include "toworksheetwidget.h"
#include "toworksheettext.h"
#include "toresulttableview.h"
#include "tomain.h"


toWorksheetWidget::toWorksheetWidget(QWidget *main,
                                     const char *name,
                                     toConnection &connection)
        : toWorksheet(main, name, connection)
{

    setup(false);
}


void toWorksheetWidget::setup(bool autoLoad)
{
    QToolBar *workToolbar = toAllocBar(this, tr("Worksheet Widget"));
    layout()->addWidget(workToolbar);

    workToolbar->addAction(executeAct);
    workToolbar->addAction(executeStepAct);
    workToolbar->addAction(executeAllAct);
    workToolbar->addAction(refreshAct);

    RefreshSeconds = 60;
    connect(&RefreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    Editor = new toWorksheetText(this, this);
    layout()->addWidget(Editor);

    Current = Result = new toResultTableView(false, true, this);
    layout()->addWidget(Result);
    Result->hide();

    connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
    connect(
        Result,
        SIGNAL(firstResult(
                   const QString &,
                   const toConnection::exception &, bool)),
        this,
        SLOT(addLog(
                 const QString &,
                 const toConnection::exception &, bool)));

    ResultTab    = NULL;
    Plan         = NULL;
    CurrentTab   = NULL;
    Resources    = NULL;
    Statistics   = NULL;
    Logging      = NULL;
    LastLogItem  = NULL;
    StatTab      = NULL;
    Columns      = NULL;
    Refresh      = NULL;
    ToolMenu     = NULL;
    Visualize    = NULL;
    WaitChart    = NULL;
    IOChart      = NULL;
    EditSplitter = NULL;

    workToolbar->addSeparator();

    workToolbar->addAction(stopAct);
    stopAct->setEnabled(false);

    workToolbar->addWidget(Started = new QLabel(workToolbar));
    Started->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    context = NULL;
    Editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Editor,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(createPopupMenu(const QPoint &)));
}


void toWorksheetWidget::createPopupMenu(const QPoint &pos)
{
    if (!context)
    {
        context = new QMenu;

        context->addAction(executeAct);
        context->addAction(executeStepAct);
        context->addAction(executeAllAct);
        context->addAction(executeNewlineAct);
        context->addAction(refreshAct);

        context->addSeparator();

        context->addAction(toMainWidget()->getUndoAction());
        context->addAction(toMainWidget()->getRedoAction());

        context->addSeparator();

        context->addAction(toMainWidget()->getCutAction());
        context->addAction(toMainWidget()->getCopyAction());
        context->addAction(toMainWidget()->getPasteAction());

        context->addSeparator();

        context->addAction(toMainWidget()->getSelectAllAction());

        context->addSeparator();

        context->addAction(stopAct);
    }

    context->exec(QCursor::pos());
}


bool toWorksheetWidget::describe(const QString &query)
{
    try
    {
        QRegExp white(QString::fromLatin1("[ \r\n\t.]+"));
        QStringList part = query.split(white);
        if (part[0].toUpper() == QString::fromLatin1("DESC") ||
                part[0].toUpper() == QString::fromLatin1("DESCRIBE"))
        {
            return true;
        }
        else
            return false;
    }
    TOCATCH;

    return false;
}

bool toWorksheetWidget::checkSave(bool input)
{
    return false;
}


// override parent.
void toWorksheetWidget::windowActivated(QWidget *widget)
{
    return;
}
