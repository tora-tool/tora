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

#include <algorithm>
#include <cctype>       // std::toupper
#include <string>
#include <assert.h>

#ifdef WIN32
#include "trotl_parser.h"
#include <stdio.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_stat.h"
#include "trotl_convertor.h"

namespace trotl {

int TROTL_EXPORT g_OCIPL_BULK_ROWS = 3;
int TROTL_EXPORT g_OCIPL_MAX_LONG = 30000;


SqlStatement::SqlStatement(OciConnection& conn, const tstring& stmt, ub4 lang, int bulk_rows)
: super(conn._env),
//_svchp(conn._svc_ctx),
_conn(conn),
_lang(lang),
_orig_stmt(stmt),
_parsed_stmt(""),
_state(UNINITIALIZED),
_stmt_type(STMT_NONE),
_param_count(0), _column_count(0),
_in_cnt(0), _out_cnt(0),
_last_row(-1),
_last_fetched_row(-1),
_in_pos(0), _out_pos(0),
_last_buff_row(0), _buff_size(g_OCIPL_BULK_ROWS),
_all_binds(NULL), _in_binds(NULL), _out_binds(NULL),
_bound(false)
//	_res(NULL),
//	_bulk_rows(bulk_rows),
//	_result_buffers(0),
{
        _errh.alloc(_env);
	
	SimplePlsqlParser parser;
	parser.parse(stmt);

	_parsed_stmt= parser.getNonColored();

	prepare(_parsed_stmt, lang);

// 	if(get_bindpar_count() != parser._bindvars.size())
// 		throw_ocipl_exception(
// 				OciException(__HERE__, "Wrong bindvar count(%d vs. %d)"
// 					     ).arg(get_bindpar_count()).arg(parser._bindvars.size())
// 		);

	if(get_stmt_type() == STMT_SELECT)
	{
		execute_describe();
		_columns.resize(get_column_count()+1);	// we do not use zero-th position

		_all_defines= new std::auto_ptr<BindPar> [get_column_count()+1];

		for(unsigned dpos = 1; dpos <= get_column_count(); ++dpos)
		{
			std::auto_ptr<BindPar> bp;
			dvoid* parmdp;
			sword res = OCICALL(OCIParamGet(_handle, get_type_id(), _errh, &parmdp, dpos));
			oci_check_error(__HERE__, _errh, res);
			
			_columns[dpos].describe(_errh, parmdp);
			
			res = OCICALL(OCIDescriptorFree(parmdp, OCI_DTYPE_PARAM));
			oci_check_error(__HERE__, _env, res);
			
			//get_log().ts( std::string(__HERE_SHORT__))
			//std::cout 
			//	<< "This: " << this << std::endl
			//	<< "Columns:" << _columns[dpos].get_type_str(true) << std::endl;
			
			// Use column datatype for lookup in hash table
			// and call appropriate create function from the factory
			if( _columns[dpos]._data_type != SQLT_NTY)
				_all_defines[dpos] = DefineParFactTwoParmSing::Instance().create(
					_columns[dpos]._data_type,
					dpos,
					*this,
					_columns[dpos] );
			else
				_all_defines[dpos] = CustDefineParFactTwoParmSing::Instance().create(
					_columns[dpos]._data_type_name.c_str(),
					dpos,
					*this,
					_columns[dpos] );
			
			if(_all_defines[dpos].get() == NULL)
				throw OciException(__HERE__, "DefinePar: Data type not registered: %s(%d)\n"
					).arg(_columns[dpos]._data_type_name).arg(_columns[dpos]._data_type);
			define(*_all_defines[dpos]);
		}
		_state |= DEFINED;
	}

	if(get_bindpar_count())
	{
		_all_binds = new std::auto_ptr<BindPar> [get_bindpar_count()+1];
		_in_binds = new unsigned [get_bindpar_count()+1];
		_out_binds = new unsigned [get_bindpar_count()+1];
	}

	int ipos=1;
	for(std::vector<BindVarDecl>::iterator it = parser._bindvars.begin(); it != parser._bindvars.end(); ++it, ++ipos)
	{
		if(it->inout == "in")
		{
			_in_binds[++_in_pos] = ipos;
		} else if(it->inout == "inout") {
			_in_binds[++_in_pos] = ipos;
			_out_binds[++_out_pos] = ipos;
		} else if(it->inout == "out") {
			_out_binds[++_out_pos] = ipos;
		} else {
			throw OciException(__HERE__, "Unsupported bindpar parameter: %s\n").arg(it->inout);
		};


		//Create BindPar instance, constructor takes two arguments (position, BindVarDecl&)
		_all_binds[ipos] = BindParFactTwoParmSing::Instance().create(it->bindtype, ipos, *this, *it);

		if ( _all_binds[ipos].get() == NULL )
			throw OciException(__HERE__, "BindPar: Data type not registered: %s\n").arg(it->bindtype);
	}

	_in_cnt = _in_pos; _in_pos=0;
	_out_cnt = _out_pos; _out_pos=0;
	if(_in_binds) _in_binds[0]=0;
	if(_out_binds) _out_binds[0]=0;
//	if(_binds_all) _binds_all[0]=0;

//	get_log().ts( std::string(__HERE_SHORT__))
// 	std::cout	  
// 	  << "This: " << this << std::endl
// 	  << "Stat: " << stmt << std::endl
// 	  << "In Binds: " << _in_cnt << std::endl
// 	  << "Out Binds: " << _out_cnt << std::endl
// 	  << "All Binds: " << get_bindpar_count() << std::endl;
};

void SqlStatement::prepare(const tstring& sql, ub4 lang)
{
	ub2 stmt_type;	// OCI_STMT_SELECT, OCI_STMT_UPDATE, ...
	ub4 size = sizeof(stmt_type);
	sword res;

	res = OCICALL(OCIStmtPrepare(_handle/*stmtp*/, _errh, (text*)sql.c_str(), (ub4)sql.length(), lang, OCI_DEFAULT));
	oci_check_error(__HERE__, _errh, res);

	res = OCICALL(OCIAttrGet(_handle/*stmtp*/, get_type_id(), &stmt_type, &size, OCI_ATTR_STMT_TYPE, _errh));
	oci_check_error(__HERE__, _errh, res);

	_stmt_type = (STMT_TYPE)stmt_type;
	_state |= PREPARED;
}

void SqlStatement::execute_describe()
{
	sword res;

	res = OCICALL(OCIStmtExecute(_conn._svc_ctx, _handle, _errh, _stmt_type == STMT_SELECT ? (ub4) 0 : (ub4) 1,
			0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DESCRIBE_ONLY));
	oci_check_error(__HERE__, *this, res);

	_state |= DESCRIBED;
}

ub4 SqlStatement::get_column_count() const
{
	if(get_stmt_type() != STMT_SELECT)
		return 0;
	
	sword res;
	ub4 size = sizeof(_column_count);

	if(_column_count)
		return _column_count;

	res = OCICALL(OCIAttrGet(_handle, get_type_id(), &_column_count, &size, OCI_ATTR_PARAM_COUNT, _errh));
	oci_check_error(__HERE__, _errh, res);

	//get_log().ts( std::string(__HERE_SHORT__))
// 	std::cout
// 	  << "This: " << this << std::endl
// 	  << "Column count:" << _column_count << std::endl;
	return _column_count;
}

ub4 SqlStatement::get_bindpar_count() const
{
	sword res;
	ub4 size = sizeof(_param_count);

	if(_param_count)
		return _param_count;
	
	res = OCICALL(OCIAttrGet(_handle, get_type_id(), &_param_count, &size, OCI_ATTR_BIND_COUNT, _errh));
	oci_check_error(__HERE__, _errh, res);

	//get_log().ts( std::string(__HERE_SHORT__))
	//std::cout
	//	<< "This: " << this << std::endl 
	//	<< "bindpar count:" << _param_count << std::endl;
	return _param_count;
}

//TODO add some boundary checks into these three functions
SqlStatement::BindPar& SqlStatement::get_next_in_bindpar()
{
	if( _all_binds == 0 || _in_cnt == 0 )
		throw OciException(__HERE__, "No in Bindpars specified");

	ub4 pos = _in_pos < _in_cnt ? ++_in_pos : _in_pos=1;  //Round robin hack
	return *_all_binds[_in_binds[pos]];
}

const SqlStatement::BindPar& SqlStatement::get_curr_in_bindpar()
{
	if( _all_binds == 0 || _in_binds == 0 )
		throw OciException(__HERE__, "No in Bindpars specified");

  	ub4 pos = _in_pos < _in_cnt ? (_in_pos+1) : 1;  //Round robin hack
	return *_all_binds[_in_binds[ pos ]];
}

SqlStatement::BindPar& SqlStatement::get_next_out_bindpar()
{
	if( _all_binds == 0 || _out_binds == 0 )
		throw OciException(__HERE__, "No out Bindpars specified");
	
	ub4 pos = _out_pos < _out_cnt ? ++_out_pos : _out_pos=1;  //Round robin hack
	return *_all_binds[_out_binds[ pos ]];
}

const SqlStatement::BindPar& SqlStatement::get_curr_out_bindpar()
{
	if( _all_binds == 0 || _out_binds == 0 )
		throw OciException(__HERE__, "No out Bindpars specified");

	ub4 pos = _out_pos < _out_cnt ? (_out_pos+1) : 1;  //Round robin hack
	return *_all_binds[_out_binds[_out_pos ? _out_pos : 1 ]];
}

SqlStatement::BindPar& SqlStatement::get_next_column()
{
	ub4 pos = _out_pos < _column_count ? ++_out_pos : _out_pos=1;
	return *_all_defines[ pos ];
}

const SqlStatement::BindPar& SqlStatement::get_curr_column()
{
	ub4 pos = _out_pos < _column_count ? (_out_pos+1) : 1;
	return *_all_defines[ pos ];
}
  
bool SqlStatement::eof()
{
  	if((_state & EXECUTED) == 0)
	  execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return _state >= EOF_DATA && _last_buff_row >= fetched_rows();
}

bool SqlStatement::execute_internal(ub4 rows, ub4 mode)
{
	ub4 iters;

	//	TODO replace rows by something else - &FETCHED
	if (rows==0 && get_stmt_type() == STMT_SELECT)
	{
		// Optimizing of TCP packets by transfering multiple datasets in one packet
		try {
			set_attribute(OCI_ATTR_PREFETCH_MEMORY, 1500);
			//set_attribute(OCI_ATTR_PREFETCH_ROWS, 10);
			set_attribute(OCI_ATTR_PREFETCH_ROWS, g_OCIPL_BULK_ROWS);
		} catch(std::exception&) {
			// ignore exception of this optional performance tuning
			//TODO add some debug output here
		}
	}

	switch(STMT_TYPE t = get_stmt_type())
	{
	case STMT_OTHER:
		iters = 1;
		_state |= EOF_DATA;
		break;
	case STMT_SELECT:
		iters = rows;
		break;
	case STMT_UPDATE:
		iters = 1;
		break;
	case STMT_DELETE:
		iters = 1;
		break;
	case STMT_INSERT:
		if( _in_cnt == 0 )
			break;
		iters  = _all_binds[_in_binds[1]]->_cnt;
		// Loop over input bind vars - insert can have out binds too(i.e. returning clause)
		for(unsigned i=1; i<=_in_cnt; ++i)
			if(_all_binds[_in_binds[i]]->_cnt != iters)
				throw OciException(__HERE__, "Wrong count of bindvars: (%d vs. %d)\n"
					).arg(iters).arg(_all_binds[_in_binds[i]]->_cnt);
		break;
	case STMT_BEGIN:
	case STMT_DECLARE:
		iters = 1;
		_last_buff_row = 0;
		if( _out_cnt == 0 && _in_cnt == 0)
			_state |= EOF_DATA;
		break;
	case STMT_CREATE:
	case STMT_DROP:
	case STMT_ALTER:
		iters = 1;
		_last_buff_row = 0;
		_state |= EOF_DATA;
		break;
	default:
		std::cerr << "Unknown statement type: " << t
			  << std::endl << _parsed_stmt << std::endl;
		exit(-1);
	};

	if(!_bound && _out_cnt)
	{
		// loop over remaining out variables
		for(unsigned i=1; i<=_out_cnt; ++i)
		{
			BindPar &BP(get_next_out_bindpar());
			if(!BP._bound) bind(BP);
		}
	}

	// execute and fetch
	sword res = OCICALL(OCIStmtExecute(
			_conn._svc_ctx,
			_handle, // *stmtp
			_errh, // *errhp
			iters,//_stmt_type == STMT_SELECT ? rows : 1, // iters
			0, // rowoff
			(CONST OCISnapshot*)0, (OCISnapshot*)0, mode));

	//std::cout << std::endl
	//	<< "iters:" << iters << std::endl;

	_state |= EXECUTED;
	_last_row = _last_buff_row = 0;
	if(get_stmt_type() == STMT_SELECT)
	  _last_fetched_row = row_count();
	_bound = true;

	switch(res)
	{
	case OCI_NO_DATA:
		_state |= EOF_DATA;
		return false;	// There are no additional rows pending to be fetched.
	case OCI_ERROR:
		_state = STMT_ERROR;
		throw OciException(__HERE__, *this);
	default:
		if (rows > 0)
			_state |= EXECUTED|FETCHED;
		else
			_state = (_state|EXECUTED) & ~FETCHED;
		if(res != OCI_SUCCESS_WITH_INFO) oci_check_error(__HERE__, _errh, res);
		return true;	// There may be more rows available to be fetched (for queries) or the DML statement succeeded.
	}
}

bool SqlStatement::fetch(ub4 rows/*=-1*/)
{
	sword res = OCICALL(OCIStmtFetch2(_handle, _errh, rows, OCI_FETCH_NEXT, 0, OCI_DEFAULT));

 	_last_row += _last_buff_row;
	_last_fetched_row = row_count();
	_last_buff_row = 0;

	switch(res)
	{
	case OCI_SUCCESS:
		_state |= FETCHED;
		return true;
	case OCI_NO_DATA:
		_state |= EOF_DATA;
		return false;
	default:
		oci_check_error(__HERE__, _errh, res);
		//oci_check_error(__HERE__, *this, res);
		return true;
	}
			
}

ub4 SqlStatement::row_count() const
{
	ub4 row_count;
	ub4 size = sizeof(row_count);

	sword res = OCICALL(OCIAttrGet(_handle/*stmtp*/, get_type_id(), &row_count, &size, OCI_ATTR_ROW_COUNT, _errh));
	oci_check_error(__HERE__, _errh, res);

	//std::cout << "ub4 SqlStatement::row_count() const: " << row_count << std::endl;

	return row_count;
}

ub4 SqlStatement::fetched_rows() const
{
	if( _stmt_type != STMT_SELECT)
		return 0;
	ub4 row_count;
	ub4 size = sizeof(row_count);

	sword res = OCICALL(OCIAttrGet(_handle, get_type_id(), &row_count, &size, OCI_ATTR_ROWS_FETCHED, _errh));
	oci_check_error(__HERE__, _errh, res);

	_fetched_row = row_count;

	// std::cout << "ub4 SqlStatement::fetched_rows() const: " << row_count << std::endl;
	
	return row_count;
}

void SqlStatement::bind(BindPar &bp)
{
// 	sword res = OCICALL(OCIBindByPos (_handle,
// 			&bp.bindp,
// 			_errh,
// 			bp._pos,
// 			bp.valuep,
// 			bp.value_sz,
// 			bp.dty,
// 			bp.indp,
// 			(bp._bind_type == BindPar::BIND_IN ? NULL : bp.alenp),
// 			0, // *rcodep
// 			//NULL for non-PL/SQL statements - maybe
// 			(((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN ) && bp._max_cnt > 1) ? bp._max_cnt : 0),
// 			//NULL for non-PL/SQL statements
// 			(ub4*)(((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN ) && bp._max_cnt > 1) ? &bp._cnt : NULL),
// 			OCI_DEFAULT));

  //get_log().ts( std::string(__HERE_SHORT__))
  //   std::cout  
  // 	  << "This: " << this << std::endl
  // 	  << "Bind:'"<< bp.bind_name << "' " << bp.bind_name.length() << std::endl;
  
	sword res = OCICALL(OCIBindByName (_handle,
			&bp.bindp,
			_errh,
			(const OraText*)bp.bind_name.c_str(),
			bp.bind_name.length(),
			bp.valuep,
			bp.value_sz,
			bp.dty,
			bp.indp,
			(bp._bind_type == BindPar::BIND_IN ? NULL : bp.alenp),
			0, // *rcodep
			//NULL for non-PL/SQL statements - maybe
			(((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN ) && bp._max_cnt > 1) ? bp._max_cnt : 0),
			//NULL for non-PL/SQL statements
			(ub4*)(((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN ) && bp._max_cnt > 1) ? &bp._cnt : NULL),
			OCI_DEFAULT));

	oci_check_error(__HERE__, _errh, res);

//	std::cout << std::endl
//	<< "alen:" << bp.alenp << std::endl
//	<< "_stmt_type:" << _stmt_type << std::endl
//	<< "stmt_type:" << (_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) << std::endl
//	<< "stmt_type2:" << ((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) ? bp._max_cnt : 0) << std::endl
//	<< "stmt_type3:" << ((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) ? bp._cnt : NULL) << std::endl
//	<< std::endl;
 	bp.bind_hook(*this);
//TODO use atribute OCI_ATTR_MAXDATA_SIZE here
	bp._bound = true;
}

void SqlStatement::define(BindPar &dp)
{
	sword res = OCICALL(OCIDefineByPos(_handle/*stmtp*/,
			&dp.defnpp,
			_errh,
			dp._pos,
			dp.valuep,
			dp.value_sz,
			dp.dty,
			dp.indp,
			dp.rlenp,
			dp.rcodep,
//			(ub2*)0, (ub2*)0,
			OCI_DEFAULT));
	oci_check_error(__HERE__, _errh, res);

	dp.define_hook(*this);
}

SqlStatement::~SqlStatement()
{
	if( get_stmt_type() == STMT_SELECT )
		delete [] _all_defines;
	
	if ( get_bindpar_count() )
	{
		delete [] _all_binds;
		delete [] _in_binds;
		delete [] _out_binds;
	}
	_state |= 255;
};

template<>
SqlStatement& SqlStatement::operator<< <int>(const int &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<int>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <unsigned int>(const unsigned int &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<unsigned int>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <long>(const long &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<long>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <unsigned long>(const unsigned long &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<unsigned long>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <float>(const float &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<float>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <double>(const double &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	BP2.set_number<double>(0, val);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <tstring> (const tstring &val)
{
	BindPar &BP(get_next_in_bindpar());

	// Check type correctness
	if( BP.dty != SQLT_CHR && BP.dty != SQLT_STR )
		throw OciException(__HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)\n").arg(BP.dty);

	//Check string length
	if( val.length()+(BP.dty==SQLT_STR ? 1 : 0) > (unsigned)BP.value_sz )
		throw OciException(__HERE__, "String too long (%d max. %d) at pos %d %s\n"
				).arg(val.length()).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0) ).arg(_in_pos);

	// store value inside BindPar class
	if( BP.dty == SQLT_STR )
	{
		//VARCHAR2
		strcpy((char*)BP.valuep, val.c_str());
	} else { // BP.dty == SQLT_CHR
		//CHAR
		memset(BP.valuep, ' ', BP.value_sz);
		memcpy(BP.valuep, val.c_str(), val.size());
	}
	BP.alenp[0] = (ub2)val.size();
	BP._cnt = 1;

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator<< <int>(const std::vector<int> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<int>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <unsigned int>(const std::vector<unsigned int> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__, "Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<unsigned int>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <long>(const std::vector<long> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__, "Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<long>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <unsigned long>(const std::vector<unsigned long> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<unsigned long>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <float>(const std::vector<float> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<float>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <double>(const std::vector<double> &val)
{
	BindPar &BP(get_next_in_bindpar());
	prefferedNumericType &BP2 = dynamic_cast<prefferedNumericType&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<double>(pos, val[pos]);
	}

	BP2._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator<< <tstring> (const std::vector<tstring> &val)
{
	BindPar &BP(get_next_in_bindpar());

	// Check type correctness
	if( BP.dty != SQLT_CHR && BP.dty != SQLT_STR )
		throw OciException(__HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)").arg(BP.dty);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw OciException(__HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt);

	char *lastpos = (char*)BP.valuep;

//	for(const std::vector<tstring>::const_iterator it=val.begin(); it!=val.end(); ++it, ++pos)
	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		//Check string length
		if( val[pos].length()+(BP.dty==SQLT_STR ? 1 : 0) > (unsigned)BP.value_sz )
			throw OciException(__HERE__, "String too long (length: %d max. %d)\n"
				).arg(val[pos].length()).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0));

		// store value inside BindPar class
		if( BP.dty == SQLT_STR )
		{
			//VARCHAR2
			memcpy(lastpos, val[pos].c_str(), val[pos].size());
			lastpos[val[pos].size()] = 0;
			lastpos += BP.value_sz;
			BP.alenp[pos] = val[pos].size();
		} else {
			//CHAR
			memset(lastpos, ' ', BP.value_sz);
			memcpy(lastpos, val[pos].c_str(), val[pos].size());
			lastpos += BP.value_sz;
			BP.alenp[pos] = val[pos].size();
		}
	}
	BP._cnt = val.size();

	// perform real bind operation
	if(!BP._bound) bind(BP);

	// execute if all varialbles bound
	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

SqlStatement& SqlStatement::operator<< (const char *val)
{
	BindPar &BP(get_next_in_bindpar());
	unsigned int l = strlen(val);
	// Check type correctness
	if( BP.dty != SQLT_CHR && BP.dty != SQLT_STR )
		throw OciException(__HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)\n").arg(BP.dty);

	//Check string length
	if( l+(BP.dty==SQLT_STR ? 1 : 0)  > (unsigned)BP.value_sz )
		throw OciException(__HERE__,"String too long (%d max. %d) at pos %d\n"
			).arg(l).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0) ).arg(_in_pos);

	// store value inside BindPar class
	if( BP.dty == SQLT_STR )
	{
		//VARCHAR2
		strcpy((char*)BP.valuep, val); // todo use memcpy here
	} else { // BP.dty == SQLT_CHR
		//CHAR
		memset(BP.valuep, ' ', BP.value_sz);
		memcpy(BP.valuep, val, l);
	}
	BP.alenp[0] = (ub2)l;
	BP._cnt = 1;

	// perform real bind operation
	if(!BP._bound) bind(BP);

	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
};

SqlStatement& SqlStatement::operator<< (const SqlValue &val)
{
	BindPar &BP( get_next_in_bindpar() );

	ConvertorForWrite c(_last_buff_row);
	DispatcherForWrite::Go(val, BP, c);

	BP._cnt = 1;

	// perform real bind operation
	if( !BP._bound) bind(BP);

	// execute if all varialbles bound
	if(_in_pos == _in_cnt)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	return *this;
}


template<>
SqlStatement& SqlStatement::operator>> <int> (int &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_int( _last_buff_row);
	val = BP2.get_number<int>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;		

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <unsigned int> (unsigned int &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_uint( _last_buff_row);
	val = BP2.get_number<unsigned int>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 

	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <long> (long &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_long( _last_buff_row);
	val = BP2.get_number<long>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};


template<>
SqlStatement& SqlStatement::operator>> <unsigned long> (unsigned long &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_ulong( _last_buff_row);
	val = BP2.get_number<unsigned long>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <float> (float &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_float( _last_buff_row);
	val = BP2.get_number<float>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 	
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <double> (double &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	//val = BP.get_double( _last_buff_row);
	val = BP2.get_number<double>( _last_buff_row );

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
};


template<>
SqlStatement& SqlStatement::operator>> <tstring> (tstring &val)
{
	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val = BP.get_string(_last_buff_row);

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
}

//this method is not used
//template<>
//SqlStatement& SqlStatement::operator>> <SqlValue> (SqlValue &val)
//{
//	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
//
//	std::cerr << "SqlStatement& SqlStatement::operator>> <SqlValue> (SqlValue &val)\n";
//
////	throw_ocipl_exception(
////			OciException(
////					__HERE__,
////					"!!!! Misssing conversion(%s%d to SqlValue)\n"
////			).arg(BP.name).arg(BP.value_sz)
////	);
//
//	return *this;
//}

SqlStatement& SqlStatement::operator>> (SqlValue &val)
{
	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	// TODO this multimethod dispatcher uses dynamic_cast so it knows the real type of &val
	ConvertorForRead c(_last_buff_row);
	DispatcherForRead::Go(BP, val, c);

	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
		fetch(_buff_size);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <int>(std::vector<int> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);
	//val = BP.get_int( _last_buff_row);
	for(unsigned i=0; i < BP._cnt; ++i)
	{
		val.at(i) = BP2.get_number<int>(i);
//		std::cout << "setting:" << BP2.get_number<int>(i) << std::endl;
	}

//	if(_out_pos == _column_count && BP._bind_type == BP.DEFINE_SELECT)
//		++_last_buff_row;

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
//	if(_last_buff_row == fetched_rows() && ((_state & EOF_DATA) == 0) && get_stmt_type() == STMT_SELECT)
//		fetch(_buff_size);

	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <unsigned int>(std::vector<unsigned int> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<unsigned int>(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <long>(std::vector<long> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<long>(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <unsigned long>(std::vector<unsigned long> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<unsigned long>(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <float>(std::vector<float> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<float>(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <double>(std::vector<double> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	prefferedNumericType const &BP2 = dynamic_cast<const prefferedNumericType&>(BP);

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<double>(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
		
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <tstring>(std::vector<tstring> &val)
{
	if( get_stmt_type() != STMT_DECLARE && get_stmt_type() != STMT_BEGIN)
		throw OciException(__HERE__, "Reading vectors from SQL statements is not implemented yet");

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

	if((_state & EXECUTED) == 0)
		execute_internal(g_OCIPL_BULK_ROWS, OCI_DEFAULT);

	val.resize(BP._cnt);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP.get_string(i);

	if(_out_pos == _out_cnt && get_stmt_type() != STMT_SELECT )
		_state |= EOF_DATA; 
	
	return *this;
}

};
