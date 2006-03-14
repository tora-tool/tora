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

#include <qfile.h>
#include <qregexp.h>

#ifdef WIN32
#include <winsock.h>
#endif

#include "toconnection.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

#include "ibase.h"

static toSQL SQLListObjects("toOracleConnection:ListObjects",
			    "select \"RDB$OWNER_NAME\", \"RDB$RELATION_NAME\", \n"
				"'TABLE', \"RDB$DESCRIPTION\" from \"RDB$RELATIONS\" ",
			    "List the objects to cache for a connection, should have same "
			    "columns and binds");

//
// Helper function to deal with named parameters ":name<type>" syntax
//

static QString QueryParam(const QString &query, toQList &params)
{
	QString ret;
	bool inString=false;
	toQList::iterator cpar=params.begin();
	
	std::map<QString,QString> binds;
	
	for(unsigned int i=0;i<query.length();i++) {
		QChar c=query.at(i);
		switch(char(c)) {
		case '\\':
			ret+=c;
			ret+=query.at(++i);
			break;
		case '\'':
			inString=!inString;
			ret+=c;
			break;
		case ':':
			if (!inString) {
				QString nam;
				for (i++;i<query.length();i++) {
					c=query.at(i);
					if (!c.isLetterOrNumber())
						break;
					nam+=c;
				}
				QString in;
				if (c=='<') {
					for (i++;i<query.length();i++) {
						c=query.at(i);
						if (c=='>') {
							i++;
							break;
						}
						in+=c;
					}
				}
				i--;
				
				if (nam.isEmpty())
					throw QString("No bind name");
				
				if (binds.find(nam)!=binds.end()) {
					ret+=binds[nam];
					break;
				}
				if (cpar==params.end())
					throw QString("Not all bind variables suplied");
				QString str;
				if ((*cpar).isNull()) {
					str="NULL";
				} else if ((*cpar).isInt()||(*cpar).isDouble()) {
					str=QString(*cpar);
				} else {
					if (in!="noquote")
						str+="'";
					QString tmp=(*cpar);
					for(unsigned int j=0;j<tmp.length();j++) {
						QChar d=tmp.at(j);
						switch(char(d)) {
						case 0:
							str+="\\0";
							break;
						case '\n':
							str+="\\n";
							break;
						case '\t':
							str+="\\t";
							break;
						case '\r':
							str+="\\r";
							break;
						case '\'':
							str+="\\\'";
							break;
						case '\"':
							str+="\\\"";
							break;
						case '\\':
							str+="\\\\";
							break;
						default:
							str+=d;
						}
					}
					if (in!="noquote")
						str+="'";
				}
				binds[nam]=str;
				ret+=str;
				cpar++;
				break;
			}
		default:
			ret+=c;
		}
	}
	// More complex conversion here later
	//  printf("Parsed: %s\n",(const char *)ret);
	return ret.latin1();
}

//
// Helper function to turn isc_status errors into exceptions
//

static void ThrowException(long* isc_status)
{
	QString err;
	char msg[512];
	
	if( !isc_status[1] )
		return;
	
	isc_interprete(msg, &isc_status);
	err = msg;
	msg[0] = '-';
	while( isc_interprete(msg + 1,&isc_status) )
	{
		err += "\n";
		err += msg;
	}
	
	toConnection::exception ret = err;
	throw ret;
}

//
// Fyracle connection plugin
//

class toFyracleProvider : public toConnectionProvider {
	
public:
	
	class fyracleConnection;
	
	//
	// fyracleSub holds the Fyracle-specific connection data
	//
	struct fyracleSub : public toConnectionSub {
		
		isc_db_handle Connection;
		isc_tr_handle Transaction;
		
		long isc_status[20];
		
		fyracleSub(isc_db_handle connection)	{ Connection = connection; }
	};
	
	static fyracleSub* fyracle(toConnectionSub* conn) {
		fyracleSub* fs;
		fs = dynamic_cast<fyracleSub*>(conn);
		if( !fs )
			throw QString("Internal error mismatched connection");
		return fs;
	}
	
	//
	// fyracleQuery manages SQL statements
	//
	class fyracleQuery : public toQuery::queryImpl {
		
