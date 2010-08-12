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

#ifndef TROTL_BASE_H_
#define TROTL_BASE_H_

#include <exception>
#include <vector>
#include <string>
#include <oci.h>

#include "trotl_export.h"
#include "trotl_common.h"

namespace trotl {
  class SqlStatement;
  
  void oci_check_error(tstring where, OCIError* errh, sword res);
  void oci_check_error(tstring where, OCIEnv* envh, sword res);
  void oci_check_error(tstring where, SqlStatement &stmt, sword res); // defined in trotl_stat.cpp
  
  template<typename TYPE>
  struct OciHandleID
  {
    static ub4 get_type_id();
    static ub4 get_attr_id();
  };

  template <> inline ub4 OciHandleID<OCIEnv>::get_type_id()		{return OCI_HTYPE_ENV;}
  template <> inline ub4 OciHandleID<OCIError>::get_type_id()	{return OCI_HTYPE_ERROR;}
  template <> inline ub4 OciHandleID<OCISvcCtx>::get_type_id()	{return OCI_HTYPE_SVCCTX;}
  template <> inline ub4 OciHandleID<OCIStmt>::get_type_id()		{return OCI_HTYPE_STMT;}
  template <> inline ub4 OciHandleID<OCIBind>::get_type_id()		{return OCI_HTYPE_BIND;}
  template <> inline ub4 OciHandleID<OCIDefine>::get_type_id()	{return OCI_HTYPE_DEFINE;}
  template <> inline ub4 OciHandleID<OCIDescribe>::get_type_id()	{return OCI_HTYPE_DESCRIBE;}
  template <> inline ub4 OciHandleID<OCIServer>::get_type_id()	{return OCI_HTYPE_SERVER;}
  template <> inline ub4 OciHandleID<OCISession>::get_type_id()	{return OCI_HTYPE_SESSION;}
  template <> inline ub4 OciHandleID<OCITrans>::get_type_id()	{return OCI_HTYPE_TRANS;}
  template <> inline ub4 OciHandleID<OCIComplexObject>::get_type_id()	{return OCI_HTYPE_COMPLEXOBJECT;}
  template <> inline ub4 OciHandleID<OCISecurity>::get_type_id()			{return OCI_HTYPE_SECURITY;}
  template <> inline ub4 OciHandleID<OCISubscription>::get_type_id()		{return OCI_HTYPE_SUBSCRIPTION;}
  /*
    template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_CTX;}
    template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_COLUMN_ARRAY;}
    template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_DIRPATH_STREAM;}
    template <> inline ub4 OciHandleID<***>::get_type_id() {return OCI_HTYPE_PROC;}
  */
  template <> inline ub4 OciHandleID<OCIServer>::get_attr_id()	{return OCI_ATTR_SERVER;}
  template <> inline ub4 OciHandleID<OCISession>::get_attr_id()	{return OCI_ATTR_SESSION;}
  template <> inline ub4 OciHandleID<OCIEnv>::get_attr_id()	{std::cout << "Error:" << __TROTL_HERE__ << std::endl; return -1;}
  template <> inline ub4 OciHandleID<OCIError>::get_attr_id()	{std::cout << "Error:" << __TROTL_HERE__ << std::endl; return -1;}
  template <> inline ub4 OciHandleID<OCISvcCtx>::get_attr_id()	{std::cout << "Error:" << __TROTL_HERE__ << std::endl; return -1;}
  template <> inline ub4 OciHandleID<OCIStmt>::get_attr_id()	{std::cout << "Error:" << __TROTL_HERE__ << std::endl; return -1;}

  struct TROTL_EXPORT OciEnv;

  /// OCI handle holder - except OciEnv
  template<typename TYPE>
  struct OciSimpleHandle : public OciHandleID<TYPE>
  {
    typedef TYPE HandleType;
    OciSimpleHandle():
      _handle(0), _envh(0)
    {}

    OciSimpleHandle(OCIEnv* envh, HandleType* handle):
      _envh(envh), _handle(handle)
    {}

    OciSimpleHandle(OCIEnv* envh):
      _handle(0)
    {
      alloc(envh);
    }

    ~OciSimpleHandle()
    {
      if (_handle)
	destroy();
    }

    operator HandleType*() {return _handle;}

    void alloc(OCIEnv* envh)
    {
      if (_handle)
	destroy();
      _envh = envh;

      sword res = OCICALL(OCIHandleAlloc(envh, (dvoid**)&_handle, OciHandleID<TYPE>::get_type_id(), 0, 0));
      oci_check_error(__TROTL_HERE__, envh, res);
    }

    void destroy()
    {
      sword res = OCICALL(OCIHandleFree(_handle, OciHandleID<TYPE>::get_type_id()));
      _handle = 0;

      if (_envh) {
	oci_check_error(__TROTL_HERE__, _envh, res);
	_envh = 0;
      }
    }

