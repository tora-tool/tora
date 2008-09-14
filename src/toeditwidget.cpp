/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "toeditwidget.h"
#include "tomain.h"

std::list<toEditWidget::editHandler *> *toEditWidget::Handlers;

void toEditWidget::setMainSettings(void)
{
    toMain::editEnable(this);
}

toEditWidget::toEditWidget()
{
    Open = Save = Print = Undo = Redo = Cut = Copy = Paste = Search = SelectAll = ReadAll = false;
}

toEditWidget::toEditWidget(bool open, bool save, bool print,
                           bool undo, bool redo,
                           bool cut, bool copy, bool paste,
                           bool search,
                           bool selectAll, bool readAll)
{
    Open = open;
    Save = save;
    Print = print;
    Undo = undo;
    Redo = redo;
    Cut = cut;
    Copy = copy;
    Paste = paste;
    Search = search;
    SelectAll = selectAll;
    ReadAll = readAll;
}

void toEditWidget::setEdit(bool open, bool save, bool print,
                           bool undo, bool redo,
                           bool cut, bool copy, bool paste,
                           bool search,
                           bool selectAll, bool readAll)
{
    Open = open;
    Save = save;
    Print = print;
    Undo = undo;
    Redo = redo;
    Cut = cut;
    Copy = copy;
    Paste = paste;
    Search = search;
    SelectAll = selectAll;
    ReadAll = readAll;

    setMainSettings();
}

toEditWidget::~toEditWidget()
{
    toMain::editDisable(this);
}

void toEditWidget::addHandler(toEditWidget::editHandler *handler)
{
    if (!Handlers)
        Handlers = new std::list<toEditWidget::editHandler *>;
    for (std::list<toEditWidget::editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
        if (*i == handler)
            return ;
    (*Handlers).insert((*Handlers).end(), handler);
}

void toEditWidget::delHandler(toEditWidget::editHandler *handler)
{
    for (std::list<toEditWidget::editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
        if (*i == handler)
        {
            (*Handlers).erase(i);
            break;
        }
}

void toEditWidget::lostFocus(void)
{
    if (!Handlers)
        return ;
    for (std::list<editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
    {
        (*i)->lostFocus(this);
    }
}

void toEditWidget::receivedFocus(void)
{
    toMain::setEditWidget(this);

    if (!Handlers)
        return ;
    for (std::list<editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
    {
        (*i)->receivedFocus(this);
    }
}

toEditWidget::editHandler::editHandler()
{
    toEditWidget::addHandler(this);
}

toEditWidget::editHandler::~editHandler()
{
    toEditWidget::delHandler(this);
}

bool toEditWidget::editOpen(QString)
{
    return true;
}

bool toEditWidget::editSave(bool)
{
    return true;
}

bool toEditWidget::searchNext(toSearchReplace *)
{
    return false;
}

void toEditWidget::searchReplace(const QString &)
{}

bool toEditWidget::searchCanReplace(bool)
{
    return false;
}
