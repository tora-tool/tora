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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_stat.h"
#include "trotl_error.h"
#include "trotl_describe.h"
#include "trotl_parser.h"
#include "trotl_convertor.h"
#include "trotl_cursor.h"
#include "trotl_int.h"

#ifdef DEBUG
#include "stack.hpp"
#endif

#include <algorithm>
#include <cctype>       // std::toupper
#include <string>
//#include <assert.h>

namespace trotl
{


int TROTL_EXPORT g_OCIPL_BULK_ROWS = 256;
int TROTL_EXPORT g_OCIPL_MAX_LONG = 0x20000; //128 KB
const char TROTL_EXPORT *g_TROTL_DEFAULT_NUM_FTM = "TM";
const char TROTL_EXPORT *g_TROTL_DEFAULT_DATE_FTM = "YYYY:MM:DD HH24:MI:SS";

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
	  _in_pos(0), _out_pos(0), _iters(0),
	  _last_buff_row(0), _buff_size(g_OCIPL_BULK_ROWS), _fetch_rows(g_OCIPL_BULK_ROWS),
	  _all_binds(NULL), _all_defines(NULL),
	  _in_binds(NULL), _out_binds(NULL),
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
// 				OciException(__TROTL_HERE__, "Wrong bindvar count(%d vs. %d)"
// 					     ).arg(get_bindpar_count()).arg(parser._bindvars.size())
// 		);

	if(get_stmt_type() == STMT_SELECT)
	{
		execute_describe();
	}

	if(get_bindpar_count())
	{
		_all_binds = new std::auto_ptr<BindPar> [get_bindpar_count()+1];
		_in_binds = new unsigned [get_bindpar_count()+1];
		_out_binds = new unsigned [get_bindpar_count()+1];
	}


	if( get_stmt_type() == STMT_SELECT ||
	                get_stmt_type() == STMT_UPDATE ||
	                get_stmt_type() == STMT_DELETE ||
	                get_stmt_type() == STMT_INSERT ||
	                get_stmt_type() == STMT_BEGIN  ||
	                get_stmt_type() == STMT_DECLARE )
	{
		int ipos=1;
		for(std::vector<BindVarDecl>::iterator it = parser._bindvars.begin(); it != parser._bindvars.end(); ++it, ++ipos)
		{
			if(it->inout == "in")
			{
				_in_binds[++_in_pos] = ipos;
			}
			else if(it->inout == "inout")
			{
				_in_binds[++_in_pos] = ipos;
				_out_binds[++_out_pos] = ipos;
			}
			else if(it->inout == "out")
			{
				_out_binds[++_out_pos] = ipos;
			}
			else
			{
				throw_oci_exception(OciException(__TROTL_HERE__, "Unsupported bindpar parameter: %s\n").arg(it->inout));
			};


			//Create BindPar instance, constructor takes two arguments (position, BindVarDecl&)
			_all_binds[ipos] = BindParFactTwoParmSing::Instance().create(it->bindtype, ipos, *this, *it);

			if ( _all_binds[ipos].get() == NULL )
				throw_oci_exception(OciException(__TROTL_HERE__, "BindPar: Data type not registered: %s\n").arg(it->bindtype));
		}
	}

	_in_cnt = _in_pos;
	_in_pos=0;
	_out_cnt = _out_pos;
	_out_pos=0;
	if(_in_binds) _in_binds[0]=0;
	if(_out_binds) _out_binds[0]=0;
//	if(_binds_all) _binds_all[0]=0;

};

SqlStatement::SqlStatement(OciConnection& conn, OciHandle<OCIStmt>& handle, ub4 lang, int bulk_rows)
	: super(handle),
	  _conn(conn),
	  _lang(lang),
	  _orig_stmt(""),
	  _parsed_stmt(""),
	  _state(UNINITIALIZED),
	  _stmt_type(STMT_NONE),
	  _param_count(0), _column_count(0),
	  _in_cnt(0), _out_cnt(0),
	  _last_row(-1),
	  _last_fetched_row(-1),
	  _in_pos(0), _out_pos(0), _iters(0),
	  _last_buff_row(0), _buff_size(g_OCIPL_BULK_ROWS), _fetch_rows(g_OCIPL_BULK_ROWS),
	  _all_binds(NULL), _all_defines(NULL),
	  _in_binds(NULL), _out_binds(NULL),
	  _bound(false)