    void get_attribute(dvoid* attributep, ub4* sizep, ub4 attrtype, OCIError* errh)
    {
      sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<TYPE>::get_type_id(), attributep, sizep, attrtype, errh));
      //oci_check_error(__TROTL_HERE__, _envh, res); //?? TODO I am not here which error handle should be used here
      oci_check_error(__TROTL_HERE__, errh, res);
    }

  protected:
    HandleType*	_handle;
    OCIEnv*		_envh;
  };

  /// wrapping of OCI handles without freeing them in destructor
  //  TODO some reference counting
  template<typename TYPE>
  struct OciHandleWrapper : public OciHandleID<TYPE>
  {
    typedef TYPE HandleType;

    OciHandleWrapper(HandleType* handle):
      _handle(handle)
    {}

    operator HandleType*() {return _handle;}

  protected:
    HandleType*	_handle;
  };

  /// OCI Error Handles
  TROTL_EXPORT typedef OciSimpleHandle<OCIError> OciError;

  struct TROTL_EXPORT OciEnv : public OciHandleWrapper<OCIEnv>
  {
    typedef OciHandleWrapper<OCIEnv> super;

    OciEnv(OCIEnv* envh)
    : super(envh)
    {
      // allocate error handle
      _errh.alloc(_handle);
    }
#ifdef WIN32 // is ithis really neccessary?
    template struct TROTL_EXPORT OciSimpleHandle<OCIError>;
#endif
    OciError _errh;
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
    	const_cast<HandleType*>(other._handle) = NULL;
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

    void set_attribute(ub4 attrtype, ub4 value)
    {
      sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), (OraText*)&value, (ub4)sizeof(value), attrtype, _env._errh));
      oci_check_error(__TROTL_HERE__, _env._errh, res);
    }

    void get_attribute(dvoid* attributep, ub4* sizep, ub4 attrtype)
    {
      sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<TYPE>::get_type_id(), attributep, sizep, attrtype, _env._errh));
      oci_check_error(__TROTL_HERE__, _env._errh, res);
    }

    void set_attribute_handle(ub4 attrtype, void* handle)
    {
      sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), handle, 0, attrtype, _env._errh));
      oci_check_error(__TROTL_HERE__, _env._errh, res);
    }

    template<typename ATTR> void set_attribute(ATTR& attr)
    {
      sword res = OCICALL(OCIAttrSet(_handle, OciHandleID<TYPE>::get_type_id(), attr, 0, ATTR::get_attr_id(), _env._errh));
      oci_check_error(__TROTL_HERE__, _env._errh, res);
    }

    operator HandleType*() {return _handle;}

    OciEnv& _env;

  protected:
    HandleType*	_handle;
  };

  template<typename TYPE, int i>
  struct OciDescriptorID
  {
    static ub4 get_descr_id();
  };

  template <> inline ub4 OciDescriptorID<OCISnapshot, 0>::get_descr_id()		{return OCI_DTYPE_SNAP;}
  template <> inline ub4 OciDescriptorID<OCILobLocator, 0>::get_descr_id()		{return OCI_DTYPE_LOB;}
  template <> inline ub4 OciDescriptorID<OCILobLocator, 1>::get_descr_id()		{return OCI_DTYPE_FILE;}
  template <> inline ub4 OciDescriptorID<OCIRowid,    0>::get_descr_id()		{return OCI_DTYPE_ROWID;}
  template <> inline ub4 OciDescriptorID<OCIDateTime, 0>::get_descr_id()		{return OCI_DTYPE_DATE;}
  template <> inline ub4 OciDescriptorID<OCIDateTime, 1>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP;}
  template <> inline ub4 OciDescriptorID<OCIDateTime, 2>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP_TZ;}
  template <> inline ub4 OciDescriptorID<OCIDateTime, 3>::get_descr_id()		{return OCI_DTYPE_TIMESTAMP_LTZ;}
  
  template <> inline ub4 OciDescriptorID<OCIAQMsgProperties, 0>::get_descr_id()         {return OCI_DTYPE_AQMSG_PROPERTIES;}
  template <> inline ub4 OciDescriptorID<OCIAQDeqOptions, 0>::get_descr_id()            {return OCI_DTYPE_AQDEQ_OPTIONS;}

  template<typename TYPE, int i>
  struct OciSimpleDescriptor: public OciDescriptorID<TYPE,i>
  {
	  typedef TYPE HandleType;

	  OciSimpleDescriptor(OCIEnv* envh, HandleType* handle) : _envh(envh), _descriptor(handle)
	  {}

	  OciSimpleDescriptor(OCIEnv* envh) : _descriptor(0)
	  {
		  alloc(envh);
	  }

	  ~OciSimpleDescriptor()
	  {
		  if (_descriptor)
			  destroy();
	  }

	  operator HandleType*() {return _descriptor;}

	  void alloc(OCIEnv* envh)
	  {
		  if (_descriptor)destroy();
		  _envh = envh;

		  sword res = OCICALL(OCIDescriptorAlloc(envh, (dvoid**)&_descriptor, OciDescriptorID<TYPE,i>::get_descr_id(), 0, 0));
		  oci_check_error(__TROTL_HERE__, envh, res);
	  }

	  void destroy()
	  {
		  sword res = OCICALL(OCIDescriptorFree(_descriptor, OciDescriptorID<TYPE,i>::get_descr_id()));
		  _descriptor = 0;

		  if (_envh) {
			  oci_check_error(__TROTL_HERE__, _envh, res);
			  _envh = 0;
		  }
	  }
  private:
    HandleType *_descriptor;
    OCIEnv *_envh;
  };

  /// OCI Describe Handles
  typedef TROTL_EXPORT OciSimpleHandle<OCIDescribe> OciDescribe;
	  
  struct OciEnv;
  
  /// wrapper for the OCIEnv handle - OCIEnv* holder
  struct /*TROTL_EXPORT*/ OciEnvAlloc : public OciSimpleHandle<OCIEnv>
  {
	  typedef OciSimpleHandle<OCIEnv> super;
	  
	  OciEnvAlloc(ub4 oci_mode=OCI_OBJECT)	// OCI_OBJECT for OCINumber... functions
	  {
		  sword res = OCICALL(OCIEnvCreate(&_handle, oci_mode, 0/*ctxp*/, 0, 0, 0, 0/*xtramem_sz*/, 0/*usrmempp*/));
		  //std::cerr << "OCIEnvCreate:" << res << std::endl;
		  oci_check_error(__TROTL_HERE__, _handle, res);
	  }

	  ~OciEnvAlloc()
	  {
//		  sword res = OCITerminate (OCI_DEFAULT);
//		  oci_check_error(__TROTL_HERE__, _handle, res);
	  }
  };

  /* This set of specialized templates represent translation table
   * "userspace datatype" => "OCI datatype constant"
   * This template is used as parameter in specialization of the template TBindParInt
   * It is not necessary to register all datatypes in this template
   * look bellow for usage examples
   **/
  template <class type>
  struct SQLT
  {
    // Almost template specializations should have enum value set to appropriate SQLT_*
    //enum { value = SQLT_* };
  };

  /* This set of specialized templates represents translation table
   * "userspace datatype" => "OCI datatype constant"
   * This template is used as parameter in specialization of the template TBindParInt
   * It is not necessary to register all datatypes in this template
   * it is used for numerical datatypes only
   *
   * See trotl_stat.h
   **/
  template <> struct SQLT<int>                  { enum { value = SQLT_INT }; };
  template <> struct SQLT<unsigned int>         { enum { value = SQLT_UIN }; };
  template <> struct SQLT<long>                 { enum { value = SQLT_INT }; };
  template <> struct SQLT<unsigned long>        { enum { value = SQLT_UIN }; };
  template <> struct SQLT<long long>            { enum { value = SQLT_INT }; };
  template <> struct SQLT<float>                { enum { value = SQLT_FLT }; };
  template <> struct SQLT<double>               { enum { value = SQLT_FLT }; };
  
