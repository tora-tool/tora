/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/



#ifndef __TORESULTRESOURCES_H
#define __TORESULTRESOURCES_H

#include "toresultitem.h"

class toResultResources : public toResultItem {
public:
  toResultResources(toConnection &conn,QWidget *parent,const char *name=NULL)
    : toResultItem(3,true,conn,parent,name)
  {
    setSQL("SELECT Loads,		Sorts,			First_Load_Time,"
	   "       Parse_Calls,		Disk_Reads,		Parsing_User_Id,"
	   "       Executions,		Buffer_Gets,		Parsing_Schema_Id,"
	   "       Users_Executing,	Users_Opening,		Open_Versions,"
	   "       Sharable_Mem,	Rows_Processed,		Kept_Versions,"
	   "       Persistent_Mem,	Optimizer_Mode,		Loaded_Versions,"
	   "       Runtime_Mem,		Serializable_Aborts,	Invalidations"
	   " FROM v$sqlarea WHERE Address||':'||Hash_Value = :f1<char[100]>");
  }
};

#endif