		bool Cancel;
		bool Running;
		bool Eof;
		long rows, curcol;

		fyracleSub *fs;

		isc_stmt_handle stmt;
		XSQLDA*			in_sqlda;		// input parameter area
		XSQLDA*			out_sqlda;		// result area
		char*			buf;			// "autobind" data area

	public:
		
		fyracleQuery(toQuery *query,toConnectionSub *sub)
			: toQuery::queryImpl(query)
		{
			Running = Cancel = false;
			Eof = false;
			buf = NULL;
			rows = curcol = 0;

			fs = toFyracleProvider::fyracle(sub);
			stmt = 0;
			isc_dsql_allocate_statement(fs->isc_status, &fs->Connection, &stmt);
			ThrowException(fs->isc_status);

			// allocate out & in xsqlda block with space for 20 sqlvar's
			out_sqlda          = (XSQLDA *) new char[(XSQLDA_LENGTH(20))];
			out_sqlda->sqln    = 20;
			out_sqlda->version = 1;
			
			in_sqlda          = (XSQLDA *) new char[(XSQLDA_LENGTH(20))];
			in_sqlda->sqln    = 20;
			in_sqlda->version = 1;
		}
		
		virtual ~fyracleQuery()
		{
			long isc_status[20];

			isc_dsql_free_statement(isc_status, &stmt, DSQL_drop);
			ThrowException(isc_status);

			delete (char*) out_sqlda;
			delete (char*) in_sqlda;
			delete buf;
		}
		
		virtual void execute(void);
		
		void autobind(void);

		virtual toQValue readValue(void);

		virtual void cancel(void)
		{
			// Not implemented
		}
		
		virtual bool eof(void)
		{
			return Eof;
		}
		
		virtual int rowsProcessed(void)
		{ return rows; }
		
		virtual int columns(void)
		{ return out_sqlda->sqld; }
		
		virtual std::list<toQuery::queryDescribe> describe(void)
		{
			std::list<toQuery::queryDescribe> ret;
			int descriptionLen = out_sqlda->sqld;
			
			for (int i=0;i<descriptionLen;i++) {
				toQuery::queryDescribe desc;
				XSQLVAR* var = &out_sqlda->sqlvar[i];
				
				var->aliasname[var->aliasname_length] = 0;
				desc.Name=QString::fromUtf8(var->aliasname);
				if( desc.Name.length()==0 ) {
					var->sqlname[var->sqlname_length] = 0;
					desc.Name=QString::fromUtf8(var->sqlname);
				}
				
				int prec = 0;
				switch(var->sqltype) {
				case SQL_VARYING:
				case SQL_VARYING+1:
					desc.Datatype="VARCHAR2";
					break;
				case SQL_SHORT:
				case SQL_SHORT+1:
					prec = 5;
					desc.Datatype="NUMBER";
					break;
				case SQL_LONG:
				case SQL_LONG+1:
					prec = 9;
					desc.Datatype="NUMBER";
					break;
				case SQL_INT64:
				case SQL_INT64+1:
					prec = 18;
					desc.Datatype="NUMBER";
					break;
				case SQL_DOUBLE:
				case SQL_DOUBLE+1:
					desc.Datatype="NUMBER";
					break;
				case 11:
				case 104:
					desc.Datatype="ROWID";
					break;
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIME+1:
				case SQL_TYPE_DATE:
				case SQL_TYPE_DATE+1:
				case SQL_TIMESTAMP:
				case SQL_TIMESTAMP+1:
					desc.Datatype="DATE";
					break;
				case SQL_TEXT:
				case SQL_TEXT+1:
					desc.Datatype="CHAR";
					break;
				case SQL_BLOB:
				case SQL_BLOB+1:
					desc.Datatype="BLOB";
					break;
				}
				
				if (desc.Datatype=="NUMBER") {
					if (prec) {
						desc.Datatype.append(" (");
						desc.Datatype.append(QString::number(prec));
						if (var->sqlscale!=0) {
							desc.Datatype.append(",");
							desc.Datatype.append(QString::number(-var->sqlscale));
						}
						desc.Datatype.append(")");
					}
				}
				desc.Null = true;
				
				ret.insert(ret.end(),desc);
			}
			return ret;
		}
	};
	
