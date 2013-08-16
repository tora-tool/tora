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


#include "trotl_xml.h"
#include <string.h>

namespace trotl
{

Util::RegisterInFactory<BindParXML, BindParFactTwoParmSing> regBindXML("SYS.XMLTYPE");

Util::RegisterInFactory<BindParXML, CustDefineParFactTwoParmSing> regCustDefineNTY_XML("SYS.XMLTYPE");

BindParXML::BindParXML(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	free(valuep);
	valuep = NULL;
	dty =  SQLT_NTY;
	value_sz = 0;
	_type_name = ct->typeName();
	init(stmt);
}

BindParXML::BindParXML(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	free(valuep);
	valuep = NULL;
	dty =  SQLT_NTY;
	value_sz = 0;
	_type_name = "SYS.XMLTYPE";
	init(stmt);
};

void BindParXML::init(SqlStatement &stmt)
{
	sword res;
	xmltdo = 0;

	_xmlvaluep = (OCIXMLType**) calloc(_cnt, sizeof(OCIXMLType*));
	_xmlindp = (OCIInd*) calloc(_cnt, sizeof(OCIInd));

//	xmlsize = new ub4 [_cnt];

//	std::cout << "BindParXML::init\n";

	res = OCICALL(OCITypeByName(_stmt._env, _stmt._errh, _stmt._conn._svc_ctx,
	                            (const oratext*)"SYS", strlen("SYS"),
	                            (const oratext*)"XMLTYPE", strlen("XMLTYPE"),
	                            0, 0,
	                            OCI_DURATION_SESSION, OCI_TYPEGET_HEADER,
	                            (OCIType**) &xmltdo
	                           ));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	if(xmltdo == NULL)
		throw OciException(__TROTL_HERE__, "Unknown datatype in the database: SYS.XMLTYPE");

	for(int i=0; i<g_OCIPL_BULK_ROWS; i++)
	{
		_xmlvaluep[i] = NULL;
		_xmlindp[i] = OCI_IND_NULL;
	}
//	for(int i=0; i<_cnt; i++)
//	{
////		xmlsize[i] = 0;
//		res = OCICALL(OCIObjectNew(conn._env,
//				conn._env._errh,
//				conn._svc_ctx,
//				OCI_TYPECODE_OPAQUE,
//				xmltdo,
//				0,
//				OCI_DURATION_SESSION,
//				FALSE,
//				(dvoid **) &xml[i]
//		));
//		oci_check_error(__TROTL_HERE__, conn._env._errh, res);
//	}
}

void BindParXML::define_hook()
{
//	xmlsize = 0;

//	std::cout << "OCIDefineObject" << std::endl;

	sword res = OCICALL(OCIDefineObject(defnpp, _stmt._errh,
	                                    xmltdo,
	                                    // TODO If the application wants the buffer to be implicitly allocated in the cache,
	                                    // *pgvpp should be passed in as null.
	                                    (dvoid **) &(_xmlvaluep[0]),
	                                    0,
	                                    0, //(dvoid **) &xml_ind,
	                                    0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

//	res = OCICALL(OCIDefineArrayOfStruct(defnpp, stmt._errh, sizeof(OCIXMLType*), 0, 0, 0));
//	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

void BindParXML::bind_hook()
{
//	xmlsize = 0;

//	std::cout << "OCIBindObject" << std::endl;

	sword res = OCICALL(OCIBindObject(bindp, _stmt._errh,
	                                  xmltdo,
	                                  // TODO If the application wants the buffer to be implicitly allocated in the cache,
	                                  // *pgvpp should be passed in as null.
	                                  (dvoid **) &(_xmlvaluep[0]),
	                                  0,
	                                  0, //(dvoid **) &xml_ind,
	                                  0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
//  TODO
//	if( _cnt > 1 ) {
//		res = OCICALL(OCIDefineArrayOfStruct(bindp, conn._env._errh, sizeof(OCIXMLType*), 0, 0, 0));
//		oci_check_error(__TROTL_HERE__, conn._env._errh, res);
//	}
}

// TODO - use NULL indicator for SYS.XMLTYPE
tstring BindParXML::get_string(unsigned int row) const
{
	if((xmlnode*)_xmlvaluep[row] == NULL)
		return "";

	OCIDuration dur;
	xmlerr xerr = (xmlerr)0;
	oraerr oerr = (oraerr)0;
	struct xmlctx *xctx = (xmlctx*) 0;
//	xmldocnode *doc = (xmldocnode*) 0;
	ocixmldbparam params[1];

	/* Get an XML context */
	params[0].name_ocixmldbparam = XCTXINIT_OCIDUR;
	params[0].value_ocixmldbparam = &dur;

	// TODO add some error checking here
	// TODO this xml context should be "more global"
	xctx = OCIXmlDbInitXmlCtx(_env, _stmt._conn._svc_ctx, _env._errh, params, 1);

	orastream *os = OraStreamInit((void*)0xDEAD, (void*)this, &oerr, "write", stream_write_callback, "open", stream_open_callback, NULL);

	oerr = OraStreamOpen(os, NULL);
	//boolean op;
	//op = OraStreamReadable(os);
	//printf("Stream os readable %d\n", op);
	//op = OraStreamWriteable(os);
	//printf("Stream os writeable %d\n", op);

	XmlSaveDom(xctx, &xerr, (xmlnode*)_xmlvaluep[row], "stream", os, NULL);

	OCIXmlDbFreeXmlCtx(xctx);

	return _stringrepres.str();
};


ORASTREAM_OPEN_F(BindParXML::stream_open_callback, sctx, sid, hdl, length)
{
//	std::cout << "BindParXML::stream_open_callback" << std::endl;
	return 0;
}

ORASTREAM_CLOSE_F(BindParXML::stream_close_callback, sctx, sid, hdl)
{
//	std::cout << "BindParXML::stream_close_callback" << std::endl;
	return 0;
}

ORASTREAM_WRITE_F(BindParXML::stream_write_callback, sctx, sid, hdl, src, size, written)
{
	BindParXML *bp = (BindParXML*) sid;
	bp->_stringrepres.write((const char*)src,size);
	*written = size;
	return 0;
}

tstring SqlXML::str() const
{
	OCIDuration dur;
	xmlerr xerr = (xmlerr)0;
	oraerr oerr = (oraerr)0;
	struct xmlctx *xctx = (xmlctx*) 0;
//	xmldocnode *doc = (xmldocnode*) 0;
	ocixmldbparam params[1];

	/* Get an XML context */
	params[0].name_ocixmldbparam = XCTXINIT_OCIDUR;
	params[0].value_ocixmldbparam = &dur;

	// TODO add some error checking here
	// TODO this xml context should be "more global"
	xctx = OCIXmlDbInitXmlCtx(_conn._env, _conn._svc_ctx, _conn._env._errh, params, 1);

	orastream *os = OraStreamInit((void*)0xDEAD, (void*)this, &oerr, "write", stream_write_callback, "open", stream_open_callback, NULL);

	boolean op = OraStreamReadable(os);
	oerr = OraStreamOpen(os, NULL);
	printf("Stream os readable %d\n", op);
	op = OraStreamWriteable(os);
	printf("Stream os writeable %d\n", op);

	XmlSaveDom(xctx, &xerr, (xmlnode*)xml, "stream", os, NULL);

	OCIXmlDbFreeXmlCtx(xctx);

	return _stringrepres.str();
};

ORASTREAM_OPEN_F(SqlXML::stream_open_callback, sctx, sid, hdl, length)
{
//	std::cout << "SqlXML::stream_open_callback" << std::endl;
	return 0;
}

ORASTREAM_CLOSE_F(SqlXML::stream_close_callback, sctx, sid, hdl)
{
//	std::cout << "SqlXML::stream_close_callback" << std::endl;
	return 0;
}

ORASTREAM_WRITE_F(SqlXML::stream_write_callback, sctx, sid, hdl, src, size, written)
{
//	std::cout << "SqlXML::stream_write_callback" << std::endl;
	SqlXML *bp = (SqlXML*) sid;
	bp->_stringrepres.write((const char*)src,size);
	*written = size;
	return 0;
}


};
