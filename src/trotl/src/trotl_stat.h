// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Martin Fuchs <martin-fuchs@gmx.net>
//
// forked from OCIPL Version 1.3
// by Ivan Brezina <ivan@cvut.cz>

/*

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in
	the documentation and/or other materials provided with the
	distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TROTL_STAT_H_
#define TROTL_STAT_H_

#include <assert.h>
#include <vector>
#include <map>

#include <boost/ptr_container/ptr_vector.hpp>

#include "trotl_export.h"
#include "trotl_var.h"
#include "trotl_conn.h"
#include "trotl_parser.h"

//#include "trotl_int.h"

#include "Loki/static_check.h"
#include "Loki/TypeTraits.h"

namespace trotl {

  
inline void oci_check_error(tstring where, SqlStatement &stmt, sword res)
{
	if (res != OCI_SUCCESS)
		throw OciException(where, stmt);
};


class TROTL_EXPORT SqlStatement : public OciHandle<OCIStmt>
{
	typedef OciHandle<OCIStmt> super;

public:
	enum STMT_TYPE {
		STMT_OTHER = 0, // truncate table, ...
		STMT_SELECT = OCI_STMT_SELECT,
		STMT_UPDATE = OCI_STMT_UPDATE,
		STMT_DELETE = OCI_STMT_DELETE,
		STMT_INSERT = OCI_STMT_INSERT,
		STMT_CREATE = OCI_STMT_CREATE,
		STMT_DROP = OCI_STMT_DROP,
		STMT_ALTER = OCI_STMT_ALTER,
		STMT_BEGIN = OCI_STMT_BEGIN,
		STMT_DECLARE = OCI_STMT_DECLARE,
		STMT_NONE
	};

	/* This structure represents a pack of all values needed for bind/define operation
	 * All descendents of this class are created using factory BindParFactTwoParmSing
	 * or DefineParFactTwoParm
	 * All descendents have to be resistered by creating an instance of the template
	 * Util::RegisterInFactory - look at trotl_int.cpp
	 **/
	struct TROTL_EXPORT BindPar
	{
		enum {
			BIND_IN=1,
			BIND_INOUT=2,
			BIND_OUT=4,
			DEFINE_SELECT=8
		};

		/* Placeholder for Define operations */		
		BindPar(unsigned int pos, SqlStatement &stmt, ColumnType &ct)
			: valuep(NULL)
			, indp(NULL)
			, rlenp(NULL)
			// , rcodep(NULL)
			, alenp(NULL)
			, mode(OCI_DEFAULT)
			, bindp(NULL)
			, defnpp(NULL)
			, _env(stmt._env)
			, _stmt(stmt)
			, _pos(pos)
			, _max_cnt(g_OCIPL_BULK_ROWS)
			, _cnt(g_OCIPL_BULK_ROWS)
			, _bound(false)
			, type_name("")
			, reg_name("")
			, bind_name("")
			, bind_typename("")
		{
			indp = (OCIInd*) calloc(_cnt, sizeof(OCIInd));
			rlenp = calloc(_cnt, sizeof(ub2)); // OciDefineByPos uses ub2* for lenp 
		
			_bind_type = DEFINE_SELECT;
		};
		
		/* Placeholder for Bind operations */
		BindPar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
			: valuep(NULL)
			, indp(NULL)
			, rlenp(NULL)
			// , rcodep(NULL)
			, alenp(NULL)
			, mode(OCI_DEFAULT)
			, bindp(NULL)
			, defnpp(NULL)
			, _env(stmt._env)
			, _stmt(stmt)
			, _pos(pos)
			, _max_cnt(decl.bracket[1])
			, _cnt(decl.bracket[1])
			, _bound(false)
			, type_name("")
			, reg_name("")		  
			, bind_name(decl.bindname)
			, bind_typename(decl.bindtype)
		{
			indp = (OCIInd*) calloc(_cnt, sizeof(OCIInd));
			rlenp = calloc(_cnt, sizeof(ub4)); // OCIBindByPos uses ub4* for lenp			
			
			if(decl.inout == "in")
				_bind_type = BIND_IN;
			else if(decl.inout == "inout")
				_bind_type = BIND_INOUT;
			else if(decl.inout == "out")
				_bind_type = BIND_OUT;
		};


		virtual ~BindPar()
		{
			if(indp) { free(indp); indp = NULL; }
			if(rlenp) { free(rlenp); rlenp = NULL; }
			// if(rcodep) { delete[] rcodep; rcodep = NULL; }
			if(alenp) { free(alenp); alenp = NULL; }
		};
		
		// every datatype can be converted to a string
		virtual tstring get_string(unsigned int row) const = 0;
	
		/* These two callbacks are used by descendents to call
		 * OCIBindArrayOfStructures, OCIBindObject etc, ...
		 * is called *after* OCIDefineByPos
		 */
		virtual void bind_hook(SqlStatement&) {};
		virtual void define_hook(SqlStatement&) {};

		/* members used for OCI calls */
		dvoid **valuep;
		sb4 value_sz;
		ub2 dty;
		OCIInd *indp;	// OCIInd aka sb2 ignored for SQL_NTY and SQL_REF
		void *rlenp;
		//ub2 *rcodep;
		ub2 *alenp;
		ub4 mode;	// define mode = OCI_DEFAULT, except for SQLT_LONG => OCI_DYNAMIC_FETCH TODO fix long
		OCIBind *bindp;
		OCIDefine *defnpp;  //TODO union with OCIBind *bindpp
		
		OciEnv &_env;
		SqlStatement &_stmt;
		unsigned int _pos, /*_cnt,*/ _max_cnt, _bind_type;
		size_t _cnt;
		bool _bound;
		tstring type_name, reg_name, bind_name, bind_typename;

		/* NOTE these two functions do not work with complex types SQLT_NTY */
		bool is_null(unsigned row) const {return indp[row]==OCI_IND_NULL;};
		bool is_not_null(unsigned row) const {return indp[row]!=OCI_IND_NULL;};		
	protected:

		BindPar(const BindPar &other);
		friend class SqlStatement;
	}; // class BindPar
	
	SqlStatement(OciConnection& conn, const tstring& stmt, ub4 lang=OCI_NTV_SYNTAX, int bulk_rows=g_OCIPL_BULK_ROWS);

	bool execute_internal(ub4 rows, ub4 mode);
	ub4 row_count() const;
	ub4 fetched_rows() const;

	STMT_TYPE get_stmt_type() const { return _stmt_type; };
	const tstring& get_sql() const { /*return _orig_stmt;*/ return _parsed_stmt; };
	ub4 get_column_count() const;	//Return number of columns returned from SELECT stmt
	ub4 get_bindpar_count() const;
	const std::vector<ColumnType>& get_columns() const { return _columns; };

	/*** loop through input bind variables */
	BindPar& get_next_in_bindpar();
	const BindPar& get_curr_in_bindpar();
		
	/*** loop through output bind variables */
	BindPar& get_next_out_bindpar();
	const BindPar& get_curr_out_bindpar();
		
	/*** loop through select's define variables */
	BindPar& get_next_column();
	const BindPar& get_curr_column();

	bool eof();

	unsigned get_last_row() const { return _last_row + _last_buff_row; }; // _last_row is updated only by fetch
	
	void close()
	{
		_state &= ( UNINITIALIZED | PREPARED | DESCRIBED | DEFINED );
		_in_pos = _out_pos = 0;
		//get_log().ts( std::string(__HERE_SHORT__)) << "TODO: " << __TROTL_HERE__ << std::endl;
	};

	SqlStatement& operator<< (const char *);
	SqlStatement& operator<< (const SqlValue &val);	

	template<class wrapped_type>
	SqlStatement& operator<<(const wrapped_type &val)
	{
		if( ::Loki::SuperSubclassStrict<SqlValue, wrapped_type>::value )
		{
			SqlValue const &v(val);
			return operator <<(v);
		} else {
			BindPar &BP(get_next_in_bindpar());

			throw OciException(__TROTL_HERE__,"Misssing conversion(%s%d to %s%d)\n"
					).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP.type_name).arg(BP.value_sz);

			// Check type correctness
			if(  BP.value_sz != sizeof(wrapped_type) || BP.type_name != typeid(wrapped_type).name() )
				throw OciException(__TROTL_HERE__, "Invalid datatype in bind operation(sizes: %s%d vs. %s%d)\n"
						).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP.type_name).arg(BP.value_sz);

			// store value insite BindPar class
