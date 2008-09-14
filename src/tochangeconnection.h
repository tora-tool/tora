/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCHANGECONNECTION_H
#define TOCHANGECONNECTION_H

#include "config.h"
#include "utils.h"

class QAction;


/**
 * This button will allow you to change the connection for the nearest
 * toToolWidget parent.
 */
class toChangeConnection : public toPopupButton
{
    Q_OBJECT;

private slots:
    void changeConnection(QAction *);
    void popupMenu(void);

public:

    /** Create the button, only the toolbar parent is needed.
     * @param parent Toolbar to put connection in.
     * @param name Name of widget.
     */
    toChangeConnection(QToolBar *parent, const char *name = 0);
};

#endif
