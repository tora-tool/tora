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

#ifndef TOPREFERENCES_H
#define TOPREFERENCES_H

#include "config.h"

#include <qvariant.h>
#include <qdialog.h>

#include <map>

#include "tohelp.h"
#include "ui_topreferencesui.h"


class toPreferences : public QDialog,
            public Ui::toPreferencesUI,
            public toHelpContext
{

    Q_OBJECT;

    QWidget *Shown;

    std::map<QListWidgetItem *, QWidget *> Tabs;

    void addWidget(QListWidgetItem *, QWidget *);

public:
    toPreferences(QWidget* parent = 0,
                  const char* name = 0,
                  bool modal = false,
                  Qt::WFlags fl = 0);


    static void displayPreferences(QWidget *parent);

public slots:
    void saveSetting(void);
    void selectTab(QListWidgetItem *item);
    void help(void);
};

#endif // TOPREFERENCES_H
