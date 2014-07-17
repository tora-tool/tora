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

/*
 * This header file represents base templates for OCI handles manipulations.
 * all the OCI handle classes are subclases of either OciSimpleHandle or OciHandle.
 * Plus two most important wrapper classes are declared directly in this header:
 * OciEnv and OciError (note: see the diffenrence "OCI" vc. "Oci"
 */
#ifndef TROTL_HANDLE_H_
#define TROTL_HANDLE_H_

#include <typeinfo>
#include <algorithm>
#include <oci.h>

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_error.h"

#include "loki/TypeTraits.h"
#include "loki/TypeManip.h"
#include "loki/static_check.h"

namespace trotl
{

struct TROTL_EXPORT OciEnv;
struct TROTL_EXPORT OciError;

template<typename TYPE>
struct OciHandleID
{
	static ub4 get_type_id();
	static ub4 get_attr_id();
};

template <> inline ub4 OciHandleID<OCIEnv>::get_type_id()
{
	return OCI_HTYPE_ENV;
}
template <> inline ub4 OciHandleID<OCIError>::get_type_id()
{
	return OCI_HTYPE_ERROR;
}
template <> inline ub4 OciHandleID<OCISvcCtx>::get_type_id()
{
	return OCI_HTYPE_SVCCTX;
}
template <> inline ub4 OciHandleID<OCIStmt>::get_type_id()
{
	return OCI_HTYPE_STMT;
}
template <> inline ub4 OciHandleID<OCIBind>::get_type_id()
{
	return OCI_HTYPE_BIND;
}
template <> inline ub4 OciHandleID<OCIDefine>::get_type_id()
{
	return OCI_HTYPE_DEFINE;
}
template <> inline ub4 OciHandleID<OCIDescribe>::get_type_id()
{
	return OCI_HTYPE_DESCRIBE;
}
template <> inline ub4 OciHandleID<OCIServer>::get_type_id()
{
	return OCI_HTYPE_SERVER;
}
template <> inline ub4 OciHandleID<OCISession>::get_type_id()
{
	return OCI_HTYPE_SESSION;
}
template <> inline ub4 OciHandleID<OCITrans>::get_type_id()
{
	return OCI_HTYPE_TRANS;
}
template <> inline ub4 OciHandleID<OCIComplexObject>::get_type_id()
{
	return OCI_HTYPE_COMPLEXOBJECT;
}
template <> inline ub4 OciHandleID<OCISecurity>::get_type_id()
{
	return OCI_HTYPE_SECURITY;
}
template <> inline ub4 OciHandleID<OCISubscription>::get_type_id()
{
	return OCI_HTYPE_SUBSCRIPTION;
}
/*
  template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_CTX;}
  template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_COLUMN_ARRAY;}
  template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_STREAM;}
  template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_PROC;}
*/
template <> inline ub4 OciHandleID<OCIServer>::get_attr_id()
{
	return OCI_ATTR_SERVER;
}
template <> inline ub4 OciHandleID<OCISession>::get_attr_id()
{
	return OCI_ATTR_SESSION;
}
template <> inline ub4 OciHandleID<OCIEnv>::get_attr_id()
{
	LOKI_STATIC_CHECK(1, UnsupportedHanleOPonENV);
	return -1;
}
template <> inline ub4 OciHandleID<OCIError>::get_attr_id()
{
	LOKI_STATIC_CHECK(1, UnsupportedHanleOPonError);
	return -1;
}
template <> inline ub4 OciHandleID<OCISvcCtx>::get_attr_id()
{
	LOKI_STATIC_CHECK(1, UnsupportedHanleOPonSvcCtx);
	return -1;
}
template <> inline ub4 OciHandleID<OCIStmt>::get_attr_id()
{
	LOKI_STATIC_CHECK(1, UnsupportedHanleOPonSTMT);
	return -1;
}

template <> inline ub4 OciHandleID<OCIParam>::get_type_id()
{
	return OCI_DTYPE_PARAM;
}

/// OCI handle holder - subclassed by OciError
/// holds handle poiner and a pointer to OCIEnv
template<typename TYPE>
struct OciSimpleHandle : public OciHandleID<TYPE>
{
	typedef TYPE HandleType;
	operator HandleType*()
	{
		return _handle;
	}

	OciSimpleHandle()
		: _handle(0)
		, _envhp(0)
	{}

	OciSimpleHandle(OCIEnv* envh, HandleType* handle)
		: _handle(handle)
		, _envhp(envh)
	{}

	OciSimpleHandle(OCIEnv* envh)
		: _handle(0)
	{
		alloc(envh);
	}

	~OciSimpleHandle()
	{
		if (_handle)
			destroy();
	}

	void alloc(OCIEnv* envh)
	{
		if (_handle)
			destroy();

		_envhp = envh;

		sword res = OCICALL(OCIHandleAlloc(envh, (dvoid**)&_handle, OciHandleID<TYPE>::get_type_id(), 0, 0));
		oci_check_error(__TROTL_HERE__, envh, res);
	}

