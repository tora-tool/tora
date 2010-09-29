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
Util::RegisterInFactory<BindParCollectionTabVarchar, CustDefineParFactTwoParmSing> regCustDefineNTY_VARCHAR_TAB("TABLE OF VARCHAR2");

void BindParCollectionTabNum::define_hook(SqlStatement &stmt)
{
	//ub4 *size = 0;
	sword res;
	res = OCICALL(OCIDefineObject(defnpp, stmt._errh, _collection_tdo,
				      valuep,
				      (ub4 *) NULL,
				      (void**) _collection_indp, //(dvoid **) NULL
				      (ub4 *) 0));
 	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

void BindParCollectionTabNum::bind_hook(SqlStatement &stmt)
{
	//TODO
	throw OciException(__TROTL_HERE__, "Not implemented yet");
}

// TODO
tstring BindParCollectionTabNum::get_string(unsigned int row) const
{
	OCITypeCode tc;
	OCIType *type = (OCIType *)NULL;
	boolean isNull, eoc = false;
	OCIInd ind;
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
		OCINumber *ocinum = NULL;
		void *elemind = NULL;
		int i;
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
		
		if(*(sb2*)(elemind) == OCI_IND_NULL)  
		{
			_stringrepres << "NULL";
		} else {
			res = OCICALL(OCINumberToInt(_stmt._errh, ocinum, sizeof(i), 0, &i));
			oci_check_error(__TROTL_HERE__, _stmt._errh, res); 
			_stringrepres << i;
		}
		
		comma = true;
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
	OCITypeCode tc;
	OCIType *type = (OCIType *)NULL;
	boolean isNull, eoc = false;
	OCIInd ind;
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
		int i;
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
			// res = OCICALL(OCINumberToInt(_stmt._errh, ocinum, sizeof(i), 0, &i));
			// oci_check_error(__TROTL_HERE__, _stmt._errh, res); 
			// _stringrepres << i;
		}
		
		comma = true;
	}
	_stringrepres << ")";
	
	res = sword(OCIIterDelete (_env, _stmt._errh, &itr));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
	
	return _stringrepres.str();
};
	
// tstring SqlCollection::str() const
// {
// 	return _stringrepres.str();
// };


};
