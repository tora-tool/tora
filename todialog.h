#ifndef __TODIALOG_H
#define __TODIALOG_H

#include <qdialog.h>

/**
 * A baseclass to be used for modal dialogs in TOra, basically allows F1 to
 * pop up context help.
 */
class toDialog : public QDialog {
  Q_OBJECT
public slots:
    /** Display help for this dialog. Make sure you also inherit toHelpContext
     * to specify the correct context for this dialog. You could use this to
     * also have a help button in the dialog.
     * @see toHelpContext
     */
  void displayHelp(void);
public:
  /** Create dialog. Same parameters as @ref QDialog.
   */
  toDialog(QWidget*parent=0,const char *name=0,bool modal=FALSE,WFlags f=0);
};

#endif