//			((wrapped_type*)BP.valuep)[0] = val; TODO - need assigment operator and copy constructor for LOBS
			BP._cnt = 1;

			// perform real bind operation
			if( !BP._bound) bind(BP);

			if(_in_pos == _in_cnt)
				execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);
		}
		return *this;
	}

	template<class wrapped_type>
	SqlStatement& operator<<(const std::vector<wrapped_type> &val)
	{
		BindPar &BP(get_next_in_bindpar());

		// Check type correctness
		if( BP.value_sz != sizeof(typename std::vector<wrapped_type>::value_type))
			throw OciException(__TROTL_HERE__, "Invalid datatype in bind operation(sizes: %d vs. %d)\n"
					).arg(sizeof(typename std::vector<wrapped_type>::value_type)).arg(BP.value_sz);

		// Check bind datatypes
		if( BP.dty != SQLT<wrapped_type>::value )
			throw OciException(__TROTL_HERE__, "Invalid datatype in bind operation(types: %d vs. %d)\n"
					).arg(SQLT<wrapped_type>::value).arg(BP.dty);

		// Check vector size
		if(BP._max_cnt < val.size())
			throw OciException(__TROTL_HERE__, "Input vector too long(%d vs. %d)"
					).arg(val.size()).arg(BP._max_cnt);

		// store value inside BindPar class
		for(unsigned it = 0; it!=val.size(); ++it)
		{
			((wrapped_type*)BP.valuep)[it] = val[it];
		}
		BP._cnt = val.size();

		// perform real bind operation
		if(!BP._bound) bind(BP);

		if(_in_pos== _in_cnt)
			execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

		return *this;
	}
	
	SqlStatement& operator>> (SqlValue &val);

	template<class wrapped_type>
	SqlStatement& operator>>(wrapped_type &val)
	{

		if( ::Loki::SuperSubclassStrict<SqlValue, wrapped_type>::value )
		{
			return operator>>((SqlValue&)val);
		} else {
			BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

			throw OciException(__TROTL_HERE__, "Misssing conversion(%s%d to %s%d)\n"
					).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP.type_name).arg(BP.value_sz);
		}

		return *this;
	}

	template<class wrapped_type>
	SqlStatement& operator>>(std::vector<wrapped_type> &val)
	{
		BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

		throw OciException(__TROTL_HERE__, "unsupported vector conversion(%s%d to %s%d)\n"
				).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP.type_name).arg(BP.value_sz);

		return *this;
	}

