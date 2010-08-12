
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifndef TO_NO_ORACLE

#include "utils.h"
#include "tologger.h"
#include <iomanip>

#ifdef Q_OS_WIN32
#  include "windows/cregistry.h"
#include <Windows.h>
#endif

#include <trotl.h>
#include <trotl_convertor.h>

#include "toconf.h"
#include "toconfiguration.h"
#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"
#include "tooraclesetting.h"
#include "todefaultkeywords.h"

#include <stdio.h>
#include <iostream>

#include <qcheckbox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qvalidator.h>

#include <QString>
#include <QTextStream>

static toSQL SQLComment("toOracleConnection:Comments",
                        "SELECT Column_name,Comments FROM sys.All_Col_Comments\n"
                        " WHERE Owner = :f1<char[100]>\n"
                        "   AND Table_Name = :f2<char[100]>",
                        "Display column comments");

static toSQL SQLMembers("toOracleConnection:Members",
                        "SELECT object_name,overload,argument_name,data_type\n"
                        "  FROM sys.All_Arguments\n"
                        " WHERE Owner = :f1<char[100]>\n"
                        "   AND Package_Name = :f2<char[100]>\n"
                        " ORDER BY object_name,overload,DECODE(argument_name,NULL,9999,sequence)",
                        "Get list of package members");

static toSQL SQLListObjects("toOracleConnection:ListObjects",
                            "select a.owner,a.object_name,a.object_type,b.comments\n"
                            "  from sys.all_objects a,\n"
                            "       sys.all_tab_comments b\n"
                            " where a.owner = b.owner(+) and a.object_name = b.table_name(+)\n"
                            "   and a.object_type = b.table_type(+) and a.object_type <> 'SYNONYM'",
                            "List the objects to cache for a connection, should have same "
                            "columns and binds");

static toSQL SQLListSynonyms("toOracleConnection:ListSynonyms",
                             "select synonym_name,table_owner,table_name\n"
                             "  from sys.all_synonyms\n"
                             " where owner = :usr<char[101]> or owner = 'PUBLIC'\n"
                             " order by table_owner,table_name",
                             "List the synonyms available to a user, should have same columns and binds");

static void ThrowException(const ::trotl::OciException &exc)
{
	TLOG(0,toDecorator,__HERE__)
		<< "What:" << exc.what() << std::endl
		<< exc.get_sql() << std::endl
		<< "--------------------------------------------------------------------------------"
		<< std::endl;
	if (exc.get_code() == 24344)
		throw toConnection::exception(QString::fromLatin1("ORA-24344 success with compilation error"));
	else
	{
		toConnection::exception ret(QString::fromUtf8(exc.get_mesg()));
		/***
		    if (ret.isEmpty())
		    {
		    if (exc.code != 0)
		    {
		    ret = QString::fromLatin1("ORA-") +
		    QString::number(exc.code) +
		    QString::fromLatin1(" missing error description");
		    }
		    else if (ret.isEmpty())
		    {
		    ret = QString::fromLatin1(
		    "Missing error description. This may be caused by a library "
		    "version mismatch. Check that your ORACLE_HOME and library path is correct.");
		    }
		    }
		*/
		/***
		    if (exc.stm_text && strlen(exc.stm_text))
		    {
		    ret += QString::fromLatin1("\n");
		    QString sql = QString::fromUtf8((const char *)exc.stm_text);
		    if (exc.code >= 0)
		    {
		    QString t = QString::fromUtf8((const char *)exc.stm_text, exc.error_offset);
		    ret.setOffset(t.length());
		    sql.insert(t.length(), QString::fromLatin1("<ERROR>"));
		    }
		    ret += sql;
		    }
		*/
		throw ret;
	}
}

class toOracleClob: public toQValue::complexType
{
public:
	toOracleClob(trotl::OciConnection &_conn)
		: toQValue::complexType(),
		  data(_conn)
	{};
	/* virtual */ bool isBinary() const
	{
		return false;
	}
	/* virtual */ bool isLarge() const
	{
		return false;
	}
	/* virtual */ QString summary() const
	{
		return QString("Datape: Oracle [N]CLOB\n"
			       "Size: %1B\n").arg(data.get_length());
	}
	/* virtual */ QString dataTypeName() const
	{
		return QString("clob");
	}
	~toOracleClob()
	{
		TLOG(1,toDecorator,__HERE__) << "toOracleClob DELETED:" << this << std::endl;
	}

	mutable trotl::SqlClob data;
protected:
	toOracleClob(toOracleClob const&);
	toOracleClob operator=(toOracleClob const&);
	//TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleClob*)

class toOracleBlob: public toQValue::complexType
{
public:
	toOracleBlob(trotl::OciConnection &_conn)
		: toQValue::complexType(),
		  data(_conn)
	{};
	/* virtual */ bool isBinary() const
	{
		return false;
	}
	/* virtual */ bool isLarge() const
	{
		return false;
	}
	/* virtual */ QString summary() const
	{
		return QString("Datape: Oracle BLOB\n"
			       "Size: %1B\n").arg(data.get_length());
	}
	/* virtual */ QString dataTypeName() const
	{
		return QString("blob");
	}	
	~toOracleBlob()
	{
		TLOG(1,toDecorator,__HERE__) << "toOracleBlob DELETED:" << this << std::endl;
	}

	mutable trotl::SqlBlob data;
protected:
	toOracleBlob(toOracleBlob const&);
	toOracleBlob operator=(toOracleBlob const&);
	//TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleBlob*)

class toOracleProvider : public toConnectionProvider
{
	::trotl::OciEnv *_envp;
public:
	class oracleSub : public toConnectionSub
	{
	public:

		::trotl::OciConnection *_conn;
		::trotl::OciLogin *_login;

		oracleSub(::trotl::OciConnection *conn, ::trotl::OciLogin *login):
			_conn(conn), _login(login)
		{}

