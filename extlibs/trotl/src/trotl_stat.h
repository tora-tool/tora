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

#include "loki/static_check.h"
#include "loki/TypeTraits.h"

// OCI_STMT_UNKNOWN is not defined in 10.x instantlient (at least on mac)
#ifndef OCI_STMT_UNKNOWN
#define OCI_STMT_UNKNOWN 0
#endif

namespace trotl
{

class BindParCursor;
class SqlCursor;

inline void oci_check_error(tstring where, SqlStatement &stmt, sword res)
{
	if (res != OCI_SUCCESS)
		throw_oci_exception(OciException(where, stmt));
};


class TROTL_EXPORT SqlStatement : public OciHandle<OCIStmt>
{
	typedef OciHandle<OCIStmt> super;
	friend class BindParCursor;
public:
	enum STMT_TYPE
	{
		STMT_OTHER = 0, // truncate table, ..., OCI_STMT_UNKNOWN on 11g OCI
		STMT_SELECT = OCI_STMT_SELECT,
		STMT_UPDATE = OCI_STMT_UPDATE,
		STMT_DELETE = OCI_STMT_DELETE,
		STMT_INSERT = OCI_STMT_INSERT,
		STMT_CREATE = OCI_STMT_CREATE,
		STMT_DROP = OCI_STMT_DROP,
		STMT_ALTER = OCI_STMT_ALTER,
		STMT_BEGIN = OCI_STMT_BEGIN,
		STMT_DECLARE = OCI_STMT_DECLARE,
		STMT_CALL    = 10,  // OCI_STMT_CALL on 11g OCI
		STMT_MERGE   = 16,
		STMT_NONE
	};

	SqlStatement(OciConnection& conn, const tstring& stmt, ub4 lang=OCI_NTV_SYNTAX, int bulk_rows=g_OCIPL_BULK_ROWS);
	SqlStatement(OciConnection& conn, OciHandle<OCIStmt> &handle, ub4 lang=OCI_NTV_SYNTAX, int bulk_rows=g_OCIPL_BULK_ROWS);

	bool execute_internal(ub4 rows, ub4 mode);
	ub4 row_count() const;
	ub4 fetched_rows() const;

	inline STMT_TYPE get_stmt_type() const
	{
		return _stmt_type;
	};
	const tstring& get_sql() const
	{
		/*return _orig_stmt;*/ return _parsed_stmt;
	};
	ub4 get_column_count() const;	//Return number of columns returned from SELECT stmt
	ub4 get_bindpar_count() const;
	const std::vector<DescribeColumn*>& get_columns()
	{
		// Was not executed yet - or was executed but all rows were already fetched => re-execute
		if ((_state & EXECUTED) == 0 && (_state & EOF_DATA) == 0) // Both flags are set to 0 => query was never executed
			execute_internal(_buff_size, OCI_DEFAULT);

		if ((_state & EXECUTED) == 0 && (_state & EOF_DATA) && (_state & EOF_QUERY)) // EXECUTED == 0 and EOF_QUERY was reached => re-execute
			execute_internal(_buff_size, OCI_DEFAULT);

		return _columns;
	};

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

	unsigned get_last_row() const
	{
		return _last_row + _last_buff_row;
	}; // _last_row is updated only by fetch

	void close()
	{
		_state &= ( UNINITIALIZED | PREPARED | DESCRIBED | DEFINED );
		_in_pos = _out_pos = 0;
		//get_log().ts( tstring(__HERE_SHORT__)) << "TODO: " << __TROTL_HERE__ << std::endl;
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
		}
		else
		{
			BindPar &BP(get_next_in_bindpar());

			throw_oci_exception(OciException(__TROTL_HERE__,"Misssing conversion(%s%d to %s%d)\n"
			                                ).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP._type_name).arg(BP.value_sz));

