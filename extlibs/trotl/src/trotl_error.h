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

#ifndef TROTL_ERROR_H_
#define TROTL_ERROR_H_

#include "trotl_export.h"
#include "trotl_common.h"

#include <exception>
#include <vector>
#include <set>

#define TROTL_BACKTRACE_DEPTH 10

namespace trotl
{
class SqlStatement;
struct OciException;

void oci_check_error(tstring where, OCIError* errh, sword res);
void oci_check_error(tstring where, OCIEnv* envh, sword res);
void oci_check_error(tstring where, SqlStatement &stmt, sword res); // defined in trotl_stat.cpp

/// error handling functions
extern __NORETURN void TROTL_EXPORT throw_oci_exception(OciException const & e);

#ifdef WIN32
class TROTL_EXPORT ::std::exception;
#endif

class TROTL_EXPORT CriticalErrorList : public std::set<unsigned>
{
	typedef std::set<unsigned> super;
public:
	CriticalErrorList();
};

struct TROTL_EXPORT OciException : public std::exception
{
	typedef std::exception super;

	OciException(tstring where, OCIEnv* envh);
	OciException(tstring where, OCIError* errh);
	OciException(tstring where, const char* msg);
	OciException(tstring where, SqlStatement& stmt); // defined in trotl_stat.cpp
	OciException(const OciException&);

	OciException& arg(int);
	OciException& arg(unsigned int);
	OciException& arg(unsigned long);
	OciException& arg(unsigned long long);

	OciException& arg(tstring);

	~OciException() throw() {}

	virtual const char* what() const throw()
	{
		return _mess.c_str();
	}

	inline int get_code(int pos = 0) const
	{
		return _sql_error_code.empty() ? 0 : _sql_error_code.at(pos);
	};
	inline const char* get_mesg() const
	{
		return _mess.c_str();
	};
	inline const char* get_sql() const
	{
		return _last_sql.c_str();
	};

	inline bool is_critical() const
	{
		return criticalErrorList.find(get_code()) != criticalErrorList.end();
	};

	inline int parse_offset() const
	{
		return _parse_offset;
	};

	inline int line() const
	{
		return _line;
	};

	inline int column() const
	{
		return _column;
	};
private:
	::std::vector<int> _sql_error_code;
	tstring _where;
	tostream _stack;
	tstring _mess;
	tstring _last_sql;
	ub2 _parse_offset, _line, _column;
	static CriticalErrorList criticalErrorList;
};

inline void oci_check_error(tstring where, OCIError* errh, sword res)
{
	switch(res)
	{
	case OCI_SUCCESS:
	case OCI_SUCCESS_WITH_INFO:
		break;
	case OCI_ERROR:
		throw_oci_exception(OciException(where, errh));
		break;
	case OCI_INVALID_HANDLE:
		throw_oci_exception(OciException(where, "Invalid handle"));
		break;
	default:
		throw_oci_exception(OciException(where, "Unsupported result code"));
		break;
	}
}

inline void oci_check_error(tstring where, OCIEnv* envh, sword res)
{
	switch(res)
	{
	case OCI_SUCCESS:
	case OCI_SUCCESS_WITH_INFO:
		break;
	case OCI_ERROR:
		throw_oci_exception(OciException(where, envh));
		break;
	case OCI_INVALID_HANDLE:
		throw_oci_exception(OciException(where, "Invalid handle"));
		break;
	default:
		throw_oci_exception(OciException(where, "Unsupported result code"));
		break;
	}
}

};
#endif