		~oracleSub()
		{
			try {
				delete _conn;
				delete _login;
			}
			catch(...)
			{
				TLOG(1,toDecorator,__HERE__) << "	Ignored exception." << std::endl;
			}
			_conn = 0;
			_login = 0;
		}

		virtual void cancel(void)
		{
			_conn->cancel();
			TLOG(0,toDecorator,__HERE__) << "oracleSub::cancel(this=" << _conn << ")" << std::endl; 
		}

		virtual void throwExtendedException(toConnection &conn, const ::trotl::OciException &exc)
		{
			TLOG(0,toDecorator,__HERE__)
				<< "What:" << exc.what() << std::endl
				<< exc.get_sql() << std::endl
				<< "--------------------------------------------------------------------------------"
				<< std::endl;
			if (conn.version() < "0800" && exc.get_code() == 0)
			{
				// Serious OCI voodoo to get the Parse error location on Oracle7 servers
				// TODO
				/*
				  Lda_Def lda;
				  Cda_Def cda;
				  if (OCISvcCtxToLda(Connection->connect_struct.svchp,
				  Connection->connect_struct.errhp,
				  &lda) == OCI_SUCCESS)
				  {
				  if (oopen(&cda,
				  &lda,
				  (OraText *)0, -1,
				  -1,
				  (OraText *)0, -1) == OCI_SUCCESS)
				  {
				  cda.peo = 0;
				  oparse(&cda, (OraText *)exc.stm_text, -1, FALSE, 1);
				  ((::trotl::OciException &)exc).code = cda.peo;
				  oclose(&cda);
				  }
				  OCILdaToSvcCtx(&Connection->connect_struct.svchp,
				  Connection->connect_struct.errhp,
				  &lda);
				  }
				*/
			}
			ThrowException(exc);
		}
	};

	class oracleQuery : public toQuery::queryImpl
	{
		bool Cancel;
		bool Running;
		bool SaveInPool;
		
	public:
		class trotlQuery : public ::trotl::SqlStatement
		{
		public:
			typedef ::trotl::SqlStatement super;
			trotlQuery(::trotl::OciConnection &conn,
				   const ::trotl::tstring &stmt, // TODO QString here
				   ub4 lang=OCI_NTV_SYNTAX,
				   int bulk_rows=::trotl::g_OCIPL_BULK_ROWS)
				: ::trotl::SqlStatement(conn, stmt, lang, bulk_rows)
			{
				// Be compatible with otl, execute some statements immediatelly
				if(
					get_stmt_type() == STMT_ALTER
					|| get_stmt_type() == STMT_OTHER // ANALYZE
					|| _in_cnt == 0
					)
					execute_internal(::trotl::g_OCIPL_BULK_ROWS, OCI_DEFAULT);
			};
			void readValue(toQValue &value)
			{
				super::BindPar const &BP(get_stmt_type() == STMT_SELECT ?
							   get_next_column() :
							   get_next_out_bindpar());

				if((_state & EXECUTED) == 0)
					execute_internal(::trotl::g_OCIPL_BULK_ROWS, OCI_DEFAULT);

				if(BP.is_null(_last_buff_row)) {
					value = toQValue();
					TLOG(4,toDecorator,__HERE__) << "Just read: NULL" << std::endl;
				} else {
					switch(BP.dty) {
					case SQLT_NUM:
					case SQLT_VNU: {
						OCINumber* vnu = (OCINumber*) &((char*)BP.valuep)[_last_buff_row * BP.value_sz ];
						sword res;
						boolean isint;
						res = OCINumberIsInt(_errh, vnu, &isint);
						oci_check_error(__HERE__, _errh, res);
						try
						{
							if(isint)
							{
								long long i;						
								res = OCINumberToInt(_errh, 
										     vnu,
										     sizeof(long long),
										     OCI_NUMBER_SIGNED,
										     &i);
								oci_check_error(__HERE__, _errh, res);
								value = toQValue(i);
								TLOG(4,toDecorator,__HERE__) << "Just read: '" << i << '\'' << std::endl;
							} else {
								double d;
								sword res = OCINumberToReal(_errh,
											    vnu,
											    sizeof(double),
											    &d);
								oci_check_error(__HERE__, _errh, res);
								value = toQValue(d);
								TLOG(4,toDecorator,__HERE__) << "Just read: '" << d << '\'' << std::endl;
							}
						} catch(const ::trotl::OciException &e) {
							text str_buf[65];
							ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);
							//const char fmt[]="99999999999999999999999999999999999999D00000000000000000000";
							const char fmt[]="TM";
							sword res = OCINumberToText(_errh,
										    vnu,
										    (const oratext*)fmt,
										    sizeof(fmt) -1,
										    0, // CONST OraText *nls_params,
										    0, // ub4 nls_p_length,
										    (ub4*)&str_len,
										    str_buf );
							oci_check_error(__HERE__, _env._errh, res);
							str_buf[str_len+1] = '\0';
							value = toQValue(QString((const char*)str_buf));
						}							
					}
						break;
#ifdef ORACLE_HAS_XML
					case SQLT_NTY: { // NOTE: Only one SQLT_NTY is supported - SYS.XMLTYPE						
						::trotl::BindParXML const *bpx = dynamic_cast<const trotl::BindParXML *>(&BP);
						assert(bpx);
						if((xmlnode*)bpx->_xmlvaluep[_last_buff_row] == NULL)
						{
							value = toQValue();
							TLOG(4,toDecorator,__HERE__) << "Just read: NULL XML" << std::endl; 
						} else {
							std::string s(BP.get_string(_last_buff_row));
							value = toQValue(QString(s.c_str()));
							TLOG(4,toDecorator,__HERE__) << "Just read: \"" << s << "\"" << std::endl; 
						}
					}
						break;
#endif
					case SQLT_CLOB: {
						toOracleClob *i = new toOracleClob(_conn);
						trotl::ConvertorForRead c(_last_buff_row);
						trotl::DispatcherForRead::Go(BP, i->data, c);
						QVariant v;
						v.setValue((toQValue::complexType*)i);
						value = toQValue::fromVariant(v);
						//int id = qMetaTypeId<toQValue::complexType*>();
						TLOG(4,toDecorator,__HERE__) << "Just read: \"CLOB\"" << std::endl; 
					}
						break;
					case SQLT_BLOB: {
						toOracleBlob *i = new toOracleBlob(_conn);
						trotl::ConvertorForRead c(_last_buff_row);
						trotl::DispatcherForRead::Go(BP, i->data, c);
						QVariant v;
						v.setValue((toQValue::complexType*)i);
						value = toQValue::fromVariant(v);
						TLOG(4,toDecorator,__HERE__) << "Just read: \"BLOB\"" << std::endl; 
					}						
						break;
					default:
						std::string s(BP.get_string(_last_buff_row));
						value = toQValue(QString(s.c_str()));
						TLOG(4,toDecorator,__HERE__) << "Just read: \"" << s << "\"" << std::endl;
					}
				}
				