//	_res(NULL),
//	_bulk_rows(bulk_rows),
//	_result_buffers(0),
{
	_errh.alloc(_env);
};

void SqlStatement::prepare(const tstring& sql, ub4 lang)
{
	ub2 stmt_type;	// OCI_STMT_SELECT, OCI_STMT_UPDATE, ...
	ub4 size = sizeof(stmt_type);
	sword res;

	res = OCICALL(OCIStmtPrepare(_handle/*stmtp*/, _errh, (text*)sql.c_str(), (ub4)sql.length(), lang, OCI_DEFAULT));
	oci_check_error(__TROTL_HERE__, _errh, res);

	/* NOTE this call alse returns other values than mentioned in OCI docs.
	 * for example "EXPLAIN PLAN FOR ..." returns value 15
	 */
	res = OCICALL(OCIAttrGet(_handle/*stmtp*/, get_type_id(), &stmt_type, &size, OCI_ATTR_STMT_TYPE, _errh));
	oci_check_error(__TROTL_HERE__, _errh, res);

	switch(stmt_type)
	{
	case OCI_STMT_SELECT:
	case OCI_STMT_UPDATE:
	case OCI_STMT_DELETE:
	case OCI_STMT_INSERT:
	case 16:        // MERGE
	case OCI_STMT_CREATE:
	case OCI_STMT_DROP:
	case OCI_STMT_ALTER:
	case OCI_STMT_BEGIN:
	case OCI_STMT_DECLARE:
		_stmt_type = (STMT_TYPE)stmt_type;
		break;
	case 0:			// ANALYZE TABLE
	case 15:		// EXPLAIN PLAN FOR
	case 17:        // ROLLBACK
	case 21:        // COMMIT
		_stmt_type = STMT_OTHER;
		break;
	default:
		_stmt_type = STMT_OTHER;
		throw_oci_exception(OciException(__TROTL_HERE__, "Unknown statement type: %d\n%s").arg(stmt_type).arg(_parsed_stmt));
		//std::cerr << "Unknown statement type: " << stmt_type << std::endl << _parsed_stmt << std::endl;
		//exit(-1);
		break;
	};

	_state |= PREPARED;
}

void SqlStatement::execute_describe()
{
	sword res;

	res = OCICALL(OCIStmtExecute(_conn._svc_ctx, _handle, _errh, _stmt_type == STMT_SELECT ? (ub4) 0 : (ub4) 1,
	                             0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DESCRIBE_ONLY));
	oci_check_error(__TROTL_HERE__, *this, res);

	_state |= DESCRIBED;
}

void SqlStatement::define_all()
{
	_columns.resize(get_column_count()+1);	// we do not use zero-th position
	_all_defines= new std::auto_ptr<BindPar> [get_column_count()+1];

	for(unsigned dpos = 1; dpos <= get_column_count(); ++dpos)
	{
		DescribeColumn *dc = new DescribeColumn(_conn, *this, dpos, "");
		_columns[dpos] = dc;

		// Use column datatype for lookup in a hash table
		// and call appropriate create function from the factory
		if( _columns[dpos]->_data_type != SQLT_NTY)
			_all_defines[dpos] = DefineParFactTwoParmSing::Instance().create(
			                             _columns[dpos]->_data_type,
			                             dpos,
			                             *this,
			                             dc
			                     );
		else
			_all_defines[dpos] = CustDefineParFactTwoParmSing::Instance().create(
			                             _columns[dpos]->_reg_name.c_str(),
			                             dpos,
			                             *this,
			                             dc );

		if(_all_defines[dpos].get() == NULL)
			throw_oci_exception(OciException(__TROTL_HERE__, "DefinePar: Data type not registered: %s(%d:%d:%d:%s:%s)\n")
			                    .arg(_columns[dpos]->_type_name)
			                    .arg(_columns[dpos]->_data_type)
			                    .arg(_columns[dpos]->_typecode)
			                    .arg(_columns[dpos]->_collection_typecode)
			                    .arg(_columns[dpos]->_collection_data_type)
			                    .arg(_columns[dpos]->_reg_name)
			                   );
		define(*_all_defines[dpos]);

		// When using piecewise callbacks fetch rows one by one
		if(_all_defines[dpos]->dty == SQLT_LNG)
			_fetch_rows = 1;
		// Due to some ugly SEGFAULT in OCIRowidToChar I can not fetch more than 2 rows 
		// when datatype (U)ROWID is explicitly listed in queries column list
		// and when ROWID equals to this 'AAADVKAABAAAHypAA3' surprisingly some other ROWIDs are fine
		if(_all_defines[dpos]->dty == SQLT_RDD)
			_fetch_rows = min(_fetch_rows, (ub4)2);
	}
	_state |= DEFINED;
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
	oci_check_error(__TROTL_HERE__, _errh, res);

	//get_log().ts( tstring(__HERE_SHORT__))
	// std::cout
	//   << "This: " << this << std::endl
	//   << "Column count:" << _column_count << std::endl;
	return _column_count;
}