#ifdef WIN32
  class TROTL_EXPORT ::std::exception;
#endif

  struct TROTL_EXPORT OciException : public std::exception
  {
	  typedef std::exception super;

	  OciException(tstring where, OCIEnv* envh);
	  OciException(tstring where, OCIError* errh);
	  OciException(tstring where, const char* msg);
	  OciException(tstring where, SqlStatement& stmt);
	  OciException(const OciException&);

	  OciException& arg(int);
	  OciException& arg(unsigned int);
	  OciException& arg(unsigned long);

	  OciException& arg(tstring);

	  ~OciException() throw() {}

	  virtual const char* what() const throw() {return _mess.c_str();}

	  int get_code(int pos = 0) const { return _sql_error_code.empty() ? 0 : _sql_error_code.at(pos); };
	  const char* get_mesg() const { return _mess.c_str(); };
	  const char* get_sql() const { return _last_sql.c_str(); };
  private:
	  ::std::vector<int> _sql_error_code;
	  tstring _where;
	  ::std::stringstream _stack;
	  tstring _mess;
	  tstring _last_sql;
	  ub2 _parse_offset;
  };

  /// error handling functions
  //  extern void TROTL_EXPORT throw_oci_exception(tstring where, OCIError* errh, sword res);
  //  extern void TROTL_EXPORT throw_oci_exception(tstring where, OCIEnv* envh, sword res);
  //  __declspec(noreturn) void throw_ocipl_exception(OciException& e);
  //  extern void TROTL_EXPORT throw_ocipl_exception(OciException const & e);

  inline void oci_check_error(tstring where, OCIError* errh, sword res)
  {
	  if (res != OCI_SUCCESS)
		  throw OciException(where, errh);
  }

  inline void oci_check_error(tstring where, OCIEnv* envh, sword res)
  {
	  if (res != OCI_SUCCESS)
		  throw OciException(where, envh);
  }

/*
  inline void oci_check_error(tstring where, OciEnv& env, sword res)
  {
    oci_check_error(where, env._errh, res);
  }
*/

}; //namespace trotl

#endif /*TROTL_BASE_H_*/
