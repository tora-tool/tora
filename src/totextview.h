/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTEXTVIEW_H
#define TOTEXTVIEW_H

#include "config.h"

#include "toeditwidget.h"

#include <QTextBrowser>


/** A tora editwidget version of the @ref QTextView widget.
 */

class toTextView : public QTextBrowser, public toEditWidget
{
public:
    /** See @ref QTextView
     */
    toTextView(QWidget *parent = 0, const char *name = 0)
            : QTextBrowser(parent)//, name)
            , toEditWidget(false, true, false,
                           false, false,
                           false, true, false,
                           false, true, false)
    { }
    /** See @ref QTextView
     */
//     toTextView(const QString&text, const QString &context = QString::null,
//                QWidget *parent = 0, const char *name = 0)
//             : Q3TextView(text, context, parent, name)
//             , toEditWidget(false, true, false,
//                            false, false,
//                            false, true, false,
//                            false, true, false)
//     { }
    /** Reimplemented for internal reasons.
     */
    virtual void editCopy(void);
    /** Reimplemented for internal reasons.
     */
    virtual void editSelectAll(void);
    /** Reimplemented for internal reasons.
     */
    virtual bool editSave(bool);
    /** Reimplemented for internal reasons.
     */
    virtual void focusInEvent (QFocusEvent *e);
};

#endif