	void destroy()
	{
		sword res = OCICALL(OCIHandleFree(_handle, OciHandleID<TYPE>::get_type_id()));
		_handle = 0;

		if (_envhp)
		{
			oci_check_error(__TROTL_HERE__, _envhp, res);
			_envhp = 0;
		}
	}

protected:
	HandleType*	_handle;
	OCIEnv*	_envhp;
}; // template<typename TYPE> struct OciSimpleHandle : public OciHandleID<TYPE>

/// OCI Error Handler
//TROTL_EXPORT typedef OciSimpleHandle<OCIError> OciError;
struct TROTL_EXPORT OciError: public OciSimpleHandle<OCIError> {};

/// wrapping of OCI handles without freeing them in destructor
// subclassed by OciEnv
// TODO some reference counting
template<typename TYPE>
struct OciHandleWrapper : public OciHandleID<TYPE>
{
	typedef TYPE HandleType;

	OciHandleWrapper(HandleType* handle):
		_handle(handle)
	{}

	operator HandleType*()
	{
		return _handle;
	}

protected:
	HandleType*	_handle;
};

struct TROTL_EXPORT OciEnv : public OciHandleWrapper<OCIEnv>
{
	typedef OciHandleWrapper<OCIEnv> super;

	OciEnv(OCIEnv* envh) : super(envh)
	{
		// allocate error handle
		_errh.alloc(_handle);
	}

//Workaround for MSVC2008
#if _MSC_VER==1400
	template struct TROTL_EXPORT OciSimpleHandle<OCIError>;
#endif
	OciError _errh;
};


/// wrapper for the OCIEnv handle - OCIEnv* holder
struct TROTL_EXPORT OciEnvAlloc : public OciSimpleHandle<OCIEnv>
{
	typedef OciSimpleHandle<OCIEnv> super;

	OciEnvAlloc(ub4 oci_mode=OCI_OBJECT|OCI_THREADED)	// OCI_OBJECT for OCINumber... functions
	{
		//Note: the characterset constant number for UTF8 = 871 can be retrieved in NOTE.93358.1 SCRIPT: Where to Find Specifications of Character Encoding
		ub2 cs = 871, ncs =871;
		//sword res = OCICALL(OCIEnvCreate(&_handle, oci_mode, 0/*ctxp*/, 0, 0, 0, 0/*xtramem_sz*/, 0/*usrmempp*/));
		sword res = OCICALL(OCIEnvNlsCreate(&_handle, oci_mode, 0/*ctxp*/, 0, 0, 0, 0/*xtramem_sz*/, 0/*usrmempp*/, cs, ncs));
		//std::cerr << "OCIEnvCreate:" << res << std::endl;
		oci_check_error(__TROTL_HERE__, _handle, res);
	}

	~OciEnvAlloc()
	{
//		  sword res = OCITerminate (OCI_DEFAULT);
//		  oci_check_error(__TROTL_HERE__, _handle, res);
	}
};

template<typename TYPE>
struct OciHandle : public OciHandleID<TYPE>
{
	typedef TYPE HandleType;

	OciHandle(OciEnv& env):
		_env(env),_handle(0)
	{
		alloc();
	}

	OciHandle(OciEnv& env, HandleType* handle)
		: _env(env), _handle(handle)
	{}

	OciHandle(const OciHandle& other)
		: _env(other._env), _handle(other._handle)
	{
		const_cast<OciHandle&>(other)._handle = NULL;
	}

	~OciHandle()
	{
		if (_handle)
			destroy();
	}

	void alloc()
	{
		if (_handle)
			destroy();

		sword res = OCICALL(OCIHandleAlloc(_env, (dvoid**)&_handle, OciHandleID<TYPE>::get_type_id(), 0, 0));
		oci_check_error(__TROTL_HERE__, _env, res);
	}

	void destroy()
	{
		sword res = OCICALL(OCIHandleFree(_handle, OciHandleID<TYPE>::get_type_id()));
		_handle = 0;

		oci_check_error(__TROTL_HERE__, _env, res);
	}

	void set_attribute(ub4 attrtype, const tstring& str)
	{
		sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), (char*)str.c_str(), (ub4)str.length()*sizeof(char), attrtype, _env._errh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	// When called cast pointer to (void*)
	// size (IN) The size of an attribute value. This can be passed in as 0 for most attributes as the size is already known by the OCI library.
	void set_attribute(ub4 attrtype, void* valuep)
	{
		sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), valuep, 0, attrtype, _env._errh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	// TODO use template here + numeric traits
	void set_attribute(ub4 attrtype, ub4 value)
	{
		sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), (OraText*)&value, (ub4)sizeof(value), attrtype, _env._errh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	template<typename ATTR> void set_attribute(ATTR& attr)
	{
		sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), attr, 0, ATTR::get_attr_id(), _env._errh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
	}

