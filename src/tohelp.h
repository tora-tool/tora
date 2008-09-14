
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

#ifndef TOHELP_H
#define TOHELP_H

#include "config.h"

#include <QProgressBar>
#include <QWidget>

#include "ui_tohelpui.h"

class toHelpPrefs;

/**
 * This class is used to indicate a help context of an object. When
 * the current context is to be determined you start at the widget
 * that currently has focus and walk upwards to it's parent until you
 * find a widget that also inherits this class which specify the
 * context of all underlying widgets.
 */
class toHelpContext
{

    /**
     * The current context.
     */
    QString Context;

public:
    /** Create a context.
     * @param file File in the help directory that describes the current context.
     */
    toHelpContext(const QString &file)
            : Context(file)
    { }
    virtual ~toHelpContext()
    { }
    /**
     * Get this context
     * @return The location of a file that describes the current context.
     */
    virtual const QString &context(void) const
    {
        return Context;
    }
};

#include "totool.h"

/**
 * Additional help tool. Also used as object to connect to to display help
 * for modal dialogs. Only to be used internally.
 * @internal
 */
class toHelpTool : public toTool
{
    Q_OBJECT;

public:
    toHelpTool()
            : toTool(501, "Additional Help")
    { }
    virtual QWidget *toolWindow(QWidget *, toConnection &)
    {
        return NULL;
    }
    virtual QWidget *configurationTab(QWidget *parent);
    virtual void closeWindow(toConnection &connection){};
public slots:
    void displayHelp(void);
};

/**
 * Display a help browser. A help manual consists of HTML pages where at least
 * two pages are mandatory.
 *
 * The first one is toc.html which is parsed for links which are added to the left
 * pane tree view. Indentation is handled with the tag dl which open and close a
 * tree branch.
 *
 * The following file would result in two main headings with the first one having
 * one leaf widget as well.
 *
<pre>
<A HREF="first.html">1 Head</A><BR>
<DL>
<A HREF="second.html>1.1 Head Head</A></BR>
</DL>
<A HREF="third.html">2 Foot</A><BR>
</pre>
 *
 * The second file which is optional and should be called index.htm if it exists
 * should contain a file with links to keywords in the manual. This must be present
 * for search to be possible in the manual. Keywords are indicated by the <dd> tag
 * in these files as well as a link to where the keyword is described. Indentation
 * is possible using the <dl> tag is possible here as well.
 *
 * The following will make the keywords "Head", "Head, Head Head" and "Foot" available.
<pre>
<DD><A HREF="first.html">Head</A><BR>
<DL>
    <DD><A HREF="second.html">Head Head</A><BR>
</DL>
<DD><A HREF="third.html">Foot</A>
</pre>
 * The reason the files look this way is that this way they are compatible with
 * the Oracle manuals from Oracle Corporation and these can be browsed as well in
 * this help viewer.
 */

class toHelp : public QDialog, public Ui::toHelp
{
    Q_OBJECT;

    /** @internal
     * Pointer to open helpwindow if available, otherwise NULL
     */

    static toHelp *Window;

    /**
     * True if you are currently searching. You can't close the window while search, would
     * coredump.
     */
    bool Searching;

    /**
     * Set selection and also update selected item in list if any item matches the
     * selected location.
     * @param lst Listview to update selected item in.
     * @param str Location of next help text.
     */
    virtual void setSelection(QTreeWidget *lst, const QString &str);
    /**
     * Reimplemented for internal reasons.
     */
    virtual void closeEvent(QCloseEvent *e);

private slots:
    /** Initiate a search with the parameters in the internal widgets.
     * @internal
     */
    void search(void);
    /** Remove the current selection in the sections and result lists.
     * @internal
     */
    void removeSelection(void);
    /** Item selected in left pane. Change contents of help viewer.
     * @internal
     */
    void changeContent(QTreeWidgetItem * item, QTreeWidgetItem *);
public:
    /**
     * Create help widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param modal If dialog is for a modal dialog or not
     */
    toHelp(QWidget *parent, QString name, bool modal = true);
    /** Help function to get a clean path from a path string. Strips trailing / etc.
     * @param path Path to strip.
     * @return Stripped path in string.
     */
    static QString path(const QString &path = QString::null);
    virtual ~toHelp();
    /** Display a specific help context in the internal manual. Pops up a help window that
     * displays the selected topic.
     * @param context Context to diaplay.
     * @param parent If NULL use modal dialog and main window.
     */
    static void displayHelp(const QString &context, QWidget *parent = NULL);
    /** Display a specific help context in the internal manual. The context is derived from
     * the widget that currently holds the focus.
     * @param parent If NULL use modal dialog and main window.
     */
    static void displayHelp(QWidget *parent = NULL);
    /** Connect the F1 key to display help for a modal dialog. Don't use this directly,
     * instead call displayHelp directly and bind Key_F1 to that call. This is because the
     * help in modal dialogs are cludgy at best and should be avoided if possible.
     * @param dialog Dialog to connect accelerator to.
     */
    static void connectDialog(QDialog *dialog);

    friend class toHelpPrefs;
};

#endif
