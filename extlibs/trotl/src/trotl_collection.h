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

#ifndef TROTL_COLLECTION_H_
#define TROTL_COLLECTION_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_conn.h"
#include "trotl_var.h"


namespace trotl
{

struct TROTL_EXPORT BindParCollectionTabNum: public BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParCollectionTabNum() {};

	virtual tstring get_string(unsigned int row) const;

	virtual void define_hook();
	virtual void bind_hook();


	//private:
	mutable tostream _stringrepres;
	void **_collection_indp;
	OCITypeCode _collection_typecode;
	OCITypeCode _collection_data_type;
	OCIType *_collection_tdo;
};

struct TROTL_EXPORT BindParCollectionTabVarchar: public BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParCollectionTabVarchar() {};

	virtual tstring get_string(unsigned int row) const;

	virtual void define_hook();
	virtual void bind_hook();

	//private:
	mutable tostream _stringrepres;
	void **_collection_indp;
	OCITypeCode _collection_typecode;
	OCITypeCode _collection_data_type;
	OCIType *_collection_tdo;
};

struct TROTL_EXPORT SqlCollection : public SqlValue
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	SqlCollection(OciConnection &conn)
		: _conn(conn)
		, _valuep(0)
		, _collection_typecode(0)
		, _collection_data_type(0)
		, _collection_tdo(NULL)

	{}


	SqlCollection(const SqlValue &value, OciConnection &conn)
		: _conn(conn)
		, _valuep(0)
		, _collection_typecode(0)
		, _collection_data_type(0)
		, _collection_tdo(NULL)
	{
		throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
	}

	virtual ~SqlCollection() {};

	tstring str() const;

	operator tstring() const
	{
		return str();
	};

	//protected:
	OciConnection&	_conn;
	mutable tostream _stringrepres;
	tstring _data_type_name;

	OCIColl *_valuep;
	OCITypeCode _collection_typecode;
	OCITypeCode _collection_data_type;
	OCIType *_collection_tdo;
	OCIInd  _ind;
};


};
#endif /*TROTL_COLLECTION_H_*/
