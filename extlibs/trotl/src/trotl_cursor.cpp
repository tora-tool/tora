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
#include "trotl_cursor.h"
#include "trotl_describe.h"

namespace trotl
{

// // Register Bind datatypes in factory(Bind - PL/SQL)
Util::RegisterInFactory<BindParCursor, BindParFactTwoParmSing> regBindCursor("cursor");

// // Register Bind datatypes in factory(Define - SELECT)
Util::RegisterInFactory<BindParCursor, DefineParFactTwoParmSing, int> regDefineCursor(SQLT_RSET);

BindParCursor::BindParCursor(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	valuep = (void**) calloc(_cnt, sizeof(OCIStmt*));
	value_sz = sizeof(OCIStmt*);
	dty = SQLT_RSET;
	_type_name = ct->typeName();
	descAlloc();
}

BindParCursor::BindParCursor(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	valuep = (void**) calloc(_cnt, sizeof(OCIStmt*));
	value_sz = sizeof(OCIStmt*);
	dty = SQLT_RSET;
	_type_name = "cursor";
	descAlloc();
};

void BindParCursor::descAlloc(void)
{
	// Note: If you have retrieved multiple ref cursors, you must take care when fetching them into stm2p.
	// If you fetch the first one, you can then perform fetches on it to retrieve its data.
	// However, once you fetch the second ref cursor into stm2p, you no longer have access to the data from the first ref cursor.
	for(unsigned i=0; i<_cnt; ++i)
	{
		OciHandle<OCIStmt> handle(_env);
		CursorStatement *s = new CursorStatement(_stmt._conn, handle);
		((OCIStmt**)valuep)[i] = s->operator OCIStmt*();
		_cursors.push_back(s);
	}
}

void BindParCursor::descFree(void)
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		sword res = OCICALL(OCIHandleFree(((void**)valuep)[i], OCI_HTYPE_STMT));
		oci_check_error(__TROTL_HERE__, _env, res);
	}
}

void BindParCursor::redefine(ub4 i)
{
	OciHandle<OCIStmt> handle(_env);
	CursorStatement *s = new CursorStatement(_stmt._conn, handle);
	((OCIStmt**)valuep)[i] = s->operator OCIStmt*();
	_cursors.at(i) = s;
}

tstring BindParCursor::get_string(unsigned int i) const
{
	tostream retval;
	if(indp[i] == OCI_IND_NULL)
		return "";

	CursorStatement *c = _cursors.at(i);

	if( (c->_state & BindParCursor::CursorStatement::DEFINED) == 0 )
		c->define_all();
	if( (c->_state & BindParCursor::CursorStatement::FETCHED) == 0 )
		c->fetch();

	unsigned column = 0;
	while(!c->eof())
	{
		tstring s;
		*c >> s;
		retval << s << '\t';
		column++;
		if(column==c->get_column_count())
		{
			retval << std::endl;
			column=0;
		}
	}
	const_cast<BindParCursor*>(this)->redefine(i);
	return retval.str();
}


SqlCursor::SqlCursor()
	: handle(NULL)
    , c(NULL)
{
}

SqlCursor::~SqlCursor()
{
	if(c) delete c;
}

};
