#include "toeditwidget.h"
#include "tomain.h"

void toEditWidget::setMainSettings(void)
{
  toMain::editEnable(this);
}

toEditWidget::toEditWidget()
{
  Open=Save=Print=Undo=Redo=Cut=Copy=Paste=Search=SelectAll=ReadAll=false;
}

toEditWidget::toEditWidget(bool open,bool save,bool print,
			   bool undo,bool redo,
			   bool cut,bool copy,bool paste,
			   bool search,
			   bool selectAll,bool readAll)
{
  Open=open;
  Save=save;
  Print=print;
  Undo=undo;
  Redo=redo;
  Cut=cut;
  Copy=copy;
  Paste=paste;
  Search=search;
  SelectAll=selectAll;
  ReadAll=readAll;
}

void toEditWidget::setEdit(bool open,bool save,bool print,
			   bool undo,bool redo,
			   bool cut,bool copy,bool paste,
			   bool search,
			   bool selectAll,bool readAll)
{
  Open=open;
  Save=save;
  Print=print;
  Undo=undo;
  Redo=redo;
  Cut=cut;
  Copy=copy;
  Paste=paste;
  Search=search;
  SelectAll=selectAll;
  ReadAll=readAll;

  setMainSettings();
}

toEditWidget::~toEditWidget()
{
  toMain::editDisable(this);
}

void toEditWidget::receivedFocus(void)
{
  toMain::setEditWidget(this);
}