/* 	template<class wrapped_type> */
/* 	SqlStatement& operator>>(SqlInt<wrapped_type> &val) */
/* 	{ */
/* 		BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) ); */
/* 		prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP); */

/* 		wrapped_type v; */

/* 		if((_state & EXECUTED) == 0) */
/* 			execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT); */

/* 		v = BP2.template get_number<wrapped_type>(_last_buff_row); //NOTE _last_row=0 for PL/SQL stats */
/* 		val.set(v); */
/*  		val.set_null(); //TODO - this is really weird !!! */

/*  		if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT) //TODO */
/*  			++_last_buff_row; */

/*		if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT ) */
/* 		_state |= EOF_DATA; */
		
/*  		if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT) */
/*  			fetch(_buff_size); */

/* 		return *this; */
/* 	} */

	virtual ~SqlStatement();
	// { // delete _res; // _res = NULL; };

	OciConnection &_conn;
	mutable OciError _errh;

protected:
	enum STATE {
		UNINITIALIZED=0,
		PREPARED=1,
		DESCRIBED=2,
		DEFINED=4,
		EXECUTED=8,
		FETCHED=16,
		EOF_DATA=32,
		STMT_ERROR=64
	};

	virtual void prepare(const tstring& sql, ub4 lang=OCI_NTV_SYNTAX);

	void execute_describe();

	bool fetch(ub4 rows=-1);
	
	/* OCIBindByPos - for PL/SQL statements */
	void bind(BindPar &bp);
	/* OCIDefineByPos - for SELECT statements */
	void define(BindPar &dp);

	//OCISvcCtx* _svchp;
public:	//todo delete me - these fields should not be public
	const ub4 _lang;
	const tstring _orig_stmt;
	mutable tstring _parsed_stmt;

	int _state;
	STMT_TYPE _stmt_type;
	mutable ub4 _param_count, _column_count, _in_cnt, _out_cnt;
	ub4	_last_row, _last_fetched_row, _in_pos, _out_pos;

	ub4 _last_buff_row, _buff_size; // used in select statements
	mutable ub4 _fetched_row;

	std::vector<ColumnType> _columns; // TODO move into some SQL-result class

	std::auto_ptr<BindPar> *_all_binds;
	std::auto_ptr<BindPar> *_all_defines;
	ub4 *_in_binds, *_out_binds;
	bool _bound;
};

/*
 * Setters
 */
