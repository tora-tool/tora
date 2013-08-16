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
#include "trotl_conn.h"

#if 0
#include <sql2oci.h>
#endif

namespace trotl
{

// thread_safe_log get_log()
// {
// 	// static std::ofstream out( "out.txt");
// 	static internal_thread_safe_log log( std::cerr);
// 	return thread_safe_log( log);
// }

#if 0
OciConnection* OciConnection::CreateFromESqlContext(void* context)
{
	OCIEnv* envhp = NULL;
	sword res = SQLEnvGet(context, &envhp);

	if (res != OCI_SUCCESS)
		return NULL;

	OCISvcCtx* svchp = NULL;
	res = SQLSvcCtxGet(context, NULL, 0, &svchp);
	oci_check_error(__TROTL_HERE__, envhp, res);

	return new OciConnection(envhp, svchp);
}
#endif

/// cancel a pending OCI call in the worker thread
void OciConnection::cancel()
{
	sword res = OCICALL(OCICALL(OCIBreak(_svc_ctx, _env._errh)));

	oci_check_error(__TROTL_HERE__, _env._errh, res);
}

/// reset() is to be called after handling the cancellation in the calling worker thread.
void OciConnection::reset()
{
	sword res = OCICALL(OCIReset(_svc_ctx, _env._errh));

	oci_check_error(__TROTL_HERE__, _env._errh, res);
}

tstring OciConnection::getNLS_LANG()
{
	OraText infoBuf[OCI_NLS_MAXBUFSZ];
	sword res;
	tostream retval;

	res = OCICALL(OCINlsGetInfo(_env,                            /* environment handle */
	                            _env._errh,                            /* error handle */
	                            infoBuf,                         /* destination buffer */
	                            (size_t) OCI_NLS_MAXBUFSZ,              /* buffer size */
	                            (ub2) OCI_NLS_LANGUAGE));                       /* item */
	oci_check_error(__TROTL_HERE__, _env._errh, res);

	retval << (const char*) infoBuf;

	res = OCICALL(OCINlsGetInfo(_env,                            /* environment handle */
	                            _env._errh,                            /* error handle */
	                            infoBuf,                         /* destination buffer */
	                            (size_t) OCI_NLS_MAXBUFSZ,              /* buffer size */
	                            (ub2) OCI_NLS_TERRITORY));                      /* item */
	oci_check_error(__TROTL_HERE__, _env._errh, res);

	retval << '_' << (const char *)infoBuf;

	res = OCICALL(OCINlsGetInfo(_env,                            /* environment handle */
	                            _env._errh,                            /* error handle */
	                            infoBuf,                         /* destination buffer */
	                            (size_t) OCI_NLS_MAXBUFSZ,              /* buffer size */
	                            (ub2) OCI_NLS_CHARACTER_SET));                  /* item */
	oci_check_error(__TROTL_HERE__, _env._errh, res);

	retval << '.' << (const char *) infoBuf;
	return retval.str();
}

void OciConnection::changePassword(tstring userid, tstring password, tstring new_password)
{
	sword res;
	/*
	res = OCICALL(OCIAttrSet (reinterpret_cast<dvoid* >(_svc_ctx),
		      static_cast<ub4 >(OCI_HTYPE_SVCCTX),
		      reinterpret_cast<dvoid * >(_authp),
		      0,
		      static_cast<ub4 >(OCI_ATTR_SESSION),
		      _env._errh));
	oci_check_error(__TROTL_HERE__, _env._errh, res);
	*/
	res = OCICALL(OCIPasswordChange (_svc_ctx,
	                                 _env._errh,
	                                 (text*)(userid.c_str()),
	                                 (ub4)(userid.length()),
	                                 (text*)(password.c_str()),
	                                 (ub4)(password.length()),
	                                 (text*)(new_password.c_str()),
	                                 (ub4)(new_password.length()),
	                                 //OCI_AUTH);
	                                 OCI_DEFAULT));
	oci_check_error(__TROTL_HERE__, _env._errh, res);
}

}; //namespace trotl
