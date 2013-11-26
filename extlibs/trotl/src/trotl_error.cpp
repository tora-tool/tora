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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_error.h"

#ifdef DEBUG
#include "stack.hpp"
#endif

#include <iterator>

namespace trotl
{

CriticalErrorList::CriticalErrorList()
{
	insert(28);    // ORA-00028: your session has been killed
	insert(600);   // ORA-00600: internal error code
	insert(1012);  // ORA-01012: not logged on
	insert(1014);  // ORA-01014: ORACLE shutdown in progress
	insert(1033);  // ORA-01033: ORACLE initialization or shutdown in progress
	insert(1034);  // ORA-01034: ORACLE not available
	insert(1035);  // ORA-01035: ORACLE only available to users with RESTRICTED SESSION privilege
	insert(1089);  // ORA-01089: immediate shutdown in progress - no operations are permitted
	insert(1090);  // ORA-01090: shutdown in progress - connection is not permitted
	insert(1092);  // ORA-01092: ORACLE instance terminated. Disconnection forced
	insert(1094);  // ORA-01094: ALTER DATABASE CLOSE in progress. Connections not permitted
	insert(2396);  // ORA-02396: exceeded maximum idle time, please connect again
	insert(3106);  // ORA-03106: fatal two-task communication protocol error
	//insert(3111);  // ORA-03111: break received on communication channel (TODO does it apply also for OCIBreak?)
	insert(3113);  // ORA-03113: end-of-file on communication channel
	insert(3114);  // ORA-03114: not connected to ORACLE - this one is returned for 2nd try on broken conn
	insert(3135);  // ORA-03135: connection lost contact
	insert(7445);  // ORA-07445: exception encountered
	insert(12151); // ORA-12151: TNS:received bad packet type from network layer
	insert(12154); // ORA-12154: TNS:could not resolve the connect identifier specified
	insert(12157); // ORA-12157: TNS:internal network communication error
	insert(12500); // ORA-12500: TNS:listener failed to start a dedicated server process
	insert(12504); // ORA-12504: TNS:listener was not given the SERVICE_NAME in CONNECT_DATA
	insert(12505); // ORA-12505: TNS listener could not resolve SID given in connect descriptor
	insert(12508); // ORA-12508: TNS:listener could not resolve the COMMAND given
	insert(12509); // ORA-12509: TNS:listener failed to redirect client to service handler
	insert(12510); // ORA-12510: TNS:database temporarily lacks resources to handle the request
	insert(12511); // ORA-12511: TNS:service handler found but it is not accepting connections
	insert(12513); // ORA-12513: TNS:service handler found but it has registered for a different protocol
	insert(12514); // ORA-12514: TNS:listener does not currently know of service requested in connect descriptor
	insert(12515); // ORA-12515: TNS:listener could not find a handler for this presentation
	insert(12516); // ORA-12516: TNS:listener could not find available handler with matching protocol stack
	insert(12518); // ORA-12518: TNS:listener could not hand off client connection
	insert(12519); // ORA-12519: TNS:no appropriate service handler found
	insert(12520); // ORA-12520: TNS:listener could not find available handler for requested type of server
	insert(12521); // ORA-12521: TNS:listener does not currently know of instance requested in connect descriptor
	insert(12522); // ORA-12522: TNS:listener could not find available instance with given INSTANCE_ROLE
	insert(12523); // ORA-12523: TNS:listener could not find instance appropriate for the client connection
	insert(12522); // ORA-12522: TNS:listener could not find available instance with given INSTANCE_ROLE
	insert(12523); // ORA-12523: TNS:listener could not find instance appropriate for the client connection
	insert(12524); // ORA-12524: TNS:listener could not resolve HANDLER_NAME given in connect descriptor
	insert(12525); // ORA-12525: TNS:listener has not received client's request in time allowed
	insert(12526); // ORA-12526: TNS:listener: all appropriate instances are in restricted mode
	insert(12527); // ORA-12527: TNS:listener: all instances are in restricted mode or blocking new connections
	insert(12528); // ORA-12528: TNS:listener: all appropriate instances are blocking new connections
	insert(12529); // ORA-12529: TNS:connect request rejected based on current filtering rules
	insert(12537); // ORA-12537: TNS:connection closed
	insert(12540); // ORA-12540: TNS:internal limit restriction exceeded
	insert(12541); // ORA-12541: TNS:no listener
	insert(12543); // ORA-12543: TNS:destination host unreachable
	insert(12571); // ORA-12571: TNS:packet writer failure
	insert(24315); // ORA-24315: illegal attribute type
};

OciException::OciException(tstring where, OCIEnv* envh) :
	_where(where), _mess(where), _line(0), _column(0)
{
	OraText buffer[1024];

	sb4 errorcode;
	sword res = OCICALL(OCIErrorGet(envh, 1, NULL, &errorcode, buffer, sizeof(buffer), OCI_HTYPE_ENV));

	//TODO loop over OCIErrorGet.
	switch(res)
	{
	case OCI_SUCCESS:
		_sql_error_code.push_back(errorcode);
		_mess.append((const char *)buffer);
		break;
	case OCI_INVALID_HANDLE:
		_sql_error_code.push_back(21001); // my bogus value
		_mess.append("ORACLE_HOME not found\n");
		break;
	case OCI_NO_DATA:
		_sql_error_code.push_back(21002); // my bogus value
		_mess.append("OCIErrorGet: OCI_NO_DATA\n");
		break;
	default:
		_sql_error_code.push_back(0);
		//no break here
	}

#ifdef DEBUG
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess.push_back('\n');
	_mess.append(_stack.str());
#endif

	_parse_offset = 0;
}

OciException::OciException(tstring where, OCIError* errh) :
	_where(where), _mess(where), _line(0), _column(0)
{
	char buffer[1024];
	sb4 errorcode;
	ub4 recordno = 1; // Indicates the status record from which the application seeks info. Starts from 1.

	sword res = OCICALL(OCIErrorGet(errh, recordno, NULL, &errorcode, (OraText*)buffer, sizeof(buffer), OCI_HTYPE_ERROR));
	switch(res)
	{
	case OCI_SUCCESS:
	{
		_sql_error_code.push_back(errorcode);
		_mess.push_back('\n');
		_mess.append(buffer);
		break;
	}
	case OCI_NO_DATA:
	{
		_sql_error_code.push_back(21002);
		_mess.push_back('\n');
		_mess.append("OCIErrorGet: OCI_NO_DATA\n");
		break;
	}
	default:
	{
		tostream s;
		_sql_error_code.push_back(0);
		_mess.push_back('\n');
		_mess.append("OCIErrorGet: unknown error: \n");
		s << res;
		_mess.append(s.str());
	}
	// no break here
	}

#ifdef DEBUG
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess.push_back('\n');
	_mess.append(_stack.str());
#endif

	_parse_offset = 0;
}

OciException::OciException(tstring where, const char* msg) :
	_where(where), _mess(where), _last_sql(""), _parse_offset(0), _line(0), _column(0)
{
	_mess.push_back('\n');
	_mess.append(msg);
#ifdef DEBUG
	dbg::stack s;
	std::copy(s.begin(), s.end(), std::ostream_iterator<dbg::stack_frame>(_stack, "\n"));
	_mess.push_back('\n');
	_mess.append(_stack.str());
#endif
}

OciException::OciException(const OciException& other)
	: _sql_error_code(other._sql_error_code)
	, _where() //_where(other._where)
	, _stack()
	, _mess(other._mess)
	, _last_sql(other._last_sql)
	, _parse_offset(other._parse_offset)
	, _line(other._line)
	, _column(other._column)
{
	//throw std::logic_error("OciException should not be copied");
	//std::cerr << __TROTL_HERE__ << "OciException should not be copied" << std::endl;
}

OciException& OciException::arg(int d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		tostream o;
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
		tostream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}

OciException& OciException::arg(unsigned long d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		tostream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}

OciException& OciException::arg(unsigned long long d)
{
	size_t pos = _mess.find("%d");

	if(pos != tstring::npos)
	{
		tostream o;
		o << d;
		_mess.replace(pos, 2, o.str());
	}
	return *this;
}

OciException& OciException::arg(tstring s)
{
	size_t pos = _mess.find("%s");

	if(pos != tstring::npos)
	{
		_mess.replace(pos, 2, s);
	}
	return *this;
}

CriticalErrorList OciException::criticalErrorList;

__NORETURN void TROTL_EXPORT throw_oci_exception(OciException const & e)
{
	throw(e);
}
//
//void TROTL_EXPORT throw_oci_exception(tstring where, OCIError* errh, sword res)
//{
//	switch(res) {
//	case OCI_ERROR:
//		throw_ocipl_exception(OciException(where, errh));
//	case OCI_INVALID_HANDLE:
//		throw_ocipl_exception(OciException(where, "invalid handle"));
//		//@todo handle other error states
//	}
//}
//
//void TROTL_EXPORT throw_oci_exception(tstring where, OCIEnv* envh, sword res)
//{
//	switch(res) {
//	case OCI_ERROR:
//		throw_ocipl_exception(OciException(where, envh));
//	case OCI_INVALID_HANDLE:
//		throw_ocipl_exception(OciException(where, envh));
//		//@todo handle other error states
//	}
//}

};
