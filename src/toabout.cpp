/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "toabout.h"

#include <qtextview.h>
#include <qmime.h>
#include <qpushbutton.h>

#include "config.h"

#include "toconf.h"
#include "LICENSE.h"
#include "icons/largelogo.xpm"

#include "toabout.moc"
#include "toaboutui.moc"

static const char *AboutText = "<IMG SRC=largelogo.xpm><BR>\n"
                               "Version %1 (<A HREF=http://tora.sourceforge.net>http://tora.sourceforge.net</A>)\n"
                               "<P>\n"
                               "&copy; 2003-2005 Quest Software, Inc. "
                               "(<A HREF=http://www.quest.com>http://www.quest.com/</a>).<P>\n"
                               "Portions &copy; 2005 other contributors<P>\n"
                               "Written by Henrik Johnson with contributions by the Open Source community.<P>\n"
                               "SourceForge project administered by Nathan Neulinger <A HREF=\"mailto:nneul@umr.edu\">nneul@umr.edu</A><P>\n"
                               "This program is available\n"
                               "under the GNU General Public License.\n"
                               "Uses the Qt library by TrollTech\n"
                               "(<A HREF=http://www.trolltech.com>http://www.trolltech.com/</A>).<P>\n"
                               "\n"
                               "<HR BREAK=ALL>\n"
                               "This program is distributed in the hope that it will be useful,\n"
                               "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                               "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                               "GNU General Public License for more details."
                               "<HR BREAK=ALL>\n"
                               "Development team (in alphabetical order):<P>\n"
                               "<UL>\n"
                               "<LI><B>Alexey Danilchenko</B>\n"
                               "<LI><B>Henrik Johnson</B>\n"
                               "<LI><B>Ivan Brezina</B>\n"
                               "<LI><B>Mike Johnson</B>\n"
                               "<LI><B>Nathan Neulinger</B>\n"
                               "<LI><B>Oliver Jehle</B>\n"
                               "<LI><B>Raj Thukral</B>\n"
                               "<LI><B>Thomas Porschberg</B>\n"
                               "<LI><B>Volker Götz</B>\n"
                               "</UL>\n"
                               "Contributors (In alphabetical order):<P>\n"
                               "<UL>\n"
                               "<LI><B>Alexander (Leo) Bergolth</B> - Access to Oracle 7 database.\n"
                               "<LI><B>Volker Götz</B> - Improved PostgreSQL support.\n"
                               "<LI><B>Oliver Jehle</B> - Object caching to disc, SAP and much more.\n"
                               "<LI><B>Sergei Kuchin</B> - Oracle Template Library.\n"
                               "<LI><B>Michael Meskes</B> - Debian maintainer (<A HREF=http://www.debian.org>http://www.debian.org/</A>).\n"
                               "<LI><B>Selcuk Ozturk</B> - Improved content editor.\n"
                               "<LI><B>Jeff Price</B> - Endless feature requests and more.\n"
                               "<LI><B>Stefan Rindeskär</B> - Improved PostgreSQL support.\n"
                               "<LI><B>Adam vonNieda</B> - Improvements to RMAN monitoring from OracleTool (<A HREF=http://www.oracletool.com>http://www.oracletool.com</A>)\n"
                               "<LI><B>Guillaume Moulard</B> - Log4PL/SQL documentation.\n"
                               "</UL>\n"
                               "Other contributors:<P>\n"
                               "<B>Nuno Araujo</B>,\n"
                               "<B>Ivan Brezina</B>,\n"
                               "<B>Nathan Bryant</B>,\n"
                               "<B>Robert Cheramy</B>,\n"
                               "<B>Francesco Dandrea</B>,\n"
                               "<B>Ferdinand Gassauer</B>,\n"
                               "<B>Robert Ham</B>,\n"
                               "<B>Rick Hall</B>,\n"
                               "<B>Rico Hendriks</B>,\n"
                               "<B>Frits Hoogland</B>,\n"
                               "<B>Mihai Ibanescu</B>,\n"
                               "<B>Bo Johansson</B>,\n"
                               "<B>Olof Jönsson</B>,\n"
			       "<B>Michael Kaes</B>,\n"
                               "<B>Norbert Kiesel</B>,\n"
			       "<B>Pawe=B3 Kucharczyk</B>,\n"
                               "<B>Jacob LauemÃ¸lle</B>,\n"
                               "<B>Laramie Leavitt</B>,\n"
                               "<B>Paolo Magnoli</B>,\n"
                               "<B>Vladimir Naprstek</B>,\n"
                               "<B>Peter Nyborg</B>,\n"
                               "<B>Frank Pavageau</B>,\n"
                               "<B>Andreas Piesk</B>,\n"
			       "<B>Gerard H. Pille</B>,\n"
                               "<B>Thomas Porschberg</B>,\n"
                               "<B>Jan Pruner</B>,\n"
                               "<B>Jonas Rathert</B>,\n"
                               "<B>Christopher R. Redinger</B>,\n"
                               "<B>Erik Selberg</B>,\n"
                               "<B>Dennis Selsky</B>,\n"
                               "<B>Roman Stepanov</B>,\n"
			       "<B>Petr Vanek</B>,\n"
                               "<B>Daniel Vérité</B>,\n"
                               "<B>Matthias Wolle</B>,\n"
                               "<B>Joachim Zetzsche</B>,\n"
                               "<B>Massimiliano Ziccardi</B>,\n"
                               "<B>Raj Thukral</B>...\n";

#define ABOUT_CAPTION TOAPPNAME " %1"


toAbout::toAbout(int page, QWidget* parent, const char* name, bool modal, WFlags fl)
        : toAboutUI(parent, name, modal, fl)
{
    QMimeSourceFactory::defaultFactory()->setPixmap(QString::fromLatin1("largelogo.xpm"),
						    QPixmap(const_cast<const char**>(largelogo_xpm)));

    switch (page)
    {
    case 1:
        setCaption(tr("GNU General Public License"));
        TextView->setText(tr(LicenseText));
        TextView->setPaper(QColor(255, 255, 255));
        TextView->setTextFormat(PlainText);
        if (parent)
            CancelButton->hide();
        break;
    default:
        setCaption(tr(ABOUT_CAPTION).arg(QString::fromLatin1(TOVERSION)));
        QString buffer = tr(AboutText).arg(QString::fromLatin1(TOVERSION));
        TextView->setText(buffer);
        TextView->setPaper(QColor(227, 184, 54));
        TextView->setTextFormat(RichText);
        CancelButton->hide();
        break;
    }
}

toAbout::~toAbout()
{
    QMimeSourceFactory::defaultFactory()->setPixmap(QString::fromLatin1("largelogo.xpm"),0);
}
