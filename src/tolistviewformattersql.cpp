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

#include "utils.h"

#include "toconf.h"
#include "toconfiguration.h"
#include "tolistviewformattersql.h"
#include "toconnection.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"

#include <qheader.h>

namespace
{
  toListViewFormatter* createSQL()
  {
    return new toListViewFormatterSQL();
  }
  const bool registered = toListViewFormatterFactory::Instance().Register(toListViewFormatterIdentifier::SQL, createSQL);
}


toListViewFormatterSQL::toListViewFormatterSQL() : toListViewFormatter()
{}

toListViewFormatterSQL::~toListViewFormatterSQL()
{}

SQLTypeMap toListViewFormatterSQL::getSQLTypes(toListView& tListView)
{
  SQLTypeMap m;
  toConnection conn = toCurrentConnection(&tListView);
  toConnection::objectName on = toConnection::objectName(tListView.getOwner(),tListView.getObjectName());
  toQDescList desc = conn.columns(on);
  for (toQDescList::iterator i = desc.begin();i != desc.end(); ++i)
  {
        if ( std::find(headerFields.begin(),headerFields.end(),((*i).Name)) != headerFields.end() )
    {    
      if ( ((*i).Datatype).find("NUMBER") != -1 )
        m.insert(std::make_pair((*i).Name,static_cast<int>(SQL_NUMBER)));
      else if ( ((*i).Datatype).find("VARCHAR") != -1 )
        m.insert(std::make_pair((*i).Name,static_cast<int>(SQL_STRING)));
      else if ( ((*i).Datatype).find("DATE") != -1 )
        m.insert(std::make_pair((*i).Name,static_cast<int>(SQL_DATE)));
    }
  }
  return m;

}

QString toListViewFormatterSQL::getFormattedString(toListView& tListView)
{
  int column_count = tListView.columns();
  SQLTypeMap sqltypemap;
  QString output;
  QListViewItem* next = 0;
  unsigned int startIndex = 0;
  try
  {
    for (int j = 0;j < column_count;j++)
    {
      headerFields.push_back((tListView.header())->label(j));
    }

    if ( tListView.getIncludeHeader() )
    {
      startIndex = 1;
      output += "-- INSERT for table " + tListView.getOwner() + "." + tListView.getObjectName() + "\n";      
    }
    else
    {
      startIndex = 0;
    }


    sqltypemap = getSQLTypes(tListView);
    for ( QListViewItem *item = tListView.firstChild();item;item = next)
    {
      if (!tListView.getOnlySelection() || item->isSelected())
      {

        toResultViewItem * resItem = dynamic_cast<toResultViewItem *>(item);
        toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);

        QString line, text;

        for (unsigned int i = startIndex; i < column_count; i++)
        {
          if (resItem)
            text = resItem->allText(i);
          else if (chkItem)
            text = chkItem->allText(i);
          else
            text = item->text(i);

          if ( i == startIndex )
          {
            line += "INSERT INTO " + tListView.getOwner() + "." + tListView.getObjectName() + "(";
            for (int j = startIndex;j < column_count;j++)
            {
              line += (tListView.header())->label(j) + ",";
            }
            line.remove(line.length()-1,1);
            line += ") VALUES (";
          }

          switch ( sqltypemap[(tListView.header())->label(i)])
          {
          case SQL_NUMBER:
            line += QString::fromLatin1("%1").arg(text) + ",";
            break;
          case SQL_STRING:
            line += QString::fromLatin1("\'%1\'").arg(text) + ",";
            break;
          case SQL_DATE:
            line += "TO_DATE(" + QString::fromLatin1("\'%1\'").arg(text) + ",\'" + toConfigurationSingle::Instance().globalConfig(CONF_DATE_FORMAT, DEFAULT_DATE_FORMAT) + "\')" + ",";
            break;
          }
          if ( i == (column_count-1) )
          {
            line.replace(line.length()-1,1,")");
#ifdef WIN32

            line += "\r\n/";
#else

            line += "\n/";
#endif
          }

        }
#ifdef WIN32

        line += "\r\n";
#else

        line += "\n";
#endif

        output += line;
      }

      if (item->firstChild())
      {
        next = item->firstChild();
      }
      else if (item->nextSibling())
        next = item->nextSibling();
      else
      {
        next = item;
        do
        {
          next = next->parent();
        }
        while (next && !next->nextSibling());
        if (next)
          next = next->nextSibling();
      }
    }
    return output;
  }
  catch (...)
  {
    throw;
  }
}
