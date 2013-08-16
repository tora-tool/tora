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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"

#include "loki/Factory_alt.h"

namespace trotl
{
/* This set of specialized templates represent translation table
 * "userspace datatype" => "OCI datatype constant"
 * This template is used as parameter in specialization of the template TBindParInt
 * It is not necessary to register all datatypes in this template
 * look bellow for usage examples
 **/
template <class type>
struct SQLT
{
	// Almost template specializations should have enum value set to appropriate SQLT_*
	//enum { value = SQLT_* };
};

/* This set of specialized templates represents translation table
 * "userspace datatype" => "OCI datatype constant"
 * This template is used as parameter in specialization of the template TBindParInt
 * It is not necessary to register all datatypes in this template
 * it is used for numerical datatypes only
 *
 * See trotl_stat.h
 **/
template <> struct SQLT<int>
{
	enum { value = SQLT_INT };
};
template <> struct SQLT<unsigned int>
{
	enum { value = SQLT_UIN };
};
template <> struct SQLT<long>
{
	enum { value = SQLT_INT };
};
template <> struct SQLT<unsigned long>
{
	enum { value = SQLT_UIN };
};
template <> struct SQLT<long long>
{
	enum { value = SQLT_INT };
};
template <> struct SQLT<float>
{
	enum { value = SQLT_FLT };
};
template <> struct SQLT<double>
{
	enum { value = SQLT_FLT };
};

// /// column description
// struct TROTL_EXPORT ColumnType
// {
// 	tstring _column_name;
// 	tstring _data_type_name;
// 	tstring _data_type_dblink; // ?? TODO
// 	tstring _reg_name;

// 	ub2	_data_type;
// 	ub2	_width;
// 	ub1	_char_semantics;	// for Oracle 9
// 	sb1	_scale;
// 	sb2	_precision;
// 	ub1	_nullable;
// 	bool    _utf16;

// 	// for complex types
// 	OCITypeCode _typecode, _collection_typecode;
// 	dvoid *_collection_dschp;
// 	OCIType *_tdo;
// 	OCIRef *_oref;

// 	ColumnType(): _data_type(0)
// 		, _width(0)
// 		, _char_semantics(0)
// 		, _scale(-127)
// 		, _precision(0)
// 		, _nullable(false)
// 		, _utf16(false)
// 		, _collection_dschp(NULL)
// 		, _tdo(NULL)
// 		, _oref(NULL)
// 	{};

// 	ColumnType(SqlStatement &stat, dvoid* handle);
// 	void describe(SqlStatement &stat, dvoid* handle);

// 	tstring get_type_str(bool show_null=false) const;
// };


/// Client side base class All classes Sql* are descendents of SqlValue
struct TROTL_EXPORT SqlInd
{
	SqlInd(OCIInd ind=-1) : _ind(ind)
	{}

	operator OCIInd() const
	{
		return _ind;
	}
	operator OCIInd*()
	{
		return &_ind;
	}

	bool is_null() const
	{
		return _ind==-1;
	}
	bool is_not_null() const
	{
		return _ind!=-1;
	}

	void clear()
	{
		_ind = -1;
	}
	void set()
	{
		_ind = 0;
	}
	void set(const SqlInd &other)
	{
		_ind=other._ind;
	}
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
		throw_oci_exception(OciException(__TROTL_HERE__, "Invalid SqlValue initialization: const char*"));
	};

	bool is_null() const
	{
		return _ind.is_null();
	}
	bool is_not_null() const
	{
		return _ind.is_not_null();
	}

	void set_null()
	{
		_ind.set();
	};
	void clear_null()
	{
		_ind.clear();
	};

	const SqlInd& ind() const
	{
		return _ind;
	}
	SqlInd& ind()
	{
		return _ind;
	}

	virtual ~SqlValue() {};

	virtual tstring str() const
	{
		return "";
	}; //TODO  should be pure virtual??
protected:
	friend struct Convertor;
	SqlInd	_ind;
};

struct BindVarDecl;
class DescribeColumn;
/* This structure represents a pack of all values needed for bind/define operation
 * All descendents of this class are created using factory BindParFactTwoParmSing
 * or DefineParFactTwoParm
 * All descendents have to be resistered by creating an instance of the template
 * Util::RegisterInFactory - look at trotl_int.cpp
 **/
struct TROTL_EXPORT BindPar
{
	enum
	{
		BIND_IN=1,
		BIND_INOUT=2,
		BIND_OUT=4,
		DEFINE_SELECT=8
	};

	/* Placeholder for Define operations */
	BindPar(unsigned int pos, SqlStatement &stmt, DescribeColumn *ct);

