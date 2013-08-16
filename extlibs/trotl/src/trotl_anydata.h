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

#ifndef TROTL_ANYDATA_H_
#define TROTL_ANYDATA_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_conn.h"
#include "trotl_var.h"

namespace trotl
{

struct TROTL_EXPORT BindParANYDATA: public BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	// TODO remember OCIConn or at least svcctx in this class
	BindParANYDATA(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParANYDATA(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParANYDATA()
	{
		// TODO free _any_indp, _oan_buffer
	}

	virtual tstring get_string(unsigned int row) const;

	void init();

	virtual void define_hook();
	virtual void bind_hook();


//private:
	tostream _stringrepres;

	OCIType *_anydatatdo;
	OCIAnyData **_oan_buffer;
	OCIInd *_any_indp;
};

struct TROTL_EXPORT SqlANYDATA : public SqlValue
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	SqlANYDATA(OciConnection &conn): _conn(conn) // NULL constructor
	{}


	SqlANYDATA(const SqlValue &value, OciConnection &conn): _conn(conn)
	{
		//_ind.set();
		throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
	}

	tstring str() const;

	operator tstring() const
	{
		return str();
	};

//protected:

	OCIType *anydatatdo;
	OciConnection&	_conn;
};


};
#endif /*TROTL_ANYDATA_H_*/
