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

#ifndef TROTL_MFILE_H_
#define TROTL_MFILE_H_

#include "trotl_handle.h"

namespace trotl
{
// needs initialization in OCI_OBJECT mode
struct TROTL_EXPORT OciMessageFile
{
	OciMessageFile(OciEnv& env, const char* product, const char* facility)
		: _env(env), _msgh(0)
	{
		open(product, facility);
	}

	~OciMessageFile()
	{
		if (_msgh)
			close();
	}

	operator OCIMsg*()
	{
		return _msgh;
	}

	void open(const char* product, const char* facility, OCIDuration dur=OCI_DURATION_PROCESS)
	{
		sword res = OCICALL(OCIMessageOpen(_env, _env._errh, &_msgh, (OraText*)product, (OraText*)facility, dur));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void close()
	{
		sword res = OCICALL(OCIMessageClose(_env, _env._errh, _msgh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	OraText* get_msg(ub4 msgno) const
	{
		OraText* retval = OCICALL((OraText*)OCIMessageGet(_msgh, msgno, 0, 0));
		return retval;
	}

protected:
	OciEnv& _env;
	OCIMsg* _msgh;
};

};

#endif
