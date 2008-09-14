/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOWORKSHEET_WIDGET_H
#define TOWORKSHEET_WIDGET_H

#include "config.h"

#include "toworksheet.h"


class toWorksheetWidget : public toWorksheet
{
    Q_OBJECT

public:
    toWorksheetWidget(QWidget *parent,
                      const char *name,
                      toConnection &connection);

    void setup(bool autoLoad);
    void windowActivated(QWidget *widget);
    virtual bool describe(const QString &query);

    /*! \brief Prevent unecessary "save changes to editor"
    that don't make much sense.
    toWorksheetWidget objects are used for diplaying data
    only - so no saving is required at all.
    This method cuts off toWorksheet::checkSave()s
    */
    bool checkSave(bool input);

public slots:
    /**
     * create context menus
     *
     */
    void createPopupMenu(const QPoint &pos);
};

#endif
