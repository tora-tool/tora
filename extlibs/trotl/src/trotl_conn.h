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

#ifndef TROTL_CONN_H_
#define TROTL_CONN_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"

namespace trotl
{
/// encapsulation of the OCIServer handle, used in OCIPL::OciLogin
TROTL_EXPORT typedef OciHandleWrapper<OCISvcCtx> OciContext;

struct TROTL_EXPORT OracleServer : public OciHandle<OCIServer>
{
	typedef OciHandle<OCIServer> super;

	OracleServer(OciEnv& env)
		:	super(env)
	{
		_version_string[0] = '\0';
		_version = 0;
	}

	~OracleServer()
	{
		sword res = OCICALL(OCIServerDetach(_handle, _env._errh, OCI_DEFAULT));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void attach(const tstring& tnsname)
	{
		sword res = OCICALL(OCIServerAttach(_handle, _env._errh, (OraText*)tnsname.c_str(), (sb4)tnsname.length()*sizeof(char), OCI_DEFAULT));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		// version might be wrong, depending on parameter sec_return_server_release_banner
		// to get the correct version getVersion() will be called a second time in connect() after the session has been sucessfully authenticated

		// This can result ORA-12737 when using Oracle instant client and the databases native charset is WE8ISO8859P9
		//try {
		//	getVersion();
		//} catch(...) {}
	}

	void getVersion()
	{
		sword res = OCICALL(OCIServerRelease(_handle, _env._errh, (OraText*)_version_string, sizeof(_version_string), OCI_HTYPE_SERVER, &_version));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void detach()
	{
		sword res = OCICALL(OCIServerDetach(_handle, _env._errh, OCI_DEFAULT));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	sword versionNumber() const
	{
		return  ((sword)(((_version) >> 24) & 0x000000FF));
	}
	sword releaseNumber() const
	{
		return  ((sword)(((_version) >> 20) & 0x0000000F));
	}
	sword updateNumber() const
	{
		return  ((sword)(((_version) >> 12) & 0x000000FF));
	}
	sword portReleaseNumber() const
	{
		return  ((sword)(((_version) >> 8) & 0x0000000F));
	}
	sword portUpdateNumber() const
	{
		return  ((sword)(((_version) >> 0) & 0x000000FF));
	}

	ub4	_version;	// e.g. 0x9200500
	char	_version_string[2000];
};


/// database login parameter structure
struct TROTL_EXPORT LoginPara
{
	tstring _username;
	tstring _password;
	tstring _tnsname;
	ub4 _mode;

	LoginPara() : _mode(OCI_DEFAULT) {};

	LoginPara(const char* username, const char* password, const char* tnsname)
		: _username(username), _password(password), _tnsname(tnsname), _mode(OCI_DEFAULT)
	{}

	LoginPara(const char* connect_string)
		: _mode(OCI_DEFAULT)
	{
		parse(connect_string);
	}

	void parse(const char* conn_str)
	{
		const char*i = conn_str;
		tstring *cw = &_username; // current word in a split
		while(*i)
		{
			if( *i == '/' )
			{
				cw = &_password;
				i++;
				continue;
			}

			if( *i == '@' )
			{
				cw = &_tnsname;
				i++;
				continue;
			}
			*cw += *i;
			i++;
		}
	}
};

struct TROTL_EXPORT LoginAndPChangePara : public LoginPara
{
	tstring	_new_password;
	LoginAndPChangePara(const char* username, const char* password, const char* new_password, const char* tnsname)
		: LoginPara(username, password, tnsname), _new_password(new_password)
	{}
};

/// simple OCI login to database
struct TROTL_EXPORT OciLogin : public OciHandle<OCISvcCtx>
{
	typedef OciHandle<OCISvcCtx> super;

	OciLogin(OciEnv& env, const LoginPara& login_para, ub4 mode=OCI_DEFAULT)
		: super(env)
		, _server(env)
		, _session(env)
		, _connected(false)
		, _sid(0)
		, _serial(0)

	{
		connect(login_para._username, login_para._password, login_para._tnsname, mode);
		getSidAndSerial();
	}

	OciLogin(OciEnv& env, const LoginAndPChangePara& login_para, ub4 mode=OCI_DEFAULT)
		: super(env),
		  _server(env),
		  _session(env),
		  _connected(false)
	{
		connect_and_pchange(login_para._username, login_para._password, login_para._new_password, login_para._tnsname, mode);
		getSidAndSerial();
	}

	OciLogin(OciEnv& env, const tstring& username, const tstring& password, const tstring& tnsname, ub4 mode=OCI_DEFAULT)
		: super(env),
		  _server(env),
		  _session(env),
		  _connected(false)
	{
		connect(username, password, tnsname, mode);
		getSidAndSerial();
	}

	void connect(const tstring& username, const tstring& password, const tstring& tnsname, ub4 mode=OCI_DEFAULT)
	{
		// set server name
		_server.attach(tnsname);
		_tnsname = tnsname;	// store database name for further reference in the application

		// set session parameters
		_session.set_attribute(OCI_ATTR_USERNAME, username);
		_session.set_attribute(OCI_ATTR_PASSWORD, password);

		// login using _server and _session
		set_attribute(_server);

		sword res = OCICALL(OCISessionBegin(_handle, _env._errh, _session, OCI_CRED_RDBMS, mode));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		// look for warning messages ("ORA-28002: the password will expire within ... days")
		if (res == OCI_SUCCESS_WITH_INFO)
		{
			OraText buffer[1024];

			sb4 errorcode;
			sword res = OCICALL(OCIErrorGet(_env._errh, 1, NULL, &errorcode, (OraText*)buffer, sizeof(buffer), OCI_HTYPE_ERROR));

			if (res == OCI_SUCCESS)
				_warnings = (const char*)buffer;
		}

		_connected = true;

		// second call
		_server.getVersion();

		set_attribute(_session);

		/* automatically allocate and initialize server and session handles
		   sword res = OCICALL(OCILogon(_env, _env._errh, &_svchp,
		   (TOraText*)username.c_str(), username.length(),
		   (TOraText*)password.c_str(), password.length(),
		   (TOraText*)tnsname.c_str(), tnsname.length()));
		   oci_check_error(__TROTL_HERE__, _env._errh, res);
		*/
	}

	void connect_and_pchange(const tstring& username, const tstring& password, const tstring new_password, const tstring& tnsname, ub4 mode=OCI_DEFAULT)
	{
		sword res;
		_server.attach(tnsname);
		_tnsname = tnsname;	// store database name for further reference in the application
		set_attribute(_server);
		set_attribute(_session);

		res = OCICALL(OCIPasswordChange (_handle,
		                                 _env._errh,
		                                 (text*)(username.c_str()),
		                                 (ub4)(username.length()),
		                                 (text*)(password.c_str()),
		                                 (ub4)(password.length()),
		                                 (text*)(new_password.c_str()),
		                                 (ub4)(new_password.length()),
		                                 OCI_AUTH));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	~OciLogin()
	{
		disconnect();
	}

	void disconnect()
	{
		if (_connected)
		{
			/* automatically free server and session handles
			 * sword res = OCICALL(OCILogoff(_svchp, _env._errh));
			 * _svchp = 0;
			 */
			sword res = OCICALL(OCISessionEnd(_handle, _env._errh, _session, OCI_DEFAULT));
			oci_check_error(__TROTL_HERE__, _env._errh, res);

			_server.detach();
			_connected = false;
		}
	}

	void commit(ub4 flags=OCI_DEFAULT)
	{
		sword res = OCICALL(OCITransCommit(_handle, _env._errh, flags));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void rollback(ub4 flags=OCI_DEFAULT)
	{
		sword res = OCICALL(OCITransRollback(_handle, _env._errh, flags));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void getSidAndSerial()
	{
		// See metalink note 971323.1
		UB10 s = _session.get_attribute<UB10>(OCI_ATTR_MIGSESSION);
		ub1 * sessionInfo = &s.bytes[0];

		// calculate SID by bitshifting - memcpy() would do fals order
		for (int i=0; i< sessionInfo[1];i++)
			_sid = _sid + ((ub4)(sessionInfo[2+i]) << ((3-i)*8));
		// calculate SERIAL# by bitshifting - memcpy() would do fals order
		for (int i=0; i< sessionInfo[6];i++)
			_serial = _serial + ((ub4)(sessionInfo[7+i]) << ((1-i)*8));
	}

	bool hasTransaction()
	{
		boolean t = true;
#if (OCI_MAJOR_VERSION >= 12)
		t = _session.get_attribute<boolean>(OCI_ATTR_TRANSACTION_IN_PROGRESS);
#endif
		return t;
	}

	OracleServer	_server;
	tstring	_warnings;
	tstring	_tnsname;
	ub4 _sid;
	ub2 _serial;

protected:
#if _MSC_VER==1400
	template struct TROTL_EXPORT OciHandle<OCISession>;
#endif
	OciHandle<OCISession>	_session;
	bool	_connected;
};

/// wrap OCIEnv and OCISvcCtx together
struct TROTL_EXPORT OciConnection
{
	OciEnv	_env;
#if _MSC_VER==1400
	template struct TROTL_EXPORT OciHandleWrapper<OCISvcCtx>;
#endif
	OciContext	_svc_ctx;

	OciConnection(OCIEnv* envh, OCISvcCtx* svc_ctx)
		: _env(envh), _svc_ctx(svc_ctx)
	{}

	//OciConnection(OCIEnv* envh) :
	//	_env(envh), _svc_ctx(0)
	//{}

	//      If the application terminates, and OCITransCommit() (transaction commit)
	//      has not been called, any pending transactions are automatically rolled back,
	//   	but ... - OCI Programmer's Guide should be trusted:)
	~OciConnection()
	{
		rollback();
	}

#if 0
	static OciConnection* CreateFromESqlContext(void* context);
#endif
	void	cancel();
	void	reset();
	tstring	getNLS_LANG();
	void	changePassword (tstring userid, tstring password, tstring new_password);

	void commit(ub4 flags=OCI_DEFAULT)
	{
		sword res = OCICALL(OCITransCommit(_svc_ctx, _env._errh, flags));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	void commit_nowait()
	{
		commit(static_cast<ub4>(OCI_TRANS_WRITENOWAIT)); /* 0x00000008 */
	}

	void rollback(ub4 flags=OCI_DEFAULT)
	{
		sword res = OCICALL(OCITransRollback(_svc_ctx, _env._errh, flags));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	bool is_noblocking()
	{
		/* Some OCI statements silently enter blocking mode without any warning
		 * we call OCIAttrGet everytime
		 */
		ub1 nonblocking = 0;
		//_svc_ctx.get_attribute(&nonblocking, NULL, OCI_ATTR_NONBLOCKING_MODE);
		return nonblocking != 0;
	}

	void set_noblocking()
	{
		//_svc_ctx.set_attribute_handle(OCI_ATTR_NONBLOCKING_MODE, NULL);
	}

	// TODO check server's version too
	void ping()
	{
#if (OCI_MAJOR_VERSION == 10 && OCI_MINOR_VERSION >=2) || (OCI_MAJOR_VERSION > 10)
		sword res = OCICALL(OCIPing(_svc_ctx, _env._errh, OCI_DEFAULT));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
#endif
	}

	//StatementCache	_stmt_cache; TODO

private:
	OciConnection(const OciConnection&);	// disallow copy constructor calls
};

};

#endif