				if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
					++_last_buff_row;

				if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
					_state |= EOF_DATA; 
				
				if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
					fetch(_buff_size);				
			}
		};
		trotlQuery * Query;
		
		oracleQuery(toQuery *query, oracleSub *)
			: toQuery::queryImpl(query)
		{
			TLOG(0,toDecorator,__HERE__) << std::endl;
			Running = Cancel = false;
			SaveInPool = false;
			Query = NULL;
		}
		virtual ~oracleQuery()
		{
			if (!Query)
				return;

			Query->close();
			delete Query;
		}

		virtual void execute(void);

		virtual toQValue readValue(void)
		{
			toQValue retval;
			Query->readValue(retval);
			return retval;			
		}

		virtual void cancel(void);

		virtual bool eof(void)
		{
			if (!Query || Cancel) {
				TLOG(0,toDecorator,__HERE__) << "eof - on canceled query" << std::endl;
				return true;
			}
			try {
				bool e = Query->eof();
				if(e)
				{
					Running = false;
					TLOG(0,toDecorator,__HERE__) << "eof(" << Query->row_count() << ')' << std::endl;
				}
				return e; //Query->eof();
			}
			catch (const ::trotl::OciException &exc)
			{
				TLOG(0,toDecorator,__HERE__) << "eof(e) - " << exc.what() << std::endl;
				if(query())
				{
					oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
					if(conn)
						conn->throwExtendedException(query()->connection(), exc);
				}
				return true;
			}
		}

		virtual int rowsProcessed(void)
		{
			if (!Query)
			{
				TLOG(0,toDecorator,__HERE__) << "rowsProcessed() - non-query" << std::endl;
				return 0;
			}
			//return Query->get_last_row(); ????
			unsigned i = Query->get_last_row();
			TLOG(0,toDecorator,__HERE__) << "rowsProcessed(" << i << ")" << std::endl;
			return i;
		}

		virtual int columns(void)
		{
			//int descriptionLen;
			//Query->describe_select(descriptionLen);
			TLOG(0,toDecorator,__HERE__) << "columns(" << Query->get_column_count() << ")" << std::endl;
			return Query->get_column_count();
		}

		virtual std::list<toQuery::queryDescribe> describe(void)
		{
			TLOG(0,toDecorator,__HERE__) << std::endl;
			std::list<toQuery::queryDescribe> ret;

			int datatypearg1 = 0;
			int datatypearg2 = 0;
			TLOG(0,toDecorator,__HERE__) << "TODO describe:" << std::endl;

			// TODO trotl should return const iterator

			const std::vector<trotl::ColumnType> &col = Query->get_columns();
			if(col.empty()) return ret;
			std::vector<trotl::ColumnType>::const_iterator it = col.begin(); ++it; // starts with 1st
			//TLOG(0,toDecorator,__HERE__) << "Columns: " << q1.get_column_count() << std::endl;
			for(; it != col.end(); ++it)
			{
				TLOG(0,toDecorator,__HERE__) << "Var: " << (*it).get_type_str(true) << std::endl;
				toQuery::queryDescribe desc;
 				desc.AlignRight = false;
 				desc.Name = QString::fromUtf8( (*it)._column_name.c_str() );
				desc.Datatype = QString::fromLatin1( (*it).get_type_str(true).c_str() );

				//datatypearg1 = description[i].char_size;
				desc.Datatype.sprintf(desc.Datatype.toUtf8().constData(), datatypearg1, datatypearg2);
				//desc.Null = description[i].nullok;

				ret.insert(ret.end(), desc);
			}

			return ret;
		}
	}; //	class oracleQuery : public toQuery::queryImpl

	class oracleConnection : public toConnection::connectionImpl
	{
		::trotl::OciEnv &_env;
		QString connectString(void);

		oracleSub *oracleConv(toConnectionSub *sub)
		{
			oracleSub *conn = dynamic_cast<oracleSub *>(sub);
			if (!conn)
				throw QString::fromLatin1("Internal error, not oracle sub connection");
			return conn;
		}
	public:
		oracleConnection(toConnection *conn, ::trotl::OciEnv &env) :
			toConnection::connectionImpl(conn), _env(env)
		{ }

		/** Return a string representation to address an object.
		 * Checks if identifier has illegal characters, starts with digit, is a reserved
		 * word etc. - if so - returns it enclosed with quotes (otherwise returns the same string).
		 * Note that when identifier name returned from oracle data dictionary is in lowercase
		 * - it MUST be enclosed with quotes (case insensitive "normal" identifiers are always
		 * returned in uppercase).
		 * @param name The name to be quoted.
		 * @param quoteLowercase Enclose in quotes when identifier has lowercase letters.
		 *   When processing data returned by dada dictionary quoteLowercase should be true
		 *   When processing data entered by user quoteLowercase should be false
		 * @return String addressing table.
		 */
		virtual QString quote(const QString &name, const bool quoteLowercase)
		{
			bool ok = true;
			// Identifiers starting with digit should be quoted
			if (name.at(0).isDigit())
				ok = false;
			else
				for (int i = 0; i < name.length(); i++)
				{
					if ((name.at(i).toUpper() != name.at(i) && quoteLowercase) || !toIsIdent(name.at(i)))
						ok = false;
				}

			// Check if given identified is a reserved word
			int i = 0;
			while (ok && (DefaultKeywords[i] != NULL))
			{
				if (name.compare(DefaultKeywords[i], Qt::CaseInsensitive) == 0)
					ok = false;
				i++;
			}
			if (ok)
			{
				if (toConfigurationSingle::Instance().objectNamesUpper())
					return name.toUpper();
				else
					return name.toLower();
			}
			else
				return QString::fromLatin1("\"") + name + QString::fromLatin1("\"");
		}
		
