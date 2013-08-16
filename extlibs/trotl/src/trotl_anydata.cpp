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
#include "trotl_anydata.h"
#include "trotl_stat.h"
#include "trotl_describe.h"

namespace trotl
{

// TODO
//Util::RegisterInFactory<BindParANYDATA, BindParFactTwoParmSing> regBindXML("SYS.ANYDATA");

Util::RegisterInFactory<BindParANYDATA, CustDefineParFactTwoParmSing> regCustDefineNTY_ANYDATA("SYS.ANYDATA");

BindParANYDATA::BindParANYDATA(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
	, _oan_buffer(NULL)
	, _any_indp(NULL)
{
	dty =  SQLT_NTY;
	_type_name = ct->typeName();
	value_sz = sizeof(OCIAnyData*);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub2*)rlenp)[i] = (ub2) value_sz;
	}

	init();
}

BindParANYDATA::BindParANYDATA(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindPar(pos, stmt, decl)
	, _oan_buffer(NULL)
	, _any_indp(NULL)
{
	dty =  SQLT_NTY;
	_type_name = "SYS.ANYDATA";
	value_sz = sizeof(OCIAnyData*);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub4*)rlenp)[i] = (ub4) value_sz;
	}

	init();
}

void BindParANYDATA::init()
{
	sword res;
	_anydatatdo = 0;
	_oan_buffer = (OCIAnyData**) calloc(_cnt, sizeof(OCIAnyData*));

	res = OCICALL(OCITypeByName(_stmt._env, _stmt._errh, _stmt._conn._svc_ctx,
	                            (const oratext*)"SYS", (ub4)strlen("SYS"),
	                            (const oratext*)"ANYDATA", (ub4)strlen("ANYDATA"),
	                            0, 0,
	                            OCI_DURATION_SESSION, OCI_TYPEGET_ALL,
	                            (OCIType**) &_anydatatdo));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	if(_anydatatdo == NULL)
		throw_oci_exception(OciException(__TROTL_HERE__, "Unknown datatype in the database: SYS.ANYDATA"));

	// for(int i=0; i<g_OCIPL_BULK_ROWS; i++)
	// {
	// 	_oan_buffer[i] = NULL;
	// }
}

