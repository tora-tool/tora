#ifndef TOEDITWIDGET_H
#define TOEDITWIDGET_H

#include <qwidget.h>

class toSearchReplace;

/** This is an abstract base class for any widget (Not included in object) that wants to
 * access copy, cut, paste, open, save commands of the user interface.
 */
class toEditWidget {
  bool Open;
  bool Save;
  bool Print;
  bool Undo;
  bool Redo;
  bool Cut;
  bool Copy;
  bool Paste;
  bool Search;
  bool SelectAll;
  bool ReadAll;
  void setMainSettings(void);
public:
  /** Empty constructor, all functions are disabled.
   */
  toEditWidget();
  /** Specified constructor, all functions are specified in the constructor.
   */
  toEditWidget(bool open,bool save,bool print,
	       bool undo,bool redo,
	       bool cut,bool copy,bool paste,
	       bool search,
	       bool selectAll,bool readAll);
  virtual ~toEditWidget();

  /** Specify new enabled functions.
   */
  void setEdit(bool open,bool save,bool print,
	       bool undo,bool redo,
	       bool cut,bool copy,bool paste,
	       bool search,
	       bool selectAll,bool readAll);

  /** Perform an open on this widget. Default NOP.
   */
  virtual void editOpen(void)
  { }
  /** Perform a save on this widget. Default NOP.
   * @param askfile Ask filename even if default filename is available.
   */
  virtual void editSave(bool askfile)
  { }
  /** Print this widgets contents. Default NOP.
   */
  virtual void editPrint(void)
  { }
  /** Perform undo. Default NOP.
   */
  virtual void editUndo(void)
  { }
  /** Perform redo. Default NOP.
   */
  virtual void editRedo(void)
  { }
  /** Perform cut. Default NOP.
   */
  virtual void editCut(void)
  { }
  /** Perform copy. Default NOP.
   */
  virtual void editCopy(void)
  { }
  /** Perform paste. Default NOP.
   */
  virtual void editPaste(void)
  { }
  /** Perform search. Default NOP.
   * @param search Search dialog.
   */
  virtual void editSearch(toSearchReplace *search)
  { }
  /** Select all contents. Default NOP.
   */
  virtual void editSelectAll(void)
  { }
  /** Read all available data. Default NOP.
   */
  virtual void editReadAll(void)
  { }

  /** Set if open is enabled.
   */
  void openEnabled(bool val)
  { Open=val; setMainSettings(); }
  /** Set if save is enabled.
   */
  void saveEnabled(bool val)
  { Save=val; setMainSettings(); }
  /** Set if print is enabled.
   */
  void printEnabled(bool val)
  { Print=val; setMainSettings(); }
  /** Set if undo is enabled.
   */
  void undoEnabled(bool val)
  { Undo=val; setMainSettings(); }
  /** Set if redo is enabled.
   */
  void redoEnabled(bool val)
  { Redo=val; setMainSettings(); }
  /** Set if cut is enabled.
   */
  void cutEnabled(bool val)
  { Cut=val; setMainSettings(); }
  /** Set if copy is enabled.
   */
  void copyEnabled(bool val)
  { Copy=val; setMainSettings(); }
  /** Set if paste is enabled.
   */
  void pasteEnabled(bool val)
  { Paste=val; setMainSettings(); }
  /** Set if search is enabled.
   */
  void searchEnabled(bool val)
  { Search=val; setMainSettings(); }
  /** Set if select all is enabled.
   */
  void selectAllEnabled(bool val)
  { SelectAll=val; setMainSettings(); }
  /** Set if read all is enabled.
   */
  void readAllEnabled(bool val)
  { ReadAll=val; setMainSettings(); }
  
  /** Check if open is enabled.
   */
  bool openEnabled(void)
  { return Open; }
  /** Check if save is enabled.
   */
  bool saveEnabled(void)
  { return Save; }
  /** Check if print is enabled.
   */
  bool printEnabled(void)
  { return Print; }
  /** Check if undo is enabled.
   */
  bool undoEnabled(void)
  { return Undo; }
  /** Check if redo is enabled.
   */
  bool redoEnabled(void)
  { return Redo; }
  /** Check if cut is enabled.
   */
  bool cutEnabled(void)
  { return Cut; }
  /** Check if copy is enabled.
   */
  bool copyEnabled(void)
  { return Copy; }
  /** Check if paste is enabled.
   */
  bool pasteEnabled(void)
  { return Paste; }
  /** Check if search is enabled.
   */
  bool searchEnabled(void)
  { return Search; }
  /** Check if select all is enabled.
   */
  bool selectAllEnabled(void)
  { return SelectAll; }
  /** Check if read all is enabled.
   */
  bool readAllEnabled(void)
  { return ReadAll; }

  /** Call this when this widget has received the focus.
   */
  void receivedFocus(void);
  /** Call this when this widget has lost the focus.
   */
  void lostFocus(void);
};

#endif
