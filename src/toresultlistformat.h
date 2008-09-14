/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTLISTFORMAT_H
#define TORESULTLISTFORMAT_H

#include "ui_toresultlistformatui.h"

#include "config.h"

#include <QDialog>


/**
 * Used internally by toListView.
 *
 * @internal
 */
class toResultListFormat : public QDialog, public Ui::toResultListFormatUI
{
    Q_OBJECT;

public:
    toResultListFormat(QWidget *parent, const char *name);
    void saveDefault(void);

public slots:
    virtual void formatChanged(int pos);
};

#endif
