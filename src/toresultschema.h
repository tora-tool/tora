/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTSCHEMA_H
#define TORESULTSCHEMA_H

#include "config.h"

#include "toresultcombo.h"

class toConnection;


/**
 * This widget displays a list of schemas
 *
 */
class toResultSchema : public toResultCombo
{
    Q_OBJECT;

    QString ConnectionKey;

public:

    /**
     * Create the widget.
     *
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultSchema(toConnection &conn,
                   QWidget *parent,
                   const char *name = NULL);

private slots:

    // stores last schema selected in qsettings
    void updateLastSchema(const QString &schema);

public slots:

    /**
     * Updates connections to use schema
     *
     */
    void update(const QString &schema);


    /**
     * Update to currently selected schema
     *
     */
    void update(void);
};

#endif