		virtual QString unQuote(const QString &str)
		{
			if (str.at(0).toLatin1() == '\"' && str.at(str.length() - 1).toLatin1() == '\"')
				return str.left(str.length() - 1).right(str.length() - 2);
			return str.toUpper();
		}
		
		virtual std::list<toConnection::objectName> objectNames(void)
		{
			std::list<toConnection::objectName> ret;

			std::list<toQValue> par;
			toQuery objects(connection(), toQuery::Long,
					SQLListObjects, par);
			toConnection::objectName cur;
			while (!objects.eof())
			{
				cur.Owner = objects.readValueNull();
				cur.Name = objects.readValueNull();
				cur.Type = objects.readValueNull();
				cur.Comment = objects.readValueNull();
				ret.insert(ret.end(), cur);
			}
			TLOG(0,toDecorator,__HERE__)
				<< "++++ objectNames +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
				<< std::endl;

			return ret;
		}
		virtual std::map<QString, toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
		{
			std::map<QString, toConnection::objectName> ret;

			toConnection::objectName cur;
			cur.Type = QString::fromLatin1("A");
			std::list<toQValue> par;
			par.insert(par.end(), toQValue(connection().user().toUpper()));
			toQuery synonyms(connection(), toQuery::Long,
					 SQLListSynonyms, par);
			std::list<toConnection::objectName>::iterator i = objects.begin();
			while (!synonyms.eof())
			{
				QString synonym = synonyms.readValueNull();
				cur.Owner = synonyms.readValueNull();
				cur.Name = synonyms.readValueNull();
				while (i != objects.end() && (*i) < cur)
					i++;
				if (i == objects.end())
					break;
				if (cur.Name == (*i).Name && cur.Owner == (*i).Owner)
				{
					ret[synonym] = (*i);
					(*i).Synonyms.insert((*i).Synonyms.end(), synonym);
				}
			}
			TLOG(0,toDecorator,__HERE__)
				<< "++++ synonymMap ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
				<< std::endl;			

			return ret;
		}
		virtual toQDescList columnDesc(const toConnection::objectName &table)
		{
			toBusy busy;
			if (table.Type == QString::fromLatin1("PACKAGE"))
			{
				toQDescList ret;
				try
				{
					toQuery::queryDescribe desc;
					desc.Datatype = ("MEMBER");
					desc.Null = false;
					QString lastName;
					QString lastOver;
					toQuery member(connection(), SQLMembers, table.Owner, table.Name);
					bool hasArgs = false;
					while (!member.eof())
					{
						QString name = member.readValue();
						QString overld = member.readValue();
						QString arg = member.readValueNull();
						QString type = member.readValueNull();
						if (lastName != name || overld != lastOver)
						{
							if (hasArgs)
								desc.Name += ")";
							if (!desc.Name.isEmpty())
								ret.insert(ret.end(), desc);
							desc.Name = name;
							lastName = name;
							lastOver = overld;
							hasArgs = false;
						}
						if (arg.isEmpty())
						{
							if (hasArgs)
							{
								desc.Name += ")";
								hasArgs = false;
							}
							desc.Name += " RETURNING ";
						}
						else
						{
							if (hasArgs)
								desc.Name += ", ";
							else
							{
								desc.Name += "(";
								hasArgs = true;
							}
							desc.Name += arg;
							desc.Name += " ";
						}
						desc.Name += type;
					}
					if (desc.Name.contains("("))
						desc.Name += ")";
					if (!desc.Name.isEmpty())
						ret.insert(ret.end(), desc);
				}
				catch (...)
				{
					TLOG(1,toDecorator,__HERE__) << "	Ignored exception." << std::endl;
				}
				return ret;
			}

			std::map<QString, QString> comments;
			try
			{
				toQuery comment(connection(), SQLComment, table.Owner, table.Name);
				while (!comment.eof())
				{
					QString col = comment.readValue();
					comments[col] = comment.readValueNull();
				}
			}
			catch (...)
			{
				TLOG(1,toDecorator,__HERE__) << "	Ignored exception." << std::endl;
			}

			try
			{
				toQuery query(connection());
#ifdef OTL_STREAM_POOLING_ON
				// Need to clear the stream cache first.
				oracleSub *sub = dynamic_cast<oracleSub *>(query.connectionSub());
				sub->Lock.down();
				sub->Connection->set_stream_pool_size(std::max(toConfigurationSingle::Instance().openCursors(), 1));
				sub->Lock.up();
#endif

				QString SQL = QString::fromLatin1("SELECT * FROM \"");
				SQL += table.Owner;
				SQL += QString::fromLatin1("\".\"");
				SQL += table.Name;
				SQL += QString::fromLatin1("\" WHERE NULL=NULL");
				toQList par;
				query.execute(SQL, par);
				toQDescList desc = query.describe();
				for (toQDescList::iterator j = desc.begin();j != desc.end();j++)
					(*j).Comment = comments[(*j).Name];

				return desc;
			}
			catch ( ... )
			{
				throw;
			}

			toQDescList ret;
			return ret;
		}

		virtual void commit(toConnectionSub *sub)
		{
			oracleSub *conn = oracleConv(sub);
			try
			{
				conn->_conn->commit();
			}
			catch (const ::trotl::OciException &exc)
			{
				ThrowException(exc);
			}
		}
		virtual void rollback(toConnectionSub *sub)
		{
			oracleSub *conn = oracleConv(sub);
			try
			{
				conn->_conn->rollback();
			}
			catch (const ::trotl::OciException &exc)
			{
				ThrowException(exc);
			}
		}

