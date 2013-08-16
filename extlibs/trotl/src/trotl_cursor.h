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

#ifndef TROTL_CURSOR_H_
#define TROTL_CURSOR_H_

#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_var.h"
#include "trotl_stat.h"

namespace trotl
{

class SqlCursor;

class TROTL_EXPORT BindParCursor: public BindPar
{
public:
	BindParCursor(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParCursor(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParCursor()
	{
		descFree();
	};

	virtual tstring get_string(unsigned int) const;

	friend class SqlCursor;
	class CursorStatement : public SqlStatement
	{
	public:
		CursorStatement(OciConnection& conn, OciHandle<OCIStmt> handle, ub4 lang=OCI_NTV_SYNTAX, int bulk_rows=g_OCIPL_BULK_ROWS)
			: SqlStatement(conn, handle, bulk_rows)
		{
			_stmt_type = STMT_SELECT;
			_state |= PREPARED | DESCRIBED | EXECUTED;
		};

		void define_all()
		{
			SqlStatement::define_all();
		};
		void fetch()
		{
			SqlStatement::fetch(_fetch_rows);
		};
	};
	void descAlloc(void);
	void descFree(void);
	void redefine(ub4);

	std::vector<CursorStatement*> _cursors;
protected:
	BindParCursor(const BindParCursor &other);
};

class TROTL_EXPORT SqlCursor: public SqlValue
{
public:
	SqlCursor(OciEnv &env);

	template<class wrapped_type>
	SqlCursor& operator>>(wrapped_type &val)
	{
		if( (c->_state & BindParCursor::CursorStatement::DEFINED) == 0 )
			c->define_all();
		if( (c->_state & BindParCursor::CursorStatement::FETCHED) == 0 )
			c->fetch();
		*c >> val;
		return *this;
	}

	bool eof()
	{
		return c->eof();
	}

	ub4 get_column_count()
	{
		return c->get_column_count();
	}
	//protected:
	OciHandle<OCIStmt> *handle;

	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	SqlCursor();
	virtual ~SqlCursor();
	BindParCursor::CursorStatement *c;
};

};

#endif