ub4 SqlStatement::get_bindpar_count() const
{
	sword res;
	ub4 size = sizeof(_param_count);

	if(_param_count)
		return _param_count;

	res = OCICALL(OCIAttrGet(_handle, get_type_id(), &_param_count, &size, OCI_ATTR_BIND_COUNT, _errh));
	oci_check_error(__TROTL_HERE__, _errh, res);

	//get_log().ts( tstring(__HERE_SHORT__))
	//std::cout
	//	<< "This: " << this << std::endl
	//	<< "bindpar count:" << _param_count << std::endl;
	return _param_count;
}

//TODO add some boundary checks into these three functions
BindPar& SqlStatement::get_next_in_bindpar()
{
	if( _all_binds == 0 || _in_cnt == 0 )
		throw_oci_exception(OciException(__TROTL_HERE__, "No in Bindpars specified"));

	ub4 pos = _in_pos < _in_cnt ? ++_in_pos : _in_pos=1;  //Round robin hack
	return *_all_binds[_in_binds[pos]];
}

const BindPar& SqlStatement::get_curr_in_bindpar()
{
	if( _all_binds == 0 || _in_binds == 0 )
		throw_oci_exception(OciException(__TROTL_HERE__, "No in Bindpars specified"));

	ub4 pos = _in_pos < _in_cnt ? (_in_pos+1) : 1;  //Round robin hack
	return *_all_binds[_in_binds[ pos ]];
}

BindPar& SqlStatement::get_next_out_bindpar()
{
	if( _all_binds == 0 || _out_binds == 0 )
		throw_oci_exception(OciException(__TROTL_HERE__, "No out Bindpars specified"));

	ub4 pos = _out_pos < _out_cnt ? ++_out_pos : _out_pos=1;  //Round robin hack
	return *_all_binds[_out_binds[ pos ]];
}

const BindPar& SqlStatement::get_curr_out_bindpar()
{
	if( _all_binds == 0 || _out_binds == 0 )
		throw_oci_exception(OciException(__TROTL_HERE__, "No out Bindpars specified"));

	//ub4 pos = _out_pos < _out_cnt ? (_out_pos+1) : 1;  //Round robin hack
	return *_all_binds[_out_binds[_out_pos ? _out_pos : 1 ]];
}

BindPar& SqlStatement::get_next_column()
{
	ub4 pos = _out_pos < _column_count ? ++_out_pos : _out_pos=1;
	return *_all_defines[ pos ];
}

const BindPar& SqlStatement::get_curr_column()
{
	ub4 pos = _out_pos < _column_count ? (_out_pos+1) : 1;
	return *_all_defines[ pos ];
}

bool SqlStatement::eof()
{
	if (_state & EOF_QUERY)
		return true;

	pre_read_value();  // will possibly call execute_internal or fetch (if necessary), but not if EOF flag is already set

	if (_state >= EOF_DATA && _last_buff_row >= fetched_rows())
	{
		_state |= EOF_QUERY;
	}
	return _state >= EOF_QUERY;
}

