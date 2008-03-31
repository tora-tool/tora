//
// C++ Implementation: toListViewFormatterSQL
//
// Copyright: See COPYING file that comes with this distribution
//

#include "utils.h"

#include "toconf.h"
#include "toconfiguration.h"
#include "tolistviewformattersql.h"
#include "toconnection.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"


namespace
{
toListViewFormatter* createSQL()
{
    return new toListViewFormatterSQL();
}
const bool registered = toListViewFormatterFactory::Instance().Register( toListViewFormatterIdentifier::SQL, createSQL );
}


toListViewFormatterSQL::toListViewFormatterSQL() : toListViewFormatter()
{}

toListViewFormatterSQL::~toListViewFormatterSQL()
{}

SQLTypeMap toListViewFormatterSQL::getSQLTypes( toListView& tListView )
{
    SQLTypeMap m;
    toConnection &conn = toCurrentConnection( &tListView );
    toConnection::objectName on = toConnection::objectName( tListView.getOwner(), tListView.getObjectName() );
    toQDescList desc = conn.columns( on );
    for ( toQDescList::iterator i = desc.begin();i != desc.end(); ++i )
    {
        if ( std::find( headerFields.begin(), headerFields.end(), ( ( *i ).Name ) ) != headerFields.end() )
        {
            if ( ( ( *i ).Datatype ).indexOf( "NUMBER" ) != -1 )
                m.insert( std::make_pair( ( *i ).Name, static_cast<int>( SQL_NUMBER ) ) );
            else if ( ( ( *i ).Datatype ).indexOf( "VARCHAR" ) != -1 )
                m.insert( std::make_pair( ( *i ).Name, static_cast<int>( SQL_STRING ) ) );
            else if ( ( ( *i ).Datatype ).indexOf( "DATE" ) != -1 )
                m.insert( std::make_pair( ( *i ).Name, static_cast<int>( SQL_DATE ) ) );
        }
    }
    return m;

}

QString toListViewFormatterSQL::getFormattedString( toListView& tListView )
{
    unsigned int column_count = tListView.columns();
    SQLTypeMap sqltypemap;
    QString output;
    toTreeWidgetItem* next = 0;
    unsigned int startIndex = 0;
    try
    {
        for ( unsigned int j = 0;j < column_count;j++ )
        {
            headerFields.push_back(tListView.headerItem()->text(j));
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


        sqltypemap = getSQLTypes( tListView );
        for ( toTreeWidgetItem * item = tListView.firstChild();item;item = next )
        {
            if ( !tListView.getOnlySelection() || item->isSelected() )
            {

                toResultViewItem * resItem = dynamic_cast<toResultViewItem *>( item );
                toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>( item );

                QString line, text;

                for ( unsigned int i = startIndex; i < column_count; i++ )
                {
                    if ( resItem )
                        text = resItem->allText( i );
                    else if ( chkItem )
                        text = chkItem->allText( i );
                    else
                        text = item->text( i );

                    if ( i == startIndex )
                    {
                        line += "INSERT INTO " + tListView.getOwner() + "." + tListView.getObjectName() + "(";
                        for ( unsigned int j = startIndex;j < column_count;j++ )
                        {
                            line += tListView.headerItem()->text(j) + ",";
                        }
                        line.remove( line.length() - 1, 1 );
                        line += ") VALUES (";
                    }

                    switch ( sqltypemap[ ( tListView.headerItem() ) ->text( i ) ] )
                    {
                    case SQL_NUMBER:
                        line += ( text.isEmpty() ) ? "NULL," : QString::fromLatin1( "%1," ).arg( text );
                        break;
                    case SQL_STRING:
                        line += ( text.isEmpty() ) ? "NULL," : QString::fromLatin1( "\'%1\'," ).arg( text );
                        break;
                    case SQL_DATE:
                        if ( text.isEmpty() )
                        {
                            line += "NULL,";
                        }
                        else
                        {
                            line += "TO_DATE(" + QString::fromLatin1( "\'%1\'" ).arg( text ) + ",\'" + toConfigurationSingle::Instance().dateFormat() + "\')" + ",";
                        }
                        break;
                    }
                    if ( i == ( column_count - 1 ) )
                    {
                        line.replace( line.length() - 1, 1, ")" );
#ifdef Q_OS_WIN32

                        line += ";\r\n";
#else

                        line += ";\n";
#endif

                    }

                }
                output += line;
            }

            if ( item->firstChild() )
            {
                next = item->firstChild();
            }
            else if ( item->nextSibling() )
                next = item->nextSibling();
            else
            {
                next = item;
                do
                {
                    next = next->parent();
                }
                while ( next && !next->nextSibling() );
                if ( next )
                    next = next->nextSibling();
            }
        }
        return output;
    }
    catch ( ... )
    {
        throw;
    }
}


QString toListViewFormatterSQL::getFormattedString(toExportSettings &settings,
        const toResultModel *model)
{
    return QString::null;
}
