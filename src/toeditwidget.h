
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOEDITWIDGET_H
#define TOEDITWIDGET_H

#include "config.h"

#include <list>

#include <qwidget.h>

class toSearchReplace;

/** This is an abstract base class for any widget (Not included in object) that wants to
 * access copy, cut, paste, open, save commands of the user interface.
 */
class toEditWidget
{
public:
    /** Class used to extent how to enable/disable parts of the interface as
     * focus changes.
     */
    class editHandler
    {
    public:
        /** Constructor.
         */
        editHandler();
        /** Destructor.
         */
        virtual ~editHandler();
        /** Called when a new widget receives the focus.
         */
        virtual void receivedFocus(toEditWidget *widget) = 0;
        /** Called when a widget loses the focus.
         */
        virtual void lostFocus(toEditWidget *widget) = 0;
    };
private:
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

    static std::list<editHandler *> *Handlers;
public:
    /** Empty constructor, all functions are disabled.
     */
    toEditWidget();
    /** Specified constructor, all functions are specified in the constructor.
     */
    toEditWidget(bool open, bool save, bool print,
                 bool undo, bool redo,
                 bool cut, bool copy, bool paste,
                 bool search,
                 bool selectAll, bool readAll);
    virtual ~toEditWidget();

    /** Specify new enabled functions.
     */
    void setEdit(bool open, bool save, bool print,
                 bool undo, bool redo,
                 bool cut, bool copy, bool paste,
                 bool search,
                 bool selectAll, bool readAll);

    /** Perform an open on this widget. Default NOP.
     * @param file The file to open, if not specified prompt for file.
     */
    virtual bool editOpen(QString file = QString::null);
    /** Perform a save on this widget. Default NOP.
     * @param askfile Ask filename even if default filename is available.
     */
    virtual bool editSave(bool askfile);
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
    {
        Open = val;
        setMainSettings();
    }
    /** Set if save is enabled.
     */
    void saveEnabled(bool val)
    {
        Save = val;
        setMainSettings();
    }
    /** Set if print is enabled.
     */
    void printEnabled(bool val)
    {
        Print = val;
        setMainSettings();
    }
    /** Set if undo is enabled.
     */
    void undoEnabled(bool val)
    {
        Undo = val;
        setMainSettings();
    }
    /** Set if redo is enabled.
     */
    void redoEnabled(bool val)
    {
        Redo = val;
        setMainSettings();
    }
    /** Set if cut is enabled.
     */
    void cutEnabled(bool val)
    {
        Cut = val;
        setMainSettings();
    }
    /** Set if copy is enabled.
     */
    void copyEnabled(bool val)
    {
        Copy = val;
        setMainSettings();
    }
    /** Set if paste is enabled.
     */
    void pasteEnabled(bool val)
    {
        Paste = val;
        setMainSettings();
    }
    /** Set if search is enabled.
     */
    void searchEnabled(bool val)
    {
        Search = val;
        setMainSettings();
    }
    /** Set if select all is enabled.
     */
    void selectAllEnabled(bool val)
    {
        SelectAll = val;
        setMainSettings();
    }
    /** Set if read all is enabled.
     */
    void readAllEnabled(bool val)
    {
        ReadAll = val;
        setMainSettings();
    }

    /** Check if open is enabled.
     */
    bool openEnabled(void)
    {
        return Open;
    }
    /** Check if save is enabled.
     */
    bool saveEnabled(void)
    {
        return Save;
    }
    /** Check if print is enabled.
     */
    bool printEnabled(void)
    {
        return Print;
    }
    /** Check if undo is enabled.
     */
    bool undoEnabled(void)
    {
        return Undo;
    }
    /** Check if redo is enabled.
     */
    bool redoEnabled(void)
    {
        return Redo;
    }
    /** Check if cut is enabled.
     */
    bool cutEnabled(void)
    {
        return Cut;
    }
    /** Check if copy is enabled.
     */
    bool copyEnabled(void)
    {
        return Copy;
    }
    /** Check if paste is enabled.
     */
    bool pasteEnabled(void)
    {
        return Paste;
    }
    /** Check if search is enabled.
     */
    bool searchEnabled(void)
    {
        return Search;
    }
    /** Check if select all is enabled.
     */
    bool selectAllEnabled(void)
    {
        return SelectAll;
    }
    /** Check if read all is enabled.
     */
    bool readAllEnabled(void)
    {
        return ReadAll;
    }

    /** Move to top of data
     */
    virtual void searchTop(void)
    { }
    /** Search for next entry
     * @return True if found, should select the found text.
     */
    virtual bool searchNext(toSearchReplace *search);
    /** Replace entry with new data
     */
    virtual void searchReplace(const QString &newData);
    /** Check if data can be modified by search
     * @param all If true can replace all, otherwise can replace right now.
     */
    virtual bool searchCanReplace(bool all);

    /** Call this when this widget has received the focus. Must be called by the implementor.
     */
    virtual void receivedFocus(void);
    /** Called when this widget has lost the focus. Don't forget to call parent when reimplemented.
     */
    virtual void lostFocus(void);

    /** Add a hook to be called every time focus changes.
     */
    static void addHandler(editHandler *handler);
    /** Removed a hook from being called every time focus changes.
     */
    static void delHandler(editHandler *handler);
};

#endif