	//
	// fyracleConnection manages a single attachment to the engine
	//
	class fyracleConnection : public toConnection::connectionImpl {
		
	public:
		fyracleConnection(toConnection *conn)
			: toConnection::connectionImpl(conn)
		{
		}
		
		virtual std::list<toConnection::objectName> objectNames(void)
		{
			std::list<toConnection::objectName> ret;
			
			std::list<toQValue> par;
			toQuery objects(connection(),toQuery::Long,	SQLListObjects, par);
			toConnection::objectName cur;
			while(!objects.eof()) {
				cur.Owner   = objects.readValueNull();
				cur.Name    = objects.readValueNull();
				cur.Type    = objects.readValueNull();
//				cur.Comment = objects.readValueNull();
				ret.insert(ret.end(),cur);
			}
			return ret;
		}
		
		virtual toQDescList columnDesc(const toConnection::objectName &table)
		{
			try {
				QString SQL="SELECT * FROM ";
				SQL+=table.Name;
				SQL+=" WHERE NULL=NULL";
				toQuery query(connection(),SQL);
				return query.describe();
			}
			catch(...) {}
			
			toQDescList ret;
			return ret;
		}
		
		virtual void commit(toConnectionSub *sub);

		virtual void rollback(toConnectionSub *sub);
		
		virtual toConnectionSub *createConnection(void);
		
		void closeConnection(toConnectionSub *conn);
		
		virtual QString version(toConnectionSub *sub)
		{
			return "Fyracle 0.8.0";
		}
		
		virtual toConnection::connectionImpl *clone(toConnection *newConn) const
		{ return new fyracleConnection(newConn); }
		
		virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
		{ return new fyracleQuery(query,sub); }
		
		virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
		{
		}
	};
	
	toFyracleProvider(void)
		: toConnectionProvider("Oracle")
	{}
	
	virtual toConnection::connectionImpl *connection(toConnection *conn)
	{ return new fyracleConnection(conn); }
	
	virtual std::list<QString> modes(void)
	{
		std::list<QString> ret;
		ret.insert(ret.end(),"Normal");
		return ret;
	}

	virtual std::list<QString> hosts(void)
	{
		std::list<QString> ret;
		ret.insert(ret.end(),"localhost");
		return ret;
	}
	
	virtual std::list<QString> databases(const QString &host,const QString &user,const QString &pwd)
	{
		std::list<QString> ret;
		return ret;
	}
	
};

//
// Connection management methods
//

static const char	tpb[] = {		// transaction parameter block
	1,9,15,6
};

toConnectionSub *toFyracleProvider::fyracleConnection::createConnection(void)
{
	const char* usr = connection().user().data();
	const char* pwd = connection().password().data();
	
	int usrlen = strlen(usr);
	int pwdlen = strlen(pwd);
	int dpblen = usrlen + pwdlen + 5;
	int i;
	
	// allocate and fill database parameter block
	char* dpb = new char[dpblen];
	dpb[0] = isc_dpb_version1;
	dpb[1] = isc_dpb_user_name;
	dpb[2] = usrlen;
	for(i=3; i<(usrlen+3); i++)
		dpb[i] = *usr++;
	dpb[usrlen+3] = isc_dpb_password;
	dpb[usrlen+4] = pwdlen;
	for(i=usrlen+5; i<(usrlen+pwdlen+5); i++)
		dpb[i] = *pwd++;
	
	// combine host and database path
	QCString path;
	if( connection().host().isEmpty() ) {
		path = connection().database().utf8();
	} else {
		path  = connection().host();
		path += ":";
		path += connection().database().utf8();
	}
	
	// attach to database
	fyracleSub *fs = new fyracleSub(NULL);
	
	isc_attach_database (fs->isc_status, 0, path.data(), &fs->Connection, dpblen, dpb);
	delete[] dpb;
    ThrowException(fs->isc_status);
	
	fs->Transaction = NULL;
	isc_start_transaction (fs->isc_status, &fs->Transaction, 1, &fs->Connection, sizeof(tpb), tpb);
    ThrowException(fs->isc_status);
	
    return fs;
}

