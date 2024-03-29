
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

#include "widgets/toaboutcopying.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

toAboutCopying::toAboutCopying(QWidget* parent, const char* name)
    : QWidget(parent)
{
    if (name)
        setObjectName(name);

    setupUi(this);
    {
    	QFile f(":/widgets/COPYRIGHT.TOra.txt");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textTora->setPlainText(LicenseText);
    }
    {
    	QFile f(":/widgets/LICENSE.OpenSSL.txt");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textOpenSSL->setPlainText(LicenseText);
    }
    {
    	QFile f(":/widgets/LICENSE.ANTLR.txt");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textANTLR->setPlainText(LicenseText);
    }
    {
    	QFile f(":/widgets/LICENSE.KGraphViewer.txt");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textKgraph->setPlainText(LicenseText);
    }
    {
    	QFile f(":/widgets/LICENSE.ExtendedTabs.txt");
    	f.open(QFile::ReadOnly);
    	QString LicenseText = QString::fromUtf8(f.readAll());
    	textExtendedTabs->setPlainText(LicenseText);
    }
    {
        QFile f(":/widgets/LICENSE.dtl.txt");
        f.open(QFile::ReadOnly);
        QString LicenseText = QString::fromUtf8(f.readAll());
        textDTL->setPlainText(LicenseText);
    }
    {
        // Check whether dot was embedded inside .msi
        QFileInfo GvToraDir(QCoreApplication::applicationDirPath() + QDir::separator() + "Graphviz2.38" + QDir::separator() + "bin");
        QString p = GvToraDir.absoluteFilePath();
        if (GvToraDir.isDir() && GvToraDir.exists())
        {
            QFile f(":/widgets/LICENSE.GraphViz.txt");
            f.open(QFile::ReadOnly);
            QString LicenseText = QString::fromUtf8(f.readAll());
            textGraphViz->setPlainText(LicenseText);
        } else {
            tabWidget->removePage(6); // No page after this one
        }
    }
}