		virtual toConnectionSub *createConnection(void);

		void closeConnection(toConnectionSub *conn)
		{
			delete conn;
		}

		virtual QString version(toConnectionSub *sub)
		{
			oracleSub *connp = oracleConv(sub);
			try
			{
				::trotl::OciLogin &login( *(connp->_login) );
				::std::stringstream version;
				version << login._server.versionNumber() << "."
					<< login._server.releaseNumber() << "."
					<< login._server.updateNumber() << "."
					<< login._server.portReleaseNumber() << "."
					<< login._server.portUpdateNumber();

				return QString(version.str().c_str());
			}
			catch (::trotl::OciException e) {
				TLOG(0,toDecorator,__HERE__) << e.what() << std::endl;
			}
			catch (...)
			{
				TLOG(1,toDecorator,__HERE__) << "	Ignored exception." << std::endl;
				// Ignore any errors here
			}
			return QString::QString();
		}

		virtual toQuery::queryImpl *createQuery(toQuery *query, toConnectionSub *sub)
		{
			return new oracleQuery(query, oracleConv(sub));
		}


		/* TODO I have no idea when this method is used.
		   instance of toQuery is created and then dropped - why ?
		   NOTE: there is one more execute method
		*/
		virtual void execute(toConnectionSub *sub, const QString &sql, toQList &params)
		{
			oracleSub *conn = oracleConv(sub);
			TLOG(0,toDecorator,__HERE__) << std::endl;
			if ( params.empty() )
			{
				try
				{
				  std::string _sql(sql.toUtf8().constData()); 
				  oracleQuery::trotlQuery query(*conn->_conn, _sql);
				  query.execute_internal(::trotl::g_OCIPL_BULK_ROWS, OCI_DEFAULT);
				  //otl_cursor::direct_exec(*(conn->Connection), sql.toUtf8().constData());
				  //TODO
				  TLOG(0,toDecorator,__HERE__) << "TODO: otl_cursor::direct_exec" << std::endl;
				}
				catch (const ::trotl::OciException &exc)
				{
					conn->throwExtendedException(connection(), exc);
				}
			}
			else
				toQuery query(connection(), sql, params);
		}

		virtual void parse(toConnectionSub *sub, const QString &sql)
		{
			oracleSub *conn = oracleConv(sub);
			try
			{
			        //oracleQuery::oracleSqlStatement s1(*conn->_conn, sql.toUtf8().constData());
				oracleQuery::trotlQuery s1(*conn->_conn, ::std::string(sql.toUtf8().constData())); 
				//conn->Connection->reset_throw_count();
				//conn->Connection->syntax_check(sql.toUtf8().constData());

			}
			catch (const ::trotl::OciException &exc)
			{
				//conn->Connection->throw_count++;
				ThrowException(exc);
			}
		}

	}; // class oracleConnection : public toConnection::connectionImpl

	toOracleProvider(void)
		: toConnectionProvider("Oracle", false)
	{}

	virtual void initialize(void)
	{
		//toMaxLong = toConfigurationSingle::Instance().maxLong();
		::trotl::g_OCIPL_MAX_LONG = ( toConfigurationSingle::Instance().maxLong() == -1 ? ::trotl::g_OCIPL_MAX_LONG : toConfigurationSingle::Instance().maxLong() );
		::trotl::OciEnvAlloc *_envallocp = new ::trotl::OciEnvAlloc;

		_envp = new ::trotl::OciEnv(*_envallocp);

		//TLOG(0,toDecorator,__HERE__) << "TODO: initialize" << std::endl << __HERE__ << std::endl;

		addProvider("Oracle");
	}

	virtual toConnection::connectionImpl *provideConnection(const QString &, toConnection *conn)
	{
		return new oracleConnection(conn, *_envp);
	}

	virtual std::list<QString> providedOptions(const QString &)
	{
		std::list<QString> ret;
		ret.insert(ret.end(), "*SQL*Net");
		ret.insert(ret.end(), "-");
		ret.insert(ret.end(), "SYS_OPER");
		ret.insert(ret.end(), "SYS_DBA");
		return ret;
	}

	virtual std::list<QString> providedHosts(const QString &)
	{
		std::list<QString> ret;
		ret.insert(ret.end(), QString::null);
		ret.insert(ret.end(), QString::fromLatin1("SQL*Net"));
		return ret;
	}

	virtual std::list<QString> providedDatabases(const QString &,
						     const QString &host,
						     const QString &,
						     const QString &)
	{
		QString str;
#ifdef Q_OS_WIN32
		CRegistry registry;
		DWORD siz = 1024;
		char buffer[1024];
		try
		{
			if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
						    "SOFTWARE\\ORACLE\\HOME0",
						    "TNS_ADMIN",
						    buffer, siz))
			{
				if (siz > 0)
					str = buffer;
				else
					throw 0;
			}
			else
				throw 0;
		}
		catch (...)
		{
			try
			{
				if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
							    "SOFTWARE\\ORACLE\\HOME0",
							    "ORACLE_HOME",
							    buffer, siz))
				{
					if (siz > 0)
					{
						str = buffer;
						str += "\\network\\admin";
					}
					else
						throw 0;
				}
				else
					throw 0;
			}
			catch (...)
			{
				if (GetEnvironmentVariableA("TNS_ADMIN", buffer, siz) > 0)
					str = buffer;
			}
		}
		str.append(QString::fromLatin1("/tnsnames.ora"));
#else
		if (getenv("TNS_ADMIN"))
		{
			str = getenv("TNS_ADMIN");
		}
		else
		{
			str = getenv("ORACLE_HOME");
			str.append(QString::fromLatin1("/network/admin"));
		}

		str.append(QString::fromLatin1("/tnsnames.ora"));

		

		str = QString("/home/ibrezina/.tnsnames.ora");
