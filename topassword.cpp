//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

#include <qinputdialog.h>

static toSQL SQLChangePassword("toPassword:ChangePassword",
			       "ALTER USER %1 IDENTIFIED BY \"%2\"",
			       "Change password of a user, must have same %");

class toPasswordTool : public toTool {
protected:
public:
  toPasswordTool()
    : toTool(599,"Password Change")
  { }
  virtual const char *menuItem()
  { return "Password Change"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QString np=QInputDialog::getText("Enter new password",
				     "Enter new password",
				     QLineEdit::Password,
				     QString::null,
				     NULL,
				     toMainWidget());
    if (!np.isNull()) {
      try {
	connection.execute(toSQL::string(SQLChangePassword,connection).
			   arg(connection.user()).arg(np));
      } catch(const QString &exc) {
	toStatusMessage(exc);
      }
    }
    return NULL;
  }
};

static toPasswordTool PasswordTool;