template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <int>(const int &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <unsigned int>(const unsigned int &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <long>(const long &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <unsigned long>(const unsigned long &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <float>(const float &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <double>(const double &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <tstring> (const tstring &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <int>(const std::vector<int> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <unsigned int>(const std::vector<unsigned int> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <long>(const std::vector<long> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <unsigned long>(const std::vector<unsigned long> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <float>(const std::vector<float> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <double>(const std::vector<double> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator<< <tstring> (const std::vector<tstring> &val);
 
/*
 * Getters
 */
template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <int> (int &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <unsigned int> (unsigned int &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <long> (long &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <unsigned long> (unsigned long &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <float> (float &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <double> (double &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <tstring> (tstring &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <int>(std::vector<int> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <unsigned int>(std::vector<unsigned int> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <long>(std::vector<long> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <unsigned long>(std::vector<unsigned long> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <float>(std::vector<float> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <double>(std::vector<double> &val);

template<> TROTL_EXPORT
SqlStatement& SqlStatement::operator>> <tstring> (std::vector<tstring> &val);

//template<>
//SqlStatement& SqlStatement::operator>> <SqlValue> (SqlValue &val);


/** Factory for creating instantions of BindPar structure.
 *  name of datatype is used as a key
 *  each constructor takes three arguments - position,  SqlStatement, BindVarDecl
 *  BindVarDecl - is created by simple PL/SQL parser
 *  this factory is used for PL/SQL statements
 **/
TROTL_EXPORT typedef Util::GenericFactory<SqlStatement::BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, BindVarDecl&)> BindParFactTwoParm;
//class TROTL_EXPORT BindParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, BindVarDecl&)> {};
//TROTL_EXPORT  typedef Loki::SingletonHolder<BindParFactTwoParm> BindParFactTwoParmSing;
class TROTL_EXPORT  BindParFactTwoParmSing: public Loki::SingletonHolder<BindParFactTwoParm> {};

 
/** Factory for creating instantions of BindPar structure.
 *  SQLT_<X> const is used as a key
 *  each constructor takes three arguments - position,  SqlStatement, ColumnType
 *  this factory is used in SELECT statements
 **/
TROTL_EXPORT typedef Util::GenericFactory<SqlStatement::BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&), int> DefineParFactTwoParm;
//class TROTL_EXPORT DefineParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&), int> {};
//typedef Loki::SingletonHolder<DefineParFactTwoParm> DefineParFactTwoParmSing;
class TROTL_EXPORT  DefineParFactTwoParmSing: public Loki::SingletonHolder<DefineParFactTwoParm> {};

 
/** Factory for creating instantions of BindPar structure.
 *  Name of the type is used as a key, i.e. 'SYS.XMLTYPE'
 *  each constructor takes the arguments - position,  SqlStatement, ColumnType
 *  this factory will be used for complex/custom datatypes
 **/
TROTL_EXPORT typedef Util::GenericFactory<SqlStatement::BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&)> CustDefineParFactTwoParm;
//class TROTL_EXPORT CustDefineParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&)> {};
//typedef Loki::SingletonHolder<CustDefineParFactTwoParm> CustDefineParFactTwoParmSing;
class TROTL_EXPORT  CustDefineParFactTwoParmSing: public Loki::SingletonHolder<CustDefineParFactTwoParm> {};

template<class wrapped_type, unsigned int SQLT_TYPE>
class TBindParInt: public SqlStatement::BindPar
{
public:
	TBindParInt(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : SqlStatement::BindPar(pos, stmt, decl)
	{
		valuep = new wrapped_type [_max_cnt];
		dty = SQLT<wrapped_type>::value;
		value_sz = sizeof(wrapped_type);
		type_name = typeid(wrapped_type).name();
	}

	TBindParInt(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : SqlStatement::BindPar(pos, stmt, ct)
	{
		valuep = new wrapped_type [_cnt];
		dty = SQLT<wrapped_type>::value;
		value_sz = sizeof(wrapped_type);
		type_name = typeid(wrapped_type).name();
	}

	~TBindParInt()
	{
		if(valuep)
		{
			delete[] (wrapped_type*)valuep;
			valuep = NULL;
		}			       
	}

	virtual tstring get_string(unsigned int row) const
	{
		tostream s;
		s << ((wrapped_type*)valuep)[row];
		return s.str();
	}

	template<class param_type>
	void set(const param_type &var)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<param_type>::isIntegral, isNotIntegral);
		((wrapped_type*)valuep)[0] = var;
	}
protected:
	TBindParInt(const TBindParInt &other);
};

};

#endif /*TROTL_STAT_H_*/
