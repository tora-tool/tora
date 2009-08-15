
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#include "toresultdata.h"
#include "utils.h"
#include "tobrowseraccesswidget.h"


toBrowserAccessWidget::toBrowserAccessWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserAccessWidget");

#ifdef TOEXTENDED_MYSQL
    // This is propably never compiled...
    // I'm leaving this code as is
    UserPanel = new toMySQLUser(curr, TAB_ACCESS_USER);
    curr->addTab(UserPanel, tr("&User"));
    SecondMap[TAB_ACCESS] = UserPanel;
    SecondMap[TAB_ACCESS_USER] = UserPanel;

    AccessPanel = new toMySQLUserAccess(curr, TAB_ACCESS_OBJECTS);
    curr->addTab(AccessPanel, tr("&Objects"));
    SecondMap[TAB_ACCESS_OBJECTS] = UserPanel; // Yes, it should be
    // this one, it will
    // signal the
    // TAB_ACCESS_OBJECTS
    // to update.
    connect(AccessPanel, SIGNAL(hasChanged()), UserPanel, SLOT(hasChanged()));
    connect(UserPanel,
            SIGNAL(saveChanges(const QString &, const QString &)),
            AccessPanel,
            SLOT(saveChanges(const QString &, const QString &)));
    connect(UserPanel,
            SIGNAL(changeUser(const QString &)),
            AccessPanel,
            SLOT(changeUser(const QString &)));

    accessContent = new toResultData(curr, TAB_ACCESS_CONTENT);
    accessContent->setObjectName("accessContent");

#else

    accessContent = new toResultData(this);
    accessContent->setObjectName("accessContent");
    connect(accessContent, SIGNAL(changesSaved()), this, SLOT(flushPrivs()));

#endif

    changeConnection();
}

void toBrowserAccessWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();

    if (toIsMySQL(toCurrentConnection(this)))
        addTab(accessContent, "&Data");
    else
        accessContent->setVisible(false);
}

void toBrowserAccessWidget::changeParams(const QString & schema,
                                         const QString & object)
{
    toBrowserBaseWidget::changeParams("mysql", object);
}

void toBrowserAccessWidget::flushPrivs(void)
{
    try
    {
        toCurrentConnection(this).execute("FLUSH PRIVILEGES");
    }
    TOCATCH
}