#endif
		std::list<QString> ret;

		QFile file(str);
		if (!file.open(QIODevice::ReadOnly))
			return ret;

		QTextStream in(&file);
		QByteArray barray = in.readAll().toUtf8();
		const char *buf = barray.constData();

		int begname = -1;
		int parambeg = -1;
		int pos = 0;
		int param = 0;
		while (pos < barray.size())
		{
			if (buf[pos] == '#')
			{
				while (pos < barray.size() && buf[pos] != '\n')
					pos++;
			}
			else if (buf[pos] == '=')
			{
				if (param == 0)
				{
					if (begname >= 0 && !host.isEmpty())
						ret.insert(ret.end(), QString::fromLatin1(buf + begname, pos - begname));
				}
			}
			else if (buf[pos] == '(')
			{
				begname = -1;
				parambeg = pos + 1;
				param++;
			}
			else if (buf[pos] == ')')
			{
				if (parambeg >= 0 && host.isEmpty())
				{
					QString tmp = QString::fromLatin1(buf + parambeg, pos - parambeg);
					tmp.replace(QRegExp(QString::fromLatin1("\\s+")), QString::null);
					if (tmp.toLower().startsWith(QString::fromLatin1("sid=")))
						ret.insert(ret.end(), tmp.mid(4));
				}
				begname = -1;
				parambeg = -1;
				param--;
			}
			else if (!isspace(buf[pos]) && begname < 0)
			{
				begname = pos;
			}
			pos++;
		}
		return ret;
	}

	virtual QWidget *providerConfigurationTab(const QString &provider, QWidget *parent);
};

static toOracleProvider OracleProvider;

void toOracleProvider::oracleQuery::execute(void)
{
	oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
	if (!conn)
		throw QString::fromLatin1("Internal error, not oracle sub connection");
	try
	{
		if(Query) delete Query;
		Query = NULL;

		if (Cancel)
			throw QString::fromLatin1("Query aborted before started");
		Running = true;

		QRegExp stripnl("\r");
		QString sql = this->query()->sql();
		sql.replace(stripnl, "");
		
		//Query = new oracleQuery::oracleSqlStatement(*conn->_conn, sql.toUtf8().constData());
		Query = new oracleQuery::trotlQuery(*conn->_conn, ::std::string(sql.toUtf8().constData()));
		TLOG(0,toDecorator,__HERE__) << "SQL(conn=" << conn->_conn << ", this=" << Query << "): " << ::std::string(sql.toUtf8().constData()) << std::endl;
		// TODO autocommit ??
		// Query->set_commit(0);
		//if (toQValue::numberFormat() == 0)
		//    Query->set_all_column_types(otl_all_num2str | otl_all_date2str);
		//else
		//    Query->set_all_column_types(otl_all_date2str);

		//Query->open(1, sql.toUtf8().constData(), *(conn->Connection));
	}
	catch (const ::trotl::OciException &exc)
	{
		delete Query;
		Query = NULL;

		Running = false;
		conn->throwExtendedException(query()->connection(), exc);
	}
	try
	{
		for (toQList::iterator i = query()->params().begin();i != query()->params().end();i++)
		{
 			const trotlQuery::BindPar& bp = (*Query).get_curr_in_bindpar();
			
			if( (bp._bind_type & ( trotlQuery::BindPar::BIND_IN | trotlQuery::BindPar::BIND_OUT )) == 0)
				throw toConnection::exception(QString::fromLatin1("Fatal pruser error - invalid BindPar"));
			
			if( bp.bind_typename == "int" /*&& (*i).isInt()*/ ) {
				(*Query) << (*i).toInt();
				TLOG(0,toDecorator,__HERE__)
					<< "<<(conn=" << conn->_conn << ", this=" << Query << ")"
					<< "::operator<<(" << bp.type_name << " ftype=" << bp.dty
					<< ", placeholder=" << bp.bind_name
					<< ", value=" << (*i).toInt() << ");"
					<< "\t of:" << query()->params().size() << std::endl;
			} else if( (bp.bind_typename == "char" || bp.bind_typename == "varchar") && (*i).isString()) {
				std::string param((const char*)((*i).toString().toUtf8().constData()));
				(*Query) << ::std::string((const char*)((*i).toString().toUtf8().constData()));
				TLOG(0,toDecorator,__HERE__)
					<< "<<(conn=" << conn->_conn << ", this=" << Query << ")"
					<< "::operator<<(" << bp.type_name << " ftype=" << bp.dty
					<< ", placeholder=" << bp.bind_name
					<< ", value=" << ::std::string((const char*)((*i).toString().toUtf8().constData())) << ");"
					<< "\t of:" << query()->params().size() << std::endl;
			} else {
				std::cerr << "Fatal pruser error - unsupported BindPar:" << std::endl;
				throw toConnection::exception(
					QString::fromLatin1("Fatal pruser error - unsupported BindPar:") + bp.bind_typename.c_str());
				exit(-1);
			}			
		}

		//Running = false;
	}
	catch (const ::trotl::OciException &exc)
	{
		conn->_conn->reset();
		delete Query;
		Query = NULL;

		Running = false;
		conn->throwExtendedException(query()->connection(), exc);
	}
}

void toOracleProvider::oracleQuery::cancel(void)
{
	oracleSub *conn = dynamic_cast<oracleSub *>(query()->connectionSub());
	if(!Running || Cancel)
	{
		TLOG(0,toDecorator,__HERE__) << ":oracleQuery::cancel(conn=" << conn->_conn << ", this=" << Query << ") on non-running query" << std::endl;	
		return;
	}
	
	if (!conn)
		throw QString::fromLatin1("Internal error, not oracle sub connection");

	conn->_conn->cancel();
	conn->_conn->reset();
	Cancel = true;
	Running = false;
	TLOG(0,toDecorator,__HERE__) << ":oracleQuery::cancel(conn=" << conn->_conn << ", this=" << Query << ")" << std::endl;
}