void toFyracleProvider::fyracleConnection::commit(toConnectionSub *sub)
{
	fyracleSub *fs = toFyracleProvider::fyracle(sub);
	
	isc_commit_transaction(fs->isc_status, &fs->Transaction);
	ThrowException(fs->isc_status);
	fs->Transaction = NULL;
	isc_start_transaction (fs->isc_status, &fs->Transaction, 1, &fs->Connection, sizeof(tpb), tpb);
	ThrowException(fs->isc_status);
}

void toFyracleProvider::fyracleConnection::rollback(toConnectionSub *sub)
{
	fyracleSub *fs = toFyracleProvider::fyracle(sub);
	
	isc_rollback_transaction(fs->isc_status, &fs->Transaction);
	ThrowException(fs->isc_status);
	fs->Transaction = NULL;
	isc_start_transaction (fs->isc_status, &fs->Transaction, 1, &fs->Connection, sizeof(tpb), tpb);
	ThrowException(fs->isc_status);
}

void toFyracleProvider::fyracleConnection::closeConnection(toConnectionSub *sub)
{
	fyracleSub *fs = toFyracleProvider::fyracle(sub);
	
	isc_rollback_transaction(fs->isc_status, &fs->Transaction);
	ThrowException(fs->isc_status);
	
	isc_detach_database(fs->isc_status, &fs->Connection);
	ThrowException(fs->isc_status);
		  
	delete sub;
}

//
// Query management methods
//

void toFyracleProvider::fyracleQuery::execute(void)
{
	// clear any pending queries
	isc_dsql_free_statement(fs->isc_status, &stmt, DSQL_close);
    if (Cancel)
		throw QString("Query aborted before started");

    Running=true;
    QString sqlp = QueryParam(QString::fromUtf8(query()->sql()),
			                   query()->params());
	char* sql = (char*) sqlp.data();
	isc_dsql_prepare(fs->isc_status, &fs->Transaction, &stmt, 0, sql, 3, out_sqlda);
	ThrowException(fs->isc_status);

	// Bind output parameters
	int out_cols = out_sqlda->sqld;
	if( out_sqlda->sqln < out_cols ) {
		// Need more room
		delete (char*) out_sqlda;
		out_sqlda = (XSQLDA *) new char[XSQLDA_LENGTH(out_cols)];
		out_sqlda->sqln = out_cols;
		out_sqlda->version = 1;
		
		// repeat describe to capture all columns
		isc_dsql_describe(fs->isc_status, &stmt, 1, out_sqlda);
		ThrowException(fs->isc_status);
		out_cols = out_sqlda->sqld;
	}

	// Bind input parameters
	isc_dsql_describe_bind(fs->isc_status, &stmt, 1, in_sqlda);
	ThrowException(fs->isc_status);
	
	int in_cols = in_sqlda->sqld;
	if( in_sqlda->sqln < in_cols ) {
		// Need more room
		delete (char*) in_sqlda;
		in_sqlda = (XSQLDA *) new char[XSQLDA_LENGTH(in_cols)];
		in_sqlda->sqln = in_cols;
		in_sqlda->version = 1;
		
		// repeat describe to capture all columns
		isc_dsql_describe_bind(fs->isc_status, &stmt, 1, in_sqlda);
		ThrowException(fs->isc_status);
		in_cols = in_sqlda->sqld;
	}
	
/*
	otl_null null;
	for(toQList::iterator i=query()->params().begin();i!=query()->params().end();i++) {
		if ((*i).isNull())
			(*Query)<<null;
		else {
			otl_var_desc *next=Query->describe_next_in_var();
			switch(next->ftype) {
			case otl_var_double:
			case otl_var_float:
				(*Query)<<(*i).toDouble();
				break;
			case otl_var_int:
			case otl_var_unsigned_int:
			case otl_var_short:
			case otl_var_long_int:
				(*Query)<<(*i).toInt();
				break;
			default:
				(*Query)<<QString(*i).utf8();
				break;
			}
		}
	}
	Running=false;
	conn->Lock.up();
*/
	isc_dsql_execute(fs->isc_status, &fs->Transaction, &stmt, 1, in_sqlda);
	ThrowException(fs->isc_status);

	Eof = false;
	if( out_sqlda->sqld>0 ) {
		autobind();
		curcol = 0;
		rows = 1;

		int rc = isc_dsql_fetch(fs->isc_status, &stmt, 1, out_sqlda);
		if( rc==100 ) 
			Eof = true;
		else
			ThrowException(fs->isc_status);
	}
	else
		Eof = true;
}

