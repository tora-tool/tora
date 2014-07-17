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

#ifndef TROTL_XML_H_
#define TROTL_XML_H_

// include rpcndr.h/wtypes.h before oratypes.h(from xml.h)
// #ifdef WIN32
// #include <stdio.h>
// #include <wtypes.h>
// #define snprintf _snprintf
// //#define gmtime gmtime_s
// //#define localtime localtime_s
// #endif
#include "trotl_common.h"

extern "C" {
#include "xml.h"
}

#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_describe.h"
#include "trotl_stat.h"

namespace trotl
{

struct TROTL_EXPORT BindParXML: public BindPar
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	// TODO remember OCIConn or at least svcctx in this class
	BindParXML(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParXML(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParXML()
	{
		if(_xmlvaluep)
		{
			free(_xmlvaluep);
			_xmlvaluep = NULL;
		}
		if(_xmlindp)
		{
			free(_xmlindp);
			_xmlindp = NULL;
		}
	}

	virtual tstring get_string(unsigned int row) const;

	void init(SqlStatement &stmt);

	virtual void define_hook();
	virtual void bind_hook();


//private:
	static ORASTREAM_OPEN_F(stream_open_callback, sctx, sid, hdl, length);
	static ORASTREAM_CLOSE_F(stream_close_callback, sctx, sid, hdl);
	static ORASTREAM_WRITE_F(stream_write_callback, sctx, sid, hdl, src, size, written);

	tostream _stringrepres;

	OCIType *xmltdo;
	OCIXMLType **_xmlvaluep;
//	ub4 *xmlsize;
	OCIInd *_xmlindp;

};

struct TROTL_EXPORT SqlXML : public SqlValue
{
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	// Classes used for constructor parameter
	struct Filename
	{
		Filename(tstring filename) : _filename(filename) {};
		tstring _filename;
	};

	struct URI
	{
		URI(tstring uri) : _uri(uri) {};
		tstring _uri;
	};

	SqlXML(OciConnection &conn): _conn(conn) // NULL constructor
	{}

	SqlXML(OciConnection &conn, Filename f): _conn(conn)
	{
		// TODO try to use different xml context (or context singleton?)
		xmlerr xerr = (xmlerr)0;
		OCIDuration dur = OCI_DURATION_SESSION;
		struct xmlctx *xctx = (xmlctx *)0;
//		xmldocnode *doc = (xmldocnode *)0;
		ocixmldbparam params[1];

		params[0].name_ocixmldbparam = XCTXINIT_OCIDUR;
		params[0].value_ocixmldbparam = &dur;
		xctx = OCIXmlDbInitXmlCtx(_conn._env, _conn._svc_ctx, _conn._env._errh, params, 1);
		xmldocnode *x = XmlLoadDom(xctx, &xerr, "file", f._filename.c_str(), NULL);
		if(xerr)
		{
			throw_oci_exception(OciException(__TROTL_HERE__, "Error parsing XML %d\n").arg(xerr));
		}
		xml = (OCIXMLType*) x;
	}

	SqlXML(OciConnection &conn, URI u): _conn(conn)
	{
//		xmldocnode *XmlLoadDom(xmlctx *xctx, xmlerr *err, ...);
		// TODO try to use different xml context (or context singleton?)
		xmlerr xerr = (xmlerr)0;
		OCIDuration dur = OCI_DURATION_SESSION;
		struct xmlctx *xctx = (xmlctx *)0;
//		xmldocnode *doc = (xmldocnode *)0;
		ocixmldbparam params[1];

		params[0].name_ocixmldbparam = XCTXINIT_OCIDUR;
		params[0].value_ocixmldbparam = &dur;
		xctx = OCIXmlDbInitXmlCtx(_conn._env, _conn._svc_ctx, _conn._env._errh, params, 1);
		xmldocnode *x = XmlLoadDom(xctx, &xerr, "uri", u._uri.c_str(), NULL);
		if(xerr)
		{
			throw_oci_exception(OciException(__TROTL_HERE__, "Error parsing XML %d\n").arg(xerr));
		}
		xml = (OCIXMLType*) x;
	}


	SqlXML(const SqlValue &value, OciConnection &conn): _conn(conn)
	{
		//_ind.set();
		throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
	}

	tstring str() const;

	operator tstring() const
	{
		return str();
	};

//	void set(wrapped_int val) { _value = val; };

	//	int* get_ref() {return &_value;}

//protected:
	static ORASTREAM_OPEN_F(stream_open_callback, sctx, sid, hdl, length);
	static ORASTREAM_CLOSE_F(stream_close_callback, sctx, sid, hdl);
	static ORASTREAM_WRITE_F(stream_write_callback, sctx, sid, hdl, src, size, written);

	tostream _stringrepres;

	OCIType *xmltdo;
	OCIXMLType *xml;
//	ub4 *xmlsize;
	OciConnection&	_conn;
};


};
#endif /*TROTL_XML_H_*/