QString	toOracleProvider::oracleConnection:: connectString(void)
{
	QString ret;
	ret = connection().user().toUtf8();
	ret += QString::fromLatin1("/");
	ret += connection().password().toUtf8();
	if (!connection().host().isEmpty())
	{
		ret += QString::fromLatin1("@");
		ret += connection().database().toUtf8();
	}
	return ret;
}

toConnectionSub* toOracleProvider::oracleConnection::createConnection(void)
{
  	::trotl::OciConnection *conn = NULL;
	::trotl::OciLogin *login = NULL;  
	QString oldSid;

	std::set<QString> options = connection().options();

	bool sqlNet = (options.find("SQL*Net") != options.end());
	if (!sqlNet)
	{
		oldSid = getenv("ORACLE_SID");
		toSetEnv("ORACLE_SID", connection().database());
	}
	try
	{
		int session_mode = OCI_DEFAULT;
		if (options.find("SYS_OPER") != options.end())
			session_mode = OCI_SYSOPER;
		else if (options.find("SYS_DBA") != options.end())
			session_mode = OCI_SYSDBA;
		do
		{

#ifdef OTL_STREAM_POOLING_ON
			conn->set_stream_pool_size(std::max(toConfigurationSingle::Instance().openCursors(), 1));
#endif
			/* TODO
			   if (!sqlNet)
			   conn->server_attach();
			   else
			*/
			QString user = connection().user();
			QString pass = connection().password();

			try
			{
				// TODO what does _login destructor? and where is it?
				/*::trotl::OciLogin */ login = new ::trotl::OciLogin(_env,
								::trotl::LoginPara(
									user.isEmpty() ? "" : user.toUtf8().constData(),
									pass.isEmpty() ? "" : pass.toUtf8().constData(),
									connection().database().toUtf8().constData()
									),
								(ub4) session_mode);
				conn = new ::trotl::OciConnection(_env, *login);

				TLOG(0,toDecorator,__HERE__) << "Oracle database version: "
					    << ::std::hex << ::std::showbase << ::std::setw(10)
					    << ::std::setfill('0') << ::std::internal
					    << login->_server._version << ::std::endl
					    << login->_server._version_string << ::std::endl
					    << login->_server.versionNumber() << "."
					    << login->_server.releaseNumber() << "."
					    << login->_server.updateNumber() << "."
					    << login->_server.portReleaseNumber() << "."
					    << login->_server.portUpdateNumber()
					    << ::std::dec << ::std::endl;
				
			}
			catch (const ::trotl::OciException &exc)
			{
			        TLOG(0,toDecorator,__HERE__)
					<< "TODO: catch" << std::endl << __HERE__ << std::endl;
 				if (toThread::mainThread() && exc.get_code() == 28001)
 				{
 					bool ok = false;
 					QString newpass = QInputDialog::getText(
 						toMainWidget(),
 						qApp->translate("toOracleConnection", "Password expired"),
 						qApp->translate("toOracleConnection", "Enter new password"),
 						QLineEdit::Password,
 						QString::null,
 						&ok);
 					if (!ok)
						throw exc;
					
					QString newpass2 = QInputDialog::getText(
						toMainWidget(),
						qApp->translate("toOracleConnection", "Password expired"),
						qApp->translate("toOracleConnection", "Enter password again for confirmation"),
						QLineEdit::Password,
						QString::null,
						&ok);
					
					if (!ok)
						throw exc;
					
					if (newpass2 != newpass)
						throw qApp->translate("toOracleConnection", "The two passwords doesn't match");
					QString nputf = newpass;
					if( login ) delete login;
					login = new ::trotl::OciLogin(_env,
								      ::trotl::LoginAndPChangePara(
									      user.isEmpty() ? "" : user.toUtf8().constData(),
									      pass.isEmpty() ? "" : pass.toUtf8().constData(),
									      newpass.isEmpty() ? "" : newpass.toUtf8().constData(), 
									      connection().database().toUtf8().constData()
									      ),
								      (ub4) session_mode);
					conn = new ::trotl::OciConnection(_env, *login);
					
					connection().setPassword(newpass);
 				} else { 
					TLOG(0,toDecorator,__HERE__) << std::endl;
					throw exc;
				} //  (toThread::mainThread() && exc.get_code() == 28001)
			} // catch (const ::trotl::OciException &exc)
		}
		while (!conn);
	}
	catch (const ::trotl::OciException &exc)
	{
		if (!sqlNet)
		{
			if (oldSid.isNull())
				toUnSetEnv("ORACLE_SID");
			else
				toSetEnv("ORACLE_SID", oldSid.toLatin1());
		}
		delete conn;
		ThrowException(exc);
	}
	if (!sqlNet)
	{
		if (oldSid.isNull())
			toUnSetEnv("ORACLE_SID");
		else
		{
			toSetEnv("ORACLE_SID", oldSid.toLatin1());
		}
	}

	try
	{
		QString str = QString::fromLatin1("ALTER SESSION SET NLS_DATE_FORMAT = '");
		str += toConfigurationSingle::Instance().dateFormat();
		str += QString::fromLatin1("'");
		oracleQuery::trotlQuery date(*conn, ::std::string(str.toAscii().constData()));
	}
	catch (...)
	{
		printf("Failed to set new default date format for session\n");
		toStatusMessage(QObject::tr("Failed to set new default date format for session: %1")
				.arg(toConfigurationSingle::Instance().dateFormat()));
	}

	try
	{
		//oracleQuery::oracleSqlStatement info(*conn,
		oracleQuery::trotlQuery info(*conn,  std::string(
						     "BEGIN\n"
						     "  SYS.DBMS_APPLICATION_INFO.SET_CLIENT_INFO('" TOAPPNAME
						     " (http://tora.sf.net)"
						     "');\n"
						     "  SYS.DBMS_APPLICATION_INFO.SET_MODULE('" TOAPPNAME "','Access Database');\n"
						     "END;"));
	}
	catch (::trotl::OciException const&e)
	{
		TLOG(0,toDecorator,__HERE__) << "Failed to set client info for session:\n" << e.what();
	}
	catch (...)
	{
		TLOG(0,toDecorator,__HERE__) << "Failed to set client info for session\n";
	}

	return new oracleSub(conn, login);
}

