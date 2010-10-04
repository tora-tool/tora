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

#ifndef TROTL_INT_H_
#define TROTL_INT_H_

#include "trotl_export.h"
#include "trotl_base.h"
#include "trotl_stat.h"
#include "Loki/static_check.h"
#include "Loki/TypeTraits.h"

#include <string.h>

namespace trotl {

enum {INT_NULL = 0};

struct BindParNumber;
typedef BindParNumber prefferedNumericType;

/* The template TBindPar is defined in trotl_stat.h */   
typedef TROTL_EXPORT TBindParInt<int,           SQLT<int>::value> BindParInt;
typedef TROTL_EXPORT TBindParInt<long,          SQLT<long>::value> BindParLong;
typedef TROTL_EXPORT TBindParInt<unsigned int,  SQLT<unsigned int>::value> BindParUInt;
typedef TROTL_EXPORT TBindParInt<unsigned long, SQLT<unsigned long>::value> BindParULong;
typedef TROTL_EXPORT TBindParInt<double,        SQLT<double>::value> BindParDouble;
typedef TROTL_EXPORT TBindParInt<float,         SQLT<float>::value> BindParFloat;

namespace Private
{

  /*
   * Helper class for BindParNumber
   */
template<class wrapped_type>
struct NumericConvertor
{
	inline static void Cast(OciError &_errh, OCINumber const *number, wrapped_type &rsl, Loki::Int2Type<false>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_type>::isIntegral, isNotIntegral);

		try {
			
//#ifdef WIN32
			/* This is lauzy workaround
			Windows version of OCINumberToInt segfaults on floats <0,1>
			while Linux version returns the underflow error
			*/
			{
				sword res;
				boolean isint;
				res = OCICALL(OCINumberIsInt(_errh, number, &isint));
				oci_check_error(__TROTL_HERE__, _errh, res);

				if(!isint)
				{
					OCINumber truncated;
					res = OCICALL(OCINumberRound (_errh, number, 0 /*decplace*/, &truncated));
					oci_check_error(__TROTL_HERE__, _errh, res);
					res = OCICALL(OCINumberToInt(_errh, &truncated, sizeof(wrapped_type),
						(Loki::TypeTraits<wrapped_type>::isSignedInt ? OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
						&rsl
						));
					oci_check_error(__TROTL_HERE__, _errh, res);
					return;
				}
			}
//#endif
			sword res = OCICALL(OCINumberToInt(_errh, 
							   number, sizeof(wrapped_type),
							   (Loki::TypeTraits<wrapped_type>::isSignedInt ?
							    OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
							   &rsl
						    ));
			oci_check_error(__TROTL_HERE__, _errh, res);
		} catch (OciException const &e) {
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			rsl = 0;
		}		    		 
	}

	inline static void Cast(OciError &_errh, OCINumber const *number, wrapped_type &rsl, Loki::Int2Type<true>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_type>::isFloat, isNotReal);

		try {
			sword res = OCICALL(OCINumberToReal(_errh,
							    number,
							    sizeof(wrapped_type),
							    &rsl
						    ));
			oci_check_error(__TROTL_HERE__, _errh, res);
		} catch (OciException const &e) {
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			rsl = 0;
		} 
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const wrapped_type &val, Loki::Int2Type<false>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_type>::isIntegral, isNotIntegral);

		try {
			sword res = OCICALL(OCINumberFromInt (_errh,
							      (CONST dvoid *) &val,
							      sizeof(wrapped_type),
							      (Loki::TypeTraits<wrapped_type>::isSignedInt ?
							       OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
							      number
						    ));
			oci_check_error(__TROTL_HERE__, _errh, res);
		} catch (OciException const &e) {
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			exit(0);
		}
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const wrapped_type &val, Loki::Int2Type<true>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_type>::isFloat, isNotReal);

		try {
			sword res = OCICALL(OCINumberFromReal (_errh,
							       (CONST dvoid *) &val,
							       sizeof(wrapped_type),
							       number
						    ));			
			oci_check_error(__TROTL_HERE__, _errh, res);
		} catch (OciException const &e) {
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			exit(0);
		}
	}

	inline static void Cast(OciError &_errh, OCINumber const *number, wrapped_type &rsl)
	{
		Cast(_errh, number, rsl, Loki::Int2Type< Loki::TypeTraits<wrapped_type>::isFloat >() );
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const wrapped_type &val)
	{
		RevCast(_errh, number, val, Loki::Int2Type< Loki::TypeTraits<wrapped_type>::isFloat >() );
	}
};

}; // namespace Private

