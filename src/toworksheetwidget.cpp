/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
