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

#ifndef TROTL_VAR_H_
#define TROTL_VAR_H_

#include <typeinfo>
#include "trotl_base.h"
#include "trotl_parser.h"

#include "Loki/Factory_alt.h"

#include <string.h>

namespace trotl {

// Todo this needs to be fixed on windows
#if defined(TROTL_MAKE_DLL) || defined(__GNUC__) 
extern int TROTL_EXPORT g_OCIPL_BULK_ROWS;
extern int TROTL_EXPORT g_OCIPL_MAX_LONG;
extern const char TROTL_EXPORT *g_TROTL_DEFAULT_NUM_FTM;
extern const char TROTL_EXPORT *g_TROTL_DEFAULT_DATE_FTM;
#else
int TROTL_EXPORT g_OCIPL_BULK_ROWS;
int TROTL_EXPORT g_OCIPL_MAX_LONG;
const char TROTL_EXPORT *g_TROTL_DEFAULT_NUM_FTM;
const char TROTL_EXPORT *g_TROTL_DEFAULT_DATE_FTM;
#endif

struct Convertor;

/// column description
struct TROTL_EXPORT ColumnType
{
	tstring _column_name;
	tstring _data_type_name;
	tstring _data_type_dblink; // ?? TODO
	tstring _reg_name;
					
	ub2	_data_type;
	ub2	_width;
	ub1	_char_semantics;	// for Oracle 9
	sb1	_scale;
	sb2	_precision;
	ub1	_nullable;
	bool    _utf16;

	// for complex types
	OCITypeCode _typecode, _collection_typecode;
	dvoid *_collection_dschp;	
	OCIType *_tdo;
	OCIRef *_oref;
	
	ColumnType(): _data_type(0)
	  , _width(0)
	  , _char_semantics(0)
	  , _scale(-127)
	  , _precision(0)
	  , _nullable(false)
	  , _utf16(false)
	  , _collection_dschp(NULL)
	  , _tdo(NULL)
	  , _oref(NULL)
	  {};

	ColumnType(SqlStatement &stat, dvoid* handle);
	void describe(SqlStatement &stat, dvoid* handle);
	
	tstring get_type_str(bool show_null=false) const;
};


/// Client side base class All classes Sql* are descendents of SqlValue
struct TROTL_EXPORT SqlInd
{
	SqlInd(OCIInd ind=-1) : _ind(ind)
	{}

	operator OCIInd() const {return _ind;}
	operator OCIInd*() {return &_ind;}

	bool is_null() const {return _ind==-1;}
	bool is_not_null() const {return _ind!=-1;}

	void clear() {_ind = -1;}
	void set() {_ind = 0;}
	void set(const SqlInd &other) { _ind=other._ind; }
protected:
	OCIInd	_ind;	// short
};

/// base class of bind variable structures containing the indicator value
struct TROTL_EXPORT SqlValue
{
	SqlValue() {};
        SqlValue(const SqlValue& other) : _ind(other._ind) {};
	SqlValue(const char *)
	{
		throw OciException(__TROTL_HERE__, "Invalid SqlValue initialization: const char*");
	};

	bool is_null() const {return _ind.is_null();}
	bool is_not_null() const {return _ind.is_not_null();}

	void set_null() { _ind.set(); };
	void clear_null() { _ind.clear(); };

	const SqlInd& ind() const {return _ind;}
	SqlInd& ind() {return _ind;}

	virtual ~SqlValue() {};

	virtual tstring str() const { return ""; }; //TODO  should be pure virtual??
protected:
	friend struct Convertor;
	SqlInd	_ind;
};


};

#endif