	/* Placeholder for Bind operations */
	BindPar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindPar()
	{
		if(indp)
		{
			free(indp);
			indp = NULL;
		}
		if(rlenp)
		{
			free(rlenp);
			rlenp = NULL;
		}
		// if(rcodep) { delete[] rcodep; rcodep = NULL; }
		if(alenp)
		{
			free(alenp);
			alenp = NULL;
		}
		if(valuep)
		{
			free(valuep);
			valuep = NULL;
		}
	};

	// every datatype can be converted to a string
	virtual tstring get_string(unsigned int row) const = 0;

	/* These two callbacks are used by descendents to call
	 * OCIBindArrayOfStructures, OCIBindObject etc, ...
	 * is called *after* OCIDefineByPos
	 */
	virtual void bind_hook() {};
	virtual void define_hook() {};
	virtual void fetch_hook(ub4 iter, ub4 idx, ub1 piece, ub4, sb2 ind) {};

	/* members used for OCI calls */
	dvoid **valuep;
	sb4 value_sz;
	ub2 dty;
	OCIInd *indp;	// OCIInd aka sb2 ignored for SQL_NTY and SQL_REF
	void *rlenp;
	//ub2 *rcodep;
	ub2 *alenp;	// (ub2*) in case of Bind op. (ub4*) in case of Define op.
	ub4 mode;	// define mode = OCI_DEFAULT, except for SQLT_LONG => OCI_DYNAMIC_FETCH TODO fix long
	OCIBind *bindp;
	OCIDefine *defnpp;  //TODO union with OCIBind *bindpp

	OciEnv &_env;
	SqlStatement &_stmt;
	unsigned int _pos, /*_cnt,*/ _max_cnt, _bind_type;
	ub4 _cnt;
	bool _bound;
	tstring _type_name, _reg_name, _bind_name, _bind_typename;

	//protected:
	/* NOTE these two functions do not work with complex types SQLT_NTY */
	bool is_null(unsigned row) const
	{
		return indp[row]==OCI_IND_NULL;
	};
	bool is_not_null(unsigned row) const
	{
		return indp[row]!=OCI_IND_NULL;
	};

	BindPar(const BindPar &other);
	friend class SqlStatement;
}; // class BindPar

/** Factory for creating instantions of BindPar structure.
 *  Name of datatype is used as a key
 *  each constructor takes three arguments - position,  SqlStatement, BindVarDecl
 *  BindVarDecl - is created by simple PL/SQL parser
 *  this factory is used for PL/SQL statements
 **/
//class TROTL_EXPORT BindParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, BindVarDecl&)> {};
//TROTL_EXPORT  typedef Loki::SingletonHolder<BindParFactTwoParm> BindParFactTwoParmSing;
TROTL_EXPORT typedef Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, BindVarDecl&)> BindParFactTwoParm;
class TROTL_EXPORT  BindParFactTwoParmSing: public Loki::SingletonHolder<BindParFactTwoParm> {};


/** Factory for creating instantions of BindPar structure.
 *  SQLT_<X> const is used as a key
 *  each constructor takes three arguments - position,  SqlStatement, DescribeColumn*
 *  this factory is used in SELECT statements
 **/
//class TROTL_EXPORT DefineParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&), int> {};
//typedef Loki::SingletonHolder<DefineParFactTwoParm> DefineParFactTwoParmSing;
//TROTL_EXPORT typedef Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&), int> DefineParFactTwoParm;
TROTL_EXPORT typedef Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, DescribeColumn*), int> DefineParFactTwoParm;
class TROTL_EXPORT  DefineParFactTwoParmSing: public Loki::SingletonHolder<DefineParFactTwoParm> {};


/** Factory for creating instantions of BindPar structure.
 *  Name of the type is used as a key, i.e. 'SYS.XMLTYPE'
 *  each constructor takes the arguments - position,  SqlStatement, DescribeColumn*
 *  this factory will be used for complex/custom datatypes
 **/
//class TROTL_EXPORT CustDefineParFactTwoParm: public Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, ColumnType&)> {};
//typedef Loki::SingletonHolder<CustDefineParFactTwoParm> CustDefineParFactTwoParmSing;
TROTL_EXPORT typedef Util::GenericFactory<BindPar, LOKI_TYPELIST_3(unsigned int, SqlStatement&, DescribeColumn*)> CustDefineParFactTwoParm;
class TROTL_EXPORT  CustDefineParFactTwoParmSing: public Loki::SingletonHolder<CustDefineParFactTwoParm> {};

};

#endif