			// Check type correctness
			if(  BP.value_sz != sizeof(wrapped_type) || BP._type_name != typeid(wrapped_type).name() )
				throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(sizes: %s%d vs. %s%d)\n"
				                                ).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP._type_name).arg(BP.value_sz));

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
			throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(sizes: %d vs. %d)\n"
			                                ).arg(sizeof(typename std::vector<wrapped_type>::value_type)).arg(BP.value_sz));

		// Check bind datatypes
		if( BP.dty != SQLT<wrapped_type>::value )
			throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(types: %d vs. %d)\n"
			                                ).arg(SQLT<wrapped_type>::value).arg(BP.dty));

		// Check vector size
		if(BP._max_cnt < val.size())
			throw_oci_exception(OciException(__TROTL_HERE__, "Input vector too long(%d vs. %d)"
			                                ).arg(val.size()).arg(BP._max_cnt));

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
	SqlStatement& operator>> (SqlCursor &val);

	template<class wrapped_type>
	SqlStatement& operator>>(wrapped_type &val)
	{

		if( ::Loki::SuperSubclassStrict<SqlValue, wrapped_type>::value )
		{
			return operator>>((SqlValue&)val);
		}
		else
		{
			BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

			throw_oci_exception(OciException(__TROTL_HERE__, "Misssing conversion(%s%d to %s%d)\n"
			                                ).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP._type_name).arg(BP.value_sz));
		}

		return *this;
	}

	template<class wrapped_type>
	SqlStatement& operator>>(std::vector<wrapped_type> &val)
	{
		BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

		throw_oci_exception(OciException(__TROTL_HERE__, "unsupported vector conversion(%s%d to %s%d)\n"
		                                ).arg(typeid(wrapped_type).name()).arg(sizeof(wrapped_type)).arg(BP._type_name).arg(BP.value_sz));

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
	/*  			fetch(_fetch_rows); */

	/* 		return *this; */
	/* 	} */

	virtual ~SqlStatement();
	// { // delete _res; // _res = NULL; };

	OciConnection &_conn;
	mutable OciError _errh;

	enum STATE
	{
		UNINITIALIZED=0,
		PREPARED=1,
		DESCRIBED=2,
		DEFINED=4,
		EXECUTED=8,
		FETCHED=16,
		EOF_DATA=32, // All rows were fetched from OCI but some rows may still be in our buffers
		EOF_QUERY=64,
		STMT_ERROR=128
		// NOTE: flags EXECUTED and EOF_DATA are set "exclusively" they (both) must never be set to 1
	};
protected:

	virtual void prepare(const tstring& sql, ub4 lang=OCI_NTV_SYNTAX);

	void execute_describe();

	void fetch(ub4 rows=-1);

	/* OCIBindByPos - for PL/SQL statements */
	void bind(BindPar &bp);
	/* OCIDefineByPos - for SELECT statements */
	void define(BindPar &dp);
	void define_all();

public:	//todo delete me - these fields should not be public
	inline void pre_read_value()
	{
		// Was not executed yet - or was executed but all rows were already fetched => re-execute
		if ((_state & EXECUTED) == 0 && (_state & EOF_DATA) == 0) // Both flags are set to 0 => query was never executed
			execute_internal(_buff_size, OCI_DEFAULT);

		if ((_state & EXECUTED) == 0 && (_state & EOF_DATA) && (_state & EOF_QUERY)) // EXECUTED == 0 and EOF_QUERY was reached => re-execute
			execute_internal(_buff_size, OCI_DEFAULT);

		if ((_state & FETCHED) == 0 && get_stmt_type() == STMT_SELECT)
			fetch(_fetch_rows);
	}

	inline void post_read_value(const BindPar &BP)
	{
		if (_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
			++_last_buff_row;

		if (_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		{
			_state |= EOF_DATA;
			_state |= EOF_QUERY;
			_state &= ~EXECUTED;
		}

		if (((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT && _last_buff_row == fetched_rows())
			fetch(_fetch_rows);

		if (_state >= EOF_DATA && get_stmt_type() == STMT_SELECT && _last_buff_row >= fetched_rows())
		{
			_state |= EOF_QUERY;
		}
	}

	//OCISvcCtx* _svchp;
	const ub4 _lang;
	const tstring _orig_stmt;
	mutable tstring _parsed_stmt;

	unsigned _state;
	STMT_TYPE _stmt_type;
	mutable ub4 _param_count, _column_count, _in_cnt, _out_cnt;
	ub4 _last_row, _last_fetched_row, _in_pos, _out_pos, _iters;

	ub4 _last_buff_row, _buff_size, _fetch_rows; // used in select statements

	std::vector<DescribeColumn*> _columns; // TODO move into some SQL-result class

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

};

#endif /*TROTL_STAT_H_*/
