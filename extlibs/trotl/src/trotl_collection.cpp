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

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_collection.h"
#include "trotl_describe.h"
#include "trotl_stat.h"

namespace trotl
{

//Util::RegisterInFactory<BindParCollectionTabNum, BindParFactTwoParmSing> regBindCollection("???Collection???"); TODO
//Util::RegisterInFactory<BindParCollectionTabNum, DefineParFactTwoParmSing, int> regDefineCollection((OCI_TYPECODE_VARRAY << 8) + 2);
Util::RegisterInFactory<BindParCollectionTabNum, CustDefineParFactTwoParmSing> regCustDefineNTY_NUM_TAB("TABLE OF NUMBER");
Util::RegisterInFactory<BindParCollectionTabNum, CustDefineParFactTwoParmSing> regCustDefineNTY_INT_TAB("TABLE OF INTEGER");
Util::RegisterInFactory<BindParCollectionTabVarchar, CustDefineParFactTwoParmSing> regCustDefineNTY_VARCHAR_TAB("TABLE OF VARCHAR2");
Util::RegisterInFactory<BindParCollectionTabVarchar, CustDefineParFactTwoParmSing> regCustDefineNTY_CHAR_TAB("TABLE OF CHAR");

BindParCollectionTabNum::BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct)
	: BindPar(pos, stmt, ct)
	, _collection_indp(NULL)
	, _collection_typecode(ct->_collection_typecode)
	, _collection_data_type(ct->_collection_data_type)
{
	valuep = (void**) calloc(_cnt, sizeof(void*));
	value_sz = sizeof(OCIColl*);
	_collection_indp = (void**)calloc(_cnt, sizeof(void*));

	dty =  SQLT_NTY; //  named collection type (varray or nested table)
	_type_name = ct->typeName();

	sword res = OCICALL(OCITypeByRef(stmt._env, stmt._errh, ct->_ref_tdo, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &_collection_tdo));
	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

BindParCollectionTabNum::BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
	: BindPar(pos, stmt, decl)
	, _collection_indp(NULL)
	// , _collection_typecode(???) TODO
{
	valuep = (void**) calloc(_cnt, sizeof(void*));
	value_sz = sizeof(OCIColl*);
	_collection_indp = (void**)calloc(_cnt, sizeof(void*));

	dty =  SQLT_NTY;
	_type_name = "???"; // TODO

	// _collection_tdo = ct._tdo; TODO
};

void BindParCollectionTabNum::define_hook()
{
	//ub4 *size = 0;
	sword res;
	res = OCICALL(OCIDefineObject(defnpp, _stmt._errh, _collection_tdo,
	                              valuep,
	                              (ub4 *) NULL,
	                              (void**) _collection_indp, //(dvoid **) NULL
	                              (ub4 *) 0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
}

void BindParCollectionTabNum::bind_hook()
{
	//TODO
	throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
}

// TODO
tstring BindParCollectionTabNum::get_string(unsigned int row) const
{
	bOOlean eoc = false;
	sword res;
	OCIIter *itr;

	_stringrepres.str("");

	if( *(sb2*)(_collection_indp[row]) == OCI_IND_NULL)
	{
		_stringrepres << "<NULL>";
		return _stringrepres.str();
	}

	res = OCICALL(OCIIterCreate (_env, _stmt._errh, (OCIColl*)valuep[row], &itr));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	_stringrepres << _type_name << "(";

	bool comma=false;
	while(!eoc)
	{
		OCINumber *ocinum = NULL;
		void *elemind = NULL;
		res = OCICALL(OCIIterNext (_env, _stmt._errh,
		                           itr,
		                           (void**) &ocinum,
		                           (void**) &elemind, //NULL, //void **elemind,
		                           &eoc));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res);

		if(eoc)
			break;

		if(comma)
			_stringrepres << ", ";
		comma = true;
		if(*(sb2*)(elemind) == OCI_IND_NULL)
		{
			_stringrepres << "NULL";
		}
		else
		{
			text str_buf[64];
			ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);
			sword res = OCICALL(OCINumberToText(_stmt._errh,
			                                    (OCINumber*) ocinum,
			                                    (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
			                                    (ub4)strlen(g_TROTL_DEFAULT_NUM_FTM),
			                                    0, // CONST OraText *nls_params,
			                                    0, // ub4 nls_p_length,
			                                    (ub4*)&str_len,
			                                    str_buf
			                                   ));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
			_stringrepres << tstring((const char*)str_buf, str_len);
		}
	}
	_stringrepres << ")";

	res = sword(OCIIterDelete (_env, _stmt._errh, &itr));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	return _stringrepres.str();
};

BindParCollectionTabVarchar::BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct)
	: BindPar(pos, stmt, ct)
	, _collection_indp(NULL)
	, _collection_typecode(ct->_collection_typecode)
	, _collection_data_type(ct->_collection_data_type)
{
	valuep = (void**) calloc(_cnt, sizeof(void*));
	value_sz = sizeof(OCIColl*);
	_collection_indp = (void**)calloc(_cnt, sizeof(void*));

	dty =  SQLT_NTY; //  named collection type (varray or nested table)
	_type_name = ct->typeName();

	//_collection_tdo = ct->_ref_tdo;
	sword res = OCICALL(OCITypeByRef(stmt._env, stmt._errh, ct->_ref_tdo, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &_collection_tdo));
	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

BindParCollectionTabVarchar::BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
	: BindPar(pos, stmt, decl)
	, _collection_indp(NULL)
	// , _collection_typecode(???) TODO
{
	valuep = (void**) calloc(_cnt, sizeof(void*));
	value_sz = sizeof(OCIColl*);
	indp = (OCIInd*) calloc(_cnt, sizeof(void*));
	_collection_indp = (void**)calloc(_cnt, sizeof(void*));

	dty =  SQLT_NTY;
	_type_name = "???"; // TODO
	// _collection_tdo = ct._tdo; TODO
};

void BindParCollectionTabVarchar::define_hook()
{
	sword res;
	res = OCICALL(OCIDefineObject(defnpp, _stmt._errh, _collection_tdo,
	                              valuep,
	                              (ub4 *) NULL,
	                              (void**) _collection_indp, //(dvoid **) NULL
	                              (ub4 *) 0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
}

void BindParCollectionTabVarchar::bind_hook()
{
	//TODO
	throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
}

tstring BindParCollectionTabVarchar::get_string(unsigned int row) const
{
	bOOlean eoc = false;
	sword res;
	OCIIter *itr;

	if( *(sb2*)(_collection_indp[row]) == OCI_IND_NULL)
	{
		_stringrepres << "<NULL>";
		return _stringrepres.str();
	}

	res = OCICALL(OCIIterCreate (_env, _stmt._errh, (OCIColl*)valuep[row], &itr));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	_stringrepres.str("");
	_stringrepres << _type_name << "(";

	bool comma=false;
	while(!eoc)
	{
		void *ocistring = NULL;
		void *elemind = NULL;
		res = OCICALL(OCIIterNext (_env, _stmt._errh,
		                           itr,
		                           (void**) &ocistring,
		                           (void**) &elemind, //NULL, //void **elemind,
		                           &eoc));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res);

		if(eoc)
			break;

		if(comma)
			_stringrepres << ", ";

		if(*(sb2*)(elemind) == OCI_IND_NULL)
		{
			_stringrepres << "NULL";
		}
		else
		{
			_stringrepres << '\'' << OCIStringPtr(_env, *(OCIString **)ocistring) << '\'';
		}

		comma = true;
	}
	_stringrepres << ")";

	res = sword(OCIIterDelete (_env, _stmt._errh, &itr));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	return _stringrepres.str();
};

tstring SqlCollection::str() const
{
	bOOlean eoc = false;
	sword res;
	OCIIter *itr;

	_stringrepres.str("");

	if( _ind == OCI_IND_NULL)
	{
		_stringrepres << "<NULL>";
		return _stringrepres.str();
	}

	res = OCICALL(OCIIterCreate (_conn._env, _conn._env._errh, _valuep, &itr));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);

	_stringrepres << _data_type_name <<  "(";

	bool comma=false;
	while(!eoc)
	{
		void *field = NULL;
		void *elemind = NULL;
		res = OCICALL(OCIIterNext (_conn._env, _conn._env._errh,
		                           itr,
		                           (void**) &field,
		                           (void**) &elemind, //NULL, //void **elemind,
		                           &eoc));
		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);

		if(eoc)
			break;

		if(comma)
			_stringrepres << ", ";
		comma = true;
		if(*(sb2*)(elemind) == OCI_IND_NULL)
		{
			_stringrepres << "NULL";
		}
		else
		{
			switch(_collection_data_type)
			{
			case SQLT_NUM:
			{
				text str_buf[64];
				ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);
				sword res = OCICALL(OCINumberToText(_conn._env._errh,
				                                    (OCINumber*) field,
				                                    (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
				                                    (ub4)strlen(g_TROTL_DEFAULT_NUM_FTM),
				                                    0, // CONST OraText *nls_params,
				                                    0, // ub4 nls_p_length,
				                                    (ub4*)&str_len,
				                                    str_buf
				                                   ));
				oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
				_stringrepres << tstring((const char*)str_buf, str_len);
			}
			break;
			case SQLT_AFC: // fixed char
			case SQLT_CHR: // NOTE: this is also valid for array of VARCHAR2
			{
				_stringrepres << '\'' << OCIStringPtr(_conn._env, *(OCIString **)field) << '\'';
			}
			break;
			default:
				throw_oci_exception(OciException(__TROTL_HERE__,
				                                 "Collection to string convertorsion - Not implemented yet: %d")
				                    .arg(_collection_data_type)
				                   );
				// no break here
			}
		}
	}
	_stringrepres << ")";

	res = sword(OCIIterDelete (_conn._env, _conn._env._errh, &itr));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);

	return _stringrepres.str();
};


};
