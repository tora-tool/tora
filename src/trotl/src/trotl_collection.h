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

#ifdef WIN32
#include <stdio.h>
#include <wtypes.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_export.h"
#include "trotl_base.h"
#include "trotl_stat.h"

namespace trotl {

struct TROTL_EXPORT BindParCollectionTabNum: public SqlStatement::BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

 	BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, ColumnType &ct)
		: SqlStatement::BindPar(pos, stmt, ct)
		, _collection_indp(NULL)
		, _collection_typecode(ct._collection_typecode)
	{
		valuep = (void**) calloc(_cnt, sizeof(void*));
		value_sz = sizeof(OCIColl*);
		_collection_indp = (void**)calloc(_cnt, sizeof(void*));
		
		dty =  SQLT_NTY; //  named collection type (varray or nested table)
		type_name = ct.get_type_str();
		
		_collection_tdo = ct._tdo;
	}
	
	BindParCollectionTabNum(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
		: SqlStatement::BindPar(pos, stmt, decl)
		, _collection_indp(NULL)
		// , _collection_typecode(???) TODO
	{		
		valuep = (void**) calloc(_cnt, sizeof(void*));
		value_sz = sizeof(OCIColl*);
		_collection_indp = (void**)calloc(_cnt, sizeof(void*));
		
		dty =  SQLT_NTY;
		type_name = "???"; // TODO

		// _collection_tdo = ct._tdo; TODO
	};

	~BindParCollectionTabNum() {};

	virtual tstring get_string(unsigned int row) const;

	virtual void define_hook();
	virtual void bind_hook();


//private:
	mutable std::stringstream _stringrepres;
	void **_collection_indp;
	OCITypeCode _collection_typecode; 
	OCIType *_collection_tdo;
};

struct TROTL_EXPORT BindParCollectionTabVarchar: public SqlStatement::BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

 	BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, ColumnType &ct)
		: SqlStatement::BindPar(pos, stmt, ct)
		, _collection_indp(NULL)
		, _collection_typecode(ct._collection_typecode)
	{
		valuep = (void**) calloc(_cnt, sizeof(void*));
		value_sz = sizeof(OCIColl*);
		_collection_indp = (void**)calloc(_cnt, sizeof(void*));
		
		dty =  SQLT_NTY; //  named collection type (varray or nested table)
		type_name = ct.get_type_str();
		
		_collection_tdo = ct._tdo;
	}
	
	BindParCollectionTabVarchar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
		: SqlStatement::BindPar(pos, stmt, decl)
		, _collection_indp(NULL)
		// , _collection_typecode(???) TODO
	{		
		valuep = (void**) calloc(_cnt, sizeof(void*));
		value_sz = sizeof(OCIColl*);
		indp = (OCIInd*) calloc(_cnt, sizeof(void*));
		_collection_indp = (void**)calloc(_cnt, sizeof(void*));
		
		dty =  SQLT_NTY;
		type_name = "???"; // TODO
		// _collection_tdo = ct._tdo; TODO
	};

	~BindParCollectionTabVarchar() {};

	virtual tstring get_string(unsigned int row) const;

	virtual void define_hook(SqlStatement &stmt);
	virtual void bind_hook(SqlStatement &stmt);


//private:
	mutable std::stringstream _stringrepres;
	void **_collection_indp;
	OCITypeCode _collection_typecode; 	
	OCIType *_collection_tdo;
};
	
struct TROTL_EXPORT SqlCollection : public SqlValue
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	SqlCollection(OciConnection &conn)
	  : _conn(conn)
          , _collection_typecode(0)
	  , _collection_tdo(NULL)

	{}

	
	SqlCollection(const SqlValue &value, OciConnection &conn)
	  : _conn(conn)
	  , _collection_tdo(NULL)
	{		
		throw OciException(__TROTL_HERE__, "Not implemented yet");
	}

	~SqlCollection() {};
	
	tstring str() const;

	operator tstring() const { return str(); };

//protected:
	OciConnection&	_conn;
	mutable std::stringstream _stringrepres;
	tstring _data_type_name;
	
	OCIColl *_valuep;
	OCITypeCode _collection_typecode;
	OCIType *_collection_tdo;
	OCIInd  _ind;
};


};
#endif /*TROTL_COLLECTION_H_*/
