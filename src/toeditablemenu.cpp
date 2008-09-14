/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "toeditablemenu.h"
#include <QAction>
#include <QContextMenuEvent>

#include "icons/trash.xpm"


toEditableMenu::toEditableMenu(QWidget *parent) : QMenu(parent),
                                                  Context(0) {
}


void toEditableMenu::keyPressEvent(QKeyEvent *event) {
    if(event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Delete)
        remove();
}


void toEditableMenu::contextMenuEvent(QContextMenuEvent *event) {
    event->accept();

    if(!Context) {
        Context = new QMenu(this);

        Remove = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                             tr("Remove"),
                             this);
        Remove->setShortcut(QKeySequence::Delete);
        Context->addAction(Remove);

        connect(Remove,
                SIGNAL(triggered()),
                this,
                SLOT(remove()));
    }

    Context->exec(QCursor::pos());
}


void toEditableMenu::remove() {
    QAction *active = this->activeAction();
    if(!active)
        return;

    emit actionRemoved(active);
    this->removeAction(active);
}