void BindParANYDATA::define_hook()
{
	sword res = OCICALL(OCIDefineObject(defnpp, _stmt._errh,
	                                    _anydatatdo,
	                                    (dvoid **) &(_oan_buffer[0]), //(dvoid **) &_oan_buffer,
	                                    (ub4 *) 0,
	                                    0, //(dvoid **) &_any_indp,
	                                    (ub4 *) 0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
// 	// TODO OCIDefineArrayOfStruct here ??
}

void BindParANYDATA::bind_hook()
{
	//TODO
	throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
}

// TODO
tstring BindParANYDATA::get_string(unsigned int row) const
{
	OCITypeCode tc;
	OCIType *type = (OCIType *)NULL;
	bOOlean isNull;
	OCIInd ind;

	sword res1 = OCICALL(OCIAnyDataIsNull(_stmt._conn._svc_ctx, _stmt._errh, _oan_buffer[row], &isNull));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res1);
	if(isNull) return "";

	sword res = OCICALL(OCIAnyDataGetType(_stmt._conn._svc_ctx,
	                                      _stmt._errh, _oan_buffer[row],
	                                      (OCITypeCode *)&tc, (OCIType **)&type));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	switch (tc)
	{
	case OCI_TYPECODE_NUMBER:
	{
		OCINumber num;
		OCINumber *num_ptr = &num;
		ub4 len;
		text str_buf[64];
		ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);

		sword res1 = OCICALL(OCIAnyDataAccess(_stmt._conn._svc_ctx, _stmt._errh,
		                                      _oan_buffer[row],
		                                      (OCITypeCode)OCI_TYPECODE_NUMBER,
		                                      (OCIType *)0,
		                                      (dvoid *)&ind,
		                                      (dvoid **)&num_ptr, &len));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res1);

		sword res2 = OCICALL(OCINumberToText(_stmt._errh,
		                                     (OCINumber*) &num,
		                                     (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
		                                     (ub4)strlen(g_TROTL_DEFAULT_NUM_FTM),
		                                     0, // CONST OraText *nls_params,
		                                     0, // ub4 nls_p_length,
		                                     (ub4*)&str_len,
		                                     str_buf
		                                    ));
		oci_check_error(__TROTL_HERE__, _env._errh, res2);

		return tstring((const char*)str_buf, str_len);
	}
	case OCI_TYPECODE_VARCHAR2:
	{
		OCIInd _indp; // TODO check indp here
		OCIString *str = (OCIString *) 0;
		ub4 len;

		sword res1 = OCICALL(OCIAnyDataAccess(_stmt._conn._svc_ctx, _stmt._errh,
		                                      _oan_buffer[row], (OCITypeCode)OCI_TYPECODE_VARCHAR2,
		                                      (OCIType *)0, (dvoid *)&_indp, (dvoid *)&str, &len));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res1);

		return tstring( (const char *)OCIStringPtr(_stmt._env, str), OCIStringSize(_stmt._env, str));
	}
	case OCI_TYPECODE_DATE:
	{
// 		/*checkerr(ctxptr->errhp, OCIAnyDataAccess(ctxptr->svchp,
// 		  ctxptr->errhp, oan_buffer,
// 		  (OCITypeCode)OCI_TYPECODE_DATE,
// 		  (OCIType *)0, (dvoid *)&indp, (dvoid *)&date, &len));
// 		  OCIDateGetDate( (CONST OCIDate *) &date, &year1, &month1, &day1 ); */
		OCIInd _indp; // TODO check indp here
		OCIDate *date_ptr = 0;
		text str_buf[200];
		ub4 str_len = sizeof(str_buf) / sizeof( *str_buf);
		ub4 len;

		sword res1= OCICALL(OCIAnyDataAccess(_stmt._conn._svc_ctx, _stmt._errh,
		                                     _oan_buffer[row], (OCITypeCode)OCI_TYPECODE_DATE,
		                                     (OCIType *)0, (dvoid *)&_indp,
		                                     (dvoid **)&date_ptr, &len));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res1);
// 		OCIDateGetDate( (CONST OCIDate *) date_ptr, &year1, &month1, &day1 );


		//const char fmt[] = "YYYY:MM:DD HH24:MI:SS";
		const char lang_fmt[] = "American";

		res1 = OCICALL(OCIDateToText(_stmt._errh,
		                             date_ptr,
		                             (CONST text*) g_TROTL_DEFAULT_DATE_FTM,
		                             (ub4)strlen(g_TROTL_DEFAULT_DATE_FTM),
		                             (CONST text*) lang_fmt,
		                             (ub4) sizeof(lang_fmt)-1,
		                             (ub4 *)&str_len,
		                             str_buf
		                            ));
		oci_check_error(__TROTL_HERE__, _stmt._errh, res1);
		str_buf[ min( (str_len+1) , (unsigned)sizeof(str_buf) ) ] = '\0';

		return (const char*)str_buf;
// 		printf("c2 is %d/%d/%d\n", day1, month1, year1);
	}
// 	case OCI_TYPECODE_OBJECT:
// 		checkerr(ctxptr->errhp, OCIAnyDataAccess(ctxptr->svchp, ctxptr->errhp,
// 							 oan_buffer, (OCITypeCode) OCI_TYPECODE_OBJECT,
// 							 (OCIType *)addr_tdo, (dvoid *) &ind2p,
// 							 (dvoid **)&addr_obj, &len));
// 		printf("state is %s \n",
// 		       OCIStringPtr(ctxptr->envhp, addr_obj->state));
// 		printf("zip is %s \n",
// 		       OCIStringPtr(ctxptr->envhp, addr_obj->zip));
// 		break;

