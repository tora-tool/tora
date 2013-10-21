
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

/*
* mrjohnson: this file was a split from toworksheet, which used to
* have a Light boolean that it used to trim down the editor to be
* suitable for adding to other tools. this is better.
*/

#include "tools/toworksheetwidget.h"
#include "tools/toworksheettext.h"
#include "core/utils.h"
#include "core/toresulttableview.h"
#include "core/toeditmenu.h"

toWorksheetWidget::toWorksheetWidget(QWidget *main,
                                     const char *name,
                                     toConnection &connection)
    : toWorksheet(main, name, connection)
{

    setup(false);
}


void toWorksheetWidget::setup(bool autoLoad)
{
    QToolBar *workToolbar = Utils::toAllocBar(this, tr("Worksheet Widget"));
    layout()->addWidget(workToolbar);

    workToolbar->addAction(executeAct);
    workToolbar->addAction(executeStepAct);
    workToolbar->addAction(executeAllAct);
    workToolbar->addAction(refreshAct);

    RefreshSeconds = 60;
    connect(&RefreshTimer, SIGNAL(timeout()), this, SLOT(slotRefresh()));

    Editor = new toWorksheetText(this, this);
    layout()->addWidget(Editor);

    Current = Result = new toResultTableView(false, true, this);
    layout()->addWidget(Result);
    Result->hide();

    connect(Result, SIGNAL(done(void)), this, SLOT(slotQueryDone(void)));
    connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
            this, SLOT(slotAddLog( const QString &, const toConnection::exception &, bool)));

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
    //obsolete Visualize    = NULL;
#ifdef TORA3_GRAPH    
    WaitChart    = NULL;
    IOChart      = NULL;
#endif    
    EditSplitter = NULL;

    workToolbar->addSeparator();

    workToolbar->addAction(stopAct);
    stopAct->setEnabled(false);

    workToolbar->addWidget(Started = new QLabel(workToolbar));
    Started->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    context = NULL;
    Editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Editor, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotCreatePopupMenu(const QPoint &)));
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

        context->addAction(toEditMenuSingle::Instance().undoAct);
        context->addAction(toEditMenuSingle::Instance().redoAct);

        context->addSeparator();

        context->addAction(toEditMenuSingle::Instance().cutAct);
        context->addAction(toEditMenuSingle::Instance().copyAct);
        context->addAction(toEditMenuSingle::Instance().pasteAct);

        context->addSeparator();

        context->addAction(toEditMenuSingle::Instance().selectAllAct);

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