/*
** 11g version, see $ORACLE_HOME/rdbms/admin/utlxplan.sql
*/
static toSQL SQLCreatePlanTable(toSQL::TOSQL_CREATEPLAN,
                                "CREATE TABLE %1 (\n"
                                "STATEMENT_ID        VARCHAR2(30),\n"
                                "PLAN_ID             NUMBER,\n"
                                "TIMESTAMP           DATE,\n"
                                "REMARKS             VARCHAR2(4000),\n"
                                "OPERATION           VARCHAR2(30),\n"
                                "OPTIONS             VARCHAR2(255),\n"
                                "OBJECT_NODE         VARCHAR2(128),\n"
                                "OBJECT_OWNER        VARCHAR2(30),\n"
                                "OBJECT_NAME         VARCHAR2(30),\n"
                                "OBJECT_ALIAS        VARCHAR2(65),\n"
                                "OBJECT_INSTANCE     NUMERIC,\n"
                                "OBJECT_TYPE         VARCHAR2(30),\n"
                                "OPTIMIZER           VARCHAR2(255),\n"
                                "SEARCH_COLUMNS      NUMBER,\n"
                                "ID                  NUMERIC,\n"
                                "PARENT_ID           NUMERIC,\n"
                                "DEPTH               NUMERIC,\n"
                                "POSITION            NUMERIC,\n"
                                "COST                NUMERIC,\n"
                                "CARDINALITY         NUMERIC,\n"
                                "BYTES               NUMERIC,\n"
                                "OTHER_TAG           VARCHAR2(255),\n"
                                "PARTITION_START     VARCHAR2(255),\n"
                                "PARTITION_STOP      VARCHAR2(255),\n"
                                "PARTITION_ID        NUMERIC,\n"
                                "OTHER               LONG,\n"
                                "DISTRIBUTION        VARCHAR2(30),\n"
                                "CPU_COST            NUMERIC,\n"
                                "IO_COST             NUMERIC,\n"
                                "TEMP_SPACE          NUMERIC,\n"
                                "ACCESS_PREDICATES   VARCHAR2(4000),\n"
                                "FILTER_PREDICATES   VARCHAR2(4000),\n"
                                "PROJECTION          VARCHAR2(4000),\n"
                                "TIME                NUMERIC,\n"
                                "QBLOCK_NAME         VARCHAR2(30),\n"
                                "OTHER_XML           CLOB\n"
                                ")",
                                "Create plan table, must have same % signs");


toOracleSetting::toOracleSetting(QWidget *parent)
        : QWidget(parent), toSettingTab("database.html#oracle")
{

	setupUi(this);
	DefaultDate->setText(toConfigurationSingle::Instance().dateFormat());
	CheckPoint->setText(toConfigurationSingle::Instance().planCheckpoint());
	ExplainPlan->setText(toConfigurationSingle::Instance().planTable());
	OpenCursors->setValue(toConfigurationSingle::Instance().openCursors());
	KeepPlans->setChecked(toConfigurationSingle::Instance().keepPlans());
 	VsqlPlans->setChecked(toConfigurationSingle::Instance().vsqlPlans());
	int len = toConfigurationSingle::Instance().maxLong();
	if (len >= 0)
	{
		MaxLong->setText(QString::number(len));
		MaxLong->setValidator(new QIntValidator(MaxLong));
		Unlimited->setChecked(false);
	}
	try
	{
		// Check if connection exists
		toMainWidget()->currentConnection();
		CreatePlanTable->setEnabled(true);
	}
	catch (...)
        {
		TLOG(1,toDecorator,__HERE__) << "	Ignored exception." << std::endl;
	}
}


void toOracleSetting::saveSetting()
{
	toConfigurationSingle::Instance().setKeepPlans(KeepPlans->isChecked());
 	toConfigurationSingle::Instance().setVsqlPlans(VsqlPlans->isChecked());
	toConfigurationSingle::Instance().setDateFormat(DefaultDate->text());

	// try to change NLS for already running sessions
	QString str("ALTER SESSION SET NLS_DATE_FORMAT = '%1'");
	foreach (QString c, toMainWidget()->connections())
	{
		try
		{
			toConnection &conn = toMainWidget()->connection(c);

			if(toIsOracle(conn))
			{
				conn.allExecute(
					str.arg(toConfigurationSingle::Instance().dateFormat()));
			}
		}
		catch(...) {
			toStatusMessage(tr("Failed to set new default date format for connection: %1").arg(c));
		}
	}

	toConfigurationSingle::Instance().setPlanCheckpoint(CheckPoint->text());
	toConfigurationSingle::Instance().setPlanTable(ExplainPlan->text());
	toConfigurationSingle::Instance().setOpenCursors(OpenCursors->value());

	/*
	  TODO - there is nothing like "unlimited" size for LONG.
	  max size for LONG col. is 2GB, so we should prepare
	  at least one 2GB buffer in RAM.
	  In reality we use 30000B if toMaxLong == -1.
	*/
	if (Unlimited->isChecked())
	{
		//toMaxLong = -1;
		toConfigurationSingle::Instance().setMaxLong(-1);
	}
	else
	{
		toConfigurationSingle::Instance().setMaxLong(MaxLong->text().toInt());
		//toMaxLong = MaxLong->text().toInt();
	}
}


void toOracleSetting::createPlanTable()
{
	try
	{
		toConnection &conn = toMainWidget()->currentConnection();
		conn.execute(toSQL::string(SQLCreatePlanTable, conn).
			     arg(ExplainPlan->text()));
	}
	TOCATCH;
}


QWidget *toOracleProvider::providerConfigurationTab(const QString &, QWidget *parent)
{
	return new toOracleSetting(parent);
}

#endif
