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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_var.h"

#include "loki/static_check.h"
#include "loki/TypeTraits.h"

#include <string.h>

namespace Loki
{
	template<> struct IsCustomUnsignedInt<unsigned long long>
	{
		enum { value = 1 };
	};

	template<> struct IsCustomSignedInt<long long>
	{
		enum { value = 1 };
	};
}

namespace trotl
{

enum {INT_NULL = 0};

namespace Private
{
template<class user_type>
struct NumericConvertor;
};

/*
 * Beware !! this datatype uses ugly hack
 * After long invetigation, I have not found any usage for datatype SQLT_NUM
 * OCINumberToInt and related work with SQLT_VNU
 * This datatype is registered as SQLT_NUM but defined as SQLT_VNU
 * I use SQLT_VNU for SQLT_NUM
 */
struct TROTL_EXPORT BindParNumber: public BindPar
{
	BindParNumber(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParNumber(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParNumber() {};

	template<class wrapped_int>
	inline
	wrapped_int get_number(unsigned row) const
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);

		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		wrapped_int retval = INT_NULL;

		if(!indp[row])
		{
			wrapped_int i;

			myNumericConvertor::Cast(_errh, (const OCINumber*) &((char*)valuep)[row * value_sz ], i);

			return i;
		}

		return retval;
	}

	template<class wrapped_int>
	inline
	void set_number(unsigned int row, const wrapped_int &val)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);

		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		indp[row] = 0; //set non-NULL

		myNumericConvertor::RevCast(_errh, (OCINumber*) &((char*)valuep)[row * value_sz ], val);
	};

	inline
	const OCINumber* get_ref(unsigned int row) const
	{
		return (OCINumber*) ( &((char*)valuep)[row * value_sz ]);
	};

	virtual tstring get_string(unsigned int row) const;

	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;
protected:
	BindParNumber(const BindParNumber &other);
	mutable OciError _errh;
};

namespace Private
{

/*
 * Helper class for BindParNumber
 */
template<class user_type>
struct NumericConvertor
{
	inline static void Cast(OciError &_errh, OCINumber const *number, user_type &rsl, Loki::Int2Type<false>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<user_type>::isIntegral, isNotIntegral);

		try
		{

//#ifdef WIN32
			/* This is lauzy workaround
			   Windows version of OCINumberToInt segfaults on floats <0,1>
			   while Linux version returns the underflow error
			*/
			{
				sword res;
				bOOlean isint;
				res = OCICALL(OCINumberIsInt(_errh, number, &isint));
				oci_check_error(__TROTL_HERE__, _errh, res);

				if(!isint)
				{
					OCINumber truncated;
					res = OCICALL(OCINumberRound (_errh, number, 0 /*decplace*/, &truncated));
					oci_check_error(__TROTL_HERE__, _errh, res);
					res = OCICALL(OCINumberToInt(_errh, &truncated, sizeof(user_type),
					                             (Loki::TypeTraits<user_type>::isSignedInt ? OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
					                             &rsl
					                            ));
					oci_check_error(__TROTL_HERE__, _errh, res);
					return;
				}
			}
//#endif
			sword res = OCICALL(OCINumberToInt(_errh,
			                                   number, sizeof(user_type),
			                                   (Loki::TypeTraits<user_type>::isSignedInt ?
			                                    OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
			                                   &rsl
			                                  ));
			oci_check_error(__TROTL_HERE__, _errh, res);
		}
		catch (OciException const &e)
		{
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			rsl = 0;
		}
	}

	inline static void Cast(OciError &_errh, OCINumber const *number, user_type &rsl, Loki::Int2Type<true>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<user_type>::isFloat, isNotReal);

		try
		{
			sword res = OCICALL(OCINumberToReal(_errh,
			                                    number,
			                                    sizeof(user_type),
			                                    &rsl
			                                   ));
			oci_check_error(__TROTL_HERE__, _errh, res);
		}
		catch (OciException const &e)
		{
			std::cerr << __TROTL_HERE__ << e.what() << std::endl;
			rsl = 0;
		}
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const user_type &val, Loki::Int2Type<false>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<user_type>::isIntegral, isNotIntegral);

