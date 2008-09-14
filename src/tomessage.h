/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOMESSAGE_H
#define TOMESSAGE_H

#include "ui_tomessageui.h"


/*! \brief Non-modal and non-blocking message dialog for
errors/warnings reporting.

It's used in toMain as a class member. This dialog is never
deleted. All messages are appended into end of text-log.

I've rewrote the old-modal-dialog to prevent TOra blocking
(100% CPU) on huge amount of error messages - e.g. when
I run the Server Tuning tool under user with only few grants
on system objects I had to kill the whole desktop to recover.

It takes about 4% CPU in this implementattion.

TODO: IMHO is this dialog a hot candidate for "docking" window

\author Petr Vanek <petr@scribus.info>
*/
class toMessage : public QDialog, public Ui::toMessageUI
{
    Q_OBJECT

    public:
        toMessage(QWidget * parent = 0, Qt::WindowFlags f = 0);

    public slots:
        /*! \brief Appends text to the log widget.
        The dialog is shown too when it's hidden.
        */
        void appendText(const QString & text);

    protected:
        //! Save the prefs
        void hideEvent(QHideEvent * event);

};

#endif
