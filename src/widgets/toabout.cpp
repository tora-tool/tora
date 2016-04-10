
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "widgets/toabout.h"

#include "core/toconf.h"
#include "core/COPYRIGHT.h"
#include "core/toraversion.h"

#include "icons/largelogo.xpm"

#include <QtCore/QStringRef>

toAbout::toAbout(QWidget* parent, const char* name, bool modal)
    : QDialog(parent)
{
    if (name)
        setObjectName(name);
    if (modal)
        setWindowModality(Qt::WindowModal);

    setupUi(this);

#if !defined(TOBUILDTYPE_RELEASE)
    setWindowTitle(QString(TOAPPNAME) + " " BUILD_TAG);
#endif

    // About Tab
    {
    	QFile f(":/widgets/toabout.html");
    	f.open(QFile::ReadOnly);
    	QString AboutText = QString::fromUtf8(f.readAll());
    	QString buffer = AboutText.arg(QString::fromLatin1(TORAVERSION));
    	textBrowserAbout->setHtml(buffer);
    }

    // License Tab
    {
    	QFile f(":/widgets/gpl-2.0-standalone.html");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textBrowserLicense->setHtml(LicenseText);
    }

	// Copyright Tab
	{
		textBrowserCopyright->setHtml(CopyrightText);
	}

	// Version Tab
	{
		QString version;
	#if defined(HAVE_GITREVISION_H)
		version.append("<center><table>");
		QString format = QString("<tr><td align=\"right\">%1:<td align=\"left\">%2");
		version.append(format.arg("GITVERSION").arg(GITVERSION));
		version.append(format.arg("GITVERSION_MAJOR").arg(GITVERSION_MAJOR));
		version.append(format.arg("GITVERSION_MINOR").arg(GITVERSION_MINOR));
		version.append(format.arg("GIT_BUILD_TYPE").arg(GIT_BUILD_TYPE));
		version.append(format.arg("GITVERSION_COUNT").arg(GITVERSION_COUNT));
		version.append(format.arg("GITVERSION_SHA1").arg(GITVERSION_SHA1));
		version.append(format.arg("GITVERSION_SHORT").arg(GITVERSION_SHORT));
		version.append(format.arg("GIT_BRANCH").arg(GIT_BRANCH));
		version.append(format.arg("BUILD_TAG").arg(BUILD_TAG));
		version.append(format.arg("BUILD_DATE").arg(BUILD_DATE));
		version.append("</table>");
#else
		version = TORAVERSION;
#endif
		textBrowserVersion->setHtml(version);
	}
}

toAbout::~toAbout()
{
}