toQValue toFyracleProvider::fyracleQuery::readValue(void)
{
	toQValue ret;
	
	if (eof())
		throw QString("Tried to read past end of query");
	
	XSQLVAR* var = &out_sqlda->sqlvar[curcol];
	
	curcol++;
	
	if( *(var->sqlind)!=-1 ) {
		
		switch(var->sqltype) {
		case SQL_VARYING:
		case SQL_VARYING+1:
			char* p;
			int len;
			p = (char*)(var->sqldata);
			len = *(short*)p; p += 2;
			p[len] = 0;
			ret = toQValue(p);
			break;
			
		case SQL_SHORT:
		case SQL_SHORT+1:
			int i;
			i = *(short*)(var->sqldata);
			ret = toQValue(i);
			break;
			
		case SQL_LONG:
		case SQL_LONG+1:
			long l;
			l = *(long*)(var->sqldata);
			ret = toQValue(l);
			break;
			
		case SQL_INT64:
		case SQL_INT64+1:
			__int64 ll;
			ll = *(__int64*)(var->sqldata);
			l = ll;
			ret = toQValue(l);
			break;
			
		case SQL_DOUBLE:
		case SQL_DOUBLE+1:
			double d;
			d = *(double*)(var->sqldata);
			ret = toQValue(d);
			break;
			
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIME+1:
		case SQL_TYPE_DATE:
		case SQL_TYPE_DATE+1:
		case SQL_TIMESTAMP:
		case SQL_TIMESTAMP+1:
			throw QString("Internal error type mismatch in result set");
			break;
			
		case SQL_TEXT:
		case SQL_TEXT+1:
			char* c;
			c = (char*)(var->sqldata);
			c[var->sqllen] = 0;
			ret = toQValue(c);
			break;
			
		case SQL_BLOB:
		case SQL_BLOB+1:
			// TODO: fetch (text) BLOB here 
			l = *(long*)(var->sqldata);
			ret = toQValue(l);
			break;
		}
	}
	
	if (curcol==out_sqlda->sqld) {
		curcol = 0;
		rows++;
		int rc = isc_dsql_fetch(fs->isc_status, &stmt, 1, out_sqlda);
		if( rc==100 ) 
			Eof = true;
		else
			ThrowException(fs->isc_status);
	}

	return ret;
}

#define ALIGN(n,b)          ((n + b - 1) & ~(b - 1))

void toFyracleProvider::fyracleQuery::autobind()
{
	XSQLVAR *var;
	int i, offset, length, alignment, type;
	int colcount = out_sqlda->sqld;
	
	if( !colcount ) return;
	
	// 'auto-bind' the result set columns (overwrites any existing bindings)
	// compute the required size for 'buf'
	for( var = out_sqlda->sqlvar, offset = 0, i = 0; i < colcount; var++, i++)
	{
		length = alignment = var->sqllen;
		type   = var->sqltype & ~1;
		
		switch( type ) {
		case SQL_TYPE_TIME:
		case SQL_TYPE_DATE:
		case SQL_TIMESTAMP:
			type = SQL_TEXT;
		default:
			break;
		}

		if( type == SQL_TEXT )
			alignment = 1;
		else if( type == SQL_VARYING ) {   
			// Allow space for vary's strlen & space to insert
			// a null byte for printing
			length    += sizeof(short) + 1;
			alignment  = sizeof(short);
		}   
		offset = ALIGN(offset, alignment);
		var->sqldata = (char*) offset;
		offset += length;
		offset = ALIGN(offset, sizeof (short));
		var->sqlind  = (short*) offset;
		offset += sizeof(short);
	}
	// change VAR offsets into datapointers pointing into 'buf'
	buf = new char[ offset ];
	for( var = out_sqlda->sqlvar, i = 0; i < colcount; var++, i++) {
		var->sqldata = buf + (long) var->sqldata;
		var->sqlind = (short*) (buf + (long) var->sqlind);
	}
}


//
// Fyracle driver plugin instance
//

static toFyracleProvider FyracleProvider;