/*
 * Beware !! this datatype uses ugly hack
 * After long invetigation, I have not found any usage for datatype SQLT_NUM
 * OCINumberToInt and related work with SQLT_VNU
 * This datatype is registered as SQLT_NUM but defined as SQLT_VNU
 * I use SQLT_VNU for SQLT_NUM
 *
 * TODO: There should be some easy, transparent way for choosing default
 * number representation
 * choose one of: int, float, double, OracleNumber(SQLT_NUM, SQLT_VNU).
 * I am using typedef prefferedNumericType ...
 */
struct TROTL_EXPORT BindParNumber: public SqlStatement::BindPar
{
	BindParNumber(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : SqlStatement::BindPar(pos, stmt, ct)
	{
		valuep = (void**) calloc(_cnt, (size_t)OCI_NUMBER_SIZE );

		dty =  SQLT_VNU; //dty = SQLT_NUM;
		value_sz = OCI_NUMBER_SIZE;
		for(unsigned i = 0; i < _cnt; ++i)
		{
			((ub2*)rlenp)[i] = (ub2) value_sz;
		}
		type_name = ct.get_type_str();
	}

	BindParNumber(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): SqlStatement::BindPar(pos, stmt, decl)
	{
		valuep = (void**) calloc(_cnt, (size_t)OCI_NUMBER_SIZE );

		dty =  SQLT_VNU; //dty = SQLT_NUM;
		value_sz = OCI_NUMBER_SIZE;
		for(unsigned i = 0; i < _cnt; ++i)
		{
			((ub4*)rlenp)[i] = (ub4) value_sz;
		}

		type_name = "NUMBER";
	};

	~BindParNumber() {};

	template<class wrapped_int>
	wrapped_int get_number(unsigned row) const
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);

		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		wrapped_int retval = INT_NULL;

		if(!indp[row])
		{
			wrapped_int i;
			myNumericConvertor::Cast(_stmt._errh, (OCINumber*) &((char*)valuep)[row * value_sz ], i);
			return i;
		}

		return retval;
	}

	template<class wrapped_int>
	void set_number(unsigned int row, const wrapped_int &val)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);

		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		indp[row] = 0; //set non-NULL

		myNumericConvertor::RevCast(_stmt._errh, (OCINumber*) &((char*)valuep)[row * value_sz ], val);
	};

	const OCINumber* get_ref(unsigned int row) const
	{
		return (OCINumber*) ( &((char*)valuep)[row * value_sz ]);
	};
	
	virtual tstring get_string(unsigned int row) const;

	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;
protected:
	BindParNumber(const BindParNumber &other);
};


/// int with indicator variable
template<class wrapped_int>
struct SqlInt: public SqlValue
{
	SqlInt()	// NULL constructor
	{}

	SqlInt(const wrapped_int &value):_value(value)
	{
		_ind.set();
	}

//	SqlInt(wrapped_int value):_value(value)
//	{
//		_ind.set();
//	}

//
//	SqlInt(const SqlInt& other) : SqlValue(other), _value(other._value)
//	{}

	operator wrapped_int() const {return _ind.is_not_null() ? _value : INT_NULL;}

	tstring str() const
	{
//		char buffer[16];
		if (_ind.is_not_null())
		{
			tostream s;
			s << _value;
//			snprintf(buffer, sizeof(buffer), "%d", _value);
//			return tstring(buffer);
			return s.str();
		} else {
			return tstring();
		}
	};

	operator tstring() const { return str(); };

	void set(wrapped_int val) { _value = val; };

	//	int* get_ref() {return &_value;}

protected:
	wrapped_int _value;
};

// wrapper for internal Oracle NUMBER type [length][1-20digits][mantisa]
struct SqlNumber : public SqlValue
{
	//SqlNumber(const oraclenumber* pnum);
	/* 	SqlNumber(OCIError* errh, const std::string, */
/* 		  const char* fmt="99999999999999999999999999999999999999D99999999999999999999", */
/* 		  const char* nls_fmt="NLS_NUMERIC_CHARACTERS='.,'"); */
	
/* 	SqlNumber(OCIError* errh, double d) */
/* 	{ */
/* 		sword res = OCICALL(OCINumberFromReal(errh, &d, sizeof(d), (OCINumber*)&_val)); */
/* 		oci_check_error(__TROTL_HERE__, errh, res); */
/* 		_ind.set(); */
/* 	}; */

	SqlNumber(SqlNumber const &other) : _env(other._env)
	{
		*this = other;
	};

