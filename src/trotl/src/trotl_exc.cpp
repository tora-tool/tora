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
#ifdef WIN32
#include "trotl_parser.h"
#include <stdio.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_common.h"
#include "trotl_stat.h"
#ifdef DEBUG
#include "stack/stack.hpp"
#endif

namespace trotl {

OciException::OciException(tstring where, OCIEnv* envh) :
	_where(where), _mess(where)
{
#ifdef DEBUG  
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess += "\n" + _stack.str();
#endif	  
	OraText buffer[1024];

	sb4 errorcode;
	sword res = OCICALL(OCIErrorGet(envh, 1, NULL, &errorcode, buffer, sizeof(buffer), OCI_HTYPE_ENV));

	//TODO loop over OCIErrorGet.
	switch(res)
	{
	case OCI_SUCCESS:
		_sql_error_code.push_back(errorcode);
		_mess += (const char *)buffer;
		break;
	case OCI_INVALID_HANDLE:
		_sql_error_code.push_back(21001); // my bogus value
		_mess += "ORACLE_HOME not found\n";
		break;
	case OCI_NO_DATA:
		_sql_error_code.push_back(21002); // my bogus value
		_mess += "OCIErrorGet: OCI_NO_DATA\n";
	default:
		_sql_error_code.push_back(0);
	}

	_parse_offset = 0;
}

OciException::OciException(tstring where, OCIError* errh) :
	_where(where), _mess(where)
{
#ifdef DEBUG  
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess += "\n" + _stack.str();
#endif
	char buffer[1024];

	sb4 errorcode;
	sword res = OCICALL(OCIErrorGet(errh, 1, NULL, &errorcode, (OraText*)buffer, sizeof(buffer), OCI_HTYPE_ERROR));

	switch(res)
	{
	case OCI_SUCCESS:
	{
		_sql_error_code.push_back(errorcode);
		_mess += buffer;
		break;
	}
	case OCI_NO_DATA:
	{
		_sql_error_code.push_back(21002);
		_mess += "OCIErrorGet: OCI_NO_DATA\n";
	}
	default:
	{
		_sql_error_code.push_back(0);
		_mess += "OCIErrorGet: unknown error: \n" + res;
	}
	}
	_parse_offset = 0;
}

//OciException::OciException(tstring where, OCIError* errh, SqlStatement& stmt)
	OciException::OciException(tstring where, SqlStatement& stmt) :
	_where(where), _mess(where)
{
#ifdef DEBUG  
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess += "\n" + _stack.str();
#endif
	ub4 size = sizeof(_parse_offset);
	sword res2 = OCICALL(OCIAttrGet(stmt, OCI_HTYPE_STMT, &_parse_offset, &size, OCI_ATTR_PARSE_ERROR_OFFSET, stmt._errh));
	if (res2 != OCI_SUCCESS)
		_parse_offset = 0;

//#ifdef _UNICODE
//#if 0
//	char wbuffer[4096];
//
//	sword res = OCICALL(OCIErrorGet(errh, 1, NULL, &errorcode, (OraText*)wbuffer, sizeof(wbuffer), OCI_HTYPE_ERROR));
//
//#ifdef _MSC_VER
//	std::_USE(std::locale::empty(), std::ctype<wchar_t>)
//#else
//	std::use_facet<std::ctype<wchar_t> >(std::locale::empty())
//#endif
//	.narrow((const char*)wbuffer, (const char*)wbuffer+_tcslen((const char*)wbuffer)+1, 0, buffer);
//#else
//TODO - in multithreaded environment SqlStatement should have its own error handle
	char buffer[4096];
	sb4 errorcode;
	ub4 recordno = 1;
	size_t last_length = 0;
	sword res;
	do {
		res = OCICALL(OCIErrorGet(stmt._errh, recordno, NULL, &errorcode, (OraText*)buffer+last_length, sizeof(buffer)-last_length, OCI_HTYPE_ERROR));
		if(res == OCI_SUCCESS)
			_sql_error_code.push_back(errorcode);
		else
			break;
		++recordno;
		last_length = strlen(buffer);
	} while(res == OCI_SUCCESS);
//#endif

	if ( _sql_error_code.empty()) { // No error was detecter, but the exception was thrown
		*buffer = '\0';
		_sql_error_code.push_back(0);
	}

	_last_sql = stmt.get_sql();

	if (!_last_sql.empty())
	{
		const char* sql = _last_sql.c_str();
		size_t l = strlen(buffer);

		int line = 1;
		const char* s = sql;
		const char* e = sql + _parse_offset;
		const char* lp = sql;

		while(s < e)
			if (*s == '\n') {
				++line;

				if (*++s == '\r') ++s;

				lp = s;
			} else
				++s;

		size_t column = 1 + (s-lp);

//#ifdef __STDC_WANT_SECURE_LIB__
//		char* b = buffer+l + snprintf_s(buffer+l, sizeof(buffer)-l, _TRUNCATE, "\nerror at SQL position %d [%d.%d]:\n", _parse_offset, line, column);
//#else
		char* b = buffer+l + snprintf(buffer+l, sizeof(buffer)-l-1, "\nerror at SQL position %d [%d.%d]:\n", _parse_offset, line, column);
//#endif
		char* p = b;
		s = lp;
		while(*s && *s!='\n' && p<buffer+sizeof(buffer)-3)
			*p++ = *s++;

		*p++ = '\n';

		b = p;
		const char* err_ptr = sql + _parse_offset;
		s = lp;
		for(; s<err_ptr && p<buffer+sizeof(buffer)-2; ++s) {
			if (*s=='\n' || *sql=='\r')
				p = b;
			else if (*s == '\t')
				*p++ = '\t';
			else
				*p++ = ' ';
		}

		*p++ = '^';
		*p = '\0';

		l = strlen(buffer);
//#ifdef __STDC_WANT_SECURE_LIB__
//		snprintf_s(buffer+l, sizeof(buffer)-l, _TRUNCATE, "\n\nlast SQL statement:\n%s\n", sql);
//#else
		snprintf(buffer+l, sizeof(buffer)-l, "\n\nlast SQL statement:\n%s\n", sql);
//#endif
	}
	_mess += tstring(buffer);
}

OciException::OciException(tstring where, const char* msg) :
	_where(where), _mess(where), _parse_offset(0), _last_sql("")
{
#ifdef DEBUG  
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess += "\n" + _stack.str() + msg;
#endif	
}

OciException::OciException(const OciException& other):
	_sql_error_code(other._sql_error_code),
	_where(other._where),
	_mess(other._mess),
	_stack(),
	_last_sql(other._last_sql),
	_parse_offset(other._parse_offset)
	{
		//throw std::logic_error("OciException should not be copied");
		//std::cerr << __TROTL_HERE__ << "OciException should not be copied" << std::endl;
	}

OciException& OciException::arg(int d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		std::ostringstream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}

OciException& OciException::arg(unsigned int d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		std::ostringstream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}

#ifdef __GNUC__
OciException& OciException::arg(unsigned long d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		std::ostringstream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}
#endif

OciException& OciException::arg(tstring s)
{
	size_t pos = _mess.find("%s");

	if(pos != tstring::npos)
	{
		_mess.replace(pos, 2, s);
	}
	return *this;
}

void TROTL_EXPORT throw_ocipl_exception(OciException const & e)
{
	throw(e);
}

void TROTL_EXPORT throw_oci_exception(tstring where, OCIError* errh, sword res)
{
	switch(res) {
	case OCI_ERROR:
		throw_ocipl_exception(OciException(where, errh));
	case OCI_INVALID_HANDLE:
		throw_ocipl_exception(OciException(where, "invalid handle"));
		//@todo handle other error states
	}
}

void TROTL_EXPORT throw_oci_exception(tstring where, OCIEnv* envh, sword res)
{
	switch(res) {
	case OCI_ERROR:
		throw_ocipl_exception(OciException(where, envh));
	case OCI_INVALID_HANDLE:
		throw_ocipl_exception(OciException(where, envh));
		//@todo handle other error states
	}
}

};
