/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOEDITABLEMENU_H
#define TOEDITABLEMENU_H

#include "config.h"

#include <QMenu>
#include <QKeyEvent>


/**
 * A QMenu subclass with editable entries.
 *
 */
class toEditableMenu : public QMenu {
    Q_OBJECT;

    QMenu   *Context;
    QAction *Remove;

public:
    toEditableMenu(QWidget *parent = 0);
    virtual ~toEditableMenu() {
    }

signals:
    void actionRemoved(QAction *action);

protected slots:
    void remove(void);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

};

#endif