		// try {
		sword res = OCICALL(OCINumberFromInt (_errh,
		                                      (CONST dvoid *) &val,
		                                      sizeof(user_type),
		                                      (Loki::TypeTraits<user_type>::isSignedInt ?
		                                       OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED),
		                                      number
		                                     ));
		oci_check_error(__TROTL_HERE__, _errh, res);
		// } catch (OciException const &e) {
		// 	std::cerr << __TROTL_HERE__ << e.what() << std::endl;
		// 	exit(0);
		// }
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const user_type &val, Loki::Int2Type<true>)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<user_type>::isFloat, isNotReal);

		// try {
		sword res = OCICALL(OCINumberFromReal (_errh,
		                                       (CONST dvoid *) &val,
		                                       sizeof(user_type),
		                                       number
		                                      ));
		oci_check_error(__TROTL_HERE__, _errh, res);
		// } catch (OciException const &e) {
		// 	std::cerr << __TROTL_HERE__ << e.what() << std::endl;
		// 	exit(0);
		// }
	}

	inline static void Cast(OciError &_errh, OCINumber const *number, user_type &rsl)
	{
		Cast(_errh, number, rsl, Loki::Int2Type< Loki::TypeTraits<user_type>::isFloat >() );
	}

	inline static void RevCast(OciError &_errh, OCINumber *number, const user_type &val)
	{
		RevCast(_errh, number, val, Loki::Int2Type< Loki::TypeTraits<user_type>::isFloat >() );
	}
};

}; // namespace Private

// wrapper for internal Oracle NUMBER type [length][mantisa][1-20digits]
struct TROTL_EXPORT SqlNumber : public SqlValue
{
	union OracleNumber
	{
		struct OracleNumberFormat
		{
			ub1	len;
			ub1	exp;
			ub1	mant[20];
		} OracleNumberFormat;
		//OracleNumberFormat onf;
		OCINumber ocinumber;
	};

	inline SqlNumber(SqlNumber const &other) : _env(other._env)
	{
		*this = other;
	};

	inline SqlNumber(OciEnv &env, const char* str,
	                 const char* fmt="99999999999999999999999999999999999999D99999999999999999999",
	                 const char* nls_fmt="NLS_NUMERIC_CHARACTERS='.,'") : _env(env)
	{
		if (str && *str)
		{
			sword res = OCICALL(OCINumberFromText(_env._errh,
			                                      (const text*)str, (ub4)strlen(str),
			                                      (const text*)fmt, (ub4)strlen(fmt),
			                                      (const text*)nls_fmt, (ub4)strlen(nls_fmt),
			                                      (OCINumber*)&_val));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
			_ind.set();
		}
		else
			_ind.clear();
	};

	inline SqlNumber(OciEnv &env) : _env(env)
	{
		_ind.clear();
	};

	inline SqlNumber(OciEnv &env, int i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, unsigned int i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, long long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};