// 	case OCI_TYPECODE_CHAR:
// 		checkerr(ctxptr->errhp, OCIAnyDataAccess(ctxptr->svchp,
// 							 ctxptr->errhp, oan_buffer,
// 							 (OCITypeCode)OCI_TYPECODE_CHAR,
// 							 (OCIType *)0, (dvoid *)&indp, (dvoid *)&str, &len));

// 		printf("c2 is %s \n", OCIStringPtr(ctxptr->envhp, str));
// 		break;

// 	case OCI_TYPECODE_RAW:
// 		checkerr(ctxptr->errhp, OCIAnyDataAccess(ctxptr->svchp, ctxptr->errhp,
// 							 oan_buffer, (OCITypeCode)OCI_TYPECODE_RAW,
// 							 (OCIType *)0, (dvoid *)&indp, (dvoid *)&raw_col, &len));

// 		raw_ptr = OCIRawPtr(ctxptr->envhp, raw_col);
// 		printf("RAW data is: ");
// 		for(i=0;i<BUFLEN;i++)
// 			printf("%c", (int) *(raw_ptr+i) & 0xFF);
// 		printf("\n");
// 		break;

// 	case OCI_TYPECODE_NAMEDCOLLECTION:
// 		checkerr(ctxptr->errhp,  OCITypeByName(ctxptr->envhp,
// 						       ctxptr->errhp, ctxptr->svchp,
// 						       (CONST text *)"", (ub4) strlen(""),
// 						       (const text *) "ADDR_TAB",
// 						       (ub4) strlen((const char *) "ADDR_TAB"),
// 						       (CONST text *) 0, (ub4) 0, OCI_DURATION_SESSION,
// 						       OCI_TYPEGET_ALL, &addr_tab_tdo));

// 		checkerr(ctxptr->errhp, OCIAnyDataAccess(ctxptr->svchp,
// 							 ctxptr->errhp, oan_buffer,
// 							 (OCITypeCode)OCI_TYPECODE_NAMEDCOLLECTION,
// 							 (OCIType *) addr_tab_tdo, (dvoid *) &ind2p,
// 							 (dvoid *)&addr_tab, &len));

// 		/* check how many elements in the typed table */
// 		checkerr(ctxptr->errhp, OCICollSize(ctxptr->envhp, ctxptr->errhp,
// 						    (CONST OCIColl *) addr_tab, &collsiz));
// 		printf("c2 is a typed table with %d elements:\n", collsiz);
// 		if (collsiz == 0)
// 			break;

// 		/*Dump the table from the top to the bottom. */
// 		/* go to the first element and print out the index */
// 		checkerr(ctxptr->errhp, OCITableFirst(ctxptr->envhp,
// 						      ctxptr->errhp, addr_tab, &index));
// 		checkerr(ctxptr->errhp, OCICollGetElem(ctxptr->envhp,
// 						       ctxptr->errhp,
// 						       (CONST OCIColl *) addr_tab, index,
// 						       &exist, &elem, &elemind));
// 		addr = (address *)elem;

// 		printf("\tAddress 1 is: %s",
// 		       OCIStringPtr(ctxptr->envhp,addr->state));
// 		printf("\t%s\n", OCIStringPtr(ctxptr->envhp, addr->zip));

// 		for(;!OCITableNext(ctxptr->envhp, ctxptr->errhp, index,
// 				   addr_tab, &index, &exist) && exist;)
// 		{
// 			checkerr(ctxptr->errhp, OCICollGetElem(ctxptr->envhp,
// 							       ctxptr->errhp,
// 							       (CONST OCIColl *) addr_tab,index,
// 							       &exist, &elem, &elemind));
// 			addr = (address *)elem;
// 			printf("\tAddress %d is: %s", index+1,
// 			       OCIStringPtr(ctxptr->envhp,addr->state));
// 			printf("\t%s\n", OCIStringPtr(ctxptr->envhp, addr->zip));
// 		}
// 		break;

	default:
		printf("TYPED DATA CAN'T BE DISPLAYED IN THIS PROGRAM(%d)\n", tc);
		break;
	}

	return "";
};

tstring SqlANYDATA::str() const
{
	return "";
};


};
