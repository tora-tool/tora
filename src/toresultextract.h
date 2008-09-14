/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTEXTRACT_H
#define TORESULTEXTRACT_H

#include "config.h"
#include "toresult.h"
#include "toworksheetwidget.h"

class toSQL;

/** This widget displays the SQL used to recreate an object. It uses the
 * @ref toExtract class to do this.
 */

class toResultExtract : public toWorksheetWidget, public toResult
{
    Q_OBJECT

    bool Prompt;

public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultExtract(bool prompt, QWidget *parent, const char *name = NULL);

    /** Ignores sql and extect object name and owner as parameters.
     */
    virtual void query(const QString &sql, const toQList &param);

    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        toResult::refresh();
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1)
    {
        toResult::changeParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2)
    {
        toResult::changeParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2, const QString &Param3)
    {
        toResult::changeParams(Param1, Param2, Param3);
    }
};

#endif