	inline SqlNumber(OciEnv &env, unsigned long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, unsigned long long i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, float i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline SqlNumber(OciEnv &env, double i) : _env(env)
	{
		set_number(i);
		_ind.set();
	};

	inline operator tstring() const
	{
		return toString();
	};

	inline tstring toString() const
	{
		if(is_not_null())
			return str();
		else
			return "";
	}

	inline operator int() const
	{
		return get_number<int>();
	};
	inline operator unsigned int() const
	{
		return get_number<unsigned int>();
	};
	inline operator long() const
	{
		return get_number<long>();
	};
	inline operator unsigned long() const
	{
		return get_number<unsigned long>();
	};
	inline operator float() const
	{
		return get_number<float>();
	};
	inline operator double() const
	{
		return get_number<double>();
	};

	//oraclenumber* get_ref() {return &_val;}
	inline operator OCINumber*()
	{
		return (OCINumber*)&_val;
	};
	inline operator const OCINumber*() const
	{
		return (OCINumber*)&_val;
	};

	inline SqlNumber& operator=(const SqlNumber &other)
	{
		if(other.is_not_null())
		{
			sword res = OCICALL(OCINumberAssign(_env._errh, other, &_val));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
		}
		_ind.set(other._ind);
		return *this;
	}

	inline SqlNumber& operator=(const OCINumber &other)
	{
		sword res = OCICALL(OCINumberAssign(_env._errh, &other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		_ind.set();
		return *this;
	}

	inline bool operator== (const SqlNumber &other) const
	{
		if( is_null() || other.is_null() ) // Oracle NULLs are not equal
			return false;
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres==0;
	};

	inline bool operator!= (const SqlNumber &other) const
	{
		return !(*this == other);
	};

	inline bool operator< (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "Nulls are not compareable\n"));
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres<0;
	};

	inline bool operator> (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "Nulls are not compareable\n"));
		sword cmpres;
		sword res = OCICALL(OCINumberCmp(_env._errh, &_val, other, &cmpres));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return cmpres>0;
	};

	inline SqlNumber& operator+= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		sword res = OCICALL(OCINumberAdd(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	inline SqlNumber operator+ (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		return SqlNumber(*this) += other;
	};

	inline SqlNumber& operator-= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		sword res = OCICALL(OCINumberSub(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	inline SqlNumber operator- (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		return SqlNumber(*this) -= other;
	};

	inline SqlNumber& operator*= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		sword res = OCICALL(OCINumberMul(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	inline SqlNumber operator* (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		return SqlNumber(*this) *= other;
	};

	inline SqlNumber& operator/= (const SqlNumber &other)
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n"));
		sword res = OCICALL(OCINumberDiv(_env._errh, &_val, other, &_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return *this;
	};

	inline SqlNumber operator/ (const SqlNumber &other) const
	{
		if( is_null() || other.is_null())
			throw_oci_exception(OciException(__TROTL_HERE__, "One of the operands is NULL\n" ));
		return SqlNumber(*this) /= other;
	};

	inline
	tstring dump() const
	{
		tostream ret;
		for(unsigned i=0; i<sizeof(OCINumber); i++)
		{
			char buffer[4];
			snprintf(buffer, sizeof(buffer), "%.2X", ((unsigned char*)&_val)[i]);
			ret << buffer;
		}
		return ret.str();
	};

protected:
	//oraclenumber _val;
	OCINumber _val;
	OciEnv &_env;

	template<class wrapped_int>
	inline
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
	inline
	void set_number(const wrapped_int &val)
	{
		memset(&_val, 0x00, sizeof(_val));
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);
		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		myNumericConvertor::RevCast(_env._errh, &_val, val);
	}

	inline
	tstring str() const
	{
		text str_buf[64];
		ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);

		sword res = OCICALL(OCINumberToText(
		                            _env._errh,
		                            (OCINumber*) &_val,
		                            (const oratext*) g_TROTL_DEFAULT_NUM_FTM,		//(const oratext*)fmt,
		                            (ub4)strlen(g_TROTL_DEFAULT_NUM_FTM),
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

/// int with indicator variable
template<class wrapped_int>
struct SqlInt: public SqlValue
{
	SqlInt()	// NULL constructor
	{}

	SqlInt(const wrapped_int &value) : _value(value)
	{
		_ind.set();
	}

	SqlInt(const SqlInt& other) : SqlValue(other), _value(other._value)
	{}

	inline operator wrapped_int() const
	{
		return _ind.is_not_null() ? _value : INT_NULL;
	}

	inline
	wrapped_int operator=(const SqlNumber &other)
	{
		LOKI_STATIC_CHECK(Loki::TypeTraits<wrapped_int>::isArith, isNotNumeric);
		typedef typename Private::NumericConvertor<wrapped_int> myNumericConvertor;

		if(other.is_not_null())
		{
			myNumericConvertor::Cast(other._env._errh, (const OCINumber*)other, _value);
		}
		else
		{
			_value = INT_NULL;
		}
		return *this;
	}

	inline tstring str() const
	{
		if (_ind.is_not_null())
		{
			tostream s;
			s << _value;
			return s.str();
		}
		else
		{
			return tstring();
		}
	};

	inline operator tstring() const
	{
		return str();
	};

	inline void set(wrapped_int val)
	{
		_value = val;
	};

	wrapped_int* get_ref()
	{
		return &_value;
	}

protected:
	wrapped_int _value;
};

};


#endif