bool SqlStatement::execute_internal(ub4 rows, ub4 mode)
{
	if( _state & EXECUTED)                        // Note EOF_DATA clears EXECUTED flag
		return true;

	_state &= ~FETCHED & ~EOF_DATA & ~EOF_QUERY & ~STMT_ERROR; // Clear three flags

	//	TODO replace rows by something else - &FETCHED
	if (rows==0 && get_stmt_type() == STMT_SELECT)
	{
		// Optimizing of TCP packets by transfering multiple datasets in one packet
		try
		{
			set_attribute(OCI_ATTR_PREFETCH_MEMORY, 1500);
			//set_attribute(OCI_ATTR_PREFETCH_ROWS, 10);
			set_attribute(OCI_ATTR_PREFETCH_ROWS, g_OCIPL_BULK_ROWS);
		}
		catch(std::exception&)
		{
			// ignore exception of this optional performance tuning
			//TODO add some debug output here
		}
	}

	switch(get_stmt_type())
	{
	case STMT_OTHER:
		_iters = 1;
		_state |= EOF_DATA;
		_state |= EOF_QUERY;
		_state &= ~EXECUTED;
		break;
	case STMT_SELECT:
		//_iters = rows;
		_iters = 0;
		break;
	case STMT_UPDATE:
	case STMT_MERGE:
		_iters = 1;
		break;
	case STMT_DELETE:
		_iters = 1;
		break;
	case STMT_INSERT:
		_iters = 1;
		if( _in_cnt == 0 )
			break;
		_iters  = _all_binds[_in_binds[1]]->_cnt;
		// Loop over input bind vars - insert can have out binds too(i.e. returning clause)
		// and check vector lengths
		for(unsigned i=1; i<=_in_cnt; ++i)
			if(_all_binds[_in_binds[i]]->_cnt != _iters)
				throw_oci_exception(OciException(__TROTL_HERE__, "Wrong count of bindvars: (%d vs. %d)\n")
				                    .arg(_iters).arg(_all_binds[_in_binds[i]]->_cnt));
		break;
	case STMT_BEGIN:
	case STMT_DECLARE:
		_iters = 1;
		break;
	case STMT_CREATE:
	case STMT_DROP:
	case STMT_ALTER:
		_iters = 1;
		_last_buff_row = 0;
		_state |= EOF_DATA;
		_state |= EOF_QUERY;
		_state &= ~EXECUTED;
		break;
		// do nothing
	case STMT_NONE:
		break;
	default:
		throw_oci_exception(OciException(__TROTL_HERE__, "Unknown statement type: %d\n%s").arg(get_stmt_type()).arg(_parsed_stmt));
		//no break here
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
	_bound = true;

	//define_all();

	// execute and do not fetch
	sword res = OCICALL(OCIStmtExecute(
	                            _conn._svc_ctx,
	                            _handle,	// *stmtp
	                            _errh,		// *errhp
	                            _iters,
	                            0,		// rowoff
	                            (CONST OCISnapshot*)0, (OCISnapshot*)0, mode));

	//std::cout << std::endl
	//	<< "_iters:" << _iters << std::endl;

	_state |= EXECUTED;
	_last_row = _last_buff_row = 0;
	if(get_stmt_type() == STMT_SELECT)
	{
		if((_state & DEFINED) == 0)
		{
			define_all();
		}
	}

	switch(res)
	{
	case OCI_NO_DATA:
		_state |= EOF_DATA;
		_state &= ~EXECUTED;
		return false;	// There are no additional rows pending to be fetched.
	case OCI_ERROR:
		_state |= STMT_ERROR;
		throw_oci_exception(OciException(__TROTL_HERE__, *this));
		//no break here
	default:
		if (rows > 0)
			_state |= EXECUTED;
		else
			_state = (_state|EXECUTED) & ~FETCHED;
		if(res != OCI_SUCCESS_WITH_INFO) oci_check_error(__TROTL_HERE__, _errh, res);
		return true;	// There may be more rows available to be fetched (for queries) or the DML statement succeeded.
	}
}

void SqlStatement::fetch(ub4 rows/*=-1*/)
{
	sword res = OCICALL(OCIStmtFetch(_handle, _errh, rows, OCI_FETCH_NEXT, OCI_DEFAULT));

	while (res == OCI_NEED_DATA)
	{
		// NOTE: idx is ignored here, therefore fetch for TABLE OF LONG will fail
		ub1   piece = OCI_FIRST_PIECE, in_out = 0;
		dvoid *hdlptr = (dvoid *) 0;
		ub4   hdltype = OCI_HTYPE_DEFINE, iter = 0, idx = 0;
		BindPar *BPp = 0;
		sword res2;

		res2 = OCICALL(OCIStmtGetPieceInfo(_handle, _errh, &hdlptr, &hdltype, &in_out, &iter, &idx, &piece));
		oci_check_error(__TROTL_HERE__, _errh, res2);

		switch(hdltype)
		{
		case OCI_HTYPE_DEFINE:
			for(unsigned i=1; i<=_column_count; ++i)
			{
				if(_all_defines[i].get()->defnpp == hdlptr)
				{
					BPp = _all_defines[i].get();
					break;
				}
			}
			break;
		case OCI_HTYPE_BIND:
			for(unsigned i=1; i<=_param_count; ++i)
			{
				if(_all_binds[i].get()->bindp == hdlptr)
				{
					BPp = _all_binds[i].get();
					break;
				}
			}
			break;
		}

		ub4   alen  = (ub4)BPp->alenp[0];
		sb2   indptr = 0;
		ub2   rcode = 0;

		res2 = OCICALL(OCIStmtSetPieceInfo((dvoid *)hdlptr, (ub4)hdltype, _errh, (dvoid *) BPp->valuep, &alen, piece, (dvoid *)&indptr, &rcode));
		oci_check_error(__TROTL_HERE__, _errh, res2);

		res = OCICALL(OCIStmtFetch(_handle, _errh, 1, OCI_FETCH_NEXT, OCI_DEFAULT));
		if(res == OCI_NEED_DATA || res == OCI_NO_DATA || res == OCI_SUCCESS || res == OCI_SUCCESS_WITH_INFO)
			BPp->fetch_hook(iter, idx, piece, alen, indptr);
	}

	switch(res)
	{
	case OCI_NO_DATA:
		_state |= EOF_DATA;
		_state &= ~EXECUTED;
		//no break here
	case OCI_SUCCESS:
		_state |= FETCHED;
		break;
		// case OCI_NEED_DATA:
		// 	break;
	case OCI_ERROR:
		_state |= EOF_DATA;
		_state |= EOF_QUERY;
		_state &= ~EXECUTED;
		//no break here
	default:
		oci_check_error(__TROTL_HERE__, _errh, res);
		break;
	}

	_last_row += _last_buff_row;
	_last_fetched_row = row_count();
	_last_buff_row = 0;
	if ( _last_fetched_row == 0) // nothing was fetched
		_state |= EOF_QUERY | EOF_DATA;
}

ub4 SqlStatement::row_count() const
{
	ub4 row_count;
	ub4 size = sizeof(row_count);

	sword res = OCICALL(OCIAttrGet(_handle/*stmtp*/, get_type_id(), &row_count, &size, OCI_ATTR_ROW_COUNT, _errh));
	oci_check_error(__TROTL_HERE__, _errh, res);

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
	oci_check_error(__TROTL_HERE__, _errh, res);

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

	//get_log().ts( tstring(__HERE_SHORT__))
	//   std::cout
	// 	  << "This: " << this << std::endl
	// 	  << "Bind:'"<< bp.bind_name << "' " << bp.bind_name.length() << std::endl;


	sword res = OCICALL(OCIBindByName (_handle,
	                                   &bp.bindp,
	                                   _errh,
	                                   (const OraText*)bp._bind_name.c_str(),
	                                   (ub4)bp._bind_name.length(),
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
	oci_check_error(__TROTL_HERE__, _errh, res);

//	std::cout << std::endl
//	<< "alen:" << bp.alenp << std::endl
//	<< "_stmt_type:" << _stmt_type << std::endl
//	<< "stmt_type:" << (_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) << std::endl
//	<< "stmt_type2:" << ((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) ? bp._max_cnt : 0) << std::endl
//	<< "stmt_type3:" << ((_stmt_type == STMT_DECLARE ||_stmt_type == STMT_BEGIN) ? bp._cnt : NULL) << std::endl
//	<< std::endl;
	bp.bind_hook();
//TODO use atribute OCI_ATTR_MAXDATA_SIZE here
	bp._bound = true;
}

void SqlStatement::define(BindPar &dp)
{
	sword res = OCICALL(OCIDefineByPos(_handle/*stmtp*/,
	                                   &dp.defnpp,		  // (OCIDefine **)
	                                   _errh,		  // (OCIError*)
	                                   dp._pos,		  // ub4 position
	                                   dp.valuep,		  // dvoid *valuep
	                                   dp.value_sz,		  // sb4 value_sz
	                                   dp.dty,		  // ub2 dty
	                                   dp.indp,		  // dvoid *indp
	                                   (ub2*) dp.rlenp,	  // ub2 *rlenp
	                                   NULL,		  // ub2 *rcodep
	                                   dp.mode));             // ub4 mode (OCI_DEFAULT,OCI_DYNAMIC_FETCH,...)
	oci_check_error(__TROTL_HERE__, _errh, res);

	dp.define_hook();
}

SqlStatement::~SqlStatement()
{
	std::vector<DescribeColumn*>::iterator it = _columns.begin();
	for(; it != _columns.end(); ++it)
	{
		delete *it; //_columns[dpos];
		//_columns[dpos] = 0;
	}
	if( get_stmt_type() == STMT_SELECT )
		delete [] _all_defines;

	if ( get_bindpar_count() )
	{
		delete [] _all_binds;
		delete [] _in_binds;
		delete [] _out_binds;
	}
	_state |= 0xff;
};

template<>
SqlStatement& SqlStatement::operator<< <int>(const int &val)
{
	BindPar &BP(get_next_in_bindpar());
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

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
		throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)\n").arg(BP.dty));

	//Check string length
	if( val.length()+(BP.dty==SQLT_STR ? 1 : 0) > (unsigned)BP.value_sz )
		throw_oci_exception(OciException(__TROTL_HERE__, "String too long (%d max. %d) at pos %d %s\n"
		                                ).arg(val.length()).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0) ).arg(_in_pos));

	// store value inside BindPar class
	if( BP.dty == SQLT_STR )
	{
		//VARCHAR2
		strcpy((char*)BP.valuep, val.c_str());
	}
	else     // BP.dty == SQLT_CHR
	{
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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<int>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__, "Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<unsigned int>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__, "Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<long>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<unsigned long>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<float>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
	BindParNumber &BP2 = dynamic_cast<BindParNumber&>(BP);

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		BP2.set_number<double>(pos, val[pos]);
	}

	BP2._cnt = (ub4)val.size();

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
		throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)").arg(BP.dty));

	// Check vector size
	if(BP._max_cnt < val.size())
		throw_oci_exception(OciException(__TROTL_HERE__,"Input vector too long(length:%d vs. %d)\n").arg(val.size()).arg(BP._max_cnt));

	char *lastpos = (char*)BP.valuep;