	SqlNumber(OciEnv &env, const char* s,
		  const char* fmt="99999999999999999999999999999999999999D99999999999999999999",
		  const char* nls_fmt="NLS_NUMERIC_CHARACTERS='.,'");
	
        SqlNumber(OciEnv &env) : _env(env)
	{
		_ind.clear();
	};

        SqlNumber(OciEnv &env, int i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

        SqlNumber(OciEnv &env, unsigned int i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

        SqlNumber(OciEnv &env, long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};
	
        SqlNumber(OciEnv &env, unsigned long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};
	
        SqlNumber(OciEnv &env, float i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

        SqlNumber(OciEnv &env, double i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	operator tstring() const
	{       
		if(is_not_null())
			return str();
		else
			return "";
	};
	
	operator int() const { return get_number<int>(); };
	operator unsigned int() const { return get_number<unsigned int>(); };
	operator long() const { return get_number<long>(); };
	operator unsigned long() const { return get_number<unsigned long>(); };
	operator float() const { return get_number<float>(); };
	operator double() const { return get_number<double>(); };

	operator OCINumber*() {return (OCINumber*)&_val;};
	operator const OCINumber*() const {return (OCINumber*)&_val;};

	SqlNumber& operator=(const SqlNumber &other)
	{
		if(other.is_not_null())
		{
			sword res = OCICALL(OCINumberAssign(_env._errh, other, &_val));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
		}
		_ind.set(other._ind);
		return *this;
	}
	
	bool operator== (const SqlNumber &other) const
	{
		if( is_null() || other.is_null() ) // Oracle NULLs are not equal
			return false;
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres==0;
	};

	bool operator!= (const SqlNumber &other) const
	{
		return !(*this == other);
	};
	
	bool operator< (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "Nulls are not compareable\n");
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres<0;
	};

	bool operator> (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "Nulls are not compareable\n");
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres>0;
	};
	
	SqlNumber& operator+= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		sword res = OCICALL(OCINumberAdd(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	SqlNumber operator+ (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n" );
		return SqlNumber(*this) += other;
	};
	
	SqlNumber& operator-= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		sword res = OCICALL(OCINumberSub(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	SqlNumber operator- (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		return SqlNumber(*this) -= other;
	};

	SqlNumber& operator*= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		sword res = OCICALL(OCINumberMul(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	SqlNumber operator* (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		return SqlNumber(*this) *= other;
	};

	SqlNumber& operator/= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n");
		sword res = OCICALL(OCINumberDiv(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	SqlNumber operator/ (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw OciException(__TROTL_HERE__, "One of the operands is NULL\n" );
		return SqlNumber(*this) /= other;
	};

//	tstring str(bool internal=false) const;

/* 	tstring str(OCIError* errh, */
/* 		    const char* fmt="TM9", //"FM99999999999999999999999999999999999999.99999999999999999999" */
/* 		    const char* nls_fmt="NLS_NUMERIC_CHARACTERS='.,'") const */
/* 	{ */
/* 		if (_ind.is_not_null()) */
/* 			return number_to_str(_val, errh, fmt, nls_fmt); */
/* 		else */
/* 			return tstring(); */
/* 	} */


	//oraclenumber* get_ref() {return &_val;}
	
protected:
	//oraclenumber _val;
	OCINumber _val;
	OciEnv &_env;
	
	template<class wrapped_int>
	wrapped_int get_number() const
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);
		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		wrapped_int retval = INT_NULL;

		if(is_not_null())
		{
			wrapped_int i;
			myNumericConvertor::Cast(_env._errh, &_val, i);
			return i;
		}

		return retval;
	}

	template<class wrapped_int>
	void set_number(const wrapped_int &val)
	{
		memset(&_val, 0x00, sizeof(_val));
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);
		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		myNumericConvertor::RevCast(_env._errh, &_val, val);
	}

	tstring str() const
	{
		text str_buf[64];
		ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);

		sword res = OCICALL(OCINumberToText(
					    _env._errh,
					    (OCINumber*) &_val,					    
					    (const oratext*) g_TROTL_DEFAULT_NUM_FTM,		//(const oratext*)fmt,
					    strlen(g_TROTL_DEFAULT_NUM_FTM),
					    0,							// CONST OraText *nls_params,
					    0,							// ub4 nls_p_length,
					    (ub4*)&str_len,
					    str_buf
					    ));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		return tstring((const char*)str_buf, str_len);
	};
	
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;	
};
 
//
//struct oraclenumber {
//	ub1	len;
//	ub1	exp;
//	ub1	mant[20];
//};


};


#endif
