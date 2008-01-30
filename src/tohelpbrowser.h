// /*****
// *
// * TOra - An Oracle Toolkit for DBA's and developers
// * Copyright (C) 2003-2005 Quest Software, Inc
// * Portions Copyright (C) 2005 Other Contributors
// *
// * This program is free software; you can redistribute it and/or
// * modify it under the terms of the GNU General Public License
// * as published by the Free Software Foundation;  only version 2 of
// * the License is valid for this program.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program; if not, write to the Free Software
// * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// *
// *      As a special exception, you have permission to link this program
// *      with the Oracle Client libraries and distribute executables, as long
// *      as you follow the requirements of the GNU GPL in regard to all of the
// *      software in the executable aside from Oracle client libraries.
// *
// *      Specifically you are not permitted to link this program with the
// *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
// *      And you are not permitted to distribute binaries compiled against
// *      these libraries without written consent from Quest Software, Inc.
// *      Observe that this does not disallow linking to the Qt Free Edition.
// *
// *      You may link this product with any GPL'd Qt library such as Qt/Free
// *
// * All trademarks belong to their respective owners.
// *
// *****/
//
// #ifndef TOHELPBROWSER_H
// #define TOHELPBROWSER_H
//
// #include "config.h"
//
// #include <list>
// #include <khtml_part.h>
//
// class KURL;
//
// /**
//  * Widget used to display HTML of help pages.
//  */
//
// class toHelpBrowser : public KHTMLPart
// {
//     Q_OBJECT
//
//     /** List of backward history. The current page is at the end of the list.
//      */
//     std::list<QString> Backward;
//     /** List of forward history if you have started walking backwards.
//      */
//     std::list<QString> Forward;
// private slots:
//     /** Reimplemented for internal reasons.
//      */
//     void openURLRequest(const KURL &url, const KParts::URLArgs &);
// public:
//     /** Create a help browser.
//      * @param parent Parent widget.
//      * @param name Name of widget.
//      */
//     toHelpBrowser(QWidget *parent, const char *name = NULL);
//     /** Open an URL in this widget.
//      * @param url URL to open.
//      * @return Success of operator.
//      */
//     virtual bool openURL(const KURL &url);
//     /** Get source of URL.
//      * @return String containing the current localtion.
//      */
//     QString source(void);
// public slots:
//     /** Go forward if available.
//      */
//     void forward(void);
//     /** Go backward if available.
//      */
//     void backward(void);
// signals:
//     /** Change availability of backward history.
//      * @param avail Wether history is available.
//      */
//     void backwardAvailable(bool avail);
//     /** Change availability of forward history.
//      * @param avail Wether history is available.
//      */
//     void forwardAvailable(bool avail);
//     /** The contents of the browser changed.
//      */
//     void textChanged(void);
// };
//
// #endif