	template<class RETTYPE>
	RETTYPE get_attribute(ub4 attrtype)
	{
		RETTYPE retval;
		if( Loki::TypeTraits<RETTYPE>::isIntegral)
		{
			sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<TYPE>::get_type_id(), &retval, 0, attrtype, _env._errh));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
			return retval;
		}
		else if( Loki::TypeTraits<RETTYPE>::isPointer)
		{
			sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<TYPE>::get_type_id(), &retval, 0, attrtype, _env._errh));
			oci_check_error(__TROTL_HERE__, _env._errh, res);
			return retval;
		}
		else if( Loki::IsSameType<RETTYPE, UB10>::value)
		{
			ub1* p_retval = &retval.bytes[0];
			ub4 infoSize = sizeof(retval.bytes);

			sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<TYPE>::get_type_id(), &p_retval, &infoSize, attrtype, _env._errh));
			oci_check_error(__TROTL_HERE__, _env._errh, res); // TODO potentical race. Use sessions local errh instead of "global" _env._errh

			if( p_retval != &retval.bytes[0])
			{
				// Oracle does not use our provided bytes
				// p_retval points into OCI interval structures (the example in the metalink note 971323.1 is wrong)
				memcpy(&retval.bytes[0], p_retval, (std::min)((size_t)infoSize, sizeof(retval.bytes)));
			}
			return retval;
		}
		throw_oci_exception(OciException(__TROTL_HERE__, "Unsupported datatype: %s").arg(typeid(RETTYPE).name()));
	};

	operator HandleType*()
	{
		return _handle;
	}

	OciEnv& _env;
protected:
	HandleType*	_handle;
}; // struct OciHandle : public OciHandleID<TYPE>

//template<typename TYPE, int i>
//struct OciDescriptorID
//{
// static ub4 get_descr_id();
//};

//template <> inline ub4 OciDescriptorID<OCISnapshot, 0>::get_descr_id()		{return OCI_DTYPE_SNAP;}
//template <> inline ub4 OciDescriptorID<OCILobLocator, 0>::get_descr_id()		{return OCI_DTYPE_LOB;}
//template <> inline ub4 OciDescriptorID<OCILobLocator, 1>::get_descr_id()		{return OCI_DTYPE_FILE;}
//template <> inline ub4 OciDescriptorID<OCIRowid,    0>::get_descr_id()		{return OCI_DTYPE_ROWID;}
//template <> inline ub4 OciDescriptorID<OCIDateTime, 0>::get_descr_id()		{return OCI_DTYPE_DATE;}
//template <> inline ub4 OciDescriptorID<OCIDateTime, 1>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP;}
//template <> inline ub4 OciDescriptorID<OCIDateTime, 2>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP_TZ;}
//template <> inline ub4 OciDescriptorID<OCIDateTime, 3>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP_LTZ;}
//
//template <> inline ub4 OciDescriptorID<OCIAQMsgProperties, 0>::get_descr_id()         {return OCI_DTYPE_AQMSG_PROPERTIES;}
//template <> inline ub4 OciDescriptorID<OCIAQDeqOptions, 0>::get_descr_id()            {return OCI_DTYPE_AQDEQ_OPTIONS;}

//template <> inline ub4 OciDescriptorID<OCIParam, 0>::get_descr_id()			{return OCI_DTYPE_PARAM;}

//template<typename TYPE, int i>
//struct OciSimpleDescriptor: public OciDescriptorID<TYPE,i>
//{
// typedef TYPE HandleType;

// OciSimpleDescriptor(OCIEnv* envh, HandleType* handle) : _envh(envh), _descriptor(handle)
// {}

// OciSimpleDescriptor(OCIEnv* envh) : _descriptor(0)
// {
//  alloc(envh);
// }

// ~OciSimpleDescriptor()
// {
//  if (_descriptor)
//	  destroy();
// }

// operator HandleType*() {return _descriptor;}

// void alloc(OCIEnv* envh)
// {
//  if (_descriptor)destroy();
//  _envh = envh;

//  sword res = OCICALL(OCIDescriptorAlloc(envh, (dvoid**)&_descriptor, OciDescriptorID<TYPE,i>::get_descr_id(), 0, 0));
//  oci_check_error(__TROTL_HERE__, envh, res);
// }

// void destroy()
// {
//  sword res = OCICALL(OCIDescriptorFree(_descriptor, OciDescriptorID<TYPE,i>::get_descr_id()));
//  _descriptor = 0;

//  if (_envh) {
//	  oci_check_error(__TROTL_HERE__, _envh, res);
//	  _envh = 0;
//  }
// }
//private:
// HandleType *_descriptor;
// OCIEnv *_envh;
//};

}; //namespace trotl

#endif /*TROTL_HANDLE_H_*/
