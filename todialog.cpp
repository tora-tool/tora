#include <qaccel.h>

#include "todialog.h"
#include "tomain.h"
#include "tohelp.h"

#include "todialog.moc"

toDialog::toDialog(QWidget *parent,const char *name,bool modal,WFlags f)
  : QDialog(parent,name,modal,f)
{
  QAccel *a=new QAccel(this);
  a->connectItem(a->insertItem(Key_F1),
		 this,
		 SLOT(displayHelp()));
}

void toDialog::displayHelp(void)
{
  QWidget *cur=qApp->focusWidget();
  while(cur) {
    try {
      toHelpContext *ctx=dynamic_cast<toHelpContext *>(cur);
      if (ctx) {
        toHelp::displayHelp(ctx->context(),this);
        return;
      }
    } catch(...) {
      // Catch problems with Visual C++ missing RTTI
    }
    cur=cur->parentWidget();
  }
  toHelp::displayHelp("toc.htm",this);
}