//	for(const std::vector<tstring>::const_iterator it=val.begin(); it!=val.end(); ++it, ++pos)
	for(unsigned pos=0; pos<val.size(); ++pos)
	{
		//Check string length
		if( val[pos].length()+(BP.dty==SQLT_STR ? 1 : 0) > (unsigned)BP.value_sz )
			throw_oci_exception(OciException(__TROTL_HERE__, "String too long (length: %d max. %d)\n").arg(val[pos].length()).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0)));

		// store value inside BindPar class
		if( BP.dty == SQLT_STR )
		{
			//VARCHAR2
			memcpy(lastpos, val[pos].c_str(), val[pos].size());
			lastpos[val[pos].size()] = 0;
			lastpos += BP.value_sz;
			BP.alenp[pos] = (ub4)val[pos].size();
		}
		else
		{
			//CHAR
			memset(lastpos, ' ', BP.value_sz);
			memcpy(lastpos, val[pos].c_str(), val[pos].size());
			lastpos += BP.value_sz;
			BP.alenp[pos] = (ub4)val[pos].size();
		}
	}
	BP._cnt = (ub4)val.size();

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
	size_t l = strlen(val);
	// Check type correctness
	if( BP.dty != SQLT_CHR && BP.dty != SQLT_STR )
		throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in bind operation(tstring vs. dty(%d)\n").arg(BP.dty));

	//Check string length
	if( l+(BP.dty==SQLT_STR ? 1 : 0)  > (unsigned)BP.value_sz )
		throw_oci_exception(OciException(__TROTL_HERE__,"String too long (%d max. %d) at pos %d\n"
		                                ).arg(l).arg(BP.value_sz - (BP.dty==SQLT_STR ? 1 : 0) ).arg(_in_pos));

	// store value inside BindPar class
	if( BP.dty == SQLT_STR )
	{
		//VARCHAR2
		strcpy((char*)BP.valuep, val); // todo use memcpy here
	}
	else     // BP.dty == SQLT_CHR
	{
		//CHAR
		memset(BP.valuep, ' ', BP.value_sz);
		memcpy(BP.valuep, val, l);
		//strcpy((char*)BP.valuep, val);
		BP.value_sz = l;
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
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<int>( _last_buff_row );

	post_read_value(BP);
	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <unsigned int> (unsigned int &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<unsigned int>( _last_buff_row );

	post_read_value(BP);
	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <long> (long &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<long>( _last_buff_row );

	post_read_value(BP);
	return *this;
};


template<>
SqlStatement& SqlStatement::operator>> <unsigned long> (unsigned long &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<unsigned long>( _last_buff_row );

	post_read_value(BP);
	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <float> (float &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<float>( _last_buff_row );

	post_read_value(BP);
	return *this;
};

template<>
SqlStatement& SqlStatement::operator>> <double> (double &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	val = BP2.get_number<double>( _last_buff_row );

	post_read_value(BP);
	return *this;
};


template<>
SqlStatement& SqlStatement::operator>> <tstring> (tstring &val)
{
	pre_read_value();

	BindPar const &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());

	val = BP.get_string(_last_buff_row);

	post_read_value(BP);
	return *this;
}

SqlStatement& SqlStatement::operator>> (SqlValue &val)
{
	pre_read_value();

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

	// TODO this multimethod dispatcher uses dynamic_cast so it knows the real type of &val
	ConvertorForRead c(_last_buff_row);
	DispatcherForRead::Go(BP, val, c);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <int>(std::vector<int> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < iters; ++i)
	{
		val.at(i) = BP2.get_number<int>(i);
	}

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <unsigned int>(std::vector<unsigned int> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < iters; ++i)
		val.at(i) = BP2.get_number<unsigned int>(i);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <long>(std::vector<long> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < BP._cnt; ++i)
		val.at(i) = BP2.get_number<long>(i);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <unsigned long>(std::vector<unsigned long> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < iters; ++i)
		val.at(i) = BP2.get_number<unsigned long>(i);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <float>(std::vector<float> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < iters; ++i)
		val.at(i) = BP2.get_number<float>(i);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <double>(std::vector<double> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );
	BindParNumber const &BP2 = dynamic_cast<const BindParNumber&>(BP);

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < _iters; ++i)
		val.at(i) = BP2.get_number<double>(i);

	post_read_value(BP);
	return *this;
}

template<>
SqlStatement& SqlStatement::operator>> <tstring>(std::vector<tstring> &val)
{
	pre_read_value();

	if( get_stmt_type() != STMT_DECLARE &&
	                get_stmt_type() != STMT_BEGIN &&
	                get_stmt_type() != STMT_INSERT )
		throw_oci_exception(OciException(__TROTL_HERE__, "Reading vectors from SQL statements is not implemented yet"));

	BindPar const &BP( (get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar()  ) );

	ub4 iters = get_stmt_type() == STMT_INSERT ? _iters : BP._cnt;
	val.resize(iters);

	for(unsigned i=0; i < iters; ++i)
		val.at(i) = BP.get_string(i);

	post_read_value(BP);
	return *this;
}

SqlStatement& SqlStatement::operator>> (SqlCursor &val)
{
	pre_read_value();

	BindPar &BP(get_stmt_type() == STMT_SELECT ? get_next_column() : get_next_out_bindpar());
	BindParCursor &BP2 = dynamic_cast<BindParCursor&>(BP);

	val.c = BP2._cursors.at(_last_buff_row);
	BP2.redefine(_last_buff_row);

	post_read_value(BP);
	return *this;
};

// Plus error handling functions
OciException::OciException(tstring where, SqlStatement& stmt) :
	_where(where), _mess(where), _parse_offset(0), _line(0), _column(0)
{
	ub4 size = sizeof(_parse_offset);
	sword res2 = OCICALL(OCIAttrGet(stmt, OCI_HTYPE_STMT, &_parse_offset, &size, OCI_ATTR_PARSE_ERROR_OFFSET, stmt._errh));
	if (res2 != OCI_SUCCESS)
		_parse_offset = 0;

	char buffer[4096];
	sb4 errorcode;
	ub4 recordno = 1;
	ub4 last_length = 0;
	sword res;
	do
	{
		res = OCICALL(OCIErrorGet(stmt._errh, recordno, NULL, &errorcode, (OraText*)buffer+last_length, (ub4)sizeof(buffer)-last_length, OCI_HTYPE_ERROR));
		if(res == OCI_SUCCESS)
			_sql_error_code.push_back(errorcode);
		else
			break;
		++recordno;
		last_length = (ub4)strlen(buffer);
	}
	while(res == OCI_SUCCESS);

	if ( _sql_error_code.empty())   // No error was detected, but the exception was thrown
	{
		*buffer = '\0';
		_sql_error_code.push_back(0);
	}

	_last_sql = stmt.get_sql();

	if (!_last_sql.empty())
	{
		const char* sql = _last_sql.c_str();
		size_t l = strlen(buffer);

		unsigned line = 1;
		const char* s = sql;
		const char* e = sql + _parse_offset;
		const char* lp = sql;

		while(s < e)
			if (*s == '\n')
			{
				++line;

				if (*++s == '\r') ++s;

				lp = s;
			}
			else
				++s;

		size_t column = 1 + (s-lp);

		//#ifdef __STDC_WANT_SECURE_LIB__
		//		char* b = buffer+l + snprintf_s(buffer+l, sizeof(buffer)-l, _TRUNCATE, "\nerror at SQL position %d [%d.%d]:\n", _parse_offset, line, column);
		//#else
		char* b = buffer+l + snprintf(buffer+l, sizeof(buffer)-l-1, "\nerror at SQL position %u [%u.%u]:\n", (unsigned)_parse_offset, line, (unsigned)column);
		//#endif
		char* p = b;
		s = lp;
		while(*s && *s!='\n' && p<buffer+sizeof(buffer)-3)
			*p++ = *s++;

		*p++ = '\n';

		b = p;
		const char* err_ptr = sql + _parse_offset;
		s = lp;
		for(; s<err_ptr && p<buffer+sizeof(buffer)-2; ++s)
		{
			if (*s=='\n' || *sql=='\r')
				p = b;
			else if (*s == '\t')
				*p++ = '\t';
			else
				*p++ = ' ';
		}

		*p++ = '^';
		*p = '\0';

		_line = line;
		_column = column;

		l = strlen(buffer);
		//#ifdef __STDC_WANT_SECURE_LIB__
		//		snprintf_s(buffer+l, sizeof(buffer)-l, _TRUNCATE, "\n\nlast SQL statement:\n%s\n", sql);
		//#else
		snprintf(buffer+l, sizeof(buffer)-l, "\n\nlast SQL statement:\n%s\n", sql);
		//#endif
	}
	_mess.push_back('\n');
	_mess.append(buffer);
#ifdef DEBUG
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess.push_back('\n');
	_mess.append(_stack.str());
#endif
}

};
