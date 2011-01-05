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


#include "trotl_collection.h"
#include <string.h>
namespace trotl {

//Util::RegisterInFactory<BindParCollectionTabNum, BindParFactTwoParmSing> regBindCollection("???Collection???"); TODO
//Util::RegisterInFactory<BindParCollectionTabNum, DefineParFactTwoParmSing, int> regDefineCollection((OCI_TYPECODE_VARRAY << 8) + 2);
Util::RegisterInFactory<BindParCollectionTabNum, CustDefineParFactTwoParmSing> regCustDefineNTY_NUM_TAB("TABLE OF NUMBER");
Util::RegisterInFactory<BindParCollectionTabNum, CustDefineParFactTwoParmSing> regCustDefineNTY_INT_TAB("TABLE OF INTEGER");
Util::RegisterInFactory<BindParCollectionTabVarchar, CustDefineParFactTwoParmSing> regCustDefineNTY_VARCHAR_TAB("TABLE OF VARCHAR2");
Util::RegisterInFactory<BindParCollectionTabVarchar, CustDefineParFactTwoParmSing> regCustDefineNTY_CHAR_TAB("TABLE OF CHAR");

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
	throw OciException(__TROTL_HERE__, "Not implemented yet");
}

// TODO
tstring BindParCollectionTabNum::get_string(unsigned int row) const
{
	boolean eoc = false;
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
	
	_stringrepres << type_name << "(";

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
		} else {
			text str_buf[64];
			ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);			
			sword res = OCICALL(OCINumberToText(_stmt._errh,
							    (OCINumber*) ocinum,
							    (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
							    strlen(g_TROTL_DEFAULT_NUM_FTM),
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

void BindParCollectionTabVarchar::define_hook(SqlStatement &stmt)
{
	sword res;
	res = OCICALL(OCIDefineObject(defnpp, stmt._errh, _collection_tdo,
				      valuep,
				      (ub4 *) NULL,
				      (void**) _collection_indp, //(dvoid **) NULL
				      (ub4 *) 0));
 	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

void BindParCollectionTabVarchar::bind_hook(SqlStatement &stmt)
{
	//TODO
	throw OciException(__TROTL_HERE__, "Not implemented yet");
}
	
tstring BindParCollectionTabVarchar::get_string(unsigned int row) const
{
	boolean eoc = false;
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
	_stringrepres << type_name << "(";

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
		} else {
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
	boolean eoc = false;
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
		} else {
			switch(_collection_typecode)
			{
			case SQLT_NUM:
			{
				text str_buf[64];
				ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);			
				sword res = OCICALL(OCINumberToText(_conn._env._errh,
								    (OCINumber*) field,
								    (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
								    strlen(g_TROTL_DEFAULT_NUM_FTM),
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
				throw OciException(__TROTL_HERE__, "Collection to string convertorsion - Not implemented yet");
			}
		}	       
	}
	_stringrepres << ")";
	
	res = sword(OCIIterDelete (_conn._env, _conn._env._errh, &itr));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	
	return _stringrepres.str();
};

	
// tstring SqlCollection::str() const
// {
// 	return _stringrepres.str();
// };


};
