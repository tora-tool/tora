// ==============================================================
// Oracle, ODBC and DB2/CLI Template Library, Version 4.0.68,
// Copyright (C) Sergei Kuchin, 1996,2003
// Author: Sergei Kuchin
// This library is free software. Permission to use, copy,
// modify and redistribute it for any purpose is hereby granted
// without fee, provided that the above copyright notice appear
// in all copies.
// ==============================================================

#ifndef __OTL_H__
#define __OTL_H__

#define OTL_VERSION_NUMBER (0x040044L)

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

//======================= CONFIGURATION #DEFINEs ===========================

// Uncomment the following line in order to include the OTL for ODBC:
//#define OTL_ODBC

// Uncomment the following line in order to include the OTL for
// MySQL/MyODBC:
//#define OTL_ODBC_MYSQL

// Uncomment the following line in order to include the OTL for DB2 CLI:
//#define OTL_DB2_CLI

// Uncomment the following line in order to include the OTL for
// Oracle 7: 
//#define OTL_ORA7

// Uncomment the following line in order to include the OTL for
// Oracle 8:
//#define OTL_ORA8

// Uncomment the following line in order to include the OTL for
// Oracle 8i:
//#define OTL_ORA8I

// Uncomment the following line in order to include the OTL for
// Oracle 9i:
//#define OTL_ORA9I


// The macro definitions may be also turned on via C++ compiler command line
// option, e.g.: -DOTL_ODBC, -DOTL_ORA7, -DOTL_ORA8, -DOTL_ORA8I, -DOTL_ODBC_UNIX
// -DOTL_ODBC_MYSQL, -DOTL_DB2_CLI

#ifdef OTL_ORA8I
#define OTL_ORA8
#define OTL_ORA8_8I_REFCUR
#define OTL_ORA8_8I_DESC_COLUMN_SCALE
#endif

#ifdef OTL_ORA9I
#define OTL_ORA8
#define OTL_ORA8_8I_REFCUR
#define OTL_ORA8_8I_DESC_COLUMN_SCALE
#endif

#ifdef OTL_ODBC_MYSQL
#define OTL_ODBC
#endif

#ifdef OTL_ODBC_XTG_IBASE6
#define OTL_ODBC
#endif

#define OTL_VALUE_TEMPLATE
//#define OTL_ODBC_SQL_EXTENDED_FETCH_ON

#if defined(OTL_ODBC_UNIX) && !defined(OTL_ODBC)
#define OTL_ODBC
#endif

// ------------------- Namespace generation ------------------------
#ifdef OTL_EXPLICIT_NAMESPACES

#ifdef OTL_DB2_CLI
#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }
#else
#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc {
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }
#endif

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#else

// Only one OTL is being intantiated
#if defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&!defined(OTL_DB2_CLI)&& \
    !defined(OTL_IBASE) \
 || !defined(OTL_ODBC)&&defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&!defined(OTL_DB2_CLI)&& \
    !defined(OTL_IBASE) \
 || !defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    defined(OTL_ORA7)&&!defined(OTL_DB2_CLI)&& \
    !defined(OTL_IBASE) \
 || !defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&defined(OTL_DB2_CLI)&& \
    !defined(OTL_IBASE) \
 || !defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&!defined(OTL_DB2_CLI)&& \
    defined(OTL_IBASE)

#define OTL_ODBC_NAMESPACE_BEGIN
#define OTL_ODBC_NAMESPACE_PREFIX
#define OTL_ODBC_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_IBASE_NAMESPACE_BEGIN
#define OTL_IBASE_NAMESPACE_PREFIX
#define OTL_IBASE_NAMESPACE_END

#endif

// ================ Combinations of two OTLs =========================
#if defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    !defined(OTL_IBASE) 

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc{
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_IBASE_NAMESPACE_BEGIN
#define OTL_IBASE_NAMESPACE_PREFIX
#define OTL_IBASE_NAMESPACE_END

#endif

#if defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    !defined(OTL_IBASE) 

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc{
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_IBASE_NAMESPACE_BEGIN
#define OTL_IBASE_NAMESPACE_PREFIX
#define OTL_IBASE_NAMESPACE_END

#endif

#if !defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && defined(OTL_DB2_CLI) && \
    !defined(OTL_IBASE) 

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN
#define OTL_IBASE_NAMESPACE_PREFIX
#define OTL_IBASE_NAMESPACE_END

#endif

#if !defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && defined(OTL_DB2_CLI) && \
    !defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN
#define OTL_IBASE_NAMESPACE_PREFIX
#define OTL_IBASE_NAMESPACE_END

#endif

#if defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc {
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if !defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ODBC_NAMESPACE_BEGIN
#define OTL_ODBC_NAMESPACE_PREFIX
#define OTL_ODBC_NAMESPACE_END

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if !defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN
#define OTL_ODBC_NAMESPACE_PREFIX
#define OTL_ODBC_NAMESPACE_END

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if !defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

// ================= Combinations of three OTLs =====================
#if defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc {
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && !defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc {
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if !defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN 
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END 

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#if !defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && defined(OTL_DB2_CLI) && \
    defined(OTL_IBASE)

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_IBASE_NAMESPACE_BEGIN namespace ibase {
#define OTL_IBASE_NAMESPACE_PREFIX ibase::
#define OTL_IBASE_NAMESPACE_END }

#endif

#endif

// -------------------- End of namespace generation -------------------

// --------------------- Invalid combinations --------------------------
#if defined(OTL_ODBC) && defined(OTL_DB2_CLI)
#error Invalid combination: OTL_ODBC && OTL_DB2_CLI together
#endif

#if defined(OTL_ORA7) && defined(OTL_ORA8)
#error Invalid combination: OTL_ORA7 && OTL_ORA8(I) together
#endif

#if (defined(OTL_ORA7) || defined(OTL_ORA8) ||          \
     defined(OTL_ORA8I) || defined(OTL_OAR9I) ) &&      \
     defined(OTL_BIGINT) &&                             \
     !(defined(OTL_ODBC) || defined(OTL_DB2_CLI)) 
#error OTL_BIGINT is supported only for OTL_ODBC and OTL_DB2_CLI 
#endif

#if defined (OTL_ORA7) && defined(OTL_ORA8)
#error Invalid combination: OTL_ORA7 && OTL_ORA8(I) together
#endif
// --------------------------------------------------------------------

#ifdef OTL_UNICODE

#if defined(OTL_ORA8I)||defined(OTL_ORA9I)

#define OTL_CHAR unsigned short
#define OTL_UNICODE_CHAR OTL_CHAR
#define OTL_WCHAR unsigned long

#ifdef OTL_ORA8I
#define OTL_UNICODE_ID OCI_UCS2ID
#endif

#ifdef OTL_ORA9I 
#define OTL_UNICODE_ID OCI_UTF16ID
#endif


#else
#error Unicode is supported only for OTL_ORA8I, OTL_ORA9I
#endif
#else
#define OTL_CHAR unsigned char
#endif


#if defined(OTL_ORA7) || defined(OTL_ORA8)
#define OTL_PL_TAB
#endif

const int otl_odbc_adapter=1;
const int otl_ora7_adapter=2;
const int otl_ora8_adapter=3;
const int otl_ibase_adapter=4;

const int otl_inout_binding=1;
const int otl_select_binding=2;

#ifdef OTL_ANSI_CPP

#define OTL_SCAST(_t,_e) static_cast<_t >(_e)
#define OTL_RCAST(_t,_e) reinterpret_cast<_t >(_e)
#define OTL_DCAST(_t,_e) dynamic_cast<_t >(_e)
#define OTL_CCAST(_t,_e) const_cast<_t >(_e)

#if defined OTL_FUNC_THROW_SPEC_ON
#define OTL_THROWS_OTL_EXCEPTION throw(otl_exception)
#define OTL_NO_THROW throw()
#else
#define OTL_THROWS_OTL_EXCEPTION
#define OTL_NO_THROW
#endif

#define OTL_TYPE_NAME typename

#else

#define OTL_SCAST(_t,_e) ((_t)(_e))
#define OTL_RCAST(_t,_e) ((_t)(_e))
#define OTL_DCAST(_t,_e) ((_t)(_e))
#define OTL_CCAST(_t,_e) ((_t)(_e))
#define OTL_THROWS_OTL_EXCEPTION
#define OTL_NO_THROW
#define OTL_TYPE_NAME class

#endif

#define OTL_PCONV(_to,_from,_val) \
  OTL_SCAST(_to,*OTL_RCAST(_from*,OTL_CCAST(void*,_val)))

#ifdef OTL_ACE

#include <ace/SString.h>
#include <ace/Array.h>
#include <ace/Functor.h>
#include <ace/RB_Tree.h>

#define OTL_USER_DEFINED_STRING_CLASS_ON
#define USER_DEFINED_STRING_CLASS ACE_TString
#define OTL_VALUE_TEMPLATE_ON

const int otl_tmpl_vector_default_size=16;

template<OTL_TYPE_NAME T>
class otl_tmpl_vector: public ACE_Array<T>{
public:

 otl_tmpl_vector(const int init_size=otl_tmpl_vector_default_size)
  : ACE_Array<T>(init_size==0?otl_tmpl_vector_default_size:init_size)
 {
  _length=0;
 }

 ~otl_tmpl_vector(){}

 int capacity(void) const
 {
  return this->max_size();
 }

 int size(void) const
 {
  return _length;
 }

 void clear(void)
 {
  _length=0;
 }

 void resize(const int new_size, const T& t=T())
 {
  ACE_Array<T>::size(new_size);
  if(new_size>_length){
   for(int i=_length-1;i<new_size;++i)
    (*this)[i]=t;
  }
  _length=new_size;
 }

 void push_back(const T& elem)
 {
  int curr_max_size=this->max_size();
  if(_length==curr_max_size)
   ACE_Array<T>::size(curr_max_size*2);
  ++_length;
  (*this)[_length-1]=elem;
 }

 void pop_back(void)
 {
  if(_length>0)
   --_length;
 }

protected:

 int _length;
 
};


#endif

#ifdef OTL_STLPORT

#ifdef __STLPORT_STD
#define OTL_STLPORT_NAMESPACE __STLPORT_STD
#else
#if defined(_STLP_USE_OWN_NAMESPACE)
#define OTL_STLPORT_NAMESPACE _STL
#else
#define OTL_STLPORT_NAMESPACE std
#endif
#endif


#define OTL_STL

#endif

#if defined(OTL_VALUE_TEMPLATE_ON) && !defined(OTL_STL) && !defined(OTL_ACE)
#define STD_NAMESPACE_PREFIX
#include <iostream.h>
#endif

#ifdef OTL_USER_DEFINED_STRING_CLASS_ON

#if defined(OTL_STL)
#error OTL_STL cannot be used in combination with OTL_USER_DEFINED_STRING_CLASS_ON
#endif

#ifdef USER_DEFINED_STRING_CLASS
#define OTL_STRING_CONTAINER USER_DEFINED_STRING_CLASS
#define STD_NAMESPACE_PREFIX
#else
#error USER_DEFINED_STRING_CLASS macro needs to be defined before including otlv4.h
#endif

#endif


#ifdef OTL_STL

#ifdef _MSC_VER
#if (_MSC_VER >= 1200)
#pragma warning (disable:4786) 
#endif
#endif

#if defined(OTL_STL_NOSTD_NAMESPACE)
#ifndef OTL_STRING_CONTAINER
#define OTL_STRING_CONTAINER string
#endif
#define STD_NAMESPACE_PREFIX
#else
#ifndef OTL_STRING_CONTAINER

#if defined(OTL_STLPORT)
#define OTL_STRING_CONTAINER OTL_STLPORT_NAMESPACE ::string
#else
#define OTL_STRING_CONTAINER std::string
#endif

#endif

#if defined(OTL_STLPORT)
#define STD_NAMESPACE_PREFIX OTL_STLPORT_NAMESPACE ::
#else
#define STD_NAMESPACE_PREFIX std::
#endif


#endif

#include <string>
#include <iterator>
#include <vector>

#ifdef OTL_UNCAUGHT_EXCEPTION_ON
#include <exception>
#endif

#ifndef OTL_STL_NOSTD_NAMESPACE
#include <iostream>
#else
#include <iostream.h>
#endif

#endif

#ifdef OTL_DB2_CLI
#define OTL_ODBC
#endif

#ifdef OTL_ODBC 

#ifdef OTL_DB2_CLI
#define OTL_HENV SQLHANDLE
#define OTL_HDBC SQLHANDLE
#define OTL_SQLHANDLE SQLHANDLE
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER
#else
#if (ODBCVER >= 0x0300)
#define OTL_HENV SQLHANDLE
#define OTL_HDBC SQLHANDLE
#define OTL_SQLHANDLE SQLHANDLE
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER
#else
#define OTL_HENV HENV
#define OTL_HDBC HDBC
#define OTL_SQLHANDLE HSTMT
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER
#endif
#endif

#endif


//#define OTL_DEBUG

//======================= END OF CONFIGURATION ==============================


// ====== COMMON NON-TEMPLATE OBJECTS: CONSTANTS, CLASSES, ETC. ===========

#ifdef OTL_ORA8
const int otl_var_list_size=1024;
#else
const int otl_var_list_size=512;
#endif

const int otl_error_code_0=32000;
#define otl_error_msg_0 "Incompatible data types in stream operation"

const int otl_error_code_1=32004;
#define otl_error_msg_1 "No input variables have been defined in SQL statement"

const int otl_error_code_2=32003;
#define otl_error_msg_2 "Not all input variables have been initialized"

const int otl_error_code_3=32001;
#define otl_error_msg_3 "Row must be full for flushing output stream"

const int otl_error_code_4=32005;
#define otl_error_msg_4 "Input string value is too large to fit into the buffer"

const int otl_error_code_5=32006;
#define otl_error_msg_5 "Input otl_long_string is too large to fit into the buffer"

const int otl_error_code_6=32007;
#define otl_error_msg_6 "PL/SQL table size is too large (>32767)"

const int otl_error_code_7=32008;
#define otl_error_msg_7 "Writing CLOB/BLOB in stream mode: actual size is greater than specified"

const int otl_error_code_8=32009;
#define otl_error_msg_8 "Closing CLOB/BLOB in stream mode: actual size is not equal to specified size"

const int otl_error_code_9=32010;
#define otl_error_msg_9 "CLOB/BLOB stream is not open for writing"

const int otl_error_code_10=32011;
#define otl_error_msg_10 "CLOB/BLOB stream is not open for reading"

const int otl_error_code_11=32012;
#define otl_error_msg_11 "First session must be started with session_begin()"

const int otl_error_code_12=32013;
#define otl_error_msg_12 "Invalid bind variable declaration"

const int otl_error_code_13=32014;
#define otl_error_msg_13 "No stored procedure was found"

const int otl_error_code_14=32015;
#define otl_error_msg_14 "Unsupported data type: "

const int otl_error_code_15=32016;
#define otl_error_msg_15 "Unsupported procedure type"

const int otl_error_code_16=32017;
#define otl_error_msg_16 "Stream buffer size can't be > 1 in this case"

const int otl_error_code_17=32018;
#define otl_error_msg_17                                        \
"ODBC / DB2 CLI function name is not recognized. "              \
"It should be one of the following: SQLTables, SQLColumns, "    \
"SQLProcedures, SQLColumnPrivileges, SQLTablePrivileges, "      \
"SQLPrimaryKeys, SQLProcedureColumns, SQLForeignKeys"

const int otl_oracle_date_size=7;

const int otl_explicit_select=0;
const int otl_implicit_select=1;

const int otl_input_param=0;
const int otl_output_param=1;
const int otl_inout_param=2;

const unsigned int otl_all_num2str=1;
const unsigned int otl_all_date2str=2;

const int otl_num_str_size=60;
const int otl_date_str_size=60;

class otl_select_struct_override{
public:

 int col_ndx[otl_var_list_size];
 int col_type[otl_var_list_size];
 int col_size[otl_var_list_size];
 int len;

 unsigned int all_mask;
 bool lob_stream_mode;

 otl_select_struct_override()
 {
   reset();
 }

  void reset(void)
  {
    len=0;
    all_mask=0;
    lob_stream_mode=false;
  }
 
 ~otl_select_struct_override(){}

 void add_override(const int andx, const int atype, const int asize=0)
 {
  if(len<otl_var_list_size-1){
   ++len;
   col_ndx[len-1]=andx;
   col_type[len-1]=atype;
   col_size[len-1]=asize;
  }
 }

 int find(const int ndx)
 {int i;
  for(i=0;i<len;++i)
   if(ndx==col_ndx[i])
    return i;
  return -1;
 }

  void set_all_column_types(const unsigned int amask=0)
  {
    all_mask=amask;
  }

};


inline int otl_decimal_degree(unsigned int num)
{
  int n=0;
  while(num!=0){
    ++n;
    num/=10;
  }
  return n;
}

inline unsigned int otl_to_fraction
(unsigned int fraction,int frac_prec)
{
  if(fraction==0||frac_prec==0)return fraction;
  int degree_diff=9-frac_prec;
  for(int i=0;i<degree_diff;++i)
    fraction*=10;
  return fraction;
}

inline unsigned int otl_from_fraction
(unsigned int fraction,int frac_prec)
{
  if(fraction==0||frac_prec==0)return fraction;
  int degree_diff=9-frac_prec;
  for(int i=0;i<degree_diff;++i)
    fraction/=10;
  return fraction;
}

class otl_datetime{
public:

  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  unsigned long fraction;
  int frac_precision;

#if defined(OTL_ORA_TIMESTAMP)
  short int tz_hour;
  short int tz_minute;
#endif

 otl_datetime()
 {
  year=1900;
  month=1;
  day=1;
  hour=0;
  minute=0;
  second=0;
  fraction=0;
  frac_precision=0;
#if defined(OTL_ORA_TIMESTAMP)
  tz_hour=0;
  tz_minute=0;
#endif
 }

 otl_datetime
 (const int ayear,
  const int amonth,
  const int aday,
  const int ahour,
  const int aminute,
  const int asecond,
  const unsigned long afraction=0,
  const int afrac_precision=0,
  const short int atz_hour=0,
  const short int atz_minute=0
 )
 {
  year=ayear;
  month=amonth;
  day=aday;
  hour=ahour;
  minute=aminute;
  second=asecond;
  fraction=afraction;
  frac_precision=afrac_precision;
#if defined(OTL_ORA_TIMESTAMP)
  tz_hour=atz_hour;
  tz_minute=atz_minute;
#endif
 }

 otl_datetime(const otl_datetime& dt)
 {
   copy(dt);
 }

 ~otl_datetime(){}

 otl_datetime& operator=(const otl_datetime& dt)
 {
   copy(dt);
  return *this;
 }

protected:

  void copy(const otl_datetime& dt)
  {
    year=dt.year;
    month=dt.month;
    day=dt.day;
    hour=dt.hour;
    minute=dt.minute;
    second=dt.second;
    fraction=dt.fraction;
    frac_precision=dt.frac_precision;
#if defined(OTL_ORA_TIMESTAMP)
    tz_hour=dt.tz_hour;
    tz_minute=dt.tz_minute;
#endif
  }

};

class otl_oracle_date{
public:
 unsigned char century;
 unsigned char year;
 unsigned char month;
 unsigned char day;
 unsigned char hour;
 unsigned char minute;
 unsigned char second;

 otl_oracle_date(){}
 ~otl_oracle_date(){}

};

#ifdef OTL_DEBUG
#include <iostream.h>
inline ostream& operator<<(ostream& s, const otl_datetime& dt)
{
 s<<dt.month<<"/"<<dt.day<<"/"<<dt.year<<" "
  <<dt.hour<<":"<<dt.minute<<":"<<dt.second
  <<"."<<dt.fraction<<endl;
 return s;
}

#endif

inline void convert_date(otl_datetime& t,const otl_oracle_date& s)
{
 t.year=(OTL_SCAST(int, s.century-100)*100+(OTL_SCAST(int, s.year-100)));
 t.month=s.month;
 t.day=s.day;
 t.hour=s.hour-1;
 t.minute=s.minute-1;
 t.second=s.second-1;
}

inline void convert_date(otl_oracle_date& t,const otl_datetime& s)
{
 t.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
 t.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
 t.month=OTL_SCAST(unsigned char, s.month);
 t.day=OTL_SCAST(unsigned char, s.day);
 t.hour=OTL_SCAST(unsigned char, (s.hour+1));
 t.minute=OTL_SCAST(unsigned char, (s.minute+1));
 t.second=OTL_SCAST(unsigned char, (s.second+1));
}

class otl_null{
public:
 int dummy; // this is to fix a compiler bug in VC++ 6.0

 otl_null(){}
 ~otl_null(){}
};

class otl_column_desc{
public:
 char name[512];
 int  dbtype;
 int  otl_var_dbtype;
 int  dbsize;
 int  scale;
 int  prec;
 int  nullok;
};

class otl_var_desc{
public:
 int  param_type;
 int  ftype;
 int  elem_size;
 int  array_size;
 int  pos;
 int  name_pos;
 char name[128];
 int  pl_tab_flag;

 otl_var_desc()
 {
  param_type=0;
  ftype=0;
  elem_size=0;
  array_size=0;
  pos=0;
  name_pos=0;
  name[0]=0;
  pl_tab_flag=0;
 }

 ~otl_var_desc(){}

 void copy_name(const char* nm)
 {
  if(!nm)
   name[0]=0;
  else{
   strncpy(name,nm,sizeof(name));
   name[sizeof(name)-1]=0;
  }
 }

};

const int otl_var_none=0;
const int otl_var_char=1;
const int otl_var_double=2;
const int otl_var_float=3;
const int otl_var_int=4;
const int otl_var_unsigned_int=5;
const int otl_var_short=6;
const int otl_var_long_int=7;
const int otl_var_timestamp=8;
const int otl_var_varchar_long=9;
const int otl_var_raw_long=10;
const int otl_var_clob=11;
const int otl_var_blob=12;
const int otl_var_refcur=13;
const int otl_var_long_string=15;
const int otl_var_db2time=16;
const int otl_var_db2date=17;
const int otl_var_tz_timestamp=18;
const int otl_var_ltz_timestamp=19;
const int otl_var_bigint=20;

class otl_long_string{
public:

 unsigned char* v;
 int length;
 int extern_buffer_flag;
 int buf_size;

 otl_long_string(const int buffer_size=32760,const int input_length=0)
 {
  extern_buffer_flag=0;
  length=input_length;
  buf_size=buffer_size;
  v=new unsigned char[buffer_size+1];
  memset(v,0,buffer_size);
 }

 otl_long_string
 (const void* external_buffer, 
  const int buffer_size,
  const int input_length=0)
 {
  extern_buffer_flag=1;
  length=input_length;
  buf_size=buffer_size;
  v=OTL_RCAST(unsigned char*, OTL_CCAST(void*, external_buffer));
 }

  otl_long_string& operator=(const otl_long_string& s)
  {
    if(s.extern_buffer_flag){
      if(!extern_buffer_flag)
        delete[] v;
      v=s.v;
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      buf_size=s.buf_size;
    }else{
      if(extern_buffer_flag){
        v=new unsigned char[s.buf_size];
        buf_size=s.buf_size;
      }else if(buf_size<s.buf_size){
        delete[] v;
        v=new unsigned char[s.buf_size];
        buf_size=s.buf_size;
      }
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      memcpy(v,s.v,length);
      if(length<buf_size&&s.v[length]==0)
        v[length]=0;
    }
    return *this;
  }

  otl_long_string(const otl_long_string& s)
  {
    length=s.length;
    extern_buffer_flag=s.extern_buffer_flag;
    buf_size=s.buf_size;
    if(s.extern_buffer_flag)
      v=s.v;
    else{
      v=new unsigned char[buf_size];
      memcpy(v,s.v,length);
      if(length<buf_size&&s.v[length]==0)
        v[length]=0;
    }
  }
  
  virtual ~otl_long_string()
  {
   if(!extern_buffer_flag)delete[] v;
  }

  void set_len(const int len=0){length=len;}
  int len(void)const {return length;}

  unsigned char& operator[](int ndx){return v[ndx];}

  virtual void null_terminate_string(const int len)
  {
    (*this)[len]=0;
  }


};

#ifdef OTL_UNICODE
class otl_long_unicode_string: public otl_long_string{
public:

  otl_long_unicode_string(const int buffer_size=32760,const int input_length=0)
  {
    extern_buffer_flag=0;
    length=input_length;
    buf_size=buffer_size;
    v=new unsigned char[(buffer_size+1)*sizeof(OTL_WCHAR)];
    memset(v,0,buffer_size*sizeof(OTL_WCHAR));
  }
  
  otl_long_unicode_string
  (const void* external_buffer, 
   const int buffer_size,
   const int input_length=0)
  {
    extern_buffer_flag=1;
    length=input_length;
    buf_size=buffer_size;
    v=OTL_RCAST(unsigned char*, OTL_CCAST(void*, external_buffer));
  }
  
  virtual ~otl_long_unicode_string(){}
  
  OTL_CHAR& operator[](int ndx)
  {
    return OTL_RCAST(OTL_CHAR*,v)[ndx];
  }

  virtual void null_terminate_string(const int len)
  {
    (*this)[len]=0;
  }

};

#endif

inline const char* otl_var_type_name(const int ftype)
{
  const char* const_CHAR="CHAR";
  const char* const_DOUBLE="DOUBLE";
  const char* const_FLOAT="FLOAT";
  const char* const_INT="INT";
  const char* const_UNSIGNED_INT="UNSIGNED INT";
  const char* const_SHORT_INT="SHORT INT";
  const char* const_LONG_INT="LONG INT";
  const char* const_TIMESTAMP="TIMESTAMP";
  const char* const_TZ_TIMESTAMP="TIMESTAMP WITH TIME ZONE";
  const char* const_LTZ_TIMESTAMP="TIMESTAMP WITH LOCAL TIME ZONE";
  const char* const_BIGINT="BIGINT";
  const char* const_VARCHAR_LONG="VARCHAR LONG";
  const char* const_RAW_LONG="RAW LONG";
  const char* const_CLOB="CLOB";
  const char* const_BLOB="BLOB";
  const char* const_UNKNOWN="";
  const char* const_LONG_STRING="otl_long_string()";

 switch(ftype){
 case otl_var_char:
  return const_CHAR;
 case otl_var_double:
  return const_DOUBLE;
 case otl_var_float:
  return const_FLOAT;
 case otl_var_int:
  return const_INT;
 case otl_var_unsigned_int:
  return const_UNSIGNED_INT;
 case otl_var_short:
  return const_SHORT_INT;
 case otl_var_long_int:
  return const_LONG_INT;
 case otl_var_timestamp:
  return const_TIMESTAMP;
 case otl_var_tz_timestamp:
  return const_TZ_TIMESTAMP;
 case otl_var_ltz_timestamp:
  return const_LTZ_TIMESTAMP;
 case otl_var_bigint:
  return const_BIGINT;
 case otl_var_varchar_long:
  return const_VARCHAR_LONG;
 case otl_var_raw_long:
  return const_RAW_LONG;
 case otl_var_clob:
  return const_CLOB;
 case otl_var_blob:
  return const_BLOB;
 case otl_var_long_string:
  return const_LONG_STRING;
 default:
  return const_UNKNOWN;
 }
}

inline void otl_var_info_var
(const char* name,
 const int ftype,
 const int type_code,
 char* var_info)
{char buf1[128];
 char buf2[128];
 strcpy(buf1,otl_var_type_name(ftype));
 strcpy(buf2,otl_var_type_name(type_code));
 strcpy(var_info,"Variable: ");
 strcat(var_info,name);
 strcat(var_info,"<");
 strcat(var_info,buf1);
 strcat(var_info,">, datatype in operator <</>>: ");
 strcat(var_info,buf2);
}

inline void otl_var_info_var2
(const char* name,
 const int ftype,
 char* var_info)
{char buf1[128];
 strcpy(buf1,otl_var_type_name(ftype));
 strcpy(var_info,"Variable: ");
 strcat(var_info,name);
 strcat(var_info,"<");
 strcat(var_info,buf1);
 strcat(var_info,">");
}


inline void otl_strcpy(
  unsigned char* trg,
  unsigned char* src,
  int& overflow,
  const int inp_size=0
)
{
#ifdef OTL_UNICODE
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
 }
 *c1=0;
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#else
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
 }
 *c1=0;
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#endif
}

inline void otl_strcpy(
  unsigned char* trg,
  unsigned char* src
)
{
#ifdef OTL_UNICODE
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 while(*c2){
  *c1=*c2;
  ++c1; ++c2;
 }
 *c1=0;
#else
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 while(*c2){
  *c1=*c2;
  ++c1; ++c2;
 }
 *c1=0;
#endif
}

#ifdef OTL_UNICODE
inline void otl_strcpy2(
  unsigned char* trg,
  unsigned char* src,
  const int max_src_len
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int src_len=OTL_SCAST(int,*OTL_SCAST(unsigned short*,c2));
 int len=0;
 ++c2;
 while(*c2&&len<max_src_len&&len<src_len){
  *c1=*c2;
  ++c1; 
  ++c2;
  ++len;
 }
 *c1=0;
#else
inline void otl_strcpy2(
  unsigned char* trg,
  unsigned char* src,
  const int /* max_src_len */
)
{
 otl_strcpy(trg,src);
#endif
}

#ifdef OTL_UNICODE
inline void otl_memcpy(
  unsigned char* trg,
  unsigned char* src,
  const int src_len,
  const int ftype
)
{

  if(ftype==otl_var_raw_long){
    memcpy(trg,src,src_len);
    return;
  }

 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int len=0;
 while(len<src_len){
  *c1=*c2;
  ++c1; 
  ++c2;
  ++len;
 }

#else
inline void otl_memcpy(
  unsigned char* trg,
  unsigned char* src,
  const int src_len,
  const int /* ftype */
)
{
 memcpy(trg,src,src_len);
#endif
}

#ifdef OTL_UNICODE
inline void otl_strcpy3(
  unsigned char* trg,
  unsigned char* src,
  const int max_src_len,
  int& overflow,
  const int inp_size=0
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int len=0;
 int src_len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c2));
 ++c2;
 int out_size=0;
 overflow=0;
 while(len<src_len&&len<max_src_len&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
  ++len;
 }
 *c1=0;
 if(len<src_len&&out_size==inp_size-1)
  overflow=1;
#else
inline void otl_strcpy3(
  unsigned char* trg,
  unsigned char* src,
  const int /* max_src_len */,
  int& overflow,
  const int inp_size=0
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
 }
 *c1=0;
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#endif
}

inline void otl_strcpy4(
  unsigned char* trg,
  unsigned char* src,
  int& overflow,
  const int inp_size=0
)
{
#ifdef OTL_UNICODE
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* bc1=c1;
 ++c1;
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
 }
 *OTL_RCAST(unsigned short*,bc1)=OTL_SCAST(unsigned short,out_size);
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#else
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1=*c2;
  ++c1; ++c2;
  ++out_size;
 }
 *c1=0;
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#endif
}

inline char* otl_itoa(int i,char* a)
{
  const char* digits="0123456789";
  int n=i;
  int k;
  char buf[64];
  char* c=buf;
  char *c1=a;
  int klen=0;
  char digit=' ';
  bool negative=false;
  if(n<0){
    n=-n;
    negative=true;
  }
  do{
    if(n>=10)
      k=n%10;
    else
      k=n;
    digit=digits[k];
    *c=digit;
    ++c;
    ++klen;
    n=n/10;
  }while(n!=0);
  *c=0;
  if(negative){
    *c1='-';
    ++c1;
  }
  for(int j=klen-1;j>=0;--j){
    *c1=buf[j];
    ++c1;
  }
  *c1=0;
  return c1;
}

inline void otl_var_info_col
(const int pos,
 const int ftype,
 const int type_code,
 char* var_info)
{
 char buf1[128];
 char buf2[128];
 char name[128];

 otl_itoa(pos,name);
 strcpy(buf1,otl_var_type_name(ftype));
 strcpy(buf2,otl_var_type_name(type_code));
 strcpy(var_info,"Column: ");
 strcat(var_info,name);
 strcat(var_info,"<");
 strcat(var_info,buf1);
 strcat(var_info,">, datatype in operator <</>>: ");
 strcat(var_info,buf2);
}

inline void otl_var_info_col2
(const int pos,
 const int ftype,
 char* var_info)
{
 char buf1[128];
 char name[128];

 otl_itoa(pos,name);
 strcpy(buf1,otl_var_type_name(ftype));
 strcpy(var_info,"Column: ");
 strcat(var_info,name);
 strcat(var_info,"<");
 strcat(var_info,buf1);
 strcat(var_info,">");
}

class otl_pl_tab_generic{
public:

 unsigned char* p_v;
 short* p_null;
 int elem_size;
 int tab_size;
 int tab_len;
 int vtype;

 otl_pl_tab_generic()
 {
  elem_size=0;
  tab_size=0;
  tab_len=0;
  p_v=0;
  p_null=0;
  vtype=0;
 }

 virtual ~otl_pl_tab_generic(){}

 unsigned char* val(int ndx=0)
 {
  return p_v+(ndx*elem_size);
 }

 int is_null(int ndx=0)
 {
  return p_null[ndx]!=0;
 }

 void set_null(int ndx=0)
 {
  p_null[ndx]=1;
 }

 void set_non_null(int ndx=0)
 {
  p_null[ndx]=0;
 }

 void init_generic(void)
 {int i;
  memset(p_v,0,elem_size*tab_len);
  for(i=0;i<tab_len;++i)
   p_null[i]=0;
 }

 int len()
 {
  return tab_len;
 }

 void set_len(int new_len=0)
 {
  tab_len=new_len;
 }

};

inline int otl_numeric_convert_int(
  const int ftype,
  const void* val,
  int& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(int,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(int,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(int,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(int,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(int,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(int,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(int,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

#if defined(OTL_BIGINT)
inline int otl_numeric_convert_bigint(
  const int ftype,
  const void* val,
  OTL_BIGINT& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(OTL_BIGINT,double,val);
  break;
 case otl_var_bigint:
  n=OTL_PCONV(OTL_BIGINT,OTL_BIGINT,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(OTL_BIGINT,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(OTL_BIGINT,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(OTL_BIGINT,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(OTL_BIGINT,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(OTL_BIGINT,float,val);
  break;
 default:
  rc=0;
  break;
 }
 return rc;
}
#endif

inline int otl_numeric_convert_unsigned(
  const int ftype,
  const void* val,
  unsigned& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(unsigned,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(unsigned,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(unsigned,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(unsigned,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(unsigned,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(unsigned,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(unsigned,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

inline int otl_numeric_convert_short(
  const int ftype,
  const void* val,
  short& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(short,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(short,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(short,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(short,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(short,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(short,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(short,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

inline int otl_numeric_convert_long_int(
  const int ftype,
  const void* val,
  long int& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(long int,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(long int,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(long int,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(long int,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(long int,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(long int,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(long int,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

inline int otl_numeric_convert_float(
  const int ftype,
  const void* val,
  float& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(float,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(float,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(float,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(float,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(float,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(float,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(float,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

inline int otl_numeric_convert_double(
  const int ftype,
  const void* val,
  double& n)
{int rc=1;
 switch(ftype){
 case otl_var_double:
  n=OTL_PCONV(double,double,val);
  break;
 case otl_var_short:
  n=OTL_PCONV(double,short,val);
  break;
 case otl_var_int:
  n=OTL_PCONV(double,int,val);
  break;
 case otl_var_unsigned_int:
  n=OTL_PCONV(double,unsigned int,val);
  break;
 case otl_var_long_int:
  n=OTL_PCONV(double,long int,val);
  break;
 case otl_var_float:
  n=OTL_PCONV(double,float,val);
  break;
#if defined(OTL_BIGINT)
 case otl_var_bigint:
  n=OTL_PCONV(double,OTL_BIGINT,val);
  break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;
}

#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)

class otl_ltstr{
public:
 
 bool operator()(const OTL_STRING_CONTAINER& s1, const OTL_STRING_CONTAINER& s2) const
 {
  return strcmp(s1.c_str(), s2.c_str()) < 0;
 }
 
};

const int otl_max_default_pool_size=32;

#endif

#ifdef OTL_ACE
const int otl_max_default_pool_size=32;
#endif


class otl_stream_shell_generic{
public:

 int should_delete;

 otl_stream_shell_generic()
 {
  should_delete=0;
 }

 virtual ~otl_stream_shell_generic(){}

};

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)

#if defined(OTL_STL)
#include <map>
#endif

class otl_stream_pool_entry{
public:

#ifdef OTL_ACE
 otl_tmpl_vector<otl_stream_shell_generic*> s;
#else
 STD_NAMESPACE_PREFIX vector<otl_stream_shell_generic*> s;
#endif

 int cnt;
 
 otl_stream_pool_entry()
 {
  cnt=0;
 }
 
 otl_stream_pool_entry(const otl_stream_pool_entry& sc)
 {
  s=sc.s;
  cnt=sc.cnt;
 }
 
 otl_stream_pool_entry& operator=(const otl_stream_pool_entry& sc)
 {
  s=sc.s;
  cnt=sc.cnt;
  return *this;
 }
 
 virtual ~otl_stream_pool_entry(){}
 
};

class otl_stream_pool{
public:
 
 typedef otl_stream_pool_entry cache_entry_type;
#ifdef OTL_ACE
 typedef
 ACE_RB_Tree
   <OTL_STRING_CONTAINER,cache_entry_type,
    ACE_Less_Than<OTL_STRING_CONTAINER>,
    ACE_Null_Mutex> sc_type;
 typedef otl_tmpl_vector<otl_stream_shell_generic*> vec_type;
 typedef ACE_RB_Tree_Node<OTL_STRING_CONTAINER,cache_entry_type> ace_map_entry;
#else
  typedef STD_NAMESPACE_PREFIX
  map<OTL_STRING_CONTAINER,cache_entry_type,otl_ltstr> sc_type;
  typedef STD_NAMESPACE_PREFIX vector<otl_stream_shell_generic*> vec_type;
#endif

 sc_type sc;
  int max_size;
 int size;
 
 otl_stream_pool()
 {
  max_size=otl_max_default_pool_size;
  size=0;
 }

 void init(const int amax_size=otl_max_default_pool_size)
 {
  if(size==0&&max_size==0)return;
#ifdef OTL_ACE
  sc_type::iterator elem0=sc.begin();
  sc_type::iterator elemN=sc.end();
  for(sc_type::iterator i=elem0; i!=elemN; ++i){
   cache_entry_type& ce=(*i).item();
   int sz=ce.s.size();
   for(int j=0;j<sz;++j){
    ce.s[j]->should_delete=1;
    delete ce.s[j];
    ce.s[j]=0;
   }
   ce.s.clear();
   ce.cnt=0;
  }
  sc.clear();
#else
  sc_type::iterator elem0=sc.begin();
  sc_type::iterator elemN=sc.end();
  for(sc_type::iterator i=elem0; i!=elemN; ++i){
   cache_entry_type& ce=(*i).second;
   int sz=ce.s.size();
   for(int j=0;j<sz;++j){
    ce.s[j]->should_delete=1;
    delete ce.s[j];
    ce.s[j]=0;
   }
   ce.s.clear();
   ce.cnt=0;
  }
  sc.clear();
#endif

  size=0;
  max_size=amax_size;

 }

 otl_stream_shell_generic* find(const OTL_STRING_CONTAINER& stmtxt)
 {
  otl_stream_shell_generic* s;
  
#ifdef OTL_ACE
  ace_map_entry* ce=0;
  int found=sc.find(stmtxt,ce);
  if(found==-1)return 0; // entry not found
  s=ce->item().s[ce->item().s.size()-1];
  ce->item().s.pop_back();
  if(ce->item().s.size()==0){
   sc.unbind(ce);
   --size;
  }
#else
  sc_type::iterator cur=sc.find(stmtxt);
  if(cur==sc.end())return 0; // entry not found
  cache_entry_type& ce=(*cur).second;
  s=ce.s[ce.s.size()-1];
  ce.s.pop_back();
  if(ce.s.size()==0){
   sc.erase(cur);
   --size;
  }
#endif

  return s;
 }

 void remove(const otl_stream_shell_generic* s,const OTL_STRING_CONTAINER& stmtxt)
 {
#ifdef OTL_ACE
  ace_map_entry* cur=0;
  int found=sc.find(stmtxt,cur);
  if(found==-1)
   return;
  cache_entry_type& ce=(*cur).item();
  for(int i=0;i<ce.s.size();++i)
   if(ce.s[i]==s){
    if(ce.s.size()>1 && i!=ce.s.size()-1){
     otl_stream_shell_generic* temp_s=ce.s[i];
     ce.s[i]=ce.s[ce.s.size()-1];
     ce.s[ce.s.size()-1]=temp_s;
    }
    ce.s.pop_back();
    --size;
    return;
   }
#else
  sc_type::iterator cur=sc.find(stmtxt);
  if(cur==sc.end())
   return;
  cache_entry_type& ce=(*cur).second;
  vec_type::iterator bgn=ce.s.begin();
  vec_type::iterator end=ce.s.end();
  for(vec_type::iterator i=bgn;i!=end;++i)
   if((*i)==s){
    ce.s.erase(i);
    --size;
    return;
   }
#endif
 }

 void add(otl_stream_shell_generic* s,const char* stm_text)
 {
  OTL_STRING_CONTAINER stmtxt(stm_text);

#ifdef OTL_ACE

  ace_map_entry* cur=0;
  int found_in_map=sc.find(stmtxt,cur);
  if(found_in_map==0){ // entry found
   bool found=false;
   cache_entry_type& ce=(*cur).item();
   int sz=ce.s.size();
   for(int i=0;i<sz;++i){
    if(s==ce.s[i]){
     found=true;
     break;
    }
   }
   if(!found)ce.s.push_back(s);
   ++ce.cnt;
  }else{ // entry not found
   if(size<max_size-1){ // add new entry
    cache_entry_type ce;
    ce.s.push_back(s);
    ce.cnt=1;
    sc.bind(stmtxt,ce);
    ++size;
   }else{ // erase the least used entry and add new one

    sc_type::iterator elem0=sc.begin();
    sc_type::iterator elemN=sc.end();
    int min_cnt=0;
    ace_map_entry* min_entry;
    
    for(sc_type::iterator i=elem0;i!=elemN;++i){
     if(i==elem0){ // first element
      min_entry=&(*i);
      min_cnt=(*i).item().cnt;
     }
     if(min_cnt>(*i).item().cnt){ // found less used entry
      min_entry=&(*i);
      min_cnt=(*i).item().cnt;
     }
    }
    cache_entry_type& me=(*min_entry).item();
    int sz=me.s.size();
    for(int n=0;n<sz;++n){
     me.s[n]->should_delete=1;
     delete me.s[n];
    }
    me.s.clear();
    sc.unbind(min_entry);
    cache_entry_type ce;
    ce.cnt=1;
    ce.s.push_back(s);
    sc.bind(stmtxt,ce);
   }
  }

#else

  sc_type::iterator cur=sc.find(stmtxt);

  if(cur!=sc.end()){ // entry found
   bool found=false;
   cache_entry_type& ce=(*cur).second;
   int sz=ce.s.size();
   for(int i=0;i<sz;++i){
    if(s==ce.s[i]){
     found=true;
     break;
    }
   }
   if(!found)ce.s.push_back(s);
   ++ce.cnt;
  }else{ // entry not found
   if(size<max_size-1){ // add new entry
    cache_entry_type ce;
    ce.s.push_back(s);
    ce.cnt=1;
    sc[stmtxt]=ce;
    ++size;
   }else{ // erase the least used entry and add new one

    sc_type::iterator elem0=sc.begin();
    sc_type::iterator elemN=sc.end();
    int min_cnt=0;
    sc_type::iterator min_entry;
    
    for(sc_type::iterator i=elem0;i!=elemN;++i){
     if(i==elem0){ // first element
      min_entry=i;
      min_cnt=(*i).second.cnt;
     }
     if(min_cnt>(*i).second.cnt){ // found less used entry
      min_entry=i;
      min_cnt=(*i).second.cnt;
     }
    }
    cache_entry_type& me=(*min_entry).second;
    int sz=me.s.size();
    for(int n=0;n<sz;++n){
     me.s[n]->should_delete=1;
     delete me.s[n];
    }
    me.s.clear();
    sc.erase(min_entry);
    cache_entry_type ce;
    ce.cnt=1;
    ce.s.push_back(s);
    sc[stmtxt]=ce;
   }
  }
#endif
 }
 
 virtual ~otl_stream_pool()
 {
  init();
 }
 
};

#endif


// =========================== COMMON TEMPLATES  ============================


#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)

template <OTL_TYPE_NAME TData>
class otl_value{
public:

 TData v;
 bool ind;

 otl_value(){ind=true;}
 virtual ~otl_value(){}

 otl_value(const otl_value<TData>& var)
 {
  v=var.v;
  ind=var.ind;
 }

 otl_value(const TData& var)
 {
  v=var;
  ind=false;
 }

 otl_value(const otl_null&)
 {
  ind=true;
 }

 otl_value<TData>& operator=(const otl_value<TData>& var)
 {
  v=var.v;
  ind=var.ind;
  return *this;
 }

 otl_value<TData>& operator=(const TData& var)
 {
  v=var;
  ind=false;
  return *this;
 }

 otl_value<TData>& operator=(const otl_null&)
 {
  ind=true;
  return *this;
 }

 bool is_null(void)const {return ind;}
 void set_null(void){ind=true;}
 void set_non_null(void){ind=false;}

};

#endif

template <OTL_TYPE_NAME T>
class otl_auto_array_ptr{
public:

 T* ptr;

 otl_auto_array_ptr()
 {
  ptr=0;
 }

 otl_auto_array_ptr(const int arr_size)
 {
  ptr=new T[arr_size];
 }

 virtual ~otl_auto_array_ptr()
 {
  delete[] ptr;
 }

};

template <OTL_TYPE_NAME T>
class otl_ptr{
public:

 T** ptr;
 int arr_flag;

 otl_ptr()
 {
  ptr=0;
  arr_flag=0;
 }

 void assign(T** var)
 {
  ptr=var;
  arr_flag=0;
 }

 void assign_array(T** var)
 {
  ptr=var;
  arr_flag=1;
 }


 void disconnect(void)
 {
  if(ptr!=0)
   *ptr=0;
  ptr=0;
 }

 void destroy(void)
 {
  if(ptr==0)return;
  if(*ptr!=0){
   if(arr_flag)
    delete[] *ptr;
   else
    delete *ptr;
   *ptr=0;
  }
 }

 ~otl_ptr()
 {
  destroy();
 }

};

template <OTL_TYPE_NAME T>
class otl_Tptr{
public:

 T* ptr;

 otl_Tptr()
 {
  ptr=0;
 }

 void assign(T* var)
 {
  ptr=var;
 }

 void disconnect(void)
 {
  ptr=0;
 }

 void destroy(void)
 {
  delete ptr;
  ptr=0;
 }

 ~otl_Tptr()
 {
  destroy();
 }

};


template <OTL_TYPE_NAME OTLStream,
          OTL_TYPE_NAME OTLConnect,
          OTL_TYPE_NAME otl_exception>
class otl_tmpl_nocommit_stream: public OTLStream{
public:

 otl_tmpl_nocommit_stream() OTL_NO_THROW
   : OTLStream()
 {
  OTLStream::set_commit(0);
 }

 otl_tmpl_nocommit_stream
 (const short arr_size,
  const char* sqlstm,
  OTLConnect& db,
  const char* ref_cur_placeholder=0)
   OTL_THROWS_OTL_EXCEPTION
  : OTLStream(arr_size,sqlstm,db,ref_cur_placeholder)
 {
  OTLStream::set_commit(0);
 }

 void open
 (short int arr_size,
  const char* sqlstm,
  OTLConnect& db,
  const char* ref_cur_placeholder=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  OTLStream::open(arr_size,sqlstm,db,ref_cur_placeholder);
  OTLStream::set_commit(0);
 }

};

#if defined(OTL_STL)

class otl_pl_vec_generic{
public:

 typedef STD_NAMESPACE_PREFIX vector<bool> null_flag_type;

 void* p_v;
 null_flag_type null_flag;
 int vtype;
 int elem_size;

 otl_pl_vec_generic()
 {
  p_v=0;
  vtype=0;
  elem_size=0;
 }

 virtual int len(void) const
 {
  return 0;
 }

  virtual void set_len(const int /*new_len*/=0,
                       const bool /*set_all_to_null*/=true)

 {
   
 }

 bool is_null(const int ndx=0)
 {
  return null_flag[ndx];
 }

 void set_null(const int ndx=0)
 {
  null_flag[ndx]=true;
 }

 void set_non_null(const int ndx=0)
 {
  null_flag[ndx]=false;
 }

 virtual ~otl_pl_vec_generic(){}

};

class otl_int_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<int> v;

 otl_int_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_int;
  this->elem_size=sizeof(int);
 }

 virtual ~otl_int_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 int& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_double_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<double> v;

 otl_double_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_double;
  this->elem_size=sizeof(double);
 }

 virtual ~otl_double_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 double& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_float_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<float> v;

 otl_float_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_float;
  this->elem_size=sizeof(float);
 }

 virtual ~otl_float_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 float& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_short_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<short int> v;

 otl_short_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_short;
  this->elem_size=sizeof(short int);
 }

 virtual ~otl_short_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 short int& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_long_int_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<long int> v;

 otl_long_int_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_long_int;
  this->elem_size=sizeof(long int);
 }

 virtual ~otl_long_int_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 long int& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_string_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<OTL_STRING_CONTAINER> v;

 otl_string_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_char;
  this->elem_size=1;
 }

 virtual ~otl_string_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null==true)
   for(i=0;i<vsize;++i)
    this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 OTL_STRING_CONTAINER& operator[](int ndx)
 {
  return v[ndx];
 }

};

class otl_datetime_vec: public otl_pl_vec_generic{
public:

 STD_NAMESPACE_PREFIX vector<otl_datetime> v;

 otl_datetime_vec()
 {
  this->p_v=OTL_RCAST(void*,&v);
  this->vtype=otl_var_timestamp;
  this->elem_size=sizeof(otl_oracle_date);
 }

 virtual ~otl_datetime_vec(){}

 virtual void set_len(const int new_len=0,
                      const bool set_all_to_null=true)

 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=v.size();
  if(set_all_to_null)
   for(i=0;i<vsize;++i)
   this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return v.size();
 }

 otl_datetime& operator[](int ndx)
 {
  return v[ndx];
 }

};

#endif

template <OTL_TYPE_NAME T,const int atab_size,const int avtype>
class otl_tmpl_pl_tab: public otl_pl_tab_generic{
public:
 T v[atab_size];
 short null_flag[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=avtype;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,sizeof(v));
 }

 otl_tmpl_pl_tab()
 {
  init();
 }

 virtual ~otl_tmpl_pl_tab(){}

};

template <const int atab_size>
class otl_int_tab: public otl_tmpl_pl_tab<int,atab_size,otl_var_int>{
public:
 otl_int_tab():otl_tmpl_pl_tab<int,atab_size,otl_var_int>(){}
};

template <const int atab_size>
class otl_double_tab: public otl_tmpl_pl_tab<double,atab_size,otl_var_double>{
public:
 otl_double_tab():otl_tmpl_pl_tab<double,atab_size,otl_var_double>(){}
};

template <const int atab_size>
class otl_float_tab: public otl_tmpl_pl_tab<float,atab_size,otl_var_float>{
public:
 otl_float_tab():otl_tmpl_pl_tab<float,atab_size,otl_var_float>(){}
};

template <const int atab_size>
class otl_unsigned_tab: public otl_tmpl_pl_tab<unsigned,atab_size,otl_var_unsigned_int>{
public:
 otl_unsigned_tab():otl_tmpl_pl_tab<unsigned,atab_size,otl_var_unsigned_int>(){}
};

template <const int atab_size>
class otl_short_tab: public otl_tmpl_pl_tab<short,atab_size,otl_var_short>{
public:
 otl_short_tab():otl_tmpl_pl_tab<short,atab_size,otl_var_short>(){}
};

template <const int atab_size>
class otl_long_int_tab: public otl_tmpl_pl_tab<long,atab_size,otl_var_long_int>{
public:
 otl_long_int_tab():otl_tmpl_pl_tab<long,atab_size,otl_var_long_int>(){}
};

template <const int atab_size,const int str_size>
class otl_cstr_tab: public otl_pl_tab_generic{
public:
 typedef unsigned char T[str_size];
 T v[atab_size];
 short null_flag[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=otl_var_char;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,sizeof(v));
 }

 otl_cstr_tab()
 {
  init();
 }

 virtual ~otl_cstr_tab(){}

};

template <const int atab_size>
class otl_datetime_tab: public otl_pl_tab_generic{
public:

 typedef otl_datetime T;

 T v[atab_size];
 short null_flag[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=otl_var_timestamp;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(otl_oracle_date);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
 }

 otl_datetime_tab()
 {
  init();
 }

 virtual ~otl_datetime_tab(){}

};

template <OTL_TYPE_NAME T,const int avtype>
class otl_tmpl_dyn_pl_tab: public otl_pl_tab_generic{
public:
 T* v;
 short* null_flag;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=avtype;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=(unsigned char*)v;
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,elem_size*tab_size);
 }

 otl_tmpl_dyn_pl_tab(const int atab_size=1)
 {
  v=0;
  null_flag=0;
  init(atab_size);
 }

 virtual ~otl_tmpl_dyn_pl_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

};

class otl_dynamic_int_tab: public otl_tmpl_dyn_pl_tab<int,otl_var_int>{
public:
 otl_dynamic_int_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<int,otl_var_int>(atab_size){}
};

class otl_dynamic_double_tab: public otl_tmpl_dyn_pl_tab<double,otl_var_double>{
public:
 otl_dynamic_double_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<double,otl_var_double>(atab_size){}
};

class otl_dynamic_float_tab: public otl_tmpl_dyn_pl_tab<float,otl_var_float>{
public:
 otl_dynamic_float_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<float,otl_var_float>(atab_size){}
};

class otl_dynamic_unsigned_tab: public
otl_tmpl_dyn_pl_tab<unsigned,otl_var_unsigned_int>{
public:
 otl_dynamic_unsigned_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<unsigned,otl_var_unsigned_int>(atab_size){}
};

class otl_dynamic_short_tab: public otl_tmpl_dyn_pl_tab<short,otl_var_short>{
public:
 otl_dynamic_short_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<short,otl_var_short>(atab_size){}
};

class otl_dynamic_long_int_tab: public otl_tmpl_dyn_pl_tab<long,otl_var_long_int>{
public:
 otl_dynamic_long_int_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<long,otl_var_long_int>(atab_size){}
};

template <const int str_size>
class otl_dynamic_cstr_tab: public otl_pl_tab_generic{
public:
 typedef unsigned char T[str_size];
 T* v;
 short* null_flag;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=otl_var_char;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=(unsigned char*)v;
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,elem_size*tab_size);
 }

 otl_dynamic_cstr_tab(const int atab_size=1)
 {
  v=0;
  null_flag=0;
  init(atab_size);
 }

 virtual ~otl_dynamic_cstr_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

};

class otl_dynamic_datetime_tab: public otl_pl_tab_generic{
public:

 typedef otl_datetime T;

 T* v;
 short* null_flag;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=otl_var_timestamp;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=(unsigned char*)v;
  elem_size=sizeof(otl_oracle_date);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
 }

 otl_dynamic_datetime_tab(const int atab_size=1)
 {
  v=0;
  null_flag=0;
  init(atab_size);
 }

 virtual ~otl_dynamic_datetime_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct>
#if defined(OTL_EXCEPTION_DERIVED_FROM)
class otl_tmpl_exception: 
  public OTL_EXCEPTION_DERIVED_FROM,
  public TExceptionStruct{
#else
class otl_tmpl_exception: public TExceptionStruct{
#endif
public:

#if defined(OTL_EXCEPTION_HAS_MEMBERS)
  OTL_EXCEPTION_HAS_MEMBERS
#endif

 char stm_text[2048];
 char var_info[256];

  otl_tmpl_exception() OTL_NO_THROW
 {
  stm_text[0]=0;
  var_info[0]=0;
 }

 otl_tmpl_exception(TConnectStruct& conn_struct, const char* sqlstm=0)
   OTL_NO_THROW
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
   strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
   stm_text[sizeof(stm_text)-1]=0;
  }
  conn_struct.error(OTL_SCAST(TExceptionStruct&,*this));
 }

 otl_tmpl_exception(TCursorStruct& cursor_struct, const char* sqlstm=0)
   OTL_NO_THROW
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
   strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
   stm_text[sizeof(stm_text)-1]=0;
  }
  cursor_struct.error(OTL_SCAST(TExceptionStruct&,*this));
 }

 otl_tmpl_exception
 (const char* amsg,
  const int acode,
  const char* sqlstm=0,
  const char* varinfo=0)
   OTL_NO_THROW
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
   strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
   stm_text[sizeof(stm_text)-1]=0;
  }
  if(varinfo)strcpy(OTL_RCAST(char*,var_info),varinfo);
  TExceptionStruct::init(amsg,acode);
 }

 virtual ~otl_tmpl_exception() OTL_NO_THROW
  {}

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct>
class otl_tmpl_connect{
public:

 int connected;
 TConnectStruct connect_struct;
 int long_max_size;
 int retcode;
 int throw_count;


 void set_max_long_size(const int amax_size)
 {
  reset_throw_count();
#ifdef OTL_UNICODE
  long_max_size=amax_size*sizeof(OTL_WCHAR);
#else
  long_max_size=amax_size;
#endif
 }

 int get_max_long_size(void)
 {
  reset_throw_count();
  return long_max_size;
 }

 void set_timeout(const int atimeout=0)
 {
  reset_throw_count();
  connect_struct.set_timeout(atimeout);
 }

 void set_cursor_type(const int acursor_type=0)
 {
  reset_throw_count();
  connect_struct.set_cursor_type(acursor_type);
 }

 void reset_throw_count(void)
 {
  throw_count=0;
 }

 otl_tmpl_connect()
 {
  throw_count=0;
  connected=0;
  long_max_size=32760;
  retcode=1;
 }

 otl_tmpl_connect(const char* connect_str,const int auto_commit=0)
 {
  connected=0;
  throw_count=0;
  retcode=1;
  long_max_size=32760;
  rlogon(connect_str,auto_commit);
 }

 virtual ~otl_tmpl_connect()
 {
  logoff();
 }

 static int otl_initialize(const int threaded_mode=0)
 {
  return TConnectStruct::initialize(threaded_mode);
 }

 void rlogon(const char* connect_str,const int auto_commit=0)
 {
  throw_count=0;
  retcode=connect_struct.rlogon(connect_str,auto_commit);
  if(retcode)
   connected=1;
  else{
   connected=0;
   ++throw_count;
  if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
  }
 }

 void logoff(void)
 {
  if(!connected)return;
  retcode=connect_struct.logoff();
  connected=0;
  if(retcode)return;
  if(throw_count>0)
   return;
  ++throw_count;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
 }

 void commit(void)
 {
  if(!connected)return;
  reset_throw_count();
  retcode=connect_struct.commit();
  if(retcode)return;
  ++throw_count;
  if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
 }

 void auto_commit_on(void)
 {
  if(!connected)return;
  reset_throw_count();
  retcode=connect_struct.auto_commit_on();
  if(retcode)return;
  ++throw_count;
  if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
 }

 void auto_commit_off(void)
 {
  if(!connected)return;
  reset_throw_count();
  retcode=connect_struct.auto_commit_off();
  if(retcode)return;
  ++throw_count;
  if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
 }

 void rollback(void)
 {
  if(!connected)return;
  reset_throw_count();
  retcode=connect_struct.rollback();
  if(retcode)return;
  ++throw_count;
  if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect_struct);
 }

};

template <OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_variable{
public:

 int param_type;
 int ftype;
 int elem_size;
 int array_size;
 char* name;
 int pos;
 int name_pos;
 int bound;

 int pl_tab_flag;

 TVariableStruct var_struct;


 int actual_elem_size(void)
 {
  return var_struct.actual_elem_size();
 }

 void copy_var_desc(otl_var_desc& v)
 {
  v.param_type=param_type;
  v.ftype=ftype;
  v.elem_size=elem_size;
  v.array_size=array_size;
  v.pos=pos;
  v.name_pos=name_pos;
  if(name){
   strncpy(v.name,name,sizeof(v.name));
   v.name[sizeof(v.name)-1]=0;
  }else
   v.name[0]=0;
  v.pl_tab_flag=pl_tab_flag;
 }

 otl_tmpl_variable()
 {
  name=0;
  pos=0;
  name_pos=0;
  pl_tab_flag=0;
  bound=0;
  param_type=otl_input_param;
 }

 virtual ~otl_tmpl_variable()
 {
  delete[] name;
 }

 otl_tmpl_variable
 (const int column_num,
  const int aftype,
  const int aelem_size,
  const short aarray_size)
 {
  copy_pos(column_num);
  init(aftype,aelem_size,aarray_size);
 }

 otl_tmpl_variable
 (const char* aname,
  const int aftype,
  const int aelem_size,
  const short aarray_size,
  const int apl_tab_flag=0)
 {
  copy_name(aname);
  init
   (aftype,
    aelem_size,
    aarray_size,
    0,
    apl_tab_flag);
 }

 void init
 (const int aftype,
  const int aelem_size,
  const short aarray_size,
  const void* connect_struct=0,
  const int apl_tab_flag=0)
 {
  ftype=aftype;
  elem_size=aelem_size;
  array_size=aarray_size;
  pl_tab_flag=apl_tab_flag;
  bound=0;
  var_struct.init(aftype,elem_size,aarray_size,connect_struct,pl_tab_flag);
 }

 void set_param_type(const int aparam_type=otl_input_param)
 {
  param_type=aparam_type;
 }

 int get_param_type(void)
 {
  return param_type;
 }

 void copy_name(const char* aname)
 {
  pos=0;
  if(name==aname)return;
  if(name)delete[] name;
  name=new char[strlen(aname)+1];
  strcpy(name,aname);
 }

 void copy_pos(const int apos)
 {
  if(name){
   delete[] name;
   name=0;
   name_pos=0;
  }
  pos=apos;
 }

  
 void set_null(int ndx)
 {
  var_struct.set_null(ndx);
 }

 void set_not_null(int ndx)
 {
  var_struct.set_not_null(ndx,elem_size);
 }

 void set_len(int len, int ndx=0)
 {
  var_struct.set_len(len,ndx);
 }

 int get_len(int ndx=0)
 {
  return var_struct.get_len(ndx);
 }

 int get_pl_tab_len(void)
 {
  return this->var_struct.get_pl_tab_len();
 }

 int get_max_pl_tab_len(void)
 {
  return this->var_struct.get_max_pl_tab_len();
 }

 void set_pl_tab_len(const int pl_tab_len)
 {
  this->var_struct.set_pl_tab_len(pl_tab_len);
 }

 int is_null(int ndx)
 {
  return var_struct.is_null(ndx);
 }

 void* val(int ndx=0)
 {
  return var_struct.val(ndx,elem_size);
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& ftype,
  int& elem_size,
  otl_select_struct_override& override,
  const int column_ndx)
 {
  TVariableStruct::map_ftype(desc,max_long_size,ftype,elem_size,override,column_ndx);
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_cursor{
public:

  int connected;
  char* stm_text;
  char*stm_label;
  
 TCursorStruct cursor_struct;
 int vl_len;
 otl_tmpl_variable<TVariableStruct>** vl;
 otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>* adb;
 int eof_data;
 int eof_desc;
 int retcode;
 long _rpc;
 int in_destructor;

 otl_tmpl_cursor()
 {
  in_destructor=0;
  connected=0;
  stm_label=0;
  stm_text=0;
  vl_len=0;
  vl=0;
  eof_data=0;
  eof_desc=0;
  adb=0;
  _rpc=0;
  retcode=1;
 }

 otl_tmpl_cursor
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& connect)
 {
  in_destructor=0;
  connected=0;
  stm_text=0;
  stm_label=0;
  vl_len=0;
  vl=0;
  eof_data=0;
  eof_desc=0;
  retcode=1;
  _rpc=0;
  adb=&connect;
  open(connect);
 }

 otl_tmpl_cursor
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& connect,
  TVariableStruct* var)
 {
  in_destructor=0;
  connected=0;
  stm_text=0;
  stm_label=0;
  vl_len=0;
  vl=0;
  eof_data=0;
  eof_desc=0;
  retcode=1;
  _rpc=0;
  adb=&connect;
  open(connect,var);
 }

 virtual ~otl_tmpl_cursor()
 {
  in_destructor=1;
  close();
  delete[] stm_label;
  stm_label=0;
  delete[] stm_text;
  stm_text=0;
 }

 void open
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& connect,
  TVariableStruct* var=0)
 {
  in_destructor=0;
  eof_data=0;
  eof_desc=0;
  retcode=1;
  adb=&connect;
  _rpc=0;
  if(var==0)
   retcode=cursor_struct.open(connect.connect_struct);
  else
   retcode=cursor_struct.open(connect.connect_struct,var);
  if(retcode){
   connected=1;
   return;
  }
  if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct);
 }

 virtual void close(void)
 {_rpc=0;
  if(!connected)return;
  if(!this->adb)return;
  if(!adb->connected){
   connected=0;
   adb=0;
   retcode=1;
   return;
  }
  connected=0;
  retcode=cursor_struct.close();
  if(retcode){
   adb=0;
   return;
  }
  if(this->adb->throw_count>0){
   adb=0;
   return;
  }
  this->adb->throw_count++;
  adb=0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct);
 }

 void parse(void)
 {_rpc=0;
  if(!connected)return;
  retcode=cursor_struct.parse(stm_text);
  switch(retcode){
  case 0:
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_tmpl_exception
      <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>(cursor_struct,stm_label?stm_label:stm_text);
  case 2:
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    char var_info[1];
    var_info[0]=0;
    throw otl_tmpl_exception
     <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
     (otl_error_msg_17,
      otl_error_code_17,
      this->stm_label?this->stm_label:this->stm_text,
      var_info);
  }
 }

 void parse(const char* sqlstm)
 {
  if(!connected)return;
  if(stm_text){
   delete[] stm_text;
   stm_text=0;
  }
  stm_text=new char[strlen(sqlstm)+1];
  strcpy(stm_text,sqlstm);
  parse();
 }

 long get_rpc()
 {
  return _rpc;
 }

 void exec(const int iters=1,const int rowoff=0)
 {
  typedef otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct> otl_exception;

  if(!connected)return;
  retcode=cursor_struct.exec(iters,rowoff);
  _rpc=cursor_struct.get_rpc();
  if(retcode)return;
  if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct,stm_label?stm_label:stm_text);
 }

  virtual bool valid_binding
  (const otl_tmpl_variable<TVariableStruct>& v,
   const int binding_type)
  {
    bool rc=true;
    if((v.ftype==otl_var_varchar_long||v.ftype==otl_var_raw_long) &&
       (v.var_struct.otl_adapter==otl_ora7_adapter ||
        v.var_struct.otl_adapter==otl_ora8_adapter) &&
       v.array_size>1 ||
       (v.ftype==otl_var_blob||v.ftype==otl_var_clob) &&
       v.var_struct.otl_adapter==otl_ora8_adapter &&
       v.array_size>1 && binding_type==otl_inout_binding) 
      rc=false;
    return rc;
  }

 virtual void bind
 (const char* name,
  otl_tmpl_variable<TVariableStruct>& v)
 {
  if(!connected)return;
  if(v.bound)return;
  v.copy_name(name);
  if(!valid_binding(v,otl_inout_binding)){
    char var_info[256];
    otl_var_info_var2
      (v.name,
       v.ftype,
       var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
     (otl_error_msg_16,
      otl_error_code_16,
      stm_label?stm_label:stm_text,
      var_info);
  }
  retcode=cursor_struct.bind
   (name,
    v.var_struct,
    v.elem_size,
    v.ftype,
    v.param_type,
    v.name_pos,
    v.pl_tab_flag);
  if(retcode){
   v.bound=1;
   return;
  }
  if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct,stm_label?stm_label:stm_text);
 }

 virtual void bind
 (const int column_num,
  otl_tmpl_variable<TVariableStruct>& v)
 {
  if(!connected)return;
  v.copy_pos(column_num);
  if(!valid_binding(v,otl_select_binding)){
    char var_info[256];
    otl_var_info_col2
      (v.pos,
       v.ftype,
       var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
     (otl_error_msg_16,
      otl_error_code_16,
      stm_label?stm_label:stm_text,
      var_info);
  }
  retcode=cursor_struct.bind
   (column_num,
    v.var_struct,
    v.elem_size,
    v.ftype,
    v.param_type);
  if(retcode)return;
  if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct,stm_label?stm_label:stm_text);
 }

 virtual void bind(otl_tmpl_variable<TVariableStruct>& v)
 {
  if(!connected)return;
  if(v.name) bind(v.name,v);
  if(v.pos) bind(v.pos,v);
 }

 static long direct_exec
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& connect,
  const char* sqlstm,
  const int exception_enabled=1)
#if defined(OTL_ANSI_CPP) && defined(OTL_FUNC_THROW_SPEC_ON)
   throw(otl_tmpl_exception
         <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>)
#endif
 {
  connect.reset_throw_count();
  try{
   otl_tmpl_cursor
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct> cur(connect);
   cur.cursor_struct.set_direct_exec(1);
   cur.parse(sqlstm);
   cur.exec();
   return cur.cursor_struct.get_rpc();
  }catch(otl_tmpl_exception
          <TExceptionStruct,
           TConnectStruct,
           TCursorStruct>){
   if(exception_enabled){
    connect.throw_count++;
    throw;
   }
  }
  return -1;
 }

 virtual int eof(void){return eof_data;}

 int describe_column
 (otl_column_desc& col,
  const int column_num)
 {
  if(!connected)return 0;
  retcode=cursor_struct.describe_column
   (col,column_num,eof_desc);
  if(eof_desc)return 0;
  if(retcode)return 1;
  if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(cursor_struct,stm_label?stm_label:stm_text);
 }

};

inline char otl_to_upper(char c)
{
 return OTL_SCAST(char,toupper(c));
}

template <OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TTimestampStruct,
          OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct>
class otl_tmpl_ext_hv_decl{
public:

 enum var_status{
  in=0,
  out=1,
  io=2,
  def=3
 };

  char* hv[otl_var_list_size];
  short inout[otl_var_list_size];
  int pl_tab_size[otl_var_list_size];
  int array_size;
  short vst[4];
  int len;
  char* stm_text_;
  char* stm_label_;

 otl_tmpl_ext_hv_decl(char* stm,int arr_size=1,char* label=0)
 {int j;
  array_size=arr_size;

  stm_text_=stm;
  stm_label_=label;
  int i=0;
  short in_str=0;
  bool in_comment=false;
  bool in_one_line_comment=false;
  char *c=stm;

  hv[i]=0;
  while(*c){
    switch(*c){
    case '\'':
      if(!in_comment&&!in_one_line_comment){
        if(!in_str)
          in_str=1;
        else{
          if(c[1]=='\'')
            ++c;
          else
            in_str=0;
        }
      }
      break;
    case '/':
      if(c[1]=='*'){
        in_comment=true;
        ++c;
      }
      break;
    case '-':
      if(c[1]=='-'&&!in_str){
        in_one_line_comment=true;
        ++c;
      }
      break;
    case '*':
      if(c[1]=='/'&&in_comment){
        in_comment=false;
        ++c;
      }      
      break;
    case '\n':
      if(in_one_line_comment)
        in_one_line_comment=false;
      else if(in_str)
        in_str=0;
      break;
    }
    if(*c==':' && !in_str && !in_comment && !in_one_line_comment &&
       (c>stm && *(c-1)!='\\' || c==stm)){
      short in_out=def;
      int apl_tab_size=0;
      char var[64];
      char* v=var;
      *v++=*c++;
      while(is_id(*c))
        *v++=*c++;
      while(isspace(*c)&&*c)
        ++c;
      if(*c=='<'){
        *c=' ';
        while(*c!='>'&&*c!=','&&*c){
          *v++=*c;
          *c++=' ';
        }
        if(*c==','){
          *c++=' ';
          if(otl_to_upper(*c)=='I'){
            if(otl_to_upper(c[2])=='O')
              in_out=io;
            else
              in_out=in;
          }else if(otl_to_upper(*c)=='O')
            in_out=out;
          while(*c!='>'&&*c&&*c!='[')
            *c++=' ';
          if(*c=='['){
            char tmp[32];
            char *t=tmp;
            *c++=' ';
            while(*c!=']'&&*c!='>'&&*c){
              *t++=*c;
              *c++=' ';
            }
            *t='\0';
            apl_tab_size=atoi(tmp);
            while(*c!='>'&&*c)
              *c++=' ';
          }
        }
        if(*c)*c=' ';
        *v='\0';
        add_var(i,var,in_out,apl_tab_size);
      }
    }
    if(*c)++c;
  }
  for(j=0;j<4;++j)vst[j]=0;
  i=0;
  while(hv[i]){
    switch(inout[i]){
    case in:
      ++vst[0];
      break;
    case out:
      ++vst[1];
      break;
    case io:
      ++vst[2];
      break;
    case def:
      ++vst[3];
      break;
    }
    ++i;
  }
  len=i;
 }
  
 virtual ~otl_tmpl_ext_hv_decl()
 {int i;
  for(i=0;hv[i]!=0;++i)
   delete[] hv[i];
 }


 char* operator[](int ndx){return hv[ndx];}
 short v_status(int ndx){return inout[ndx];}
 int is_id(char c){return isalnum(c)||c=='_';}

 int name_comp(char* n1,char* n2)
 {
  while(*n1!=' '&&*n1!='\0'&&*n2!=' '&&*n2!='\0'){
   if(otl_to_upper(*n1)!=otl_to_upper(*n2))return 0;
   ++n1;
   ++n2;
  }
  if(*n1==' '&&*n2!=' '||*n2==' '&&*n1!=' ')
   return 0;
  return 1;
 }

 void add_var(int &n,char* v,short in_out,int apl_tab_size=0)
 {int i;
  for(i=0;i<n;++i)
   if(name_comp(hv[i],v))
    return;
  hv[n]=new char[strlen(v)+1];
  strcpy(hv[n],v);
  inout[n]=in_out;
  pl_tab_size[n]=apl_tab_size;
  hv[++n]=0;
  inout[n]=def;
  pl_tab_size[n]=0;
 }

 otl_tmpl_variable<TVariableStruct>* alloc_var
 (char* s,
  const int vstat,
  const int status,
  otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const int apl_tab_size=0)
 {
  char name[128];
#ifdef OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON
  char type_arr[256];
#endif
  char type;
  char t2;
  char t3;
  char t4;
  int size=0;

  char *c=name,*c1=s;
  while(*c1!=' '&&*c1)
   *c++=*c1++;
  *c='\0';
  while(*c1==' '&&*c1)
   ++c1;

#ifdef OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON
  char* ct=c1;
  char* tac=type_arr;
  size_t ta_len=0;
  while(*ct && *ct!='[' && ta_len<sizeof(type_arr)){
    *tac=otl_to_upper(*ct);
    ++ct;
    ++tac;
    ++ta_len;
  }
  *tac=0;
#endif

  type=otl_to_upper(c1[0]);
  t2=otl_to_upper(c1[1]);
  t3=otl_to_upper(c1[2]);
  t4=otl_to_upper(c1[3]);
  if(type=='C'&&t2=='H'){
   char tmp[32];
   char *t=tmp;
   while(*c1!='['&&*c1)
    ++c1;
   ++c1;
   while(*c1!=']'&&*c1)
    *t++=*c1++;
   *t='\0';
   size=atoi(tmp);
#if defined(OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE)
   size+=1;
#endif
  }

  if(status==in && (vstat==in||vstat==io))
   ;
  else if(status==out && (vstat==out||vstat==io||vstat==def))
   ;
  else if(status==def)
   ;
  else
   return 0;

#ifdef OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON
  if(strcmp(type_arr,"INT")==0||
     strcmp(type_arr,"UNSIGNED")==0||
     strcmp(type_arr,"SHORT")==0||
     strcmp(type_arr,"LONG")==0||
     strcmp(type_arr,"FLOAT")==0||
     strcmp(type_arr,"DOUBLE")==0||
     strcmp(type_arr,"TIMESTAMP")==0||
     strcmp(type_arr,"TZ_TIMESTAMP")==0||
     strcmp(type_arr,"LTZ_TIMESTAMP")==0||
     strcmp(type_arr,"BIGINT")==0||
     strcmp(type_arr,"CHAR")==0||
     strcmp(type_arr,"DB2DATE")==0||
     strcmp(type_arr,"DB2TIME")==0||
     strcmp(type_arr,"VARCHAR_LONG")==0||
     strcmp(type_arr,"RAW_LONG")==0||
     strcmp(type_arr,"CLOB")==0||
     strcmp(type_arr,"BLOB")==0||
     strcmp(type_arr,"REFCUR")==0)
    ; // legitimate data type
  else
    return 0;
#endif

  int pl_tab_flag=0;

  if(apl_tab_size){
   array_size=apl_tab_size;
   pl_tab_flag=1;
  }


  otl_tmpl_variable<TVariableStruct>* v=
    new otl_tmpl_variable<TVariableStruct>;
  v->copy_name(name);
  switch(type){
  case 'B':
   if(t2=='L')
     v->init(otl_var_blob,db.get_max_long_size(),
             array_size,&db.connect_struct);
#if defined(OTL_BIGINT)
   else if(t2=='I')
    v->init(otl_var_bigint,sizeof(OTL_BIGINT),array_size,
            &db.connect_struct,pl_tab_flag);
#endif
    break;
  case 'C':
   if(t2=='H')
    v->init(otl_var_char,size,array_size,&db.connect_struct,pl_tab_flag);
   else if(t2=='L')
    v->init(otl_var_clob,db.get_max_long_size(),
            array_size,&db.connect_struct);
   else{
    delete v;
    v=0;
   }
   break;
  case 'D':
   if(t2=='O')
    v->init(otl_var_double,sizeof(double),array_size,
            &db.connect_struct,pl_tab_flag);
   else if(t2=='B'&&t3=='2'){
    if(t4=='T')
     v->init(otl_var_db2time,sizeof(TTimestampStruct),
             array_size,&db.connect_struct,pl_tab_flag);
     else if(t4=='D')
      v->init(otl_var_db2date,sizeof(TTimestampStruct),
              array_size,&db.connect_struct,pl_tab_flag);
    else{
     delete v;
     v=0;
    }
   }else{
    delete v;
    v=0;
   }
   break;
  case 'F':
   v->init(otl_var_float,sizeof(float),array_size,&db.connect_struct,pl_tab_flag);
   break;
  case 'I':
   v->init(otl_var_int,sizeof(int),array_size,&db.connect_struct,pl_tab_flag);
   break;
  case 'U':
   v->init(otl_var_unsigned_int,sizeof(unsigned),
           array_size,&db.connect_struct,pl_tab_flag);
   break;
  case 'R':
   if(t2=='E'&&t3=='F')
    v->init(otl_var_refcur,1,array_size,&db.connect_struct,0);
   else if(t2=='A'&&t3=='W')
    v->init(otl_var_raw_long,db.get_max_long_size(),
            array_size,&db.connect_struct);
   break;
  case 'S':
   v->init(otl_var_short,sizeof(short),
           array_size,&db.connect_struct,pl_tab_flag);
   break;
  case 'L':
   if(t2=='O'&&t3=='N')
    v->init(otl_var_long_int,sizeof(long),
            array_size,&db.connect_struct,pl_tab_flag);
   else if(t2=='T'&&t3=='Z')
     v->init(otl_var_ltz_timestamp,sizeof(TTimestampStruct),
             array_size,&db.connect_struct,pl_tab_flag);
   else{
    delete v;
    v=0;
   }
   break;
  case 'T':
    if(t2=='Z')
      v->init(otl_var_tz_timestamp,sizeof(TTimestampStruct),
              array_size,&db.connect_struct,pl_tab_flag);
    else if(t2=='I' && t3=='M')
      v->init(otl_var_timestamp,sizeof(TTimestampStruct),
              array_size,&db.connect_struct,pl_tab_flag);
    else{
      delete v;
      v=0;
    }
   break;
  case 'V':
   v->init(otl_var_varchar_long,db.get_max_long_size(),
           array_size,&db.connect_struct);
   break;
  default:
   delete v;
   v=0;
   break;
  }
  return v;
 }

 void alloc_host_var_list
 (otl_tmpl_variable<TVariableStruct>** &vl,
  int& vl_len,
  otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const int status=def)
 {
  int j;
  vl_len=0;
  if(!hv[0]){
   vl=0;
   return;
  }
  otl_tmpl_variable<TVariableStruct>* tmp_vl[otl_var_list_size];
  int i=0;
  while(hv[i]){
    otl_tmpl_variable<TVariableStruct>* vp=
      alloc_var(hv[i],inout[i],status,db,pl_tab_size[i]);
    if(vp==0){
      int j;
      for(j=0;j<vl_len;++j)
        delete tmp_vl[j];
      vl_len=0;
      throw otl_tmpl_exception
        <TExceptionStruct,
        TConnectStruct,
        TCursorStruct>
        (otl_error_msg_12,
         otl_error_code_12,
         stm_label_?stm_label_:stm_text_,
         hv[i]);
    }
    vp->name_pos=i+1;
    if(vp){
      ++vl_len;
      tmp_vl[vl_len-1]=vp;
    }
    ++i;
  }
  if(vl_len>0){
   vl=new otl_tmpl_variable<TVariableStruct>*[vl_len];
   for(j=0;j<vl_len;++j)
    vl[j]=tmp_vl[j];
  }
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TSelectCursorStruct>
class otl_tmpl_select_cursor:
public otl_tmpl_cursor
<TExceptionStruct,TConnectStruct,TCursorStruct,TVariableStruct>{
public:

 int cur_row;
 int cur_size;
 int row_count;
 int array_size;

 TSelectCursorStruct select_cursor_struct;

 otl_tmpl_select_cursor
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const short arr_size=1,
  const char* sqlstm_label=0)
   : otl_tmpl_cursor
       <TExceptionStruct,
        TConnectStruct,
        TCursorStruct,
        TVariableStruct>(db)
 {
  cur_row=-1;
  row_count=0;
  cur_size=0;
  if(sqlstm_label!=0){
    if(this->stm_label!=0){
      delete[] this->stm_label;
      this->stm_label=0;
    }
    this->stm_label=new char[strlen(sqlstm_label)+1];
    strcpy(this->stm_label,sqlstm_label);
  }
  array_size=arr_size;
  select_cursor_struct.init(array_size);
 }

 otl_tmpl_select_cursor()
  : otl_tmpl_cursor
      <TExceptionStruct,
       TConnectStruct,
       TCursorStruct,
       TVariableStruct>(){}

 void open
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  short arr_size=1)
 {
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  otl_tmpl_cursor
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct,
     TVariableStruct>::open(db);
 }

 void close(void)
 {
  otl_tmpl_cursor
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct,
    TVariableStruct>::close();
 }

 int first(void)
 {
  if(!otl_tmpl_cursor<TExceptionStruct,
                      TConnectStruct,
                      TCursorStruct,
                      TVariableStruct>::connected)return 0;
  int rc=select_cursor_struct.first
   (otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                    TCursorStruct,TVariableStruct>::cursor_struct,
    cur_row,cur_size,
    row_count,
    otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                    TCursorStruct,TVariableStruct>::eof_data,
    array_size);
  if(!rc){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                                    TCursorStruct,TVariableStruct>::cursor_struct,
                    otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                                    TCursorStruct,TVariableStruct>::stm_label?
                    otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                                    TCursorStruct,TVariableStruct>::stm_label:
                    otl_tmpl_cursor<TExceptionStruct,TConnectStruct,
                                    TCursorStruct,TVariableStruct>::stm_text);
  }
  return cur_size!=0;
 }

 int next(void)
 {
  if(!otl_tmpl_cursor
       <TExceptionStruct,TConnectStruct,
        TCursorStruct,TVariableStruct>::connected)return 0;
  if(cur_row==-1)return first();
  int rc=select_cursor_struct.next
   (otl_tmpl_cursor
       <TExceptionStruct,TConnectStruct,
        TCursorStruct,TVariableStruct>::cursor_struct,
    cur_row,cur_size,
    row_count,
    otl_tmpl_cursor
       <TExceptionStruct,TConnectStruct,
        TCursorStruct,TVariableStruct>::eof_data,
    array_size);
  if(!rc){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(otl_tmpl_cursor
                    <TExceptionStruct,TConnectStruct,
                     TCursorStruct,TVariableStruct>::cursor_struct,
                    otl_tmpl_cursor
                    <TExceptionStruct,TConnectStruct,
                     TCursorStruct,TVariableStruct>::stm_label?
                    otl_tmpl_cursor
                    <TExceptionStruct,TConnectStruct,
                     TCursorStruct,TVariableStruct>::stm_label:
                    otl_tmpl_cursor
                    <TExceptionStruct,TConnectStruct,
                     TCursorStruct,TVariableStruct>::stm_text);
  }
  return cur_size!=0;
 }

};

#if defined(OTL_ORA8)||defined(OTL_ODBC)

const int otl_lob_stream_read_mode=1;
const int otl_lob_stream_write_mode=2;
const int otl_lob_stream_zero_mode=3;

const int otl_lob_stream_first_piece=1;
const int otl_lob_stream_next_piece=2;
const int otl_lob_stream_last_piece=3;

class otl_lob_stream_generic{
public:

 int mode;
 int retcode;
 int ndx;
 int offset;
 int lob_len;
 int in_destructor;
 int eof_flag;
 int lob_is_null;
 bool ora_lob;

 otl_lob_stream_generic(const bool aora_lob=true)
 {
  ora_lob=aora_lob;
 }

 virtual ~otl_lob_stream_generic(){}

 virtual void init
 (void* avar,void* aconnect,void* acursor,int andx,
  int amode,const int alob_is_null=0) = 0;
 virtual void set_len(const int new_len=0) = 0;
 virtual otl_lob_stream_generic& operator<<(const otl_long_string& s) = 0;
 virtual otl_lob_stream_generic& operator>>(otl_long_string& s) = 0;
 virtual int eof(void) = 0;
 virtual int len(void) = 0;
 virtual void close(void) = 0;

};

#endif

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TSelectCursorStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_select_stream: public otl_tmpl_select_cursor
<TExceptionStruct,TConnectStruct,TCursorStruct,
TVariableStruct,TSelectCursorStruct>
{
public:

 otl_column_desc* sl_desc;
 otl_tmpl_variable<TVariableStruct>* sl;
 int sl_len;
 int null_fetched;
 int cur_col;
 int cur_in;
 int executed;
 int eof_status;
 char var_info[256];
 otl_select_struct_override* override;
 int delay_next;
 bool lob_stream_mode;
 long _rfc;

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;
      i < otl_tmpl_select_cursor
           <TExceptionStruct,TConnectStruct,TCursorStruct,
            TVariableStruct,TSelectCursorStruct>::vl_len;
      ++i)
   delete otl_tmpl_select_cursor
           <TExceptionStruct,TConnectStruct,TCursorStruct,
            TVariableStruct,TSelectCursorStruct>::vl[i];
  delete[] otl_tmpl_select_cursor
           <TExceptionStruct,TConnectStruct,TCursorStruct,
            TVariableStruct,TSelectCursorStruct>::vl;
  delete[] sl_desc;
 }

 virtual ~otl_tmpl_select_stream()
 {
  cleanup();
 }

 otl_tmpl_select_stream
 (otl_select_struct_override* aoverride,
  const short arr_size,
  const char* sqlstm,
  otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=0)
  : otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
      TVariableStruct,TSelectCursorStruct>(db,arr_size,sqlstm_label)
 {int i;
  otl_tmpl_select_cursor
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct>::select_cursor_struct.set_select_type
   (implicit_select);
  sl=0;
  sl_len=0;
  _rfc=0;
  null_fetched=0;
  lob_stream_mode=aoverride->lob_stream_mode;
  otl_tmpl_select_cursor
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct>::retcode=0;
  sl_desc=0;
  executed=0;
  cur_in=0;
  otl_tmpl_select_cursor
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct>::stm_text=0;
  eof_status=1;
  override=aoverride;

  {
   int len=strlen(sqlstm)+1;
   otl_tmpl_select_cursor
    <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_text=new char[len];
   strcpy(otl_tmpl_select_cursor
           <TExceptionStruct,TConnectStruct,TCursorStruct,
            TVariableStruct,TSelectCursorStruct>::stm_text,
          sqlstm);
   otl_tmpl_ext_hv_decl
    <TVariableStruct,TTimestampStruct,TExceptionStruct,
     TConnectStruct,TCursorStruct> hvd
       (otl_tmpl_select_cursor
         <TExceptionStruct,TConnectStruct,TCursorStruct,
          TVariableStruct,TSelectCursorStruct>::stm_text,1,
        otl_tmpl_select_cursor
         <TExceptionStruct,TConnectStruct,TCursorStruct,
          TVariableStruct,TSelectCursorStruct>::stm_label);
   hvd.alloc_host_var_list
    (otl_tmpl_select_cursor
      <TExceptionStruct,TConnectStruct,TCursorStruct,
       TVariableStruct,TSelectCursorStruct>::vl,
     otl_tmpl_select_cursor
      <TExceptionStruct,TConnectStruct,TCursorStruct,
       TVariableStruct,TSelectCursorStruct>::vl_len,
     db
    );
  }

  try{
   otl_tmpl_select_cursor
    <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::parse();
   if(!otl_tmpl_select_cursor
      <TExceptionStruct,TConnectStruct,TCursorStruct,
       TVariableStruct,TSelectCursorStruct>
        ::select_cursor_struct.implicit_cursor){
    get_select_list();
    bind_all();
   }else{
    for(i=0;
        i < otl_tmpl_select_cursor
             <TExceptionStruct,TConnectStruct,TCursorStruct,
              TVariableStruct,TSelectCursorStruct>::vl_len;
        ++i)
     this->bind(*otl_tmpl_select_cursor
                <TExceptionStruct,TConnectStruct,TCursorStruct,
                TVariableStruct,TSelectCursorStruct>::vl[i]);
   }
   if(otl_tmpl_select_cursor
       <TExceptionStruct,TConnectStruct,TCursorStruct,
        TVariableStruct,TSelectCursorStruct>::vl_len==0){
    rewind();
    null_fetched=0;
   }
  }catch(otl_tmpl_exception
         <TExceptionStruct,
         TConnectStruct,
         TCursorStruct>){
   cleanup();
   if(this->adb)this->adb->throw_count++;
   throw;
  }

 }

 void rewind(void)
 {
  int i;
  _rfc=0;
  if(!this->select_cursor_struct.close_select(this->cursor_struct)){
   throw otl_tmpl_exception
    <TExceptionStruct,TConnectStruct,TCursorStruct>
    (this->cursor_struct,this->stm_label?this->stm_label:this->stm_text);
  }
  if(otl_tmpl_select_cursor
      <TExceptionStruct,TConnectStruct,TCursorStruct,
       TVariableStruct,TSelectCursorStruct>
      ::select_cursor_struct.implicit_cursor){
   otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
      TVariableStruct,TSelectCursorStruct>::exec(1);
   if(sl){
    delete[] sl;
    sl=0;
   }
   get_select_list();
   for(i=0;i<sl_len;++i)this->bind(sl[i]);
  }
  eof_status=otl_tmpl_select_cursor
             <TExceptionStruct,TConnectStruct,TCursorStruct,
              TVariableStruct,TSelectCursorStruct>::first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
  delay_next=0;
 }
  
  void clean(void)
  {
    _rfc=0;
    this->cursor_struct.canceled=false;
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
    delay_next=0;
    this->cur_row=-1;
    this->row_count=0;
    this->cur_size=0;
    if(!this->select_cursor_struct.close_select(this->cursor_struct)){
      throw otl_tmpl_exception
        <TExceptionStruct,TConnectStruct,TCursorStruct>
        (this->cursor_struct,this->stm_label?this->stm_label:this->stm_text);
    }
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !eof_status;
 }

 int eof_intern(void)
 {
  return !eof_status;
 }


 void bind_all(void)
 {int i;
  for(i=0;
      i < otl_tmpl_select_cursor
           <TExceptionStruct,TConnectStruct,TCursorStruct,
            TVariableStruct,TSelectCursorStruct>::vl_len;
      ++i)
   this->bind(*otl_tmpl_select_cursor
          <TExceptionStruct,TConnectStruct,TCursorStruct,
           TVariableStruct,TSelectCursorStruct>::vl[i]
       );
  for(i=0;i<sl_len;++i)this->bind(sl[i]);
 }

 void get_select_list(void)
 {int j;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.ptr;
  int sld_tmp_len=0;
  int ftype,elem_size,i;

  for(i=1;
      otl_tmpl_select_cursor
       <TExceptionStruct,TConnectStruct,TCursorStruct,
        TVariableStruct,TSelectCursorStruct>
       ::describe_column(sl_desc_tmp[i-1],i);
      ++i)
   ++sld_tmp_len;
  sl_len=sld_tmp_len;
  if(sl){
   delete[] sl;
   sl=0;
  }
  sl=new otl_tmpl_variable<TVariableStruct>[sl_len==0?1:sl_len];
  int max_long_size=otl_tmpl_select_cursor
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct>
   ::adb->get_max_long_size();
  for(j=0;j<sl_len;++j){
   otl_tmpl_variable<TVariableStruct>::map_ftype
    (sl_desc_tmp[j],max_long_size,ftype,elem_size,*override,j+1);
   sl[j].copy_pos(j+1);
   sl[j].init(ftype,
              elem_size,
              OTL_SCAST(short,(otl_tmpl_select_cursor
               <TExceptionStruct,TConnectStruct,TCursorStruct,
                TVariableStruct,TSelectCursorStruct>
               ::array_size)),
              &otl_tmpl_select_cursor
               <TExceptionStruct,TConnectStruct,TCursorStruct,
                TVariableStruct,TSelectCursorStruct>
               ::adb->connect_struct
             );
   sl[j].var_struct.lob_stream_mode=this->lob_stream_mode;
  }
  if(sl_desc){
   delete[] sl_desc;
   sl_desc=0;
  }
  sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
  memcpy(sl_desc,sl_desc_tmp,sizeof(otl_column_desc)*sl_len);
 }

 void check_if_executed(void)
 {
  if(!executed){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_2,
     otl_error_code_2,
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_label?
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_label:
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_text,
     0);
  }
 }

 int check_type(int type_code,int actual_data_type=0)
 {int out_type_code;
  if(actual_data_type!=0)
   out_type_code=actual_data_type;
  else
   out_type_code=type_code;
  if((sl[cur_col].ftype==otl_var_timestamp ||
      sl[cur_col].ftype==otl_var_tz_timestamp ||
      sl[cur_col].ftype==otl_var_ltz_timestamp) && 
     type_code==otl_var_timestamp)
    return 1;
  if(sl[cur_col].ftype!=type_code){
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     out_type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_label?
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_label:
     otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::stm_text,
     var_info);
  }else
   return 1;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null
    (otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
     TVariableStruct,TSelectCursorStruct>::cur_row);
  }else{
   eof_status=otl_tmpl_select_cursor
             <TExceptionStruct,TConnectStruct,TCursorStruct,
              TVariableStruct,TSelectCursorStruct>::next();
   cur_col=0;
  }
 }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
   eof_status=otl_tmpl_select_cursor
    <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct>::next();
   cur_col=-1;
   ++_rfc;
  }
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val
    (otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
      TVariableStruct,TSelectCursorStruct>::cur_row));
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val
    (otl_tmpl_select_cursor
     <TExceptionStruct,TConnectStruct,TCursorStruct,
      TVariableStruct,TSelectCursorStruct>::cur_row));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();

  switch(sl[cur_col].ftype){
  case otl_var_char:
    if(!eof_intern()){
      s=OTL_RCAST(char*,sl[cur_col].val(this->cur_row));
      look_ahead();
    }
    break;
#if defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,
                                 sl[cur_col].val(this->cur_row));
      int len=sl[cur_col].get_len(this->cur_row);

#if defined(OTL_STL)
      s.assign((char*)c,len);
#elif defined(OTL_ACE)
      s.set((char*)c,len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.ptr;
      int rc=sl[cur_col].var_struct.get_blob
        (this->cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
        throw otl_tmpl_exception
          <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>(this->adb->connect_struct,
                         this->stm_label?this->stm_label:
                         this->stm_text);
      }
#if defined(OTL_STL)
      s.assign((char*)temp_buf,len);
#elif defined(OTL_ACE)
      s.set((char*)temp_buf,len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row))
             );
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(unsigned char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row)),
               sl[cur_col].get_len(this->cur_row)
              );
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(int& n)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_int
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     n);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
     n=OTL_PCONV(int,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_BIGINT)
  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(OTL_BIGINT& n)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_bigint
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     n);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_bigint))
     n=OTL_PCONV(OTL_BIGINT,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }
#endif

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(unsigned& u)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_unsigned
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     u);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_unsigned_int))
     u=OTL_PCONV(unsigned,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(short& sh)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_short
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     sh);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_short))
     sh=OTL_PCONV(short,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(long int& l)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_long_int
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     l);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_long_int))
     l=OTL_PCONV(long int,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(float& f)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_float
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     f);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_float))
     f=OTL_PCONV(float,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(double& d)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_double
    (sl[cur_col].ftype,
     sl[cur_col].val(this->cur_row),
     d);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_double))
     d=OTL_PCONV(double,double,sl[cur_col].val(this->cur_row));
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(TTimestampStruct& t)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].var_struct.read_dt
     (&t,tm,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>(this->adb->connect_struct,
                      this->stm_label?this->stm_label:
                      this->stm_text);
   }
   look_ahead();
  }
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>(otl_long_string& s)
 {
  int len=0;
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_varchar_long||
      sl[cur_col].ftype==otl_var_raw_long)&&
     !eof_intern()){
#ifdef OTL_UNICODE
   unsigned char* c=
    OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
   int len=sl[cur_col].get_len(this->cur_row);
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,sl[cur_col].ftype);
   s.null_terminate_string(len);
#else
   unsigned char* c=
    OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
   int len=sl[cur_col].get_len(this->cur_row);
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,sl[cur_col].ftype);
   s.null_terminate_string(len);
#endif
   s.set_len(len);
   look_ahead();
  }else if((sl[cur_col].ftype==otl_var_blob||
            sl[cur_col].ftype==otl_var_clob)&&
           !eof_intern()){
   int rc=sl[cur_col].var_struct.get_blob(this->cur_row,s.v,s.buf_size,len);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>(this->adb->connect_struct,
                     this->stm_label?this->stm_label:
                     this->stm_text);
   }
   if(len>s.buf_size)len=s.buf_size;
   s.set_len(len);
   s.null_terminate_string(len);
   look_ahead();
  }else{
   char tmp_var_info[256];
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     otl_var_long_string,
     tmp_var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     tmp_var_info);
  }
  return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator>>
  (otl_lob_stream_generic& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(s.ora_lob&&
     (sl[cur_col].ftype==otl_var_blob||
      sl[cur_col].ftype==otl_var_clob)&&
     !eof_intern()){
   s.init
     (OTL_RCAST(void*,&sl[cur_col]),
      OTL_RCAST(void*,this->adb),
      OTL_RCAST(void*,this),
      this->cur_row,
      otl_lob_stream_read_mode,
      this->is_null());
   delay_next=1;
  }else if((sl[cur_col].ftype==otl_var_varchar_long||
            sl[cur_col].ftype==otl_var_raw_long)&&
           !eof_intern()){
   s.init
    (OTL_RCAST(void*,&sl[cur_col]),
     OTL_RCAST(void*,this->adb),
     OTL_RCAST(void*,this),
     this->cur_row,
     otl_lob_stream_read_mode);
   delay_next=1;
  }else{
   char tmp_var_info[256];
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     otl_var_long_string,
     tmp_var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     tmp_var_info);
  }
  return *this;
 }
#endif

 int check_in_type(int type_code,int tsize)
 {
  if(this->vl[cur_in]->ftype==otl_var_char&&type_code==otl_var_char)
   return 1;
  if((this->vl[cur_in]->ftype==otl_var_timestamp ||
      this->vl[cur_in]->ftype==otl_var_tz_timestamp ||
      this->vl[cur_in]->ftype==otl_var_ltz_timestamp) &&
     type_code==otl_var_timestamp)
   return 1;
  if(this->vl[cur_in]->ftype!=type_code||this->vl[cur_in]->elem_size!=tsize){
   otl_var_info_var
    (this->vl[cur_in]->name,
     this->vl[cur_in]->ftype,
     type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     var_info);
  }else
   return 1;
 }

 void check_in_var(void)
 {
  if(this->vl_len==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_1,
     otl_error_code_1,
     this->stm_label?this->stm_label:
     this->stm_text,
     0);
  }
 }

 void get_in_next(void)
 {
  if(cur_in==this->vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const otl_null& /* n */)
 {
  check_in_var();
  this->vl[cur_in]->set_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,this->vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
   this->vl[cur_in]->set_not_null(0);
  }
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=
    OTL_RCAST(unsigned char*,this->vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
   this->vl[cur_in]->set_not_null(0);
  }
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     this->vl[cur_in]->elem_size
    );
   if(overflow){
    char tmp_var_info[256];
    otl_var_info_var
     (this->vl[cur_in]->name,
      this->vl[cur_in]->ftype,
      otl_var_char,
      tmp_var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (otl_error_msg_4,
      otl_error_code_4,
      this->stm_label?this->stm_label:
      this->stm_text,
      tmp_var_info);
   }

   this->vl[cur_in]->set_not_null(0);

  }
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const OTL_STRING_CONTAINER& s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
     overflow,
     this->vl[cur_in]->elem_size
    );
   if(overflow){
    char var_info[256];
    otl_var_info_var
     (this->vl[cur_in]->name,
      this->vl[cur_in]->ftype,
      otl_var_char,
      var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (otl_error_msg_4,
      otl_error_code_4,
      this->stm_label?this->stm_label:
      this->stm_text,
      var_info);
   }

   this->vl[cur_in]->set_not_null(0);

  }
  get_in_next();
  return *this;
 }

#endif

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     this->vl[cur_in]->elem_size
    );
   if(overflow){
    char var_info[256];
    otl_var_info_var
     (this->vl[cur_in]->name,
      this->vl[cur_in]->ftype,
      otl_var_char,
      var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (otl_error_msg_4,
      otl_error_code_4,
      this->stm_label?this->stm_label:
      this->stm_text,
      var_info);
   }

   this->vl[cur_in]->set_not_null(0);

  }
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const int n)
 {
  check_in_var();
  if(check_in_type(otl_var_int,sizeof(int))){
   *OTL_RCAST(int*,this->vl[cur_in]->val())=n;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

#if defined(OTL_BIGINT)
  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const OTL_BIGINT n)
 {
  check_in_var();
  if(check_in_type(otl_var_bigint,sizeof(OTL_BIGINT))){
   *OTL_RCAST(OTL_BIGINT*,this->vl[cur_in]->val())=n;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }
#endif

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const unsigned u)
 {
  check_in_var();
  if(check_in_type(otl_var_unsigned_int,sizeof(unsigned))){
   *OTL_RCAST(unsigned*,this->vl[cur_in]->val())=u;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const short sh)
 {
  check_in_var();
  if(check_in_type(otl_var_short,sizeof(short))){
   *OTL_RCAST(short*,this->vl[cur_in]->val())=sh;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const long int l)
 {
  check_in_var();
  if(check_in_type(otl_var_long_int,sizeof(long))){
   *OTL_RCAST(long*,this->vl[cur_in]->val())=l;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const float f)
 {
  check_in_var();
  if(check_in_type(otl_var_float,sizeof(float))){
   *OTL_RCAST(float*,this->vl[cur_in]->val())=f;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const double d)
 {
  check_in_var();
  if(check_in_type(otl_var_double,sizeof(double))){
   *OTL_RCAST(double*,this->vl[cur_in]->val())=d;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

  otl_tmpl_select_stream
   <TExceptionStruct,TConnectStruct,TCursorStruct,
    TVariableStruct,TSelectCursorStruct,
    TTimestampStruct>& operator<<(const TTimestampStruct& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,sizeof(TTimestampStruct))){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,this->vl[cur_in]->val());
   int rc=this->vl[cur_in]->var_struct.write_dt
     (tm,&t,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>(this->adb->connect_struct,
                      this->stm_label?this->stm_label:
                      this->stm_text);
   }
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_out_stream: public otl_tmpl_cursor
<TExceptionStruct,TConnectStruct,TCursorStruct,
TVariableStruct
>{
public:

 int auto_commit_flag;
 int dirty;
 int cur_x;
 int cur_y;
 short array_size;
 int in_exception_flag;
 int in_destruct_flag;
 int should_delete_flag;
 char var_info[256];
 bool flush_flag;
 bool flush_flag2;
 bool lob_stream_mode;


 void cleanup(void)
 {int i;
  if(should_delete_flag){
   for(i=0;i<this->vl_len;++i)
    delete this->vl[i];
  }
  delete[] this->vl;
 }

 otl_tmpl_out_stream
 (short arr_size,
  const char* sqlstm,
  otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=0)
   : otl_tmpl_cursor
      <TExceptionStruct,
       TConnectStruct,
       TCursorStruct,
       TVariableStruct>(db)
 {
   int i;
   if(sqlstm_label!=0){
     if(this->stm_label!=0){
       delete[] this->stm_label;
       this->stm_label=0;
     }
     this->stm_label=new char[strlen(sqlstm_label)+1];
     strcpy(this->stm_label,sqlstm_label);
   }
   dirty=0;
   auto_commit_flag=1;
   flush_flag=true;
   flush_flag2=true;
   lob_stream_mode=alob_stream_mode;
   this->cursor_struct.last_param_data_token=0;
   this->cursor_struct.last_sql_param_data_status=0;
   this->cursor_struct.sql_param_data_count=0;
   cur_x=-1;
   cur_y=0;
   should_delete_flag=1;
   in_exception_flag=0;
   in_destruct_flag=0;
   this->stm_text=0;
   array_size=arr_size;
   {
     int len=strlen(sqlstm)+1;
     this->stm_text=new char[len];
     strcpy(this->stm_text,sqlstm);
     otl_tmpl_ext_hv_decl
       <TVariableStruct,TTimestampStruct,TExceptionStruct,
       TConnectStruct,TCursorStruct> hvd(this->stm_text,arr_size);
     hvd.alloc_host_var_list(this->vl,this->vl_len,db);
   }
   try{
     this->parse();
     for(i=0;i<this->vl_len;++i){
       if(this->vl[i]->var_struct.otl_adapter==otl_odbc_adapter){
         this->vl[i]->var_struct.lob_stream_mode=lob_stream_mode;
         this->vl[i]->var_struct.vparam_type=this->vl[i]->param_type;
         if(this->vl[i]->ftype==otl_var_varchar_long||
            this->vl[i]->ftype==otl_var_raw_long){
           this->vl[i]->set_not_null(0);
         }
       }
       bind(*(this->vl[i]));
     }
   }catch(otl_tmpl_exception
          <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>){
     cleanup();
     if(this->adb)this->adb->throw_count++;
     throw;
   }
 }

 otl_tmpl_out_stream
 (otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=0)
  : otl_tmpl_cursor
      <TExceptionStruct,
       TConnectStruct,
       TCursorStruct,
       TVariableStruct>(db)
 {
   if(sqlstm_label!=0){
     if(this->stm_label!=0){
       delete[] this->stm_label;
       this->stm_label=0;
     }
     this->stm_label=new char[strlen(sqlstm_label)+1];
     strcpy(this->stm_label,sqlstm_label);
   }
   should_delete_flag=1;
   in_exception_flag=0;
   in_destruct_flag=0;
   dirty=0;
   auto_commit_flag=1;
   flush_flag=true;
   flush_flag2=true;
   lob_stream_mode=alob_stream_mode;
   this->cursor_struct.last_param_data_token=0;
   this->cursor_struct.last_sql_param_data_status=0;
   this->cursor_struct.sql_param_data_count=0;
   cur_x=-1;
   cur_y=0;
   this->stm_text=0;
 }

 virtual ~otl_tmpl_out_stream()
 {in_destruct_flag=1;
  this->in_destructor=1;
  if(dirty&&!in_exception_flag&&
     flush_flag&&flush_flag2)
   flush();
  cleanup();
  in_destruct_flag=0;
 }

 virtual void flush(const int rowoff=0,const bool force_flush=false)
 {int i,rc;

 this->_rpc=0;

  if(!dirty)return;
  if(!flush_flag2)return;

  if(force_flush){
    if(rowoff>cur_y){
      clean();
      return;
    }
    int temp_rc;
    this->exec(OTL_SCAST(short,(cur_y+1)),rowoff);
    for(i=0;i<this->vl_len;++i){
      temp_rc=this->vl[i]->var_struct.put_blob();
      if(temp_rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
        throw otl_tmpl_exception
          <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>(this->adb->connect_struct,
                         this->stm_label?this->stm_label:
                         this->stm_text);
      }
    }
    if(auto_commit_flag)
      this->adb->commit();
    clean();
    return;
  }

#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception()){
   clean();
   return; 
  }
#endif

  if(this->retcode==0||this->adb->retcode==0){
    clean();
    return;
  }
  if(cur_x!=this->vl_len-1){
   in_exception_flag=1;
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception()){
    clean();
    return; 
   }
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_3,
     otl_error_code_3,
     this->stm_label?this->stm_label:
     this->stm_text,
     0);
  }
  if(in_destruct_flag){
   this->retcode=this->cursor_struct.exec(cur_y+1,rowoff);
   for(i=0;i<this->vl_len;++i){
    rc=this->vl[i]->var_struct.put_blob();
    if(rc==0){
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
     throw otl_tmpl_exception
      <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>(this->adb->connect_struct,
                      this->stm_label?this->stm_label:
                      this->stm_text);
    }
   }
   if(!this->retcode){
    clean();
    in_exception_flag=1;
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(this->cursor_struct,
                    this->stm_label?this->stm_label:
                    this->stm_text);
   }
   if(auto_commit_flag){
    this->adb->retcode=this->adb->connect_struct.commit();
    if(!this->adb->retcode){
     clean();
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
     throw otl_tmpl_exception
       <TExceptionStruct,
        TConnectStruct,
        TCursorStruct>(this->adb->connect_struct,
                       this->stm_label?this->stm_label:
                       this->stm_text);
    }
   }
  }else{
    int temp_rc;
    this->exec(OTL_SCAST(short,(cur_y+1)),rowoff);
    for(i=0;i<this->vl_len;++i){
      temp_rc=this->vl[i]->var_struct.put_blob();
      if(temp_rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
        throw otl_tmpl_exception
          <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>(this->adb->connect_struct,
                         this->stm_label?this->stm_label:
                         this->stm_text);
      }
    }
   if(auto_commit_flag)
    this->adb->commit();
   clean();
  }

 }

 virtual void clean(const int clean_up_error_flag=0)
 {int i,j;
  if(!dirty)return;
  for(j=0;j<this->vl_len;++j)
   for(i=0;i<this->vl[j]->array_size;++i)
    if(this->vl[j]->param_type!=otl_inout_param)
       this->vl[j]->set_not_null(i);
  cur_x=-1;
  cur_y=0;
  dirty=0;
  if(clean_up_error_flag)
   this->retcode=1;
   this->in_exception_flag=0;
 }

 void set_commit(int auto_commit=0){auto_commit_flag=auto_commit;}

 void get_next(void)
 {
  if(cur_x<this->vl_len-1)
   ++cur_x;
  else{
   if(cur_y<array_size-1){
    ++cur_y;
    cur_x=0;
   }else{
    flush();
    cur_x=0;
   }
  }
  dirty=1;
 }

 int check_type(int type_code, int tsize)
 {
  if(this->vl[cur_x]->ftype==otl_var_char&&type_code==otl_var_char)
   return 1;
  if(this->vl[cur_x]->ftype==otl_var_db2time&&type_code==otl_var_timestamp||
     this->vl[cur_x]->ftype==otl_var_tz_timestamp&&type_code==otl_var_timestamp||
     this->vl[cur_x]->ftype==otl_var_ltz_timestamp&&type_code==otl_var_timestamp||
     this->vl[cur_x]->ftype==otl_var_db2date&&type_code==otl_var_timestamp)
   return 1;
  if(this->vl[cur_x]->ftype==otl_var_refcur&&type_code==otl_var_refcur)
   return 1;
  if(this->vl[cur_x]->ftype!=type_code||this->vl[cur_x]->elem_size!=tsize){
   in_exception_flag=1;
   otl_var_info_var
    (this->vl[cur_x]->name,
     this->vl[cur_x]->ftype,
     type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     var_info);
  }else
   return 1;
 }

 void check_buf(void)
 {
  if(cur_x==this->vl_len-1 && cur_y==array_size-1)
   flush();
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const char c)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){
    char* tmp=OTL_RCAST(char*,this->vl[cur_x]->val(cur_y));
    tmp[0]=c;
    tmp[1]=0;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const unsigned char c)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){
    unsigned char* tmp=OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y));
    tmp[0]=c;
    tmp[1]=0;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const OTL_STRING_CONTAINER& s)
 {
   if(this->vl_len>0){
     get_next();
     
     switch(this->vl[cur_x]->ftype){
     case otl_var_char:
       {
         int overflow;
         otl_strcpy
           (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
            OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
            overflow,
            this->vl[cur_x]->elem_size);
         if(overflow){
           otl_var_info_var
             (this->vl[cur_x]->name,
              this->vl[cur_x]->ftype,
              otl_var_char,
              var_info);
           in_exception_flag=1;
           if(this->adb)this->adb->throw_count++;
           if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
           if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
           throw otl_tmpl_exception
             <TExceptionStruct,
             TConnectStruct,
             TCursorStruct>
             (otl_error_msg_4,
              otl_error_code_4,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         this->vl[cur_x]->set_not_null(cur_y);
       }
       break;
#if defined(OTL_STL) || defined(OTL_ACE)
     case otl_var_varchar_long:
     case otl_var_raw_long:
       {
         unsigned char* c=OTL_RCAST(unsigned char*,
                                    this->vl[cur_x]->val(cur_y));
         int len=s.length();
         this->vl[cur_x]->set_not_null(cur_y);
         if(len>this->vl[cur_x]->actual_elem_size()){
           otl_var_info_var
             (this->vl[cur_x]->name,
              this->vl[cur_x]->ftype,
              otl_var_char,
              var_info);
           if(this->adb)this->adb->throw_count++;
           if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
           if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
           throw otl_tmpl_exception
             <TExceptionStruct,
             TConnectStruct,
             TCursorStruct>
             (otl_error_msg_5,
              otl_error_code_5,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         
         otl_memcpy(c,
                    OTL_RCAST(unsigned char*,
                              OTL_CCAST(char*,s.c_str())),
                    len,
                    this->vl[cur_x]->ftype);
         this->vl[cur_x]->set_len(len,cur_y);
       }
       break;
     case otl_var_blob:
     case otl_var_clob:
       {
         int len=s.length();
         if(len>this->vl[cur_x]->actual_elem_size()){
           otl_var_info_var
             (this->vl[cur_x]->name,
              this->vl[cur_x]->ftype,
              otl_var_char,
              var_info);
           if(this->adb)this->adb->throw_count++;
           if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
           if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
           throw otl_tmpl_exception
             <TExceptionStruct,
             TConnectStruct,
             TCursorStruct>
             (otl_error_msg_5,
              otl_error_code_5,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         this->vl[cur_x]->set_not_null(cur_y);
         this->vl[cur_x]->var_struct.save_blob
           (OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),len,0);
       }
       break;
#endif
     default:
       check_type(otl_var_char,1);
     } // switch     
     check_buf();
   }
   return *this;
 }
#endif

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const char* s)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){

    int overflow;
    otl_strcpy
     (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
      OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
      overflow,
      this->vl[cur_x]->elem_size
     );
    if(overflow){
     otl_var_info_var
      (this->vl[cur_x]->name,
       this->vl[cur_x]->ftype,
       otl_var_char,
       var_info);
     in_exception_flag=1;
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
     throw otl_tmpl_exception
      <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
      (otl_error_msg_4,
       otl_error_code_4,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
    }
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const unsigned char* s)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){

    int overflow;
    otl_strcpy4
     (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
      OTL_CCAST(unsigned char*,s),
      overflow,
      this->vl[cur_x]->elem_size
     );
    if(overflow){
     otl_var_info_var
      (this->vl[cur_x]->name,
       this->vl[cur_x]->ftype,
       otl_var_char,
       var_info);
     in_exception_flag=1;
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
      <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
      (otl_error_msg_4,
       otl_error_code_4,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
    }
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const int n)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_int,sizeof(int))){
    *OTL_RCAST(int*,this->vl[cur_x]->val(cur_y))=n;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

#if defined(OTL_BIGINT)
 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const OTL_BIGINT n)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_bigint,sizeof(OTL_BIGINT))){
    *OTL_RCAST(OTL_BIGINT*,this->vl[cur_x]->val(cur_y))=n;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }
#endif

#ifdef OTL_PL_TAB
 otl_tmpl_out_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(otl_pl_tab_generic& tab)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(tab.vtype,tab.elem_size)){
    int i,tmp_len;
    if(tab.len()<=this->vl[cur_x]->array_size)
     tmp_len=tab.len();
    else
     tmp_len=this->vl[cur_x]->array_size;
    this->vl[cur_x]->set_pl_tab_len(tmp_len);
    if(tab.vtype==otl_var_char){
     int i;
     for(i=0;i<tmp_len;++i){
      int overflow;
      otl_strcpy4
       (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(i)),
        OTL_RCAST(unsigned char*,tab.val(i)),
        overflow,
        this->vl[cur_x]->elem_size
       );
      if(overflow){
       char tmp_var_info[256];
       otl_var_info_var
        (this->vl[cur_x]->name,
         this->vl[cur_x]->ftype,
         otl_var_char,
         tmp_var_info);
       if(this->adb)this->adb->throw_count++;
       if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
       if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
       throw otl_tmpl_exception
        <TExceptionStruct,
        TConnectStruct,
        TCursorStruct>
        (otl_error_msg_4,
         otl_error_code_4,
         this->stm_label?this->stm_label:
         this->stm_text,
         tmp_var_info);
      }
     }
    }else if(tab.vtype==otl_var_timestamp){
      otl_datetime* ext_dt=OTL_RCAST(otl_datetime*,tab.p_v);
      otl_oracle_date* int_dt=OTL_RCAST(otl_oracle_date*,
                                        this->vl[cur_x]->val());
      int j;
      for(j=0;j<tmp_len;++j){
        convert_date(*int_dt,*ext_dt);
        ++int_dt;
        ++ext_dt;
      }
    }else
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),OTL_RCAST(char*,tab.val()),
            tab.elem_size*tmp_len);
    for(i=0;i<tmp_len;++i){
     if(tab.is_null(i))
      this->vl[cur_x]->set_null(i);
     else
      this->vl[cur_x]->set_not_null(i);
    }
   }
   check_buf();
  }
  return *this;
 }
#endif

#if defined(OTL_PL_TAB) && defined(OTL_STL)
 otl_tmpl_out_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(otl_pl_vec_generic& vec)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(vec.vtype,vec.elem_size)){
    int i,tmp_len;
    if(vec.len()<=this->vl[cur_x]->array_size)
     tmp_len=vec.len();
    else
     tmp_len=this->vl[cur_x]->array_size;
    this->vl[cur_x]->set_pl_tab_len(tmp_len);
    switch(vec.vtype){
    case otl_var_char:
     int i;
     for(i=0;i<tmp_len;++i){
      int overflow;
      otl_strcpy
       (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(i)),
        OTL_RCAST(unsigned char*,
                  OTL_CCAST(char*,(*OTL_RCAST(STD_NAMESPACE_PREFIX 
                                              vector<OTL_STRING_CONTAINER>*,
                                              vec.p_v))[i].c_str())),
        overflow,
        this->vl[cur_x]->elem_size
        );
      if(overflow){
       char var_info[256];
       otl_var_info_var
        (this->vl[cur_x]->name,
         this->vl[cur_x]->ftype,
         otl_var_char,
         var_info);
       if(this->adb)this->adb->throw_count++;
       if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
       if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
       throw otl_tmpl_exception
        <TExceptionStruct,
        TConnectStruct,
        TCursorStruct>
        (otl_error_msg_4,
         otl_error_code_4,
         this->stm_label?this->stm_label:
         this->stm_text,
         var_info);
      }
     }
     break;
    case otl_var_timestamp:
     {
      otl_oracle_date* int_dt=OTL_RCAST(otl_oracle_date*,this->vl[cur_x]->val());
      int j;
      otl_datetime* ext_dt;
      for(j=0;j<tmp_len;++j){
       ext_dt=OTL_RCAST(otl_datetime*,
                        &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<otl_datetime>*,
                                     vec.p_v))[j]);
       convert_date(*int_dt,*ext_dt);
       ++int_dt;
      }
     }
     break;
    case otl_var_int:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<int>*,vec.p_v))[0]),
            sizeof(int)*tmp_len);
     break;
    case otl_var_double:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,vec.p_v))[0]),
            sizeof(double)*tmp_len);
     break;
    case otl_var_float:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,vec.p_v))[0]),
            sizeof(float)*tmp_len);
     break;
    case otl_var_unsigned_int:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<unsigned>*,vec.p_v))[0]),
            sizeof(unsigned)*tmp_len);
     break;
    case otl_var_short:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<short>*,vec.p_v))[0]),
            sizeof(short)*tmp_len);
     break;
    case otl_var_long_int:
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<long int>*,vec.p_v))[0]),
            sizeof(long int)*tmp_len);
     break;
    }
    for(i=0;i<tmp_len;++i){
     if(vec.is_null(i))
      this->vl[cur_x]->set_null(i);
     else
      this->vl[cur_x]->set_not_null(i);
    }
   }
   check_buf();
  }
  return *this;
 }
#endif


 otl_tmpl_out_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const unsigned u)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_unsigned_int,sizeof(unsigned))){
    *OTL_RCAST(unsigned *,this->vl[cur_x]->val(cur_y))=u;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const short sh)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_short,sizeof(short))){
    *OTL_RCAST(short*,this->vl[cur_x]->val(cur_y))=sh;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const long int l)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_long_int,sizeof(long))){
    *OTL_RCAST(long*,this->vl[cur_x]->val(cur_y))=l;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const float f)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_float,sizeof(float))){
    *OTL_RCAST(float*,this->vl[cur_x]->val(cur_y))=f;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const double d)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_double,sizeof(double))){
    *OTL_RCAST(double*,this->vl[cur_x]->val(cur_y))=d;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const otl_null& /* n */)
 {
  if(this->vl_len>0){
   get_next();
   this->vl[cur_x]->set_null(cur_y);
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const TTimestampStruct& t)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_timestamp,sizeof(TTimestampStruct))){
    TTimestampStruct* tm=
     OTL_RCAST(TTimestampStruct*,this->vl[cur_x]->val(cur_y));
    this->vl[cur_x]->set_not_null(cur_y);
    int rc=this->vl[cur_x]->var_struct.write_dt
      (tm,&t,sizeof(TTimestampStruct));
    if(rc==0){
      if(this->adb)this->adb->throw_count++;
      if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
      if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
      throw otl_tmpl_exception
        <TExceptionStruct,
        TConnectStruct,
        TCursorStruct>(this->adb->connect_struct,
                       this->stm_label?this->stm_label:
                       this->stm_text);
    }
   }
   check_buf();
  }
  return *this;
 }

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<(const otl_long_string& s)
 {
  if(this->vl_len>0){
   get_next();

   if(this->vl[cur_x]->ftype==otl_var_varchar_long||
      this->vl[cur_x]->ftype==otl_var_raw_long){
    unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y));
    int len=OTL_CCAST(otl_long_string*,&s)->len();

    this->vl[cur_x]->set_not_null(cur_y);

    if(len>this->vl[cur_x]->actual_elem_size()){
     otl_var_info_var
      (this->vl[cur_x]->name,
       this->vl[cur_x]->ftype,
       otl_var_char,
       var_info);
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
      <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
      (otl_error_msg_5,
       otl_error_code_5,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
    }

    otl_memcpy(c,s.v,len,this->vl[cur_x]->ftype);
    this->vl[cur_x]->set_len(len,cur_y);
   }else if(this->vl[cur_x]->ftype==otl_var_blob||
            this->vl[cur_x]->ftype==otl_var_clob){
    int len=OTL_CCAST(otl_long_string*,&s)->len();

    if(len>this->vl[cur_x]->actual_elem_size()){
     otl_var_info_var
      (this->vl[cur_x]->name,
       this->vl[cur_x]->ftype,
       otl_var_char,
       var_info);
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
      <TExceptionStruct,
      TConnectStruct,
      TCursorStruct>
      (otl_error_msg_5,
       otl_error_code_5,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
    }
    this->vl[cur_x]->set_not_null(cur_y);
    this->vl[cur_x]->var_struct.save_blob(s.v,len,s.extern_buffer_flag);
   }
   check_buf();
  }
  return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
#define OTL_TMPL_CUR_DUMMY \
        otl_tmpl_cursor    \
        <TExceptionStruct, \
         TConnectStruct,   \
         TCursorStruct,    \
         TVariableStruct>

 otl_tmpl_out_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator<<
  (otl_lob_stream_generic& s)
 {
  if(this->vl_len>0){
   get_next();
   if((s.ora_lob&&
       this->vl[cur_x]->ftype==otl_var_blob||
       this->vl[cur_x]->ftype==otl_var_clob)||
      (this->vl[cur_x]->ftype==otl_var_varchar_long||
      this->vl[cur_x]->ftype==otl_var_raw_long)){
    s.init
     (this->vl[cur_x],
      this->adb,
      OTL_RCAST(OTL_TMPL_CUR_DUMMY*,this),
      0,
      otl_lob_stream_write_mode);
    if(!s.ora_lob)
     this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }else{
   char var_info[256];
   otl_var_info_var
    (this->vl[cur_x]->name,
     this->vl[cur_x]->ftype,
     otl_var_long_string,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     var_info);
  }
  return *this;
 }
#undef OTL_TMPL_CUR_DUMMY
#endif

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_inout_stream: public otl_tmpl_out_stream
<TExceptionStruct,TConnectStruct,TCursorStruct,
TVariableStruct,TTimestampStruct
>{
public:

 otl_tmpl_variable<TVariableStruct>** in_vl;
 int iv_len;
 int cur_in_x;
 int cur_in_y;
 int in_y_len;
 int null_fetched;
 otl_tmpl_variable<TVariableStruct>** avl;
 int avl_len;
 char var_info[256];

 void cleanup(void)
 {int i;
  for(i=0;i<avl_len;++i){
   delete avl[i];
  }
  delete[] avl;
  delete[] in_vl;
 }

 otl_tmpl_inout_stream
 (short arr_size,
  const char* sqlstm,
  otl_tmpl_connect
   <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>& db,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=0)
  : otl_tmpl_out_stream
      <TExceptionStruct,TConnectStruct,TCursorStruct,
       TVariableStruct,TTimestampStruct>(db,alob_stream_mode,sqlstm_label)
 {
  int i,j;
  this->dirty=0;
  this->auto_commit_flag=1;
  this->adb=&db;
  this->in_exception_flag=0;
  this->stm_text=0;
  this->array_size=arr_size;
  this->should_delete_flag=0;

  in_vl=0;
  iv_len=0;
  avl_len=0;
  avl=0;

  {
   int len=strlen(sqlstm)+1;
   this->stm_text=new char[len];
   strcpy(this->stm_text,sqlstm);
   otl_tmpl_ext_hv_decl
    <TVariableStruct,TTimestampStruct,TExceptionStruct,
     TConnectStruct,TCursorStruct> hvd(this->stm_text,arr_size);
   if(hvd.vst[otl_tmpl_ext_hv_decl
               <TVariableStruct,TTimestampStruct,TExceptionStruct,
                TConnectStruct,TCursorStruct>::def]==hvd.len){
    this->should_delete_flag=1;
    hvd.alloc_host_var_list(this->vl,this->vl_len,db);
   }else{
    for(i=0;i<hvd.len;++i){
     if(hvd.inout[i]==otl_tmpl_ext_hv_decl
                       <TVariableStruct,
                        TTimestampStruct,
                        TExceptionStruct,
                        TConnectStruct,
                        TCursorStruct>::in)
      ++this->vl_len;
     else if(hvd.inout[i]==otl_tmpl_ext_hv_decl
                            <TVariableStruct,
                             TTimestampStruct,
                             TExceptionStruct,
                             TConnectStruct,
                             TCursorStruct>::out)
      ++iv_len;
     else if(hvd.inout[i]==otl_tmpl_ext_hv_decl
                            <TVariableStruct,
                             TTimestampStruct,
                             TExceptionStruct,
                             TConnectStruct,
                             TCursorStruct>::io){
      ++this->vl_len;
      ++iv_len;
     }
    }
    if(this->vl_len>0){
     this->vl=new otl_tmpl_variable<TVariableStruct>*[this->vl_len];
    }
    if(iv_len>0){
     in_vl=new otl_tmpl_variable<TVariableStruct>*[iv_len];
    }
    if(hvd.len>0){
     avl=new otl_tmpl_variable<TVariableStruct>*[hvd.len];
    }
    iv_len=0; this->vl_len=0; avl_len=hvd.len;
    for(j=0;j<avl_len;++j){
     if(hvd.pl_tab_size[j]>32767){
      char tmp_var_info[256];
      otl_var_info_var
       (hvd[j],
        otl_var_none,
        otl_var_none,
        tmp_var_info);
      if(this->adb)this->adb->throw_count++;
      if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
      if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
      throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (otl_error_msg_6,
        otl_error_code_6,
        this->stm_label?this->stm_label:
        this->stm_text,
        tmp_var_info);
      }
     otl_tmpl_variable<TVariableStruct>* v=hvd.alloc_var
      (hvd[j],
       hvd.inout[j],
       otl_tmpl_ext_hv_decl
        <TVariableStruct,TTimestampStruct,TExceptionStruct,
         TConnectStruct,TCursorStruct>::def,
       db,
       hvd.pl_tab_size[j]);
     if(v==0){
       int k;
       for(k=0;k<j;++k){
         delete avl[k];
         avl[k]=0;
       }
       delete[] avl;
       avl=0;
       this->vl_len=0;
       throw otl_tmpl_exception
         <TExceptionStruct,
         TConnectStruct,
         TCursorStruct>
         (otl_error_msg_12,
          otl_error_code_12,
          hvd.stm_label_?hvd.stm_label_:hvd.stm_text_,
          hvd[j]);
     }
     v->name_pos=j+1;
     avl[j]=v;
     if(hvd.inout[j]==otl_tmpl_ext_hv_decl
                        <TVariableStruct,
                         TTimestampStruct,
                         TExceptionStruct,
                         TConnectStruct,
                         TCursorStruct>::in){
      ++this->vl_len;
      this->vl[this->vl_len-1]=v;
      v->set_param_type(otl_input_param);
     }else if(hvd.inout[j]==otl_tmpl_ext_hv_decl
                              <TVariableStruct,
                               TTimestampStruct,
                               TExceptionStruct,
                               TConnectStruct,
                               TCursorStruct>::out){
      ++iv_len;
      in_vl[iv_len-1]=v;
      v->set_param_type(otl_output_param);
     }else if(hvd.inout[j]==otl_tmpl_ext_hv_decl
                              <TVariableStruct,
                               TTimestampStruct,
                               TExceptionStruct,
                               TConnectStruct,
                               TCursorStruct>::io){
      ++this->vl_len;
      ++iv_len;
      this->vl[this->vl_len-1]=v;
      in_vl[iv_len-1]=v;
      v->set_param_type(otl_inout_param);
     }
    }
   }
  }
  try{
   this->parse();
   for(i=0;i<this->vl_len;++i){
    if(this->vl[i]->var_struct.otl_adapter==otl_odbc_adapter){
     this->vl[i]->var_struct.lob_stream_mode=this->lob_stream_mode;
     this->vl[i]->var_struct.vparam_type=this->vl[i]->param_type;
     if(this->vl[i]->ftype==otl_var_varchar_long||
        this->vl[i]->ftype==otl_var_raw_long){
      this->vl[i]->set_not_null(0);
     }
    }
    this->bind(*(this->vl[i]));
   }
   for(j=0;j<iv_len;++j)
    this->bind(*in_vl[j]);
   rewind();
  }catch(otl_tmpl_exception
         <TExceptionStruct,
         TConnectStruct,
         TCursorStruct>){
   cleanup();
   if(this->adb)this->adb->throw_count++;
   throw;
  }

 }

 virtual ~otl_tmpl_inout_stream()
 {this->in_destructor=1;
  if(!this->in_exception_flag)
   flush();
  cleanup();
 }

 int eof(void)
 {
  if(iv_len==0)return 1;
  if(in_y_len==0)return 1;
  if(cur_in_y<=in_y_len-1)return 0;
  return 1;
 }

 void flush(const int rowoff=0, const bool force_flush=false)
 {
  if(this->vl_len==0)return;
  in_y_len=this->cur_y+1;
  cur_in_y=0;
  cur_in_x=0;
  if(!this->in_exception_flag)
   otl_tmpl_out_stream
     <TExceptionStruct,
      TConnectStruct,
      TCursorStruct,
      TVariableStruct,
      TTimestampStruct>::flush(rowoff,force_flush);
 }

 void clean(const int clean_up_error_flag=0)
 {
  if(this->vl_len==0)return;
  in_y_len=this->cur_y+1;
  cur_in_y=0;
  cur_in_x=0;
  otl_tmpl_out_stream
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct,
     TVariableStruct,
     TTimestampStruct>::clean
   (clean_up_error_flag);
 }

 void rewind(void)
 {
  flush();
  cur_in_x=0;
  cur_in_y=0;
  this->cur_x=-1;
  this->cur_y=0;
  in_y_len=0;
  null_fetched=0;
  if(this->vl_len==0){
   this->exec(this->array_size);
   in_y_len=this->array_size;
   cur_in_y=0;
   cur_in_x=0;
  }
 }

 int is_null(void)
 {
  return null_fetched;
 }

 void get_in_next(void)
 {
  if(iv_len==0)return;
  if(in_y_len==0)return;
  if(cur_in_x<iv_len-1)
   ++cur_in_x;
  else{
   if(cur_in_y<in_y_len-1){
    ++cur_in_y;
    cur_in_x=0;
   }else{
    cur_in_y=0;
    cur_in_x=0;
    in_y_len=0;
   }
  }
 }

 int check_in_type(int type_code,int tsize)
 {
  if(in_vl[cur_in_x]->ftype==otl_var_refcur && type_code==otl_var_refcur)
   return 1;
  if(in_vl[cur_in_x]->ftype==otl_var_db2time&&type_code==otl_var_timestamp||
     in_vl[cur_in_x]->ftype==otl_var_db2date&&type_code==otl_var_timestamp)
   return 1;
  if(in_vl[cur_in_x]->ftype==otl_var_char&&type_code==otl_var_char)
   return 1;
  if(in_vl[cur_in_x]->ftype!=type_code||in_vl[cur_in_x]->elem_size!=tsize){
   this->in_exception_flag=1;
   otl_var_info_var
    (in_vl[cur_in_x]->name,
     in_vl[cur_in_x]->ftype,
     type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     var_info);
  }else
   return 1;
 }

 int is_null_intern(void)
 {
  if(iv_len==0)return 0;
  if(in_y_len==0)return 0;
  if(in_y_len>0)
   return in_vl[cur_in_x]->is_null(cur_in_y);
  return 0;
 }


 otl_tmpl_inout_stream
   <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(char& c)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   c=*OTL_RCAST(char*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
   TCursorStruct,TVariableStruct,
   TTimestampStruct>& operator>>(unsigned char& c)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   c=*OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(OTL_STRING_CONTAINER& s)
 {
  if(eof())return *this;
  switch(in_vl[cur_in_x]->ftype){
  case otl_var_char:
    {
      s=OTL_RCAST(char*,in_vl[cur_in_x]->val(cur_in_y));
      null_fetched=is_null_intern();
    }
    break;
#if defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    {
      unsigned char* c=
        OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
      int len=in_vl[cur_in_x]->get_len();
#if defined(OTL_STL)
      s.assign((char*)c,len);
#elif defined(OTL_ACE)
      s.set((char*)c,len,1);
#endif
      null_fetched=is_null_intern();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    {
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.ptr;
      int rc=in_vl[cur_in_x]->var_struct.get_blob
        (cur_in_y,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
        throw otl_tmpl_exception
          <TExceptionStruct,
          TConnectStruct,
          TCursorStruct>(this->adb->connect_struct,
                         this->stm_label?this->stm_label:
                         this->stm_text);
      }
#if defined(OTL_STL)
      s.assign((char*)temp_buf,len);
#elif defined(OTL_ACE)
      s.set((char*)temp_buf,len,1);
#endif

      null_fetched=is_null_intern();
    }
    break;
#endif
  default:
    check_in_type(otl_var_char,1);
  } // switch
  get_in_next();
  return *this;
 }
#endif

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(char* s)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y)));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(unsigned char* s)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y)),
               in_vl[cur_in_x]->get_len(cur_in_y)
              );
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(int& n)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_int,sizeof(int))){
   n=*OTL_RCAST(int*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_BIGINT)
 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(OTL_BIGINT& n)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_bigint,sizeof(OTL_BIGINT))){
   n=*OTL_RCAST(OTL_BIGINT*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }
#endif

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(unsigned& u)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_unsigned_int,sizeof(unsigned))){
   u=*OTL_RCAST(unsigned*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(short& sh)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_short,sizeof(short))){
   sh=*OTL_RCAST(short*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(long int& l)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_long_int,sizeof(long))){
   l=*OTL_RCAST(long*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(float& f)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_float,sizeof(float))){
   f=*OTL_RCAST(float*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(double& d)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_double,sizeof(double))){
   d=*OTL_RCAST(double*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#ifdef OTL_PL_TAB
 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(otl_pl_tab_generic& tab)
 {
  if(eof())return *this;
  if(check_in_type(tab.vtype,tab.elem_size)){
    int i,tmp_len;
    tmp_len=in_vl[cur_in_x]->get_pl_tab_len();
    if(tab.tab_size<tmp_len)
     tmp_len=tab.tab_size;
    tab.set_len(tmp_len);
    if(tab.vtype==otl_var_char){
     for(i=0;i<tmp_len;++i){
      int overflow;
      otl_strcpy3
       (OTL_RCAST(unsigned char*,tab.val(i)),
        OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(i)),
        in_vl[cur_in_x]->get_len(i),
        overflow,
        tab.elem_size
       );
     }
    }else if(tab.vtype==otl_var_timestamp){
      otl_datetime* ext_dt=OTL_RCAST(otl_datetime*,tab.p_v);
      otl_oracle_date* int_dt=
        OTL_RCAST(otl_oracle_date*,in_vl[cur_in_x]->val());
      int j;
      for(j=0;j<tmp_len;++j){
        convert_date(*ext_dt,*int_dt);
        ++int_dt;
        ++ext_dt;
      }
    }else
      memcpy(OTL_RCAST(char*,tab.val()),
             OTL_RCAST(char*,in_vl[cur_in_x]->val()),
             tab.elem_size*tmp_len);
    for(i=0;i<tmp_len;++i){
     if(in_vl[cur_in_x]->is_null(i))
      tab.set_null(i);
     else
      tab.set_non_null(i);
    }
   null_fetched=0;
  }
  get_in_next();
  return *this;
 }
#endif

#if defined(OTL_PL_TAB) && defined(OTL_STL)
 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(otl_pl_vec_generic& vec)
 {
  if(eof())return *this;
  if(check_in_type(vec.vtype,vec.elem_size)){
    int i,tmp_len;
    tmp_len=in_vl[cur_in_x]->get_pl_tab_len();
    vec.set_len(tmp_len);
    switch(vec.vtype){
    case otl_var_char:
     for(i=0;i<tmp_len;++i){
      (*OTL_RCAST(STD_NAMESPACE_PREFIX vector<OTL_STRING_CONTAINER>*,vec.p_v))[i]=
       OTL_RCAST(char*,in_vl[cur_in_x]->val(i));
     }
     break;
    case otl_var_timestamp:
     {
      otl_datetime* ext_dt;
      otl_oracle_date* int_dt=
       OTL_RCAST(otl_oracle_date*,in_vl[cur_in_x]->val());
      int j;
      for(j=0;j<tmp_len;++j){
       ext_dt=OTL_RCAST
        (otl_datetime*,
         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<otl_datetime>*,vec.p_v))[j]);
       convert_date(*ext_dt,*int_dt);
       ++int_dt;
      }
     }
     break;
    case otl_var_int:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<int>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(int)*tmp_len);
     break;
    case otl_var_double:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(double)*tmp_len);
     break;
    case otl_var_float:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(float)*tmp_len);
     break;
    case otl_var_unsigned_int:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<unsigned>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(unsigned)*tmp_len);
     break;
    case otl_var_short:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<short>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(short)*tmp_len);
     break;
    case otl_var_long_int:
     memcpy(OTL_RCAST(char*,
                      &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<long int>*,vec.p_v))[0]),
            OTL_RCAST(char*,in_vl[cur_in_x]->val()),
            sizeof(long int)*tmp_len);
     break;
    }
    for(i=0;i<tmp_len;++i){
     if(in_vl[cur_in_x]->is_null(i))
      vec.set_null(i);
     else
      vec.set_non_null(i);
    }
   null_fetched=0;
  }
  get_in_next();
  return *this;
 }
#endif


 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(TTimestampStruct& t)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_timestamp,sizeof(TTimestampStruct))){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,in_vl[cur_in_x]->val(cur_in_y));
   int rc=in_vl[cur_in_x]->var_struct.read_dt
     (&t,tm,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->throw_count++;
     if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>(this->adb->connect_struct,
                      this->stm_label?this->stm_label:
                      this->stm_text);
   }
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>(otl_long_string& s)
 {
  int len=0;
  if(eof())return *this;
  if(in_vl[cur_in_x]->ftype==otl_var_varchar_long||
     in_vl[cur_in_x]->ftype==otl_var_raw_long){
      unsigned char* c=
       OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
   len=in_vl[cur_in_x]->get_len();
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,in_vl[cur_in_x]->ftype);
   s.set_len(len);
   s.null_terminate_string(len);
   null_fetched=is_null_intern();
  }else if(in_vl[cur_in_x]->ftype==otl_var_clob||
           in_vl[cur_in_x]->ftype==otl_var_blob){
   int rc=in_vl[cur_in_x]->var_struct.get_blob(cur_in_y,s.v,s.buf_size,len);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(this->adb->connect_struct,
                    this->stm_label?this->stm_label:
                    this->stm_text);
   }
   if(len>s.buf_size)len=s.buf_size;
   s.set_len(len);
   s.null_terminate_string(len);
   null_fetched=is_null_intern();
  }else{
   char var_info[256];
   otl_var_info_var
    (in_vl[cur_in_x]->name,
     in_vl[cur_in_x]->ftype,
     otl_var_long_string,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     var_info);
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
 otl_tmpl_inout_stream
  <TExceptionStruct,TConnectStruct,
    TCursorStruct,TVariableStruct,
    TTimestampStruct>& operator>>
  (otl_lob_stream_generic& s)
 {
  if(eof())return *this;
  if(s.ora_lob&&
     in_vl[cur_in_x]->ftype==otl_var_clob||
     in_vl[cur_in_x]->ftype==otl_var_blob){
   s.init
    (OTL_RCAST(void*,in_vl[cur_in_x]),
     OTL_RCAST(void*,this->adb),
     OTL_RCAST(void*,this),
     0,
     otl_lob_stream_read_mode,
     this->is_null());
  }else if(in_vl[cur_in_x]->ftype==otl_var_varchar_long||
           in_vl[cur_in_x]->ftype==otl_var_raw_long){
   s.init
    (OTL_RCAST(void*,in_vl[cur_in_x]),
     OTL_RCAST(void*,this->adb),
     OTL_RCAST(void*,this),
     0,
     otl_lob_stream_read_mode);
  }else{
   char tmp_var_info[256];
   otl_var_info_var
    (in_vl[cur_in_x]->name,
     in_vl[cur_in_x]->ftype,
     otl_var_long_string,
     tmp_var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
    TConnectStruct,
    TCursorStruct>
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     tmp_var_info);
  }
  get_in_next();
  return *this;
 }
#endif


};

// ==================== OTL-Adapter for Interbase  =========================
#ifdef OTL_IBASE

#include <ibase.h>

OTL_IBASE_NAMESPACE_BEGIN

class otl_exc{
public:
 unsigned char msg[1000];
 int code;
 char sqlstate[32];

#ifdef OTL_EXTENDED_EXCEPTION
 char** msg_arr;
 char** sqlstate_arr;
 int* code_arr;
 int arr_len;
#endif

 enum{disabled=0,enabled=1};

 otl_exc()
 {
  sqlstate[0]=0;
  msg[0]=0;
  code=0;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

 void init(const char* amsg, const int acode)
 {
  strcpy(OTL_RCAST(char*,msg),amsg);
  code=acode;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

};

// TBD...

OTL_IBASE_NAMESPACE_END
#endif


// ==================== OTL-Adapter for ODBC/CLI =========================
#ifdef OTL_ODBC

#ifndef OTL_DB2_CLI
// in case if it's ODBC for Windows, include windows.h file
#if !defined(OTL_ODBC_UNIX) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>
#else
#include <sqlcli1.h>
#endif

OTL_ODBC_NAMESPACE_BEGIN

typedef TIMESTAMP_STRUCT otl_time;
const int otl_odbc_date_prec=23;
const int otl_odbc_date_scale=0;

const int OTL_MAX_MSG_ARR=512;

class otl_exc{
public:

  unsigned char msg[1000];
  int code;
  unsigned char sqlstate[1000];
  
#ifdef OTL_EXTENDED_EXCEPTION
  char** msg_arr;
  char** sqlstate_arr;
  int* code_arr;
  int arr_len;
#endif

 enum{disabled=0,enabled=1};

 otl_exc()
 {
  sqlstate[0]=0;
  code=0;
  msg[0]=0;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

#ifdef OTL_EXTENDED_EXCEPTION
  otl_exc(const otl_exc& ex)
  {
    strcpy(OTL_RCAST(char*,msg),OTL_RCAST(const char*,ex.msg));
    strcpy(OTL_RCAST(char*,sqlstate),OTL_RCAST(const char*,ex.sqlstate));
    code=ex.code;
    arr_len=0;
    msg_arr=0;
    sqlstate_arr=0;
    code_arr=0;
    if(ex.arr_len>0){
      sqlstate_arr=new char*[ex.arr_len];
      msg_arr=new char*[ex.arr_len];
      code_arr=new int[ex.arr_len];
      int i, msg_len, sqlstate_len;
      for(i=0;i<ex.arr_len;++i){
        msg_len=strlen(ex.msg_arr[i]);
        sqlstate_len=strlen(ex.sqlstate_arr[i]);
        msg_arr[i]=new char[msg_len+1];
        sqlstate_arr[i]=new char[sqlstate_len+1];
        strcpy(msg_arr[i],ex.msg_arr[i]);
        strcpy(sqlstate_arr[i],ex.sqlstate_arr[i]);
        code_arr[i]=ex.code_arr[i];
      }
      arr_len=ex.arr_len;
    }
  }
#endif

 void init(const char* amsg, const int acode)
 {
  strcpy(OTL_RCAST(char*,msg),amsg);
  code=acode;
  sqlstate[0]=0;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

  virtual ~otl_exc()
  {
#ifdef OTL_EXTENDED_EXCEPTION
    int i;
    if(arr_len>0){
      for(i=0;i<arr_len;++i){
        delete[] msg_arr[i];
        delete[] sqlstate_arr[i];
      }
      delete[] msg_arr;
      delete[] sqlstate_arr;
      delete[] code_arr;
      arr_len=0;
      msg_arr=0;
      sqlstate_arr=0;
      code_arr=0;
    }
#endif
  }

};

#if (ODBCVER >= 0x0300)
#ifdef OTL_EXTENDED_EXCEPTION
inline void otl_fill_exception(
  otl_exc& exception_struct,
  OTL_SQLHANDLE handle,
  OTL_SQLSMALLINT htype
)
{
  OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
  char* tmp_msg_arr[OTL_MAX_MSG_ARR];
  char* tmp_sqlstate_arr[OTL_MAX_MSG_ARR];
  int tmp_code_arr[OTL_MAX_MSG_ARR];
  int tmp_arr_len=0;
  OTL_SQLSMALLINT tmp_msg_len=0;
  OTL_SQLSMALLINT tmp_sqlstate_len=0;
  int tmp_code;
  char tmp_msg[SQL_MAX_MESSAGE_LENGTH];
  char tmp_sqlstate[1000];

  strcpy(tmp_msg,OTL_RCAST(char*,exception_struct.msg));
  strcpy(tmp_sqlstate,OTL_RCAST(char*,exception_struct.sqlstate));
  tmp_code=exception_struct.code;

  do{
    tmp_sqlstate_len=strlen(tmp_sqlstate);
    tmp_msg_len=strlen(tmp_msg);
    ++tmp_arr_len;
    tmp_msg_arr[tmp_arr_len-1]=new char[tmp_msg_len+1];
    tmp_sqlstate_arr[tmp_arr_len-1]=new char[tmp_sqlstate_len+1];
    strcpy(tmp_msg_arr[tmp_arr_len-1],tmp_msg);
    strcpy(tmp_sqlstate_arr[tmp_arr_len-1],tmp_sqlstate);
    tmp_code_arr[tmp_arr_len-1]=tmp_code;
    rc=SQLGetDiagRec
      (htype,
       handle,
       tmp_arr_len+1,
       OTL_RCAST(OTL_SQLCHAR_PTR,tmp_sqlstate),
       OTL_RCAST(OTL_SQLINTEGER_PTR,&tmp_code),
       OTL_RCAST(OTL_SQLCHAR_PTR,tmp_msg),
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
    tmp_msg[msg_len]=0;
    if((rc==SQL_NO_DATA||rc==SQL_INVALID_HANDLE||
        rc==SQL_ERROR)&&tmp_arr_len==1){
      int i;
      for(i=0;i<tmp_arr_len;++i){
        delete[] tmp_msg_arr[i];
        delete[] tmp_sqlstate_arr[i];
      }
      return;
    }
  }while(rc!=SQL_NO_DATA&&rc!=SQL_INVALID_HANDLE&&
         rc!=SQL_ERROR&&tmp_arr_len<OTL_MAX_MSG_ARR);
    
  exception_struct.arr_len=tmp_arr_len;
  exception_struct.msg_arr=new char*[tmp_arr_len];
  exception_struct.sqlstate_arr=new char*[tmp_arr_len];
  exception_struct.code_arr=new int[tmp_arr_len];
  memcpy(exception_struct.msg_arr,tmp_msg_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.sqlstate_arr,tmp_sqlstate_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.code_arr,tmp_code_arr,tmp_arr_len*sizeof(int));

}
#endif
#endif

class otl_conn{
public:

 OTL_HENV henv;
 OTL_HDBC hdbc;

 int timeout;
 int cursor_type;
 int status;
 int long_max_size;
 bool extern_lda;

  static int initialize(const int /* threaded_mode */=0)
  {
    return 1;
  }

 otl_conn()
 {
  timeout=0;
  cursor_type=0;
  henv=0;
  hdbc=0;
  long_max_size=32760;
  status=SQL_SUCCESS;
  extern_lda=false;
 }

 int ext_logon(OTL_HENV ahenv,
               OTL_HDBC ahdbc,
               const int auto_commit)
 {
  if(!extern_lda){
#if (ODBCVER >= 0x0300)
    if(hdbc!=0){
      status=SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    }
#else
    if(hdbc!=0)
      status=SQLFreeConnect(hdbc);
#endif
    hdbc=0;
#if (ODBCVER >= 0x0300)
    if(henv!=0){
      status=SQLFreeHandle(SQL_HANDLE_ENV,henv);
    }
#else
   if(henv!=0)
     status=SQLFreeEnv(henv);
#endif
   henv=0;
  }
  extern_lda=true;
  henv=ahenv;
  hdbc=ahdbc;
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  if(auto_commit)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     (SQLPOINTER)SQL_AUTOCOMMIT_ON,
     SQL_IS_POINTER);
  else
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
     SQL_IS_POINTER);
#else
  if(auto_commit)
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
  else
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#ifdef OTL_DB2_CLI
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_LONGDATA_COMPAT,
    (SQLPOINTER)SQL_LD_COMPAT_YES,
    SQL_IS_INTEGER);
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

  return 1;

 }

 virtual ~otl_conn()
 {
  if(extern_lda){
   hdbc=0;
   henv=0;
   extern_lda=false;
  }else{
#if (ODBCVER >= 0x0300)
    if(hdbc!=0){
      status=SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    }
#else
    if(hdbc!=0)
      status=SQLFreeConnect(hdbc);
#endif
   hdbc=0;
#if (ODBCVER >= 0x0300)
   if(henv!=0){
     status=SQLFreeHandle(SQL_HANDLE_ENV,henv);
   }
#else
   if(henv!=0)
     status=SQLFreeEnv(henv);
#endif
   henv=0;
  }
 }

 void set_timeout(const int atimeout=0)
 {
  timeout=atimeout;
 }

 void set_cursor_type(const int acursor_type=0)
 {
  cursor_type=acursor_type;
 }

 int rlogon(const char* connect_str,const int auto_commit)
 {
  char username[256];
  char passwd[256];
  char tnsname[256];
  char* tnsname_ptr=0;
  char* c=OTL_CCAST(char*,connect_str);
  char* username_ptr=username;
  char* passwd_ptr=passwd;
  char temp_connect_str[512];

  if(extern_lda){
   extern_lda=false;
   henv=0;
   hdbc=0;
  }
  memset(username,0,sizeof(username));
  memset(passwd,0,sizeof(passwd));
  memset(tnsname,0,sizeof(tnsname));

  char* c1=OTL_CCAST(char*,connect_str);
  int oracle_format=0;

  while(*c1){
   if(*c1=='@'){
    oracle_format=1;
    break;
   }
   ++c1;
  }

  if(oracle_format){
   while(*c&&*c!='/'){
    *username_ptr=*c;
    ++c;
    ++username_ptr;
   }
   *username_ptr=0;

   if(*c=='/')++c;
   while(*c&&*c!='@'){
    *passwd_ptr=*c;
    ++c;
    ++passwd_ptr;
   }
   *passwd_ptr=0;

   if(*c=='@'){
    ++c;
    tnsname_ptr=tnsname;
    while(*c){
     *tnsname_ptr=*c;
     ++c;
     ++tnsname_ptr;
    }
    *tnsname_ptr=0;
   }
  }else{
   c1=OTL_CCAST(char*,connect_str);
   char* c2=temp_connect_str;
   while(*c1){
    *c2=otl_to_upper(*c1);
    ++c1;
    ++c2;
   }
   *c2=0;
   c1=temp_connect_str;
   char entry_name[256];
   char entry_value[256];
   while(*c1){
    c2=entry_name;
    while(*c1&&*c1!='='){
     *c2=*c1;
     ++c1;
     ++c2;
    }
    *c2=0;
    if(*c1) ++c1;
    c2=entry_value;
    while(*c1&&*c1!=';'){
     *c2=*c1;
     ++c1;
     ++c2;
    }
    *c2=0;
    if(*c1) ++c1;
    if(strcmp(entry_name,"DSN")==0)strcpy(tnsname,entry_value);
    if(strcmp(entry_name,"UID")==0)strcpy(username,entry_value);
    if(strcmp(entry_name,"PWD")==0)strcpy(passwd,entry_value);
   }
  }

  if(henv==0||hdbc==0){
#if (ODBCVER >= 0x0300)
   status=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
#else
   status=SQLAllocEnv(&henv);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;

#if (ODBCVER >= 0x0300)
   status=SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,
                        OTL_RCAST(void*,SQL_OV_ODBC3),SQL_NTS);
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#if (ODBCVER >= 0x0300)
   status=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
#else
   status=SQLAllocConnect(henv, &hdbc);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }

#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  if(auto_commit)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     (SQLPOINTER)SQL_AUTOCOMMIT_ON,
     SQL_IS_POINTER);
  else
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
     SQL_IS_POINTER);
#else
  if(auto_commit)
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
  else
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#if (ODBCVER >= 0x0300)
  if(timeout>0)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_LOGIN_TIMEOUT,
     OTL_RCAST(void*,timeout),
     0);
  if (timeout>0)
    status=SQLSetConnectOption(hdbc,SQL_ATTR_LOGIN_TIMEOUT,timeout);
#else
  if (timeout>0)
    status=SQLSetConnectOption(hdbc,SQL_LOGIN_TIMEOUT,timeout); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;


#ifdef OTL_DB2_CLI
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_LONGDATA_COMPAT,
    (SQLPOINTER)SQL_LD_COMPAT_YES,
    SQL_IS_INTEGER);
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

  if(oracle_format){
   status=SQLConnect
    (hdbc,
     OTL_RCAST(unsigned char*,tnsname),SQL_NTS,
     OTL_RCAST(unsigned char*,username),SQL_NTS,
     OTL_RCAST(unsigned char*,passwd),SQL_NTS);
  }else{
   SQLCHAR out_str[2048];
   SQLSMALLINT out_len=0;
   status=SQLDriverConnect
    (hdbc,
     0,
     OTL_RCAST(SQLCHAR*,OTL_CCAST(char*,connect_str)),
     OTL_SCAST(short,strlen(connect_str)),
     out_str,
     sizeof(out_str),
     &out_len,
     SQL_DRIVER_NOPROMPT);
  }

  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;

  return 1;

 }

  int set_transaction_isolation_level(const long int level)
  {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_TXN_ISOLATION,
     (SQLPOINTER)level,
     SQL_IS_POINTER);
#else
   status=SQLSetConnectOption(hdbc,SQL_TXN_ISOLATION,level);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;
#else
   return 1;
#endif

  }

 int auto_commit_on(void)
 {
#ifdef OTL_ODBC_MYSQL
  return 1;
#else
#if (ODBCVER >= 0x0300)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_AUTOCOMMIT,
    (SQLPOINTER)SQL_AUTOCOMMIT_ON,
    SQL_IS_POINTER);
#else
  status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
#endif
 }

 int auto_commit_off(void)
 {
#ifdef OTL_ODBC_MYSQL
  return 1;
#else
#if (ODBCVER >= 0x0300)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_AUTOCOMMIT,
    (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
    SQL_IS_POINTER);
#else
 status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
#endif
 }


 int logoff(void)
 {
  if(extern_lda){
   extern_lda=false;
   henv=0;
   hdbc=0;
   return 1;
  }else{
   commit();
   status=SQLDisconnect(hdbc);
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;
  }
 }

 void error(otl_exc& exception_struct)
 {OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;

#if (ODBCVER >= 0x0300)
  rc=SQLGetDiagRec
    (SQL_HANDLE_DBC,
     hdbc,
     1,
     OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
     OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
     OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
     SQL_MAX_MESSAGE_LENGTH-1,
     OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#else
 rc=SQLError(henv, 
             hdbc, 
             0, // hstmt
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
             OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
             SQL_MAX_MESSAGE_LENGTH-1,
             OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif
  exception_struct.msg[msg_len]=0;

  if(rc==SQL_INVALID_HANDLE||rc==SQL_ERROR)
    exception_struct.msg[0]=0;
#if (ODBCVER >= 0x0300)
#ifdef OTL_EXTENDED_EXCEPTION
  else if(rc!=SQL_NO_DATA)
    otl_fill_exception(exception_struct,hdbc,SQL_HANDLE_DBC);
#endif
#endif

 }

 int commit(void)
 {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  status=SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
#else
 status=SQLTransact(henv,hdbc,SQL_COMMIT); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
  return 1;
#else
  return 1;
#endif
 }

 int rollback(void)
 {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  status=SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_ROLLBACK);
#else
 status=SQLTransact(henv,hdbc,SQL_ROLLBACK); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
  return 1;
#else
  return 1;
#endif
 }

};

class otl_cur0{
public:

 OTL_SQLHSTMT cda;
 int last_param_data_token;
 int last_sql_param_data_status;
 int sql_param_data_count;

 otl_cur0()
 {
  cda=0;
  last_param_data_token=0;
  last_sql_param_data_status=0;
  sql_param_data_count=0;
 }

 virtual ~otl_cur0(){}

};

class otl_var{
public:

 unsigned char* p_v;
 OTL_SQLINTEGER* p_len;
 int ftype;
 int act_elem_size;
 bool lob_stream_mode;
 int lob_stream_flag;
 int vparam_type;
 int lob_len;
 int lob_pos;
 int lob_ftype;
 int otl_adapter;

 otl_var()
 {
  otl_adapter=otl_odbc_adapter;
  lob_stream_flag=0;
  p_v=0;
  p_len=0;
  act_elem_size=0;
  lob_stream_mode=false;
  vparam_type=-1;
  lob_len=0;
  lob_pos=0;
  lob_ftype=0;
 }

 virtual ~otl_var()
 {
  delete[] p_v;
  delete[] p_len;
 }

  int write_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

  int read_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

 void set_lob_stream_flag(const int flg=1)
 {
  lob_stream_flag=flg;
 }

 int get_pl_tab_len(void)
 {
  return 0;
 }

 int get_max_pl_tab_len(void)
 {
  return 0;
 }

  void set_pl_tab_len(const int /* pl_tab_len */)
 {
 }

 int write_blob
 (const otl_long_string& s,
  const int alob_len,
  int& aoffset,
  otl_cur0& cur)
 {
  SQLRETURN rc=0;
  SQLINTEGER temp_len=0;
  SQLPOINTER pToken=0;
  int param_number=0;

  if(!lob_stream_flag&&!lob_stream_mode)return 1;
  if(aoffset==1){
   if(cur.sql_param_data_count==0){
    rc=SQLParamData(cur.cda, &pToken);
    param_number=OTL_RCAST(int,pToken);
    ++cur.sql_param_data_count;
    cur.last_sql_param_data_status=rc;
    cur.last_param_data_token=param_number;
    if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO&&
       rc!=SQL_NEED_DATA)
     return 0;
   }
  }
  temp_len=s.length;
  rc=SQLPutData(cur.cda,s.v,temp_len);
  if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else{
    aoffset+=s.length;
    if(aoffset-1==alob_len){
     if(!(cur.last_param_data_token==0&&
          cur.sql_param_data_count>0)){
      rc=SQLParamData(cur.cda, &pToken);
      param_number=OTL_RCAST(int,pToken);
      ++cur.sql_param_data_count;
      cur.last_sql_param_data_status=rc;
      cur.last_param_data_token=param_number;
      if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
         rc!=SQL_NO_DATA &&
#endif
         rc!=SQL_NEED_DATA)
       return 0;
     }
    }
    return 1;
   }
 }

 int read_blob
 (otl_cur0& cur,
  otl_long_string& s,
  const int andx,
  int& aoffset,
  int& eof_flag)
 {
  SQLRETURN rc=0;
  SQLINTEGER retLen=0;
  int chunkLen=0;
  if(!lob_stream_flag&&!lob_stream_mode)return 1;
  rc=SQLGetData
   (cur.cda,
    lob_pos,
    lob_ftype, 
    s.v, 
    s.buf_size,
    &retLen);
  if(rc==SQL_SUCCESS_WITH_INFO||rc==SQL_SUCCESS){
   if(retLen==SQL_NULL_DATA){
    chunkLen=0;
    p_len[andx]=SQL_NULL_DATA;
   }else if(retLen>s.buf_size||retLen==SQL_NO_TOTAL)
    chunkLen=s.buf_size;
   else
    chunkLen=retLen;
   if(lob_ftype==SQL_C_CHAR)
    s.set_len(chunkLen-1);
   else
    s.set_len(chunkLen);
   if(lob_len==0&&aoffset==1&&
      retLen!=SQL_NULL_DATA&&
      retLen!=SQL_NO_TOTAL)
    lob_len=retLen;
   aoffset+=chunkLen;
   if(chunkLen<s.buf_size||rc==SQL_SUCCESS){
    s.set_len(chunkLen);
    eof_flag=1;
   }else
    eof_flag=0;
   return 1;
  }
#if (ODBCVER >= 0x0300)
  else if(rc==SQL_NO_DATA)
#else
  else if(rc==SQL_NO_DATA_FOUND)
#endif
   return 1;
  else
   return 0;
 }

  int get_blob_len(const int /* ndx */,int& alen)
  {
    alen=lob_len;
    return 1;
  }
  
 int put_blob(void)
 {
  return 1;
 }

 int get_blob
 (const int /* ndx */,
  unsigned char* /* abuf */,
  const int /* buf_size */,
  int& /* len */)
 {
  return 1;
 }

 int save_blob
 (const unsigned char* /* abuf  */,
  const int /* len */,
  const int /* extern_buffer_flag */)
 {
  return 1;
 }

 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const int aftype,
  int& aelem_size,
  const short aarray_size,
  const void* /* connect_struct */=0,
  const int /*apl_tab_size*/=0)
 {int i;
  ftype=aftype;
  act_elem_size=aelem_size;
  p_v=new unsigned char[aelem_size*
                        OTL_SCAST(unsigned,aarray_size)];
  p_len=new OTL_SQLINTEGER[aarray_size];
  memset(p_v,0,aelem_size*OTL_SCAST(unsigned,aarray_size));
  for(i=0;i<aarray_size;++i){
   if(ftype==otl_var_char)
    p_len[i]=OTL_SCAST(OTL_SQLINTEGER,SQL_NTS);
   else if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
    p_len[i]=0;
   else
    p_len[i]=OTL_SCAST(OTL_SQLINTEGER,aelem_size);
  }
 }

 void set_null(int ndx)
 {
  p_len[ndx]=SQL_NULL_DATA;
 }

 void set_not_null(int ndx, int pelem_size)
 {
  set_len(pelem_size,ndx);
 }

 void set_len(int len, int ndx)
 {
  if(lob_stream_mode&&
     (vparam_type==otl_input_param||
      vparam_type==otl_inout_param)&&
     (ftype==otl_var_raw_long||
      ftype==otl_var_varchar_long)){
   p_len[ndx]=SQL_DATA_AT_EXEC;
   return;
  }
  if(ftype==otl_var_char)
   p_len[ndx]=SQL_NTS;
  else
   p_len[ndx]=OTL_SCAST(OTL_SQLINTEGER,len);
 }

 int get_len(int ndx)
 {
  if(p_len[ndx]==SQL_NULL_DATA)
   return 0;
  else
   return p_len[ndx];
 }

 int is_null(int ndx)
 {
  return p_len[ndx]==SQL_NULL_DATA;
 }

 void* val(int ndx,int pelem_size)
 {
  return (void*)&p_v[(OTL_SCAST(unsigned,ndx))*pelem_size];
 }

 static int int2ext(int int_type)
 {
  switch(int_type){
  case SQL_VARCHAR: return SQL_C_CHAR;
#if (ODBCVER >= 0x0300)
  case SQL_TYPE_DATE: return SQL_C_TIMESTAMP;
  case SQL_TYPE_TIMESTAMP: return SQL_C_TIMESTAMP;
  case SQL_TYPE_TIME: return SQL_C_TIMESTAMP;
#else
  case SQL_DATE: return SQL_C_TIMESTAMP;
  case SQL_TIMESTAMP: return SQL_C_TIMESTAMP;
  case SQL_TIME: return SQL_C_TIMESTAMP;
#endif
  case SQL_CHAR: return SQL_C_CHAR;
#if defined(OTL_BIGINT)
  case SQL_BIGINT: return SQL_C_SBIGINT;
#else
  case SQL_BIGINT: return SQL_C_DOUBLE;
#endif
  case SQL_DECIMAL: return SQL_C_DOUBLE;
  case SQL_DOUBLE: return SQL_C_DOUBLE;
  case SQL_FLOAT: return SQL_C_DOUBLE;
  case SQL_INTEGER: return SQL_C_SLONG;
  case SQL_NUMERIC: return SQL_C_DOUBLE;
  case SQL_REAL: return SQL_C_DOUBLE;
  case SQL_SMALLINT: return SQL_C_SSHORT;
  case SQL_BIT: return SQL_C_SSHORT;
  case SQL_TINYINT: return SQL_C_SSHORT;
  case SQL_LONGVARCHAR: return SQL_LONGVARCHAR;
  case SQL_LONGVARBINARY: return SQL_LONGVARBINARY;
  case SQL_VARBINARY: return SQL_LONGVARBINARY;
#if (ODBCVER >= 0x0350)
  case SQL_GUID: return SQL_C_CHAR;
#endif
  case -9: return SQL_C_CHAR;
  default: return -1;
  }
 }

 static int datatype_size(int ftype,int maxsz,int int_type,int max_long_size)
 {
  switch(ftype){
  case SQL_C_CHAR:
   switch(int_type){
   case SQL_LONGVARCHAR:
    return max_long_size;
   case SQL_LONGVARBINARY:
    return max_long_size;
   case SQL_DATE:
    return 40;
#if (ODBCVER >= 0x0300)
   case SQL_TYPE_TIMESTAMP:
#else
   case SQL_TIMESTAMP:
#endif
    return 40;
#if (ODBCVER >= 0x0300)
   case SQL_TYPE_TIME:
#else
   case SQL_TIME:
#endif
    return 40;
#if (ODBCVER >= 0x0350)
   case SQL_GUID:
    return 40;
#endif
   default:
    return maxsz+1;
   }
#if defined(OTL_BIGINT)
  case SQL_C_SBIGINT:
   return sizeof(OTL_BIGINT);
#endif
  case SQL_C_DOUBLE:
   return sizeof(double);
  case SQL_C_SLONG:
   return sizeof(int);
  case SQL_C_SSHORT:
   return sizeof(short int);
  case SQL_C_TIMESTAMP:
   return sizeof(TIMESTAMP_STRUCT);
  case SQL_C_TIME:
   return sizeof(TIME_STRUCT);
  case SQL_C_DATE:
   return sizeof(DATE_STRUCT);
  case SQL_LONGVARCHAR:
   return max_long_size;
  case SQL_LONGVARBINARY:
   return max_long_size;
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& ftype,
  int& elem_size,
  otl_select_struct_override& override,
  const int column_ndx)
 {
  int ndx=override.find(column_ndx);
  if(ndx==-1){
   ftype=int2ext(desc.dbtype);
   elem_size=datatype_size(ftype,desc.dbsize,desc.dbtype,max_long_size);
   switch(ftype){
   case SQL_C_CHAR:
    ftype=otl_var_char;
    break;
   case SQL_C_DOUBLE:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_double;
    break;
#if defined(OTL_BIGINT)
   case SQL_C_SBIGINT:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_bigint;
    break;
#endif
   case SQL_C_SLONG:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_int;
    break;
   case SQL_C_SSHORT:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_short;
    break;
   case SQL_LONGVARCHAR:
    ftype=otl_var_varchar_long;
    break;
   case SQL_LONGVARBINARY:
    ftype=otl_var_raw_long;
    break;
   case SQL_C_DATE:
   case SQL_C_TIME:
   case SQL_C_TIMESTAMP:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
     ftype=otl_var_timestamp;
    break;
   default:
    ftype=0;
    break;
   }
  }else{
   ftype=override.col_type[ndx];
   switch(ftype){
   case otl_var_char:
    elem_size=override.col_size[ndx];
    break;
   case otl_var_double:
    elem_size=sizeof(double);
    break;
   case otl_var_float:
    elem_size=sizeof(float);
    break;
   case otl_var_int:
    elem_size=sizeof(int);
    break;
#if defined(OTL_BIGINT)
   case otl_var_bigint:
    elem_size=sizeof(OTL_BIGINT);
    break;
#endif
   case otl_var_unsigned_int:
    elem_size=sizeof(unsigned);
    break;
   case otl_var_short:
    elem_size=sizeof(short);
    break;
   case otl_var_long_int:
    elem_size=sizeof(double);
    break;
   default:
    elem_size=override.col_size[ndx];
    break;
   }
  }
  desc.otl_var_dbtype=ftype;
 }

};

class otl_cur: public otl_cur0{
public:

 int status;
 otl_conn* adb;
 int direct_exec_flag;
 long _rpc;
  bool canceled;

 otl_cur()
 {
  cda=0;
  adb=0;
  canceled=false;
  _rpc=0;
  direct_exec_flag=0;
  last_param_data_token=0;
  last_sql_param_data_status=0;
  sql_param_data_count=0;
 }

 virtual ~otl_cur(){}

  int cancel(void)
  {
    status=SQLCancel(cda);
    canceled=true;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  }

  int open(otl_conn& /* connect */,otl_var* /* var */)
  {
    return 1;
  }

 void set_direct_exec(const int flag)
 {
  direct_exec_flag=flag;
 }

 int open(otl_conn& connect)
 {
  direct_exec_flag=0;
  adb=&connect;
#if (ODBCVER >= 0x0300)
  status=SQLAllocHandle(SQL_HANDLE_STMT,connect.hdbc,&cda);
#else
 status=SQLAllocStmt(connect.hdbc,&cda); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  if(connect.timeout>0){
#if (ODBCVER >= 0x0300)
   status=SQLSetStmtAttr
    (cda,
     SQL_ATTR_QUERY_TIMEOUT,
     OTL_RCAST(void*,connect.timeout),
     SQL_NTS);
#else
  status=SQLSetStmtOption(cda,SQL_QUERY_TIMEOUT,connect.timeout);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
  }
  if(connect.cursor_type!=0){ // other than default
#if (ODBCVER >= 0x0300)
   status=SQLSetStmtAttr
    (cda,
     SQL_ATTR_CURSOR_TYPE,
     OTL_RCAST(void*,connect.cursor_type),
     SQL_NTS);
#else
  status=SQLSetStmtOption(cda,SQL_CURSOR_TYPE,connect.cursor_type);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
  }
  return 1;
 }

 int close(void)
 {
  status=SQLFreeStmt(cda,SQL_DROP);
  adb=0;
  cda=0;
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
 }

 int parse(char* stm_text)
 {short in_str=0;
  char *c=stm_text;

  if(*c=='$'){
    ++c;
    _rpc=0;
    direct_exec_flag=1;
    const int ctl_arr_size=6;
    struct{
      OTL_SQLCHAR_PTR name_ptr;
      OTL_SQLSMALLINT name_len;
      OTL_SQLCHAR name[512];
    } ctl_arr[ctl_arr_size];
    int i=0;
    for(i=0;i<ctl_arr_size;++i){
      ctl_arr[i].name_ptr=0;
      ctl_arr[i].name_len=0;
      ctl_arr[i].name[0]=0;
    }
    char func_name[256];
    int par_num=0;
    char par_val[512];
    size_t par_val_len=0;
    size_t fn_len=0;
    bool func_found=false;
    while(*c && *c!=' ' && fn_len<sizeof(func_name)){
      ++fn_len;
      func_name[fn_len-1]=*c;
      ++c;
    }
    if(fn_len<sizeof(func_name)-1){
      ++fn_len;
      func_name[fn_len-1]=0;
    }else
      func_name[sizeof(func_name)-1]=0;
    while(*c==' ')++c;
    while(*c){
      if(*c=='$'){
        ++c;
        par_num=OTL_SCAST(int,*c-'0')-1;
        ++c;
        while(*c && *c==' ')++c;
        if(*c==':' && (c>stm_text && *(c-1)!='\\' || c==stm_text)){
          ++c;
          while(*c && *c==' ')++c;
          if(*c=='\''){
            par_val_len=0;
            ++c;
            while(*c && *c!='\'' && par_val_len<sizeof(par_val)){
              ++par_val_len;
              par_val[par_val_len-1]=*c;
              ++c;
            }
            if(par_val_len<sizeof(par_val)-1){
              ++par_val_len;
              par_val[par_val_len-1]=0;
            }else
              par_val[sizeof(par_val)-1]=0;
            if(par_num>=0 && par_num<ctl_arr_size){
              ctl_arr[par_num].name_ptr=ctl_arr[par_num].name;
              ctl_arr[par_num].name_len=SQL_NTS;
              strcpy(OTL_RCAST(char*,ctl_arr[par_num].name),
                     OTL_RCAST(const char*,par_val));
            }
          }
          ++c;
          while(*c && *c==' ')++c;
        }
      }else
        ++c;
    }
    status=SQL_SUCCESS;
    if(strcmp(func_name,"SQLTables")==0){
      status=SQLTables
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLColumns")==0){
      status=SQLColumns
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLProcedures")==0){
      status=SQLProcedures
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLColumnPrivileges")==0){
      status=SQLColumnPrivileges
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLTablePrivileges")==0){
      status=SQLTablePrivileges
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLPrimaryKeys")==0){
      status=SQLPrimaryKeys
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLProcedureColumns")==0){
      status=SQLProcedureColumns
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
      func_found=true;
    }else if(strcmp(func_name,"SQLForeignKeys")==0){
      status=SQLForeignKeys
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len,
         ctl_arr[4].name_ptr,ctl_arr[4].name_len,
         ctl_arr[5].name_ptr,ctl_arr[5].name_len);
      func_found=true;
    }
    if(!func_found)return 2;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  }

  if(direct_exec_flag){
   _rpc=0;
   status=SQLExecDirect
    (cda,
     OTL_RCAST(OTL_SQLCHAR_PTR,stm_text),
     SQL_NTS);
   if(status!=SQL_SUCCESS&&
      status!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
      status!=SQL_NO_DATA
#else
      status!=SQL_NO_DATA_FOUND
#endif
     )
    return 0;
   else{
    SQLINTEGER tmp_rpc=0;
    SQLRETURN diag_status=SQLRowCount(cda,&tmp_rpc);
    if(diag_status==SQL_SUCCESS||
       diag_status==SQL_SUCCESS_WITH_INFO)
     _rpc=OTL_SCAST(long,tmp_rpc);
    return 1;
   }
  }

  // Converting : notation into ODBC's native notation ?
  while(*c){
   if(*c=='\''){
    if(!in_str)
     in_str=1;
    else{
     if(c[1]=='\'')
      ++c;
     else
      in_str=0;
    }
   }
   if(*c==':' && !in_str && 
      (c>stm_text && *(c-1)!='\\' || c==stm_text)){
    *c='?';
    ++c;
    while(isdigit(*c)||isalpha(*c)||*c=='_'){
     *c=' ';
     ++c;
    }
   }else if(*c==':' && !in_str && 
            (c>stm_text && *(c-1)=='\\' || c==stm_text)){
     char* c_1=c-1;
     char* c_=c;
     while(*c_){
       *c_1=*c_;
       ++c_1;
       ++c_;
     }
     if(c_1>c-1)
       *c_1=0;
     --c;
   }
   ++c;
  }

 status=SQLPrepare
  (cda,
   OTL_RCAST(OTL_SQLCHAR_PTR,stm_text),
   SQL_NTS);
 if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
  return 0;
 else
  return 1;
 }

 int exec(const int iters, const int /*rowoff*/)
 {
#if (ODBCVER >= 0x0300)
#else
  OTL_SQLUINTEGER irows;
#endif
  if(direct_exec_flag){
   return 1;
  }else{
#if !defined(OTL_ODBC_MYSQL) && !defined(OTL_ODBC_XTG_IBASE6)
#if (ODBCVER >= 0x0300)
   if(iters>1||_rpc>1){
    status=SQLSetStmtAttr
     (cda,
      SQL_ATTR_PARAMSET_SIZE,
      OTL_RCAST(void*,iters),
      SQL_NTS
      );
    if(status!=SQL_SUCCESS&&
       status!=SQL_SUCCESS_WITH_INFO)
     return 0;
   }
#else
   if(iters>1||_rpc>1){
    status=SQLParamOptions
     (cda,
      OTL_SCAST(OTL_SQLUINTEGER,iters),
      &irows); 
    if(status!=SQL_SUCCESS&&
       status!=SQL_SUCCESS_WITH_INFO)
     return 0;
   }
#endif
#endif
   _rpc=0;

   last_param_data_token=0;
   last_sql_param_data_status=0;
   sql_param_data_count=0;
   
   status=SQLExecute(cda);
   if(canceled)return 0;
   if(status!=SQL_SUCCESS&&
      status!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
      status!=SQL_NO_DATA&&
#else
      status!=SQL_NO_DATA_FOUND&&
#endif
      status!=SQL_NEED_DATA)return 0;
   if(status==SQL_NEED_DATA){
    _rpc=iters;
    return 1;
   }
   SQLINTEGER tmp_rpc=0;
   SQLRETURN diag_status=SQLRowCount(cda,&tmp_rpc);
   if(diag_status==SQL_SUCCESS||
      diag_status==SQL_SUCCESS_WITH_INFO)
    _rpc=OTL_SCAST(long,tmp_rpc);
   return 1;
  }
 }

 long get_rpc()
 {
  return _rpc;
 }

 int tmpl_ftype2odbc_ftype(const int ftype)
 {
  switch(ftype){
  case otl_var_char:
   return SQL_C_CHAR;
  case otl_var_double:
   return SQL_C_DOUBLE;
#if defined(OTL_BIGINT)
  case otl_var_bigint:
   return SQL_C_SBIGINT;
#endif
  case otl_var_float:
   return SQL_C_FLOAT;
  case otl_var_int:
  case otl_var_long_int:
   return SQL_C_SLONG;
  case otl_var_unsigned_int:
   return SQL_C_ULONG;
  case otl_var_short:
   return SQL_C_SSHORT;
  case otl_var_timestamp:
  case otl_var_db2time:
  case otl_var_db2date:
   return SQL_C_TIMESTAMP;
  case otl_var_varchar_long:
   return SQL_LONGVARCHAR;
  case otl_var_raw_long:
   return SQL_LONGVARBINARY;
  default:
   return 0;
  }
 }

 int otl_map_ext2int(int ftype)
 {
  switch(ftype){
  case SQL_LONGVARCHAR: return SQL_LONGVARCHAR;
  case SQL_LONGVARBINARY: return SQL_LONGVARBINARY;
  case SQL_C_CHAR: return SQL_VARCHAR;
  case SQL_C_DATE: return SQL_DATE;
#if (ODBCVER >= 0x0300)
  case SQL_C_TIME: return SQL_TYPE_TIME;
  case SQL_C_TIMESTAMP: return SQL_TYPE_TIMESTAMP;
#else
  case SQL_C_TIME: return SQL_TIME;
  case SQL_C_TIMESTAMP: return SQL_TIMESTAMP;
#endif
  case SQL_C_DOUBLE: return SQL_DOUBLE;
#if defined(OTL_BIGINT)
  case SQL_C_SBIGINT: return SQL_BIGINT;
#endif
  case SQL_C_FLOAT: return SQL_FLOAT;
  case SQL_C_SLONG: return SQL_INTEGER;
  case SQL_C_SSHORT: return SQL_SMALLINT;
  case SQL_C_ULONG: return SQL_DOUBLE;
  default: return -1;
  }
 }

 int bind
 (const char* /* name */,
  otl_var& v,
  const int aelem_size,
  const int aftype,
  const int aparam_type,
  const int name_pos,
  const int /* apl_tab_size */)
 {OTL_SQLSMALLINT ftype=(OTL_SQLSMALLINT)tmpl_ftype2odbc_ftype(aftype);
  OTL_SQLSMALLINT ftype_save=ftype;
  int param_type;
  int parm_pos=name_pos;
  v.vparam_type=aparam_type;
  switch(aparam_type){
  case otl_input_param:
   param_type=SQL_PARAM_INPUT;
   break;
  case otl_output_param:
   param_type=SQL_PARAM_OUTPUT;
   break;
  case otl_inout_param:
   param_type=SQL_PARAM_INPUT_OUTPUT;
   break;
  default:
   param_type=SQL_PARAM_INPUT;
   break;
  }
  if(ftype==SQL_LONGVARCHAR){
   ftype=SQL_C_CHAR;
  }else if(ftype==SQL_LONGVARBINARY){
   ftype=SQL_C_BINARY;
  }
  int sqltype=otl_map_ext2int(ftype_save);
  int mapped_sqltype=sqltype;

  if(aftype==otl_var_db2date)
#if (ODBCVER >= 0x0300)
   mapped_sqltype=SQL_TYPE_DATE;
#else
   mapped_sqltype=SQL_DATE;
#endif
  else if(aftype==otl_var_db2time)
#if (ODBCVER >= 0x0300)
   mapped_sqltype=SQL_TYPE_TIME;
#else
   mapped_sqltype=SQL_TIME;
#endif
  if(v.lob_stream_mode&&
     (ftype_save==SQL_LONGVARBINARY||
      ftype_save==SQL_LONGVARCHAR)){
   // in case of "stream mode" the variable
   // gets bound in a special way
   status=SQLBindParameter
    (cda,                                             
     OTL_SCAST(OTL_SQLUSMALLINT,parm_pos),            
     OTL_SCAST(OTL_SQLSMALLINT,param_type),           
     ftype,                                           
     OTL_SCAST(OTL_SQLSMALLINT,mapped_sqltype),       
#if (ODBCVER >= 0x0300)
     sqltype==SQL_TYPE_TIMESTAMP?otl_odbc_date_prec:aelem_size,
#else
     sqltype==SQL_TIMESTAMP?otl_odbc_date_prec:aelem_size,
#endif
#if (ODBCVER >= 0x0300)
     sqltype==SQL_TYPE_TIMESTAMP?otl_odbc_date_scale:0,
#else
     sqltype==SQL_TIMESTAMP?otl_odbc_date_scale:0,
#endif
     OTL_RCAST(OTL_SQLPOINTER,parm_pos),              
     0,                     
     v.p_len);                                        
  }else{
    int temp_column_size=0;
#if (ODBCVER >= 0x0300)
    if(sqltype==SQL_TYPE_TIMESTAMP)
      temp_column_size=otl_odbc_date_prec;
    else if(ftype==SQL_C_CHAR)
      temp_column_size=aelem_size-1;
    else
      temp_column_size=aelem_size;
#else
    if(sqltype==SQL_TIMESTAMP)
      temp_column_size=otl_odbc_date_prec;
    else if(ftype==SQL_C_CHAR)
      temp_column_size=aelem_size-1;
    else
      temp_column_size=aelem_size;
#endif
   status=SQLBindParameter
    (cda,
     OTL_SCAST(OTL_SQLUSMALLINT,parm_pos),
     OTL_SCAST(OTL_SQLSMALLINT,param_type),
     ftype,
     OTL_SCAST(OTL_SQLSMALLINT,mapped_sqltype),
     temp_column_size,
#if (ODBCVER >= 0x0300)
     sqltype==SQL_TYPE_TIMESTAMP?otl_odbc_date_scale:0,
#else
     sqltype==SQL_TIMESTAMP?otl_odbc_date_scale:0,
#endif
     OTL_RCAST(OTL_SQLPOINTER,v.p_v),
     OTL_SCAST(OTL_SQLINTEGER,aelem_size),
     v.p_len);
  }
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
 }

 int bind
 (const int column_num,
  otl_var& v,
  const int elem_size,
  const int aftype,
  const int param_type)
 {SWORD ftype=(SWORD)tmpl_ftype2odbc_ftype(aftype);
  v.vparam_type=param_type;
  SWORD ftype_save=ftype;
  if(ftype==SQL_LONGVARCHAR){
   ftype=SQL_C_CHAR;
  }else if(ftype==SQL_LONGVARBINARY){
   ftype=SQL_C_BINARY;
  }
  if(v.lob_stream_mode&&
     (ftype_save==SQL_LONGVARBINARY||
      ftype_save==SQL_LONGVARCHAR)){
   // in case of "stream mode" the variable
   // remains unbound
   v.lob_ftype=ftype;
   v.lob_pos=column_num;
   return 1;
  }else{
   status=SQLBindCol
    (cda,
     OTL_SCAST(unsigned short,column_num),
     ftype,
     OTL_RCAST(PTR,v.p_v),
     OTL_SCAST(SDWORD,elem_size),
     &v.p_len[0]);
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;
  }

 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  OTL_SQLCHAR name[256];
  OTL_SQLSMALLINT nlen;
  OTL_SQLSMALLINT dbtype;
  OTL_SQLINTEGER dbsize;
  OTL_SQLSMALLINT scale;
  OTL_SQLUINTEGER prec;
  OTL_SQLSMALLINT nullok;
  OTL_SQLSMALLINT icols;

  eof_desc=0;
  status=SQLNumResultCols(cda,&icols);
  if(status!=SQL_SUCCESS&&
     status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  if(column_num>icols){
   eof_desc=1;
   return 0;
  }
  status=SQLDescribeCol
   (cda,
    OTL_SCAST(unsigned short,column_num),
    name,
    sizeof(name),
    &nlen,
    &dbtype,
    &prec,
    &scale,
    &nullok);
  if(status!=SQL_SUCCESS&&
     status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  dbsize=prec;
  strcpy(col.name,OTL_RCAST(char*,name));
#if defined(OTL_DB2_CLI) && defined(OTL_DB2_CLI_MAP_LONG_VARCHAR_TO_VARCHAR)
  if(dbtype==SQL_LONGVARCHAR && 
     dbsize <= OTL_DB2_CLI_MAP_LONG_VARCHAR_TO_VARCHAR){
    dbtype=SQL_VARCHAR;
  }
#endif
  col.dbtype=dbtype;
  col.dbsize=dbsize;
  col.scale=scale;
  col.prec=prec;
  col.nullok=nullok;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
#if (ODBCVER >= 0x0300)
  rc=SQLGetDiagRec
   (SQL_HANDLE_STMT,
    cda,
    1,
    OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
    OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
    OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
    SQL_MAX_MESSAGE_LENGTH-1,
    OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#else
 rc=SQLError(adb->henv, 
             adb->hdbc, 
             cda,
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
             OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
             SQL_MAX_MESSAGE_LENGTH-1,
             OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif

  exception_struct.msg[msg_len]=0;

  if(rc==SQL_INVALID_HANDLE||rc==SQL_ERROR)
    exception_struct.msg[0]=0;    
#if (ODBCVER >= 0x0300)
#ifdef OTL_EXTENDED_EXCEPTION
  else if(rc!=SQL_NO_DATA)
    otl_fill_exception(exception_struct,cda,SQL_HANDLE_STMT);
#endif
#endif
 }

};

class otl_sel{
public:

  int implicit_cursor;
  int status;
  OTL_SQLUINTEGER crow;
  int in_sequence;
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
  OTL_SQLUSMALLINT* row_status;
  int row_status_arr_size;
#endif
 
 int close_select(otl_cur& cur)
 {
  if(!in_sequence)return 1;
  status=SQLFreeStmt(cur.cda,SQL_CLOSE);
  in_sequence=0;
  if(status==SQL_ERROR)
   return 0;
  else
  return 1;
 }

 otl_sel()
 {
  implicit_cursor=0;
  in_sequence=0;
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
  row_status=0;
  row_status_arr_size=0;
#endif
 }

  virtual ~otl_sel()
  {
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
    if(row_status!=0){
      delete[] row_status;
      row_status=0;
      row_status_arr_size=0;
    }
#endif
  }

#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
  void alloc_row_status(const int array_size)
  {
    if(row_status==0){
      row_status=new OTL_SQLUSMALLINT[array_size];
      row_status_arr_size=array_size;
      memset(row_status,0,sizeof(OTL_SQLUSMALLINT)*array_size);
    }else if(row_status!=0 && array_size!=row_status_arr_size){
      delete[] row_status;
      row_status=new OTL_SQLUSMALLINT[array_size];
      row_status_arr_size=array_size;
      memset(row_status,0,sizeof(OTL_SQLUSMALLINT)*array_size);
    }
  }
#endif

 void set_select_type(const int atype)
 {
  implicit_cursor=atype;
 }

  void init(const int /* array_size */)
  {
  }

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {
#ifdef OTL_ODBC_XTG_IBASE6

  cur_row=-1;
  eof_data=0;
  if(!implicit_cursor){
   status=SQLExecute(cur.cda);
   if(cur.canceled)return 0;
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }
  crow=0;
  status=SQLFetch(cur.cda);
  if(cur.canceled)return 0;
  if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
   crow=1;
   in_sequence=1;
  }

#else

#ifdef OTL_ODBC_SQL_EXTENDED_FETCH_ON
  alloc_row_status(array_size);
#endif
  cur_row=-1;
  eof_data=0;
#if (ODBCVER >= 0x0300)
  status=SQLSetStmtAttr
   (cur.cda,
    SQL_ATTR_ROW_ARRAY_SIZE,
    OTL_RCAST(void*,array_size),
    SQL_NTS);
#else
 status=SQLSetStmtOption(cur.cda,SQL_ROWSET_SIZE,array_size); 
#endif
  if(cur.canceled)return 0;
  if(status!=SQL_SUCCESS&&
     status!=SQL_SUCCESS_WITH_INFO)
   return 0;
#ifdef OTL_ODBC_SQL_EXTENDED_FETCH_ON
#else
#if (ODBCVER >= 0x0300)
  status=SQLSetStmtAttr
   (cur.cda,SQL_ATTR_ROWS_FETCHED_PTR,&crow,SQL_NTS);
  if(cur.canceled)return 0;
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#else
#endif
#endif
  if(!implicit_cursor){
    status=SQLExecute(cur.cda);
    if(cur.canceled)return 0;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }
#ifdef OTL_ODBC_SQL_EXTENDED_FETCH_ON
  if(array_size==1){
   crow=0;
   status=SQLFetch(cur.cda);
   if(cur.canceled)return 0;
   if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
    crow=1;
    in_sequence=1;
   }
  }else{
   status=SQLExtendedFetch
    (cur.cda, 
     SQL_FETCH_NEXT,
     1,
     &crow, 
     row_status); 
  }
#else
#if (ODBCVER >= 0x0300)
  status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#else
  {
    alloc_row_status(array_size);
    status=SQLExtendedFetch
      (cur.cda, 
       SQL_FETCH_NEXT,
       1,
       &crow, 
       row_status); 
  }
#endif

#endif

#endif
  
  in_sequence=1;
  if(cur.canceled)return 0;
  if(status==SQL_ERROR||
     status==SQL_INVALID_HANDLE)
   return 0;
  if(status==SQL_NO_DATA_FOUND){
   eof_data=1;
   cur_row=-1;
   crow=0;
   row_count=0;
   cur_size=0;
   status=SQLFreeStmt(cur.cda,SQL_CLOSE);
   in_sequence=0;
   if(status==SQL_ERROR)return 0;
   return 1;
  }
  row_count=crow;
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

#ifdef OTL_ODBC_SQL_EXTENDED_FETCH_ON
 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {
   alloc_row_status(array_size);
#else
 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
#if (ODBCVER >= 0x0300)
  const int /* array_size */)
#else
  const int array_size)
#endif
 {
#endif
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    in_sequence=0;
    status=SQLFreeStmt(cur.cda,SQL_CLOSE);
    if(status==SQL_ERROR)return 0;
    return 1;
   }
#ifdef OTL_ODBC_SQL_EXTENDED_FETCH_ON
  if(array_size==1){
   crow=0;
   status=SQLFetch(cur.cda);
   if(cur.canceled)return 0;
   if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
    crow=1;
    in_sequence=1;
   }
  }else{
   status=SQLExtendedFetch
    (cur.cda, 
     SQL_FETCH_NEXT,
     1,
     &crow, 
     row_status); 
 }
#else
#if (ODBCVER >= 0x0300)
  status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#else
  {
    alloc_row_status(array_size);
    status=SQLExtendedFetch
      (cur.cda, 
       SQL_FETCH_NEXT,
       1,
       &crow, 
       row_status); 
  }
#endif
#endif
   in_sequence=1;
   if(cur.canceled)return 0;
   if(status==SQL_ERROR||
//    status==SQL_SUCCESS_WITH_INFO||
      status==SQL_INVALID_HANDLE)
    return 0;
   if(status==SQL_NO_DATA_FOUND){
    eof_data=1;
    cur_row=-1;
    cur_size=0;
    in_sequence=0;
    status=SQLFreeStmt(cur.cda,SQL_CLOSE);
    if(status==SQL_ERROR)return 0;
    return 1;
   }
   cur_size=crow;
   row_count+=crow;
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }

};

typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_odbc_connect;

typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time> otl_select_stream;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time> otl_inout_stream;

const long otl_tran_read_uncommitted=SQL_TRANSACTION_READ_UNCOMMITTED;
const long otl_tran_read_committed=SQL_TRANSACTION_READ_COMMITTED;
const long otl_tran_repeatable_read=SQL_TRANSACTION_REPEATABLE_READ;
const long otl_tran_serializable=SQL_TRANSACTION_SERIALIZABLE;

class otl_connect: public otl_odbc_connect{
public:

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 otl_stream_pool sc;

 void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
 {
  sc.init(max_size);
 }

#endif

 otl_connect() OTL_NO_THROW
 :otl_odbc_connect(){}

 otl_connect(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
   : otl_odbc_connect(connect_str, aauto_commit){}
  
 otl_connect(OTL_HENV ahenv,OTL_HDBC ahdbc,const int auto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
   : otl_odbc_connect()
  {
    rlogon(ahenv,ahdbc,auto_commit);
  }

  void rlogon(OTL_HENV ahenv,OTL_HDBC ahdbc,const int auto_commit=0)
    OTL_THROWS_OTL_EXCEPTION
  {
    retcode=connect_struct.ext_logon(ahenv,ahdbc,auto_commit);
    if(retcode)
      connected=1;
   else{
     connected=0;
     throw_count++;
     if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
     throw otl_exception(connect_struct);
   }
  }
  
 virtual ~otl_connect() 
   OTL_THROWS_OTL_EXCEPTION
  {}

 void rlogon(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_odbc_connect::rlogon(connect_str,aauto_commit);
 }

 void logoff(void) 
   OTL_THROWS_OTL_EXCEPTION
 {
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
   sc.init(sc.max_size);
#endif
  otl_odbc_connect::logoff();
 }

  void set_transaction_isolation_level(const long int level)
    OTL_THROWS_OTL_EXCEPTION
  {
    retcode=connect_struct.set_transaction_isolation_level(level);
    if(!retcode){
     throw_count++;
     if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
     throw otl_exception(connect_struct);
    }
  }

};

class otl_stream_shell: public otl_stream_shell_generic{
public:

 otl_select_stream* ss;
 otl_inout_stream* io;
 otl_connect* adb;

 int auto_commit_flag;

 otl_var_desc* iov;
 int iov_len;
 int next_iov_ndx;

 otl_var_desc* ov;
 int ov_len;
 int next_ov_ndx;

 bool flush_flag;
 bool lob_stream_flag;

 otl_select_struct_override override;

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 OTL_STRING_CONTAINER orig_sql_stm;
#endif

 otl_stream_shell()
 {
  should_delete=0;
 }

 otl_stream_shell(const int ashould_delete)
 {
  should_delete=0;
  iov=0; iov_len=0;
  ov=0; ov_len=0;
  next_iov_ndx=0;
  next_ov_ndx=0;
  override.len=0;
  ss=0; io=0;
  adb=0;
  flush_flag=true;
  should_delete=ashould_delete;
  lob_stream_flag=false;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=0; iov_len=0;
   ov=0; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override.len=0;
   flush_flag=true;

   delete ss;
   delete io;
   ss=0; io=0;
   adb=0;
  }
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_lob_stream: public otl_lob_stream_generic{
public:

  typedef otl_tmpl_exception
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct> otl_exception;

  typedef otl_tmpl_variable<TVariableStruct>* p_bind_var;
  typedef otl_tmpl_connect
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct>* p_connect;

 typedef otl_tmpl_cursor
 <TExceptionStruct,
  TConnectStruct,
  TCursorStruct,
  TVariableStruct>* p_cursor;
  
  p_bind_var bind_var;
  p_connect connect;
  p_cursor cursor;
  
 void init
 (void* avar,void* aconnect,void* acursor,
  int andx,int amode,const int alob_is_null=0)
   OTL_NO_THROW
 {
  connect=OTL_RCAST(p_connect,aconnect);
  bind_var=OTL_RCAST(p_bind_var,avar);
  cursor=OTL_RCAST(p_cursor,acursor);
  mode=amode;
  retcode=0;
  lob_is_null=alob_is_null;
  ndx=andx;
  offset=0;
  lob_len=0;
  eof_flag=0;
  in_destructor=0;
  if(bind_var)
   bind_var->var_struct.set_lob_stream_flag();
 }

 void set_len(const int new_len=0) OTL_NO_THROW
 {
  lob_len=new_len;
 }

 otl_tmpl_lob_stream() OTL_NO_THROW
   : otl_lob_stream_generic(false)
 {
  init(0,0,0,0,otl_lob_stream_zero_mode);
 }

 ~otl_tmpl_lob_stream() OTL_THROWS_OTL_EXCEPTION
 {in_destructor=1;
  close();
 }

 otl_lob_stream_generic& operator<<(const otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(mode!=otl_lob_stream_write_mode){
   char* stm=0;
   char var_info[256];
   var_info[0]=0;
   if(cursor!=0){
     if(cursor->stm_label)
       stm=cursor->stm_label;
     else
       stm=cursor->stm_text;
   }
   if(bind_var!=0){
    otl_var_info_var
     (bind_var->name,
      bind_var->ftype,
      otl_var_long_string,
      var_info);
   }
   char* vinfo=0;
   if(var_info[0]!=0)
    vinfo=&var_info[0];
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (otl_error_msg_9,
      otl_error_code_9,
      stm,
      vinfo);
  }
  if(offset==0)offset=1;
  if((offset-1)+s.length>lob_len){
   char var_info[256];
   otl_var_info_var
    (bind_var->name,
     bind_var->ftype,
     otl_var_long_string,
     var_info);
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_7,
     otl_error_code_7,
     cursor->stm_label?cursor->stm_label:cursor->stm_text,
     var_info);
  }
  retcode=bind_var->var_struct.write_blob
   (s,lob_len,offset,cursor->cursor_struct);
  if(retcode){
   if((offset-1)==lob_len)
    close();
   return *this;
  }
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 otl_lob_stream_generic& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(mode!=otl_lob_stream_read_mode){
   char* stm=0;
   char var_info[256];
   var_info[0]=0;
   if(cursor!=0){
     if(cursor->stm_label)
       stm=cursor->stm_label;
     else
       stm=cursor->stm_text;
   }
   if(bind_var!=0){
    otl_var_info_var
     (bind_var->name,
      bind_var->ftype,
      otl_var_long_string,
      var_info);
   }
   char* vinfo=0;
   if(var_info[0]!=0)
    vinfo=&var_info[0];
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_10,
     otl_error_code_10,
     stm,
     vinfo);
  }
  if(offset==0)offset=1;
  retcode=bind_var->var_struct.read_blob
   (cursor->cursor_struct,s,ndx,offset,eof_flag);
  if(retcode){
   if(eof())
    close();
   return *this;
  }
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 int eof(void) OTL_NO_THROW
 {
  if(mode!=otl_lob_stream_read_mode)return 1;
  if(lob_is_null)return 1;
  return eof_flag;
 }

 int len(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(cursor==0||connect==0||bind_var==0||lob_is_null)return 0;
  int alen;
  retcode=bind_var->var_struct.get_blob_len(ndx,alen);
  if(retcode)return alen;
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 void close(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(in_destructor){
   if(mode==otl_lob_stream_read_mode){
    bind_var->var_struct.set_lob_stream_flag(0);
    bind_var->set_not_null(0);
   }
   return;
  }
  if(mode==otl_lob_stream_zero_mode)return;
  if(mode==otl_lob_stream_read_mode){
   bind_var->var_struct.set_lob_stream_flag(0);
   bind_var->set_not_null(0);
   init(0,0,0,0,otl_lob_stream_zero_mode);
  }else{
   // write mode
   if(!(offset==0&&lob_len==0)&&(offset-1)!=lob_len){
    char var_info[256];
    char msg_buf[1024];
    strcpy(msg_buf,otl_error_msg_8);
    otl_var_info_var
     (bind_var->name,
      bind_var->ftype,
      otl_var_long_string,
      var_info);
    if(this->connect)this->connect->throw_count++;
    if(this->connect&&this->connect->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (msg_buf,
      otl_error_code_8,
      cursor->stm_label?cursor->stm_label:
      cursor->stm_text,
      var_info);
   }
   bind_var->var_struct.set_lob_stream_flag(0);
   bind_var->set_not_null(0);
  }
 }

};

typedef otl_tmpl_lob_stream
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_lob_stream;

class otl_stream{
public:

  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;
  
  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;
  
  otl_select_struct_override* override;
  int end_marker;
  int oper_int_called;
  int last_eof_rc;


protected:

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

public:

 long get_rpc() OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->get_rpc();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->_rfc;
  }else
   return 0;
 }

  operator int(void) OTL_NO_THROW
  {
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

  void cancel(void) OTL_THROWS_OTL_EXCEPTION
  {
    if((*ss)){
      (*adb)->reset_throw_count();
      int status=(*ss)->cursor_struct.cancel();
      if(status==0)
        throw otl_exception((*ss)->cursor_struct);
    }else if((*io)){
      (*adb)->reset_throw_count();
      int status=(*io)->cursor_struct.cancel();
      if(status==0)
        throw otl_exception((*io)->cursor_struct);
    }
  }

 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  if((*ss)){
   if((*ss)->vl_len>0){
    (*iov)=new otl_var_desc[(*ss)->vl_len];
    (*iov_len)=(*ss)->vl_len;
    for(i=0;i<(*ss)->vl_len;++i)
     (*ss)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*ss)->sl_len>0){
    (*ov)=new otl_var_desc[(*ss)->sl_len];
    (*ov_len)=(*ss)->sl_len;
    for(i=0;i<(*ss)->sl_len;++i)
     (*ss)->sl[i].copy_var_desc((*ov)[i]);
   }
  }else if((*io)){
   if((*io)->vl_len>0){
    (*iov)=new otl_var_desc[(*io)->vl_len];
    (*iov_len)=(*io)->vl_len;
    for(i=0;i<(*io)->vl_len;++i)
     (*io)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*io)->iv_len>0){
    (*ov)=new otl_var_desc[(*io)->iv_len];
    (*ov_len)=(*io)->iv_len;
    for(i=0;i<(*io)->iv_len;++i)
     (*io)->in_vl[i]->copy_var_desc((*ov)[i]);
   }
  }
 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0)
   OTL_NO_THROW
 {
   if(shell==0){
     init_stream();
     shell->flush_flag=true;
   }
  override->add_override(column_ndx,col_type,col_size);
 }

  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    if(shell==0){
      init_stream();
      shell->flush_flag=true;
    }
    override->set_all_column_types(mask);
  }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==0)init_stream();
  shell->flush_flag=flush_flag;
 }

 void set_lob_stream_mode(const bool lob_stream_flag=false)
   OTL_NO_THROW
 {
  if(shell==0)return;
  shell->lob_stream_flag=lob_stream_flag;
 }

 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  return &(shell->iov[shell->next_iov_ndx]);
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  return &(shell->ov[shell->next_ov_ndx]);
 }

 void init_stream(void)
 {
  shell=0;
  shell=new otl_stream_shell(0);
  shell_pt.assign(&shell);
  connected=0;

  ss=&(shell->ss);
  io=&(shell->io);
  adb=&(shell->adb);
  auto_commit_flag=&(shell->auto_commit_flag);
  iov=&(shell->iov);
  iov_len=&(shell->iov_len);
  next_iov_ndx=&(shell->next_iov_ndx);
  ov=&(shell->ov);
  ov_len=&(shell->ov_len);
  next_ov_ndx=&(shell->next_ov_ndx);
  override=&(shell->override);
  
  (*io)=0;
  (*ss)=0;
  (*adb)=0;
  (*ov)=0; 
  (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*auto_commit_flag)=1;
  (*iov)=0; 
  (*iov_len)=0;
 }

 otl_stream
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  init_stream();

  (*io)=0; (*ss)=0;
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,implicit_select,sqlstm_label);
 }

 otl_stream()
   OTL_NO_THROW
 {
  init_stream();
  shell->flush_flag=true;
 }

 virtual ~otl_stream()
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  try{
   if((*io)!=0&&shell->flush_flag==false)
    (*io)->flush_flag2=false;
   close();
   if(shell!=0){
    if((*io)!=0)
     (*io)->flush_flag2=true;
   }
  }catch(otl_exception&){
   if(shell!=0){
    if((*io)!=0)
     (*io)->flush_flag2=true;
   }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=0)
    shell->should_delete=1;
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif
   throw;
  }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(adb && (*adb) && (*adb)->throw_count>0
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  || STD_NAMESPACE_PREFIX uncaught_exception()
#endif
     ){
   //
  }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void) OTL_NO_THROW
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->eof();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->eof();
  }else
   return 1;
 }

 void flush(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush();
  }
 }

 void clean(const int clean_up_error_flag=0) 
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->clean(clean_up_error_flag);
  }else if((*ss)){
    (*adb)->reset_throw_count();
    (*ss)->clean();
  }
 }

 void rewind(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->rewind();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   (*ss)->rewind();
  }
 }

 
 int is_null(void) OTL_NO_THROW
 {
  if((*io))
   return (*io)->is_null();
  else if((*ss))
   return (*ss)->is_null();
  else
   return 0;
 }

 void set_commit(int auto_commit=0) OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }
 
 void open
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if(shell==0)
   init_stream();
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  char temp_buf[128];
  otl_itoa(arr_size,temp_buf);
  OTL_STRING_CONTAINER sql_stm=
    OTL_STRING_CONTAINER(temp_buf)+
    OTL_STRING_CONTAINER("===>")+
    sqlstm;
  otl_stream_shell* temp_shell=OTL_RCAST(otl_stream_shell*,db.sc.find(sql_stm));
  if(temp_shell){
   if(shell!=0)shell_pt.destroy();
   shell=temp_shell;
   ss=&(shell->ss);
   io=&(shell->io);
   if((*io)!=0)(*io)->flush_flag2=true;
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
   override=&(shell->override);
   if((*iov_len)==0)
    this->rewind();
   connected=1;
   return;
  }
  shell->orig_sql_stm=sql_stm;
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  override->lob_stream_mode=shell->lob_stream_flag;
  while(isspace(*c)||(*c)=='(')++c;
  strncpy(tmp,c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==0)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
   if((strncmp(tmp,"SELECT",6)==0||
       strncmp(tmp,"WITH",4)==0)&&
      !implicit_select){
    (*ss)=new otl_select_stream(override,arr_size,sqlstm,
                                db,otl_explicit_select,
                                sqlstm_label);
   }else if(tmp[0]=='$'){
     (*ss)=new otl_select_stream
       (override,arr_size,
        sqlstm,db,
        1,sqlstm_label);
   }else{
    if(implicit_select)
     (*ss)=new otl_select_stream(override,arr_size,
                                 sqlstm,db,
                                 1,sqlstm_label);
    else{
     (*io)=new otl_inout_stream(arr_size,sqlstm,db,
                                shell->lob_stream_flag,
                                sqlstm_label);
     (*io)->flush_flag=shell->flush_flag;
    }
   }
  }catch(otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override->len=0;
  override->lob_stream_mode=false;

  if((*ss)){
   try{
    (*ss)->close();
   }catch(otl_exception&){
    delete (*ss);
    (*ss)=0;
    throw;
   }
   delete (*ss);
   (*ss)=0;
  }

  if((*io)){
   try{
    (*io)->flush();
   }catch(otl_exception&){
    clean(1);
    delete (*io);
    (*io)=0;
    throw;
   }
   delete (*io);
   (*io)=0;
  }
  (*ss)=0; (*io)=0;
  if(adb!=0)(*adb)=0; 
  adb=0;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
   OTL_THROWS_OTL_EXCEPTION
#else
 void close(void)
   OTL_THROWS_OTL_EXCEPTION
#endif
 {
  if(shell==0)return;
#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool&&(*adb)&&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(STD_NAMESPACE_PREFIX uncaught_exception())&&
#endif
     (*adb)->throw_count==0){
   try{
    this->flush();
    this->clean(1);
   }catch(otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->throw_count>0){
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#endif
   (*adb)->sc.add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if((*ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ss)->sl_len;
   return (*ss)->sl_desc;
  }
  return 0;
 }

 int good(void) OTL_NO_THROW
 {
  if(!connected)return 0;
  if((*io)||(*ss)){
   (*adb)->reset_throw_count();
   return 1;
  }else
   return 0;
 }

 otl_stream& operator<<(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(s);
   inc_next_iov();
  }
  return *this;
 }

 otl_stream& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_time& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
  return *this;
 }

 otl_stream& operator<<(const otl_time& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss)){
   (*ss)->operator<<(n);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  return *this;
 }

 otl_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time tmp;
  (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
   s.year=tmp.year;
   s.month=tmp.month;
   s.day=tmp.day;
   s.hour=tmp.hour;
   s.minute=tmp.minute;
   s.second=tmp.second;
   s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
  }
#else
  s.year=tmp.year;
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour;
  s.minute=tmp.minute;
  s.second=tmp.second;
  s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
#endif
  inc_next_ov();
  return *this;
 }

 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time tmp;
   reset_end_marker();
  tmp.year=s.year;
  tmp.month=s.month;
  tmp.day=s.day;
  tmp.hour=s.hour;
  tmp.minute=s.minute;
  tmp.second=s.second;
  tmp.fraction=otl_to_fraction(s.fraction,s.frac_precision);
  (*this)<<tmp;  
  inc_next_iov();
  return *this;
 }

 otl_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(s,OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(OTL_RCAST(char*,s),
            OTL_RCAST(const char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
           );
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(n);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(u);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(u);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(sh);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(sh);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(l);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(l);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

#if defined(OTL_BIGINT)
 otl_stream& operator>>(OTL_BIGINT& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(l);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(l);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(f);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(f);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(d);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(d);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss)){
   (*ss)->operator<<(c);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss)){
   (*ss)->operator<<(c);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss)){
   (*ss)->operator<<(s);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }
#endif

 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss)){
   (*ss)->operator<<(s);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss)){
   (*ss)->operator<<(s);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss)){
   (*ss)->operator<<(n);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

#if defined(OTL_BIGINT)
 otl_stream& operator<<(const OTL_BIGINT n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss)){
    (*ss)->operator<<(n);
    if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }
#endif

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(u);
  else if((*ss)){
   (*ss)->operator<<(u);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(sh);
  else if((*ss)){
   (*ss)->operator<<(sh);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(l);
  else if((*ss)){
   (*ss)->operator<<(l);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(f);
  else if((*ss)){
   (*ss)->operator<<(f);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(d);
  else if((*ss)){
   (*ss)->operator<<(d);
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

  otl_stream& operator<<(const otl_null& /* n */)
    OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))(*io)->operator<<(otl_null());
  if((*ss)){
   (*ss)->operator<<(otl_null());
   if(!(*ov)&&(*ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(d);
   inc_next_iov();
  }
  return *this;
 }

};

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

template <OTL_TYPE_NAME TData>
STD_NAMESPACE_PREFIX ostream& operator<<
  (STD_NAMESPACE_PREFIX ostream& s, 
   const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<"NULL";
 else
  s<<var.v;
 return s;
}

inline STD_NAMESPACE_PREFIX ostream& operator<<(
 STD_NAMESPACE_PREFIX ostream& s, 
 const otl_value<otl_datetime>& var
) OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<"NULL";
 else{
  s<<var.v.month<<"/"<<var.v.day<<"/"<<var.v.year<<" "
   <<var.v.hour<<":"<<var.v.minute<<":"<<var.v.second;
 }
 return s;
}

#endif


class otl_nocommit_stream: public otl_stream{
public:

 otl_nocommit_stream() OTL_NO_THROW
   : otl_stream() 
 {
  set_commit(0);
 }

 otl_nocommit_stream
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select)
   OTL_THROWS_OTL_EXCEPTION
  : otl_stream(arr_size,sqlstm,db,implicit_select)
 {
  set_commit(0);
 }

 void open
 (short int arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_stream::open(arr_size,sqlstm,db,implicit_select);
  set_commit(0);
 }

};


OTL_ODBC_NAMESPACE_END
#endif

// ==================== OTL-Adapter for Oracle 7 =====================
#ifdef OTL_ORA7

#ifdef OTL_ORA_TEXT_ON
#define text OTL_ORA_TEXT
#endif

extern "C"{
#include <ociapr.h>
}

OTL_ORA7_NAMESPACE_BEGIN

 const int inVarChar2=1;
 const int inNumber=2;
 const int inLong=8;
 const int inRowId=11;
 const int inDate=12;
 const int inRaw=23;
 const int inLongRaw=24;
 const int inChar=96;
 const int inMslabel=106;

 const int  extVarChar2=inVarChar2;
 const int  extNumber=inNumber;
 const int  extInt=3;
 const int  extFloat=4;
 const int  extCChar=5;
 const int  extVarNum=6;
 const int  extLong=inLong;
 const int  extVarChar=9;
 const int  extRowId=inRowId;
 const int  extDate=inDate;
 const int  extVarRaw=15;
 const int  extRaw=inRaw;
 const int  extLongRaw=inLongRaw;
 const int  extUInt=68;
 const int  extLongVarChar=94;
 const int  extLongVarRaw=95;
 const int  extChar=inChar;
 const int  extCharZ=97;
 const int  extMslabel=inMslabel;

typedef otl_oracle_date otl_time0;

class otl_exc{
public:
 unsigned char msg[1000];
 int code;
 char sqlstate[32];

#ifdef OTL_EXTENDED_EXCEPTION
 char** msg_arr;
 char** sqlstate_arr;
 int* code_arr;
 int arr_len;
#endif


 enum{disabled=0,enabled=1};

 otl_exc()
 {
  sqlstate[0]=0;
  msg[0]=0;
  code=0;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

 void init(const char* amsg, const int acode)
 {
  strcpy(OTL_RCAST(char*,msg),amsg);
  code=acode;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

};

class otl_conn{
public:

 Lda_Def* lda;
 unsigned char hda[512];
 int extern_lda;

 static int initialize(const int threaded_mode=0)
 {
  if(threaded_mode)
   return !opinit(1);
  else
   return 1;
 }

 otl_conn()
 {
  extern_lda=0;
  lda=new Lda_Def;
  memset(lda,0,sizeof(*lda));
  memset(hda,0,sizeof(hda));
 }

 virtual ~otl_conn()
 {
  delete lda;
 }

  void set_timeout(const int /*atimeout*/=0){}
  void set_cursor_type(const int /*acursor_type*/=0){}

 int ext_logon(Lda_Def* ext_lda,const int auto_commit)
 {int rc;
  if(!extern_lda)delete lda;
  lda=ext_lda;
  extern_lda=1;
  if(auto_commit){
   rc=ocon(lda);
   if(rc)
    return 0;
   else
    return 1;
  }
  return 1;
 }

 int rlogon(const char* connect_str,const int auto_commit)
 {
  if(!extern_lda)delete lda;
  lda=new Lda_Def;
  extern_lda=0;
  memset(lda,0,sizeof(*lda));
  memset(hda,0,sizeof(hda));
  int rc=olog(lda,
              hda,
              OTL_RCAST(unsigned char*,OTL_CCAST(char*,connect_str)),
              -1,
              0,
              -1,
              0,
              -1,
              0
             );
  if(rc)return 0;
  if(!auto_commit)return 1;
  rc=ocon(lda);
  if(rc)
   return 0;
  else
   return 1;
 }

 int logoff(void)
 {
  if(extern_lda){
   lda=0;
   extern_lda=0;
   return 1;
  }else{
   if(!lda)return 1;
   if(lda->rc==3113||lda->rc==1041||lda->rc==1033||lda->rc==1034){
    delete lda;
    lda=0;
    return 1;
   }
   int rc=ologof(lda);
   delete lda;
   lda=0;
   return !rc;
  }
 }

 void error(otl_exc& exception_struct)
 {
  if(!lda){
   exception_struct.code=3113;
   strcpy(OTL_RCAST(char*,exception_struct.msg),
          "ORA-03113: end-of-file on communication channel"
         );
   return;
  }
  int len;
  exception_struct.code=lda->rc;
  oerhms
   (lda,
    lda->rc,
    exception_struct.msg,
    sizeof(exception_struct.msg)
    );
  len = strlen(OTL_RCAST(const char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

 int commit(void)
 {
  return !ocom(lda);
 }

 int auto_commit_on(void)
 {
  return !ocon(lda);
 }

 int auto_commit_off(void)
 {
  return !ocof(lda);
 }

 int rollback(void)
 {
  return !orol(lda);
 }

};

class otl_var{
public:

 ub1* p_v;
 sb2* p_ind;
 ub2* p_rlen;
 ub2* p_rcode;
 int ftype;
 int act_elem_size;
 int array_size;
 ub4 max_tab_len;
 ub4 cur_tab_len;
 int pl_tab_flag;
 int vparam_type;
 int lob_len;
 int lob_pos;
 int lob_ftype;
 int otl_adapter;
 bool lob_stream_mode;

 otl_var()
 {
  otl_adapter=otl_ora7_adapter;
  p_v=0;
  p_ind=0;
  p_rlen=0;
  p_rcode=0;
  act_elem_size=0;
  max_tab_len=0;
  cur_tab_len=0;
  pl_tab_flag=0;
  vparam_type=-1;
  lob_len=0;
  lob_pos=0;
  lob_ftype=0;
  lob_stream_mode=false;
 }

 virtual ~otl_var()
 {
  delete[] p_v;
  delete[] p_ind;
  delete[] p_rlen;
  delete[] p_rcode;
 }

  int write_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

  int read_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const int aftype,
  int& aelem_size,
  const short aarray_size,
  const void* /*connect_struct*/=0,
  const int apl_tab_flag=0)
 {
  int i,elem_size;
  ftype=aftype;
  pl_tab_flag=apl_tab_flag;
  act_elem_size=aelem_size;
  if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long){
   elem_size=aelem_size+sizeof(sb4);
   array_size=1;
  }else{
   elem_size=aelem_size;
   array_size=aarray_size;
  }

  p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
  p_ind=new sb2[array_size];
  p_rlen=new ub2[array_size];
  p_rcode=new ub2[array_size];
  memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));

  if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long){
   if(aelem_size>32760)
    p_ind[0]=0;
   else
   p_ind[0]=(short)aelem_size;
   p_rcode[0]=0;
  }else{
   for(i=0;i<array_size;++i){
    p_ind[0]=OTL_SCAST(short,aelem_size);
    p_rlen[i]=OTL_SCAST(short,elem_size);
    p_rcode[i]=0;
   }
  }
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=OTL_SCAST(ub4,array_size);
 }

 void set_pl_tab_len(const int apl_tab_len)
 {
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=OTL_SCAST(ub4,apl_tab_len);
 }

 int get_pl_tab_len(void)
 {
  return OTL_SCAST(int,cur_tab_len);
 }

 int get_max_pl_tab_len(void)
 {
  return OTL_SCAST(int,max_tab_len);
 }

 int put_blob(void)
 {
  return 1;
 }

 int get_blob
 (const int /* ndx */,
  unsigned char* /* abuf */,
  const int /* buf_size */,
  int& /* len */)
 {
  return 1;
 }

 int save_blob
 (const unsigned char* /* abuf */,
  const int /* len */,
  const int /* extern_buffer_flag */)
 {
  return 1;
 }

 void set_null(int ndx)
 {
  p_ind[ndx]=-1;
 }

 void set_not_null(int ndx, int pelem_size)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   p_ind[0]=0;
  else
   p_ind[ndx]=OTL_SCAST(short,pelem_size);
 }

 void set_len(int len, int ndx)
 {
  if(ftype==otl_var_varchar_long||
     ftype==otl_var_raw_long)
   *OTL_RCAST(sb4*,p_v)=len;
  else
   p_rlen[ndx]=OTL_SCAST(short,len);
 }

 int get_len(int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   return *OTL_RCAST(sb4*,p_v);
  else
   return p_rlen[ndx];
 }

 int is_null(int ndx)
 {
  return p_ind[ndx]==-1;
 }

 void* val(int ndx,int pelem_size)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   return OTL_RCAST(void*,(p_v+sizeof(sb4)));
  else
  return OTL_RCAST(void*,&p_v[((unsigned)ndx)*pelem_size]);
 }

 static int int2ext(int int_type)
 {
  switch(int_type){
  case inVarChar2: return extCChar;
  case inNumber:   return extFloat;
  case inLong:     return extLongVarChar;
  case inRowId:    return extCChar;
  case inDate:     return extDate;
  case inRaw:      return extLongVarRaw;
  case inLongRaw:  return extLongVarRaw;
  case inChar:     return extCChar;
  default:
   return -1;
  }
 }

 static int datatype_size(int ftype,int maxsz,int int_type,int max_long_size)
 {
  switch(ftype){
  case extCChar:
   switch(int_type){
   case inRowId:
    return 30;
   case inDate:
    return otl_oracle_date_size;
   case inRaw:
     return max_long_size;
   default:
    return maxsz+1;
   }
  case extLongVarChar:
   return max_long_size;
  case extLongVarRaw:
   return max_long_size;
  case extFloat:
   return sizeof(double);
  case extDate:
   return otl_oracle_date_size;
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& ftype,
  int& elem_size,
  otl_select_struct_override& override,
  const int column_ndx)
 {int ndx=override.find(column_ndx);
  if(ndx==-1){
   ftype=int2ext(desc.dbtype);
   elem_size=datatype_size(ftype,desc.dbsize,desc.dbtype,max_long_size);
   switch(ftype){
   case extCChar:
    ftype=otl_var_char;
    break;
   case extFloat:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_double;
    break;
   case extLongVarChar:
    ftype=otl_var_varchar_long;
    break;
   case extLongVarRaw:
    ftype=otl_var_raw_long;
    break;
   case extDate:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
     ftype=otl_var_timestamp;
    break;
   }
  }else{
   ftype=override.col_type[ndx];
   switch(ftype){
   case otl_var_char:
    elem_size=override.col_size[ndx];
    break;
   case otl_var_double:
    elem_size=sizeof(double);
    break;
   case otl_var_float:
    elem_size=sizeof(float);
    break;
   case otl_var_int:
    elem_size=sizeof(int);
    break;
   case otl_var_unsigned_int:
    elem_size=sizeof(unsigned);
    break;
   case otl_var_short:
    elem_size=sizeof(short);
    break;
   case otl_var_long_int:
    elem_size=sizeof(long);
    break;
   default:
    elem_size=override.col_size[ndx];
    break;
   }
  }
  desc.otl_var_dbtype=ftype;
 }

};

class otl_cur{
public:

 Cda_Def cda;

 ub4& rpc; // reference to "rows processed count"
 ub2& ft; // reference to "OCI function code"
 ub2& rc; // reference to "V7 return code"
 ub2& peo; // reference to "parse error offset"
 int last_param_data_token;
 int last_sql_param_data_status;
 int sql_param_data_count;
 bool canceled;


 otl_cur& operator=(const otl_cur& cur)
 {
  rpc=cur.rpc;
  ft=cur.ft;
  rc=cur.rc;
  peo=cur.peo;
  memcpy(OTL_RCAST(void*,&cda),
         OTL_RCAST(void*,OTL_CCAST(cda_def *,&cur.cda)),
         sizeof(cda));
  return *this;
 }

 otl_cur()
  : rpc(cda.rpc), ft(cda.ft), rc(cda.rc), peo(cda.peo)
 {
  memset(&cda,0,sizeof(cda));
  last_param_data_token=0;
  last_sql_param_data_status=0;
  sql_param_data_count=0;
  canceled=false;
 }

 virtual ~otl_cur(){}

  int open(otl_conn& /* connect */,otl_var* /* var */)
  {
    return 1;
  }

 long get_rpc()
 {
  return rpc;
 }

  void set_direct_exec(const int /* flag */){}

 int open(otl_conn& connect)
 {
  memset(&cda,0,sizeof(cda));
  return !oopen(&cda,connect.lda,0,-1,-1,0,-1);
 }

 int close(void)
 {
  if(cda.rc==3113||cda.rc==1041||cda.rc==1033||cda.rc==1034)
   return 1;
  return !oclose(&cda);
 }

 int parse(const char* stm_text)
 {
  return !oparse(&cda,OTL_RCAST(unsigned char*,OTL_CCAST(char*,stm_text)),-1,0,1);
 }

  int exec(const int iters, const int /*rowoff*/)
 {
  int temp_rc=oexn(&cda,iters,0);
  return !temp_rc;
 }

 int fetch(const short iters,int& eof_data)
 {int temp_rc=ofen(&cda,iters);
  eof_data=0;
  if(cda.rc==1403){
   eof_data=1;
   return 1;
  }else if(temp_rc==0)
   return 1;
  else
   return 0;
 }

 int tmpl_ftype2ora_ftype(const int ftype)
 {
  switch(ftype){
  case otl_var_char:
   return extCChar;
  case otl_var_double:
   return extFloat;
  case otl_var_float:
   return extFloat;
  case otl_var_int:
   return extInt;
  case otl_var_long_int:
   return extInt;
  case otl_var_unsigned_int:
   return extUInt;
  case otl_var_short:
   return extInt;
  case otl_var_timestamp:
   return extDate;
  case otl_var_varchar_long:
   return extLongVarChar;
  case otl_var_raw_long:
   return extLongVarRaw;
  default:
   return 0;
  }
 }

 int bind
 (const char* name,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /* param_type */,
  const int /* name_pos */,
  const int apl_tab_flag)
 {
  if(apl_tab_flag)
   return !obndra(&cda,
                  OTL_RCAST(unsigned char*,OTL_CCAST(char*,name)),
                  -1,
                  OTL_RCAST(ub1*,v.p_v),
                  elem_size,
                  tmpl_ftype2ora_ftype(ftype),
                  -1,
                  v.p_ind,
                  v.p_rlen,
                  v.p_rcode,
                  v.max_tab_len,
                  &v.cur_tab_len,
                  0,
                  -1,
                  -1);
  else
   return !obndrv
    (&cda,
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,name)),
     -1,
     (ub1*)v.p_v,
     elem_size,
     tmpl_ftype2ora_ftype(ftype),
     -1,
     v.p_ind,
     0,
     -1,
     -1);
 }

 int bind
 (const int column_num,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /* param_type */)
 {
  return !odefin
   (&cda,
    column_num,
    OTL_RCAST(ub1*,v.p_v),
    elem_size,
    tmpl_ftype2ora_ftype(ftype),
    -1,
    v.p_ind,
    0,
    -1,
    -1,
    v.p_rlen,
    v.p_rcode);
 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  sb1  name[241];
  sb4  nlen;
  sb4  dbsize;
  sb2  dbtype;

  sb2  scale;
  sb2  prec;
  sb4  dsize;
  sb2  nullok;

  nlen=sizeof(name);
  int temp_rc=odescr
   (&cda,
    column_num,
    &dbsize,
    &dbtype,
    &name[0],
    &nlen,
    &dsize,
    &prec,
    &scale,
    &nullok);
  if(temp_rc==0)name[nlen]=0;
  eof_desc=0;
  if(cda.rc==1007){
   eof_desc=1;
   return 0;
  }
  if(temp_rc==0){
   strcpy(col.name,OTL_RCAST(char*,name));
   col.dbtype=dbtype;
   col.dbsize=dbsize;
   col.scale=scale;
   col.prec=prec;
   col.nullok=nullok;
   return 1;
  }else
   return 0;
 }

 void error(otl_exc& exception_struct)
 {
  int len;
  exception_struct.code=cda.rc;
  oerhms
   (&cda,
    cda.rc,
    exception_struct.msg,
    sizeof(exception_struct.msg)
    );
  len=strlen(OTL_RCAST(const char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

};

class otl_sel{
public:

 int implicit_cursor;

  int close_select(otl_cur& /* cur */)
  {
    int rc=1;
    return rc;
  }

 otl_sel()
 {
  implicit_cursor=0;
 }

 virtual ~otl_sel(){}

  void set_select_type(const int /* atype */)
 {
  implicit_cursor=0;
 }

  void init(const int /* array_size */){}

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  eof_data=0;
  cur_row=-1;
  rc=cur.exec(1,0);
  if(rc==0)return 0;
  rc=cur.fetch(OTL_SCAST(short,array_size),eof_data);
  if(rc==0)return 0;
  row_count=cur.rpc;
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    return 1;
   }
   rc=cur.fetch(OTL_SCAST(short,array_size),eof_data);
   if(rc==0)return 0;
   cur_size=cur.rpc-row_count;
   row_count=cur.rpc;
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }

};

typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_ora7_connect;

typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time0> otl_inout_stream;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time0> otl_select_stream;


typedef otl_tmpl_ext_hv_decl
 <otl_var,
  otl_time0,
  otl_exc,
  otl_conn,
  otl_cur> otl_ext_hv_decl;

class otl_stream_shell;

class otl_connect: public otl_ora7_connect{
public:

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 otl_stream_pool sc;

 void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
 {
  sc.init(max_size);
 }

#endif


 otl_connect() OTL_NO_THROW
   :otl_ora7_connect(){}

 otl_connect(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
   : otl_ora7_connect(connect_str, aauto_commit){}

 virtual ~otl_connect() OTL_THROWS_OTL_EXCEPTION
  {}

 void rlogon(Lda_Def* alda)
   OTL_THROWS_OTL_EXCEPTION
 {
  connected=0;
  long_max_size=32760;
  retcode=connect_struct.ext_logon(alda,0);
  if(retcode)
   connected=1;
  else{
   connected=0;
   throw_count++;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void rlogon(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_ora7_connect::rlogon(connect_str,aauto_commit);
 }

 void logoff(void)
 {
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
   sc.init(sc.max_size);
#endif
  otl_ora7_connect::logoff();
 }

};


// ============ OTL Reference Cursor Streams for Oracle 7 =================

typedef otl_tmpl_variable<otl_var> otl_generic_variable;
typedef otl_generic_variable* otl_p_generic_variable;

class otl_ref_cursor: public
otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>{
public:

 int cur_row;
 int cur_size;
 int row_count;
 int array_size;

 otl_ref_cursor
 (otl_connect& db,
  const char* cur_placeholder_name,
  const short arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db)
 {int i;
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=0;
  strcpy(cur_placeholder,cur_placeholder_name);
 }

 otl_ref_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
 sel_cur()
 {
 }

 virtual ~otl_ref_cursor()
 {
  this->in_destructor=1;
  delete[] rvl;
  rvl=0;
 }

 void open
 (otl_connect& db,
  const char* cur_placeholder_name,
  const short arr_size=1)
 {int i;
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=0;
  strcpy(cur_placeholder,cur_placeholder_name);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db);
 }

 void close(void)
 {
  delete[] rvl;
  rvl=0;
  if(sel_cur.connected && sel_cur.adb==0)
   sel_cur.adb=adb;
  sel_cur.close();
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::close();
 }

 int first(void)
 {int i,rc;
  rc=obndrv
   (&cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.cursor_struct.cda),
    sizeof(sel_cur.cursor_struct.cda),
    102,-1,0,0,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  if(cur_row==-2)
   ; // Special case -- calling describe_select() between parse() and first()
  else{
   exec(1); // Executing the PLSQL master block
   sel_cur.connected=1;
  }
  cur_row=-1;
  for(i=0;i<vl_cur_len;++i)
   sel_cur.bind(i+1,*rvl[i]);
  rc=sel_cur.cursor_struct.fetch
   (OTL_SCAST(short,array_size),
    sel_cur.eof_data);
  if(rc==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception(sel_cur.cursor_struct,stm_label?stm_label:stm_text);
  }
  row_count=sel_cur.cursor_struct.cda.rpc;
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
   if(sel_cur.eof()){
    cur_row=-1;
    return 0;
   }
   rc=sel_cur.cursor_struct.fetch
    (OTL_SCAST(short,array_size),
     sel_cur.eof_data);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
    throw otl_exception(sel_cur.cursor_struct,stm_label?stm_label:stm_text);
   }
   cur_size=sel_cur.cursor_struct.cda.rpc-row_count;
   row_count=sel_cur.cursor_struct.cda.rpc;
   if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  ++vl_cur_len;
  rvl[vl_cur_len-1]=&v;
  v.pos=column_num;
 }

 void bind(otl_generic_variable& v)
 {
  if(v.pos)
   bind(v.pos,v);
  else if(v.name)
   otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(v);
 }

 void bind
 (const char* name,
  otl_generic_variable& v)
 {
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(name,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i,rc;
  rc=obndrv
   (&cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.cursor_struct.cda),
    sizeof(sel_cur.cursor_struct.cda),
    102,-1,0,0,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  exec(1); // Executing the PLSQL master block
  sel_cur.connected=1;
  cur_row=-2; // Special case -- describe_select() before first() or next()
  desc_len=0;
  for(i=1;sel_cur.describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }

public:

 otl_cursor sel_cur;

protected:

 int rvl_len;
 otl_p_generic_variable* rvl;
 int vl_cur_len;
 char cur_placeholder[64];

};


class otl_ref_select_stream: public otl_ref_cursor{
public:

 otl_select_struct_override* override;
 long _rfc;

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

 otl_ref_select_stream
 (otl_select_struct_override* aoverride,
  const short arr_size,
  const char* sqlstm,
  const char* acur_placeholder,
  otl_connect& db,
  const char* sqlstm_label=0)
  :otl_ref_cursor(db,acur_placeholder,arr_size)
 {
   if(sqlstm_label!=0){
     if(stm_label!=0){
       delete[] stm_label;
       stm_label=0;
     }
     stm_label=new char[strlen(sqlstm_label)+1];
     strcpy(stm_label,sqlstm_label);
   }
  _rfc=0;
  init();
  {
   int len=strlen(sqlstm)+1;
   stm_text=new char[len];
   strcpy(stm_text,sqlstm);
   otl_ext_hv_decl hvd(stm_text,1);
   hvd.alloc_host_var_list(vl,vl_len,*adb);
  }
  override=aoverride;
  try{
   parse();
   if(vl_len==0){
    rewind();
    null_fetched=0;
   }
  }catch(otl_exception){
   cleanup();
   if(this->adb)this->adb->throw_count++;
   throw;
  }

 }

 virtual ~otl_ref_select_stream()
 {
  cleanup();
 }

 void rewind(void)
 {
  _rfc=0;
  get_select_list();
  ret_code=first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
 }

  void clean(void)
  {
    _rfc=0;
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
  return !ret_code;
 }

 otl_ref_select_stream& operator>>(otl_time0& t)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof()){
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(char& c)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  switch(sl[cur_col].ftype){
  case otl_var_char:
    if(!eof()){
      s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
      look_ahead();
    }
    break;
#if defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if defined(OTL_STL)
      s.assign((char*)c,len);
#elif defined(OTL_ACE)
      s.set((char*)c,len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

 otl_ref_select_stream& operator>>(char* s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char* s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(int& n)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     n);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
     n=OTL_PCONV(int,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned& u)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_unsigned
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     u);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_unsigned_int))
     u=OTL_PCONV(unsigned,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(short& sh)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_short
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     sh);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_short))
     sh=OTL_PCONV(short,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(long int& l)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_long_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     l);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_long_int))
     l=OTL_PCONV(long int,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(float& f)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_float
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     f);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_float))
     f=OTL_PCONV(float,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(double& d)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(!eof()){
   int match_found=otl_numeric_convert_double
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     d);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_double))
     d=*OTL_RCAST(double*,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(otl_long_string& s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_varchar_long||
      sl[cur_col].ftype==otl_var_raw_long)&&
     !eof()){
   unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   int len=sl[cur_col].get_len(cur_row);
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,sl[cur_col].ftype);
   s.null_terminate_string(len);
   s.set_len(len);
   look_ahead();
  }
  return *this;
 }


 otl_ref_select_stream& operator<<(const otl_time0& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,otl_oracle_date_size)){
   otl_time0* tm=OTL_RCAST(otl_time0*,vl[cur_in]->val());
   memcpy(tm,OTL_RCAST(void*,OTL_CCAST(otl_time0*,&t)),otl_oracle_date_size);
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


  otl_ref_select_stream& operator<<(const otl_null& /* n */)
 {
  check_in_var();
  this->vl[cur_in]->set_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=OTL_RCAST(unsigned char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb&&this->adb->throw_count>1)return *this;
    if(this->adb)this->adb->throw_count++;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }
#endif

 otl_ref_select_stream& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb&&this->adb->throw_count>1)return *this;
    if(this->adb)this->adb->throw_count++;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const int n)
 {
  check_in_var();
  if(check_in_type(otl_var_int,sizeof(int))){
   *OTL_RCAST(int*,vl[cur_in]->val())=n;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned u)
 {
  check_in_var();
  if(check_in_type(otl_var_unsigned_int,sizeof(unsigned))){
   *OTL_RCAST(unsigned*,vl[cur_in]->val())=u;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const short sh)
 {
  check_in_var();
  if(check_in_type(otl_var_short,sizeof(short))){
   *OTL_RCAST(short*,vl[cur_in]->val())=sh;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const long int l)
 {
  check_in_var();
  if(check_in_type(otl_var_long_int,sizeof(long))){
   *OTL_RCAST(long*,vl[cur_in]->val())=l;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const float f)
 {
  check_in_var();
  if(check_in_type(otl_var_float,sizeof(float))){
   *OTL_RCAST(float*,vl[cur_in]->val())=f;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const double d)
 {
  check_in_var();
  if(check_in_type(otl_var_double,sizeof(double))){
   *OTL_RCAST(double*,vl[cur_in]->val())=d;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


 int select_list_len(void)
 {
  return sl_len;
 }

 int column_ftype(int ndx=0)
 {
  return sl[ndx].ftype;
 }

 int column_size(int ndx=0)
 {
  return sl[ndx].elem_size;
 }

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;

protected:


 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

 void init(void)
 {
  sl=0;
  sl_len=0;
  null_fetched=0;
  ret_code=0;
  sl_desc=0;
  executed=0;
  cur_in=0;
  stm_text=0;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

 int check_type(int type_code, int actual_data_type=0)
 {int out_type_code;
  if(actual_data_type!=0)
   out_type_code=actual_data_type;
  else
   out_type_code=type_code;
  if(sl[cur_col].ftype!=type_code){
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     out_type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception
    (otl_error_msg_0,
     otl_error_code_0,
     stm_label?stm_label:stm_text,
     var_info);
  }else
   return 1;
 }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
   ret_code=next();
   cur_col=-1;
   ++_rfc;
  }
 }

 void get_select_list(void)
 {int i,j,rc;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.ptr;
  int sld_tmp_len=0;
  int ftype,elem_size;

  rc=obndrv
   (&cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.cursor_struct.cda),
    sizeof(sel_cur.cursor_struct.cda),
    102,-1,0,0,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  for(i=0;i<vl_len;++i)otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(*vl[i]);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::exec(1); // Executing the PLSQL master block
  sel_cur.connected=1;
  cur_row=-2;
  sld_tmp_len=0;
  for(i=1;sel_cur.describe_column(sl_desc_tmp[i-1],i);++i)
   ++sld_tmp_len;
  sl_len=sld_tmp_len;
  if(sl){
   delete[] sl;
   sl=0;
  }
  sl=new otl_generic_variable[sl_len==0?1:sl_len];
  int max_long_size=this->adb->get_max_long_size();
  for(j=0;j<sl_len;++j){
   otl_generic_variable::map_ftype
    (sl_desc_tmp[j],max_long_size,ftype,elem_size,*override,j+1);
   sl[j].copy_pos(j+1);
   sl[j].init(ftype,
              elem_size,
              (short)array_size,
              &adb->connect_struct
             );
  }
  if(sl_desc){
   delete[] sl_desc;
   sl_desc=0;
  }
  sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
  memcpy(sl_desc,sl_desc_tmp,sizeof(otl_column_desc)*sl_len);
  for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
 }

 void get_in_next(void)
 {
  if(cur_in==vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

 int check_in_type(int type_code,int tsize)
 {
  if(vl[cur_in]->ftype==otl_var_char&&type_code==otl_var_char)
   return 1;
  if(vl[cur_in]->ftype!=type_code||vl[cur_in]->elem_size!=tsize){
   otl_var_info_var
    (vl[cur_in]->name,
     vl[cur_in]->ftype,
     type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception
    (otl_error_msg_0,
     otl_error_code_0,
     stm_label?stm_label:stm_text,
     var_info);
  }else
   return 1;
 }

 void check_in_var(void)
 {
  if(vl_len==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception
    (otl_error_msg_1,
     otl_error_code_1,
     stm_label?stm_label:stm_text,
     0);
  }
 }

 void check_if_executed(void)
 {
  if(!executed){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception
    (otl_error_msg_2,
     otl_error_code_2,
     stm_label?stm_label:stm_text,
     0);
  }
 }

};

class otl_stream_shell: public otl_stream_shell_generic{
public:

 otl_ref_select_stream* ref_ss;
 otl_select_stream* ss;
 otl_inout_stream* io;
 otl_connect* adb;


 int auto_commit_flag;

 otl_var_desc* iov;
 int iov_len;
 int next_iov_ndx;

 otl_var_desc* ov;
 int ov_len;
 int next_ov_ndx;

 bool flush_flag;

 otl_select_struct_override override;

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 OTL_STRING_CONTAINER orig_sql_stm;
#endif


 otl_stream_shell()
 {
  should_delete=0;
 }

 otl_stream_shell(const int ashould_delete)
 {
  should_delete=0;
  iov=0; iov_len=0;
  ov=0; ov_len=0;
  next_iov_ndx=0;
  next_ov_ndx=0;
  override.len=0;
  ss=0; io=0; ref_ss=0;
  adb=0;
  flush_flag=true;
  should_delete=ashould_delete;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=0; iov_len=0;
   ov=0; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override.len=0;
   flush_flag=true;

   delete ss;
   delete io;
   delete ref_ss;
   ss=0; io=0; ref_ss=0;
   adb=0;
  }
 }

};

class otl_stream{
public:
  
  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_ref_select_stream** ref_ss;
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;
  
  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;

  otl_select_struct_override* override;

  int end_marker;
  int oper_int_called;
  int last_eof_rc;

protected:

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

public:

 long get_rpc()
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->get_rpc();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->_rfc;
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   return (*ref_ss)->_rfc;
  }else
   return 0;
 }

  operator int(void) OTL_NO_THROW
  {
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  if((*ss)){
   if((*ss)->vl_len>0){
    (*iov)=new otl_var_desc[(*ss)->vl_len];
    (*iov_len)=(*ss)->vl_len;
    for(i=0;i<(*ss)->vl_len;++i)
     (*ss)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*ss)->sl_len>0){
    (*ov)=new otl_var_desc[(*ss)->sl_len];
    (*ov_len)=(*ss)->sl_len;
    for(i=0;i<(*ss)->sl_len;++i){
     (*ss)->sl[i].copy_var_desc((*ov)[i]);
     (*ov)[i].copy_name((*ss)->sl_desc[i].name);
    }
   }
  }else if((*io)){
   if((*io)->vl_len>0){
    (*iov)=new otl_var_desc[(*io)->vl_len];
    (*iov_len)=(*io)->vl_len;
    for(i=0;i<(*io)->vl_len;++i)
     (*io)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*io)->iv_len>0){
    (*ov)=new otl_var_desc[(*io)->iv_len];
    (*ov_len)=(*io)->iv_len;
    for(i=0;i<(*io)->iv_len;++i)
     (*io)->in_vl [i]->copy_var_desc((*ov)[i]);
   }
  }else if((*ref_ss)){
   if((*ref_ss)->vl_len>0){
    (*iov)=new otl_var_desc[(*ref_ss)->vl_len];
    (*iov_len)=(*ref_ss)->vl_len;
    for(i=0;i<(*ref_ss)->vl_len;++i)
     (*ref_ss)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*ref_ss)->sl_len>0){
    (*ov)=new otl_var_desc[(*ref_ss)->sl_len];
    (*ov_len)=(*ref_ss)->sl_len;
    for(i=0;i<(*ref_ss)->sl_len;++i){
     (*ref_ss)->sl[i].copy_var_desc((*ov)[i]);
     (*ov)[i].copy_name((*ref_ss)->sl_desc[i].name);
    }
   }
  }
 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0)
   OTL_NO_THROW
 {
   if(shell==0){
     init_stream();
     shell->flush_flag=true;
   }
   override->add_override(column_ndx,col_type,col_size);
 }

 void set_all_column_types(const unsigned mask=0)
   OTL_NO_THROW
 {
   if(shell==0){
     init_stream();
     shell->flush_flag=true;
   }
   override->set_all_column_types(mask);
 }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==0)init_stream();
  if(shell==0)return;
  shell->flush_flag=flush_flag;
 }


 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  return &(shell->iov[shell->next_iov_ndx]);
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  return &(shell->ov[shell->next_ov_ndx]);
 }

 void init_stream(void)
 {
  shell=0;
  shell=new otl_stream_shell(0);
  shell_pt.assign(&shell);
  connected=0;

  ref_ss=&(shell->ref_ss);
  ss=&(shell->ss);
  io=&(shell->io);
  adb=&(shell->adb);
  auto_commit_flag=&(shell->auto_commit_flag);
  iov=&(shell->iov);
  iov_len=&(shell->iov_len);
  next_iov_ndx=&(shell->next_iov_ndx);
  ov=&(shell->ov);
  ov_len=&(shell->ov_len);
  next_ov_ndx=&(shell->next_ov_ndx);
  override=&(shell->override);
  
  (*ref_ss)=0;
  (*io)=0;
  (*ss)=0;
  (*adb)=0;
  (*ov)=0; 
  (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*auto_commit_flag)=1;
  (*iov)=0; 
  (*iov_len)=0;
 }

 otl_stream
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=0,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  init_stream();

  (*io)=0; (*ss)=0; (*ref_ss)=0;
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,ref_cur_placeholder,sqlstm_label);
 }

 otl_stream() OTL_NO_THROW
 {
  init_stream();
  shell->flush_flag=true;
 }

 virtual ~otl_stream()
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  try{
   if((*io)!=0&&shell->flush_flag==false)
    (*io)->flush_flag2=false;
   close();
   if(shell!=0){
    if((*io)!=0)
     (*io)->flush_flag2=true;
   }
  }catch(otl_exception&){
   if(shell!=0){
    if((*io)!=0)
     (*io)->flush_flag2=true;
   }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=0)
    shell->should_delete=1;
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif
   throw;
  }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(adb && (*adb) && (*adb)->throw_count>0
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  || STD_NAMESPACE_PREFIX uncaught_exception()
#endif
     ){
   //
  }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void) OTL_NO_THROW
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->eof();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->eof();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   return (*ref_ss)->eof();
  }else
   return 1;
 }

 void flush(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush();
  }
 }

 void clean(const int clean_up_error_flag=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     (*adb)->reset_throw_count();
     (*io)->clean(clean_up_error_flag);
   }else if((*ss)){
     (*adb)->reset_throw_count();
     (*ss)->clean();
   }else if(*ref_ss){
     (*adb)->reset_throw_count();
     (*ref_ss)->clean();
   }
 }

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->rewind();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   (*ss)->rewind();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   (*ref_ss)->rewind();
  }
 }

 int is_null(void) OTL_NO_THROW
 {
  if((*io))
   return (*io)->is_null();
  else if((*ss))
   return (*ss)->is_null();
  else if((*ref_ss))
   return (*ref_ss)->is_null();
  else
   return 0;
 }

 void set_commit(int auto_commit=0)
   OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override->len=0;

  if((*ref_ss)){
   try{
    (*ref_ss)->close();
   }catch(otl_exception&){
    delete (*ref_ss);
    (*ref_ss)=0;
    throw;
   }
   delete (*ref_ss);
   (*ref_ss)=0;
  }

  if((*ss)){
   try{
    (*ss)->close();
   }catch(otl_exception&){
    delete (*ss);
    (*ss)=0;
    throw;
   }
   delete (*ss);
   (*ss)=0;
  }
  
  if((*io)){
   try{
    (*io)->flush();
   }catch(otl_exception&){
    clean(1);
    delete (*io);
    (*io)=0;
    throw;
   }
   delete (*io);
   (*io)=0;
  }

  (*ss)=0; (*io)=0; (*ref_ss)=0;
  if(adb!=0)(*adb)=0; 
  adb=0;
 }
 
 void open
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=0,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if(shell==0)
   init_stream();
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  char temp_buf[128];
  otl_itoa(arr_size,temp_buf);
  OTL_STRING_CONTAINER sql_stm=OTL_STRING_CONTAINER(temp_buf)+
    OTL_STRING_CONTAINER("===>")+sqlstm;
  otl_stream_shell* temp_shell=OTL_RCAST(otl_stream_shell*,
                                         db.sc.find(sql_stm));
  if(temp_shell){
   if(shell!=0)
    shell_pt.destroy();
   shell=temp_shell;
   ref_ss=&(shell->ref_ss);
   ss=&(shell->ss);
   io=&(shell->io);
   if((*io)!=0)(*io)->flush_flag2=true;
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
   override=&(shell->override);
   if((*iov_len)==0)
    this->rewind();
   connected=1;
   return;
  }
  shell->orig_sql_stm=sql_stm;
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  while(isspace(*c)||(*c)=='(')++c;
  strncpy(tmp,c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==0)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
   if((strncmp(tmp,"SELECT",6)==0||
       strncmp(tmp,"WITH",4)==0)&&
      ref_cur_placeholder==0){
     (*ss)=new otl_select_stream
       (override,
        arr_size,
        sqlstm,
        db,
        otl_explicit_select,
        sqlstm_label);
   }else if(ref_cur_placeholder!=0){
     (*ref_ss)=new otl_ref_select_stream
       (override,arr_size,sqlstm,
        ref_cur_placeholder,db,
        sqlstm_label);
   }else {
     (*io)=new otl_inout_stream(arr_size,sqlstm,db,false,sqlstm_label);
     (*io)->flush_flag=shell->flush_flag;
   }
  }catch(otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
   OTL_THROWS_OTL_EXCEPTION
#else
 void close(void)
   OTL_THROWS_OTL_EXCEPTION
#endif
 {
  if(shell==0)return;
#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool&&(*adb)&&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(STD_NAMESPACE_PREFIX uncaught_exception())&&
#endif
     (*adb)->throw_count==0){
   try{
    this->flush();
    this->clean(1);
   }catch(otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->throw_count>0){
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#endif
   (*adb)->sc.add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len) OTL_NO_THROW
 {
  desc_len=0;
  if((*ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ss)->sl_len;
   return (*ss)->sl_desc;
  }
  if((*ref_ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ref_ss)->sl_len;
   return (*ref_ss)->sl_desc;
  }
  return 0;
 }

 int good(void) OTL_NO_THROW
 {
  if(!connected)return 0;
  if((*io)||(*ss)||(*ref_ss)){
   (*adb)->reset_throw_count();
   return 1;
  }else
   return 0;
 }


 otl_stream& operator>>(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
    last_eof_rc=(*io)->eof();
   (*io)->operator>>(tab);
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(tab);
   inc_next_iov();
  }
  return *this;
 }

#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_stream& operator>>(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(vec);
     inc_next_ov();
   }
   return *this;
 }

 otl_stream& operator<<(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(vec);
   inc_next_iov();
  }
  return *this;
 }

#endif

 otl_stream& operator>>(otl_time0& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if(*ref_ss){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
  return *this;
 }

 otl_stream& operator<<(const otl_time0& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss))
   (*ss)->operator<<(n);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(n);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  return *this;
 }

 otl_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time0 tmp;
  (*this)>>tmp;

#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
   s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
   s.month=tmp.month;
   s.day=tmp.day;
   s.hour=tmp.hour-1;
   s.minute=tmp.minute-1;
   s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  inc_next_ov();
  return *this;
 }

 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   otl_time0 tmp;
   reset_end_marker();
   tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
   tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
   tmp.month=OTL_SCAST(unsigned char, s.month);
   tmp.day=OTL_SCAST(unsigned char, s.day);
   tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
   tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
   tmp.second=OTL_SCAST(unsigned char, (s.second+1));
   (*this)<<tmp;
   inc_next_iov();
   return *this;
 }

 otl_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
  if((*this).is_null())
   c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
  if((*this).is_null())
   c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
  inc_next_ov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
   strcpy(s,OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(OTL_RCAST(char*,s),
            OTL_RCAST(const char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
           );
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(n);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(n);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(u);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(u);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(u);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(sh);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(sh);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(sh);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(l);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(l);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(l);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
   l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(f);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(f);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(f);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(d);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(d);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(d);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
  if((*this).is_null())
    d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss))
   (*ss)->operator<<(c);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(c);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss))
   (*ss)->operator<<(c);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(c);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }
#endif


 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss))
   (*ss)->operator<<(n);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(n);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(u);
  else if((*ss))
   (*ss)->operator<<(u);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(u);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(sh);
  else if((*ss))
   (*ss)->operator<<(sh);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(sh);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(l);
  else if((*ss))
   (*ss)->operator<<(l);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(l);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(f);
  else if((*ss))
   (*ss)->operator<<(f);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(f);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(d);
  else if((*ss))
   (*ss)->operator<<(d);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(d);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

  otl_stream& operator<<(const otl_null& /* n */)
    OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))(*io)->operator<<(otl_null());
  if((*ss))(*ss)->operator<<(otl_null());
  if((*ref_ss)){
   (*ref_ss)->operator<<(otl_null());
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(d);
   inc_next_iov();
  }
  return *this;
 }

};

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

template <OTL_TYPE_NAME TData>
STD_NAMESPACE_PREFIX ostream& operator<<
  (STD_NAMESPACE_PREFIX ostream& s, 
   const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<"NULL";
 else
  s<<var.v;
 return s;
}

inline STD_NAMESPACE_PREFIX ostream& operator<<(
 STD_NAMESPACE_PREFIX ostream& s, 
 const otl_value<otl_datetime>& var
) OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<"NULL";
 else{
  s<<var.v.month<<"/"<<var.v.day<<"/"<<var.v.year<<" "
   <<var.v.hour<<":"<<var.v.minute<<":"<<var.v.second;
 }
 return s;
}

#endif

typedef otl_tmpl_nocommit_stream
<otl_stream,
 otl_connect,
 otl_exception> otl_nocommit_stream;

OTL_ORA7_NAMESPACE_END
#endif

// ==================== OTL-Adapter for Oracle 8 =====================
#ifdef OTL_ORA8
#ifdef __STDC__
#define __STDC__DEFINED
#else
#define __STDC__ 1 // making OCI function prototypes show up in oci.h
#endif
#ifdef OTL_ORA_TEXT_ON
#define text OTL_ORA_TEXT
#endif
#include <oci.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef OTL_ORA8_PROC
extern "C" {
#include <sql2oci.h>
}
#endif

OTL_ORA8_NAMESPACE_BEGIN

const int inVarChar2=1;
const int inNumber=2;
const int inLong=8;
const int inRowId=11;
const int inDate=12;
const int inRaw=23;
const int inLongRaw=24;
const int inChar=96;
const int inMslabel=105;
const int inUserDefinedType=108;
const int inRef=111;
const int inCLOB=112;
const int inBLOB=113;

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
const int inTimestamp=SQLT_TIMESTAMP;
const int inTimestamp_TZ=SQLT_TIMESTAMP_TZ;
const int inTimestamp_LTZ=SQLT_TIMESTAMP_LTZ;
#endif

const int  extVarChar2=1;
const int  extNumber=2;
const int  extInt=3;
const int  extFloat=4;
#if defined(OTL_ORA_MAP_STRINGS_TO_CHARZ)
const int  extCChar=97;
#else
const int  extCChar=5;
#endif
const int  extVarNum=6;
const int  extLong=8;
const int  extVarChar=9;
const int  extRowId=11;
const int  extDate=12;
const int  extVarRaw=15;
const int  extRaw=23;
const int  extLongRaw=24;
const int  extUInt=68;
const int  extLongVarChar=94;
const int  extLongVarRaw=95;
const int  extChar=96;
const int  extCharZ=97;
const int  extMslabel=105;
const int  extCLOB=inCLOB;
const int  extBLOB=inBLOB;

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
const int extTimestamp=SQLT_TIMESTAMP;
const int extTimestamp_TZ=SQLT_TIMESTAMP_TZ;
const int extTimestamp_LTZ=SQLT_TIMESTAMP_LTZ;
#endif


#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
typedef otl_datetime otl_time0;
#else
typedef otl_oracle_date otl_time0;
#endif


class otl_exc{
public:
 unsigned char msg[1000];
 int code;
 char sqlstate[32];

#ifdef OTL_EXTENDED_EXCEPTION
 char** msg_arr;
 char** sqlstate_arr;
 int* code_arr;
 int arr_len;
#endif

 enum{disabled=0,enabled=1};

 otl_exc()
 {
  sqlstate[0]=0;
  msg[0]=0;
  code=0;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

 void init(const char* amsg, const int acode)
 {
  strcpy(OTL_RCAST(char*,msg),amsg);
  code=acode;
#ifdef OTL_EXTENDED_EXCEPTION
  msg_arr=0;
  sqlstate_arr=0;
  code_arr=0;
  arr_len=0;
#endif
 }

};

class otl_conn{
public:

 OCIEnv *envhp; // OCI environment handle
 OCIServer *srvhp; // OCI Server handle
 OCIError *errhp; // OCI Error handle
 OCISvcCtx *svchp; // OCI Service context handle
 OCISession *authp; // OCI Session handle
 int auto_commit;
 int extern_lda;
 int attached;
 int in_session;
 int char_set_;
 int session_begin_count;
 int session_mode_;
 int ext_cred;

 static int initialize(const int threaded_mode=0)
 {int status;
  int mode;
  if(threaded_mode)
    mode=OCI_THREADED;
  else
    mode=OCI_DEFAULT;
  status=OCIInitialize
   (OTL_SCAST(ub4,mode),
    OTL_RCAST(dvoid *,0),
    0,
    0,
    0
   );
  if(status!=OCI_SUCCESS)
   return 0;
  else
   return 1;
 }

 otl_conn()
 {
  envhp=0;
  srvhp=0;
  errhp=0;
  svchp=0;
  authp=0;
  auto_commit=0;
  extern_lda=0;
  attached=0;
  in_session=0;
  session_begin_count=0;
  session_mode_=OCI_DEFAULT;
  char_set_=SQLCS_IMPLICIT;
  ext_cred=0;
 }

 void set_char_set(const int char_set)
 {
  char_set_=char_set;
 }

 virtual ~otl_conn(){}

  void set_timeout(const int /*atimeout*/=0){}
  void set_cursor_type(const int /*acursor_type*/=0){}

 int cancel(void)
 {int status;
  status=OCIBreak(srvhp,errhp);
  if(status)
   return 0;
  else
   return 1;
 }

 int server_attach(const char* tnsname)
 {int status;

  envhp=0;
  srvhp=0;
  errhp=0;
  svchp=0;
  authp=0;
  extern_lda=0;
  attached=0;
  in_session=0;
  session_begin_count=0;

  status=OCIEnvInit
   (OTL_RCAST(OCIEnv**,&envhp),
    OCI_DEFAULT,
    0,
    0);
  if(status)return 0;

  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
    OTL_RCAST(dvoid**,&errhp),
    OCI_HTYPE_ERROR,
    0,
    0);
  if(status)return 0;

  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
    OTL_RCAST(dvoid**,&srvhp),
    OCI_HTYPE_SERVER,
    0,
    0);
  if(status)return 0;

  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
    OTL_RCAST(dvoid**,&svchp),
    OCI_HTYPE_SVCCTX,
    0,
    0);
  if(status)return 0;

  status=OCIServerAttach
   (srvhp,
    errhp,
    tnsname==0?OTL_RCAST(text*,OTL_CCAST(char*,"")):
               OTL_RCAST(text*,OTL_CCAST(char*,tnsname)),
    tnsname==0?0:strlen(OTL_CCAST(char*,tnsname)),
    0);
  if(status)return 0;
  status=OCIAttrSet
   (OTL_RCAST(dvoid*,svchp),
    OCI_HTYPE_SVCCTX,
    OTL_RCAST(dvoid*,srvhp),
    0,
    OCI_ATTR_SERVER,
    OTL_RCAST(OCIError*,errhp));
  if(status)return 0;
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
    OTL_RCAST(dvoid **,&authp),
    OTL_SCAST(ub4,OCI_HTYPE_SESSION),
    0,
    0);
  if(status)return 0;

  attached=1;
  return 1;

 }

 int session_begin(const int aauto_commit)
 {int status;
  int cred_type;

  if(!attached)return 0;
  if(session_begin_count==0)return 0;

  if(ext_cred)
   cred_type=OCI_CRED_EXT;
  else
   cred_type=OCI_CRED_RDBMS;
  status=OCISessionBegin
   (svchp,
    errhp,
    authp,
    cred_type,
    OTL_SCAST(ub4,session_mode_));
  if(status)return 0;

  in_session=1;
  auto_commit=aauto_commit;
  ++session_begin_count;
  return 1;

 }

 int session_begin
   (const char* userid,
    const char* password,
    const int aauto_commit,
    const int session_mode=OCI_DEFAULT)
 {int status;
  int cred_type;

  if(!attached)return 0;

  status=OCIAttrSet
   (OTL_RCAST(dvoid*,authp),
    OTL_SCAST(ub4,OCI_HTYPE_SESSION),
    OTL_RCAST(dvoid*,OTL_CCAST(char*,userid)),
    OTL_SCAST(ub4,strlen(OTL_CCAST(char*,userid))),
    OTL_SCAST(ub4,OCI_ATTR_USERNAME),
    errhp);
  if(status)return 0;

  status=OCIAttrSet
   (OTL_RCAST(dvoid*,authp),
    OTL_SCAST(ub4,OCI_HTYPE_SESSION),
    OTL_RCAST(dvoid*,OTL_CCAST(char*,password)),
    OTL_SCAST(ub4,strlen(OTL_CCAST(char*,password))),
    OTL_SCAST(ub4,OCI_ATTR_PASSWORD),
    errhp);
  if(status)return 0;

   cred_type=OCI_CRED_RDBMS;

  if(userid[0]==0&&password[0]==0){
   ext_cred=1;
   cred_type=OCI_CRED_EXT;
  }else{
   ext_cred=0;
   cred_type=OCI_CRED_RDBMS;
  }

  session_mode_=session_mode;
  status=OCISessionBegin
   (svchp,
    errhp,
    authp,
    cred_type,
    OTL_SCAST(ub4,session_mode_));
  if(status)return 0;

  status=OCIAttrSet
   (OTL_RCAST(dvoid*,svchp),
    OTL_SCAST(ub4,OCI_HTYPE_SVCCTX),
    OTL_RCAST(dvoid *,authp),
    0,
    OTL_SCAST(ub4,OCI_ATTR_SESSION),
    errhp);
  if(status)return 0;

  in_session=1;
  auto_commit=aauto_commit;
  ++session_begin_count;
  return 1;

 }

#if defined(OTL_ORA8I) || defined(OTL_ORA9I)
  int change_password
  (const char* user_name,
   const char* password,
   const char* new_password)
  {
    int status;
    
    OCIAttrSet
      (OTL_RCAST(dvoid*,svchp),
       OTL_SCAST(ub4,OCI_HTYPE_SVCCTX),
       OTL_RCAST(dvoid *,authp),
       0,
       OTL_SCAST(ub4,OCI_ATTR_SESSION),
       errhp);
    
    status=OCIPasswordChange 
      (svchp,  
       errhp,
       OTL_RCAST(text*,OTL_CCAST(char*,user_name)),
       strlen(user_name),
       OTL_RCAST(text*,OTL_CCAST(char*,password)),
       strlen(password),
       OTL_RCAST(text*,OTL_CCAST(char*,new_password)),
       strlen(new_password),
       OCI_AUTH);
    if(status)
      return 0;
    else
      return 1;
  
  }
#endif

 int server_detach(void)
 {int rc=0;
  if(attached){
   OCIServerDetach(srvhp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
   rc=1;
  }
  if(authp!=0)OCIHandleFree(OTL_RCAST(dvoid*,authp),
                            OTL_SCAST(ub4,OCI_HTYPE_SESSION));
  if(errhp!=0)OCIHandleFree(OTL_RCAST(dvoid*,errhp),
                            OTL_SCAST(ub4,OCI_HTYPE_ERROR));
  if(svchp!=0)OCIHandleFree(OTL_RCAST(dvoid*,svchp),
                            OTL_SCAST(ub4,OCI_HTYPE_SVCCTX));
  if(srvhp!=0)OCIHandleFree(OTL_RCAST(dvoid*,srvhp),
                            OTL_SCAST(ub4,OCI_HTYPE_SERVER));
  if(envhp!=0)OCIHandleFree(OTL_RCAST(dvoid*,envhp),
                            OTL_SCAST(ub4,OCI_HTYPE_ENV));
  auto_commit=0;
  attached=0;
  in_session=0;
  envhp=0;
  srvhp=0;
  errhp=0;
  svchp=0;
  authp=0;
  return rc;
 }

 int session_end(void)
 {int status;
  if(!in_session)return 0;
  status=OCISessionEnd(svchp,errhp,authp,0);
  if(status)return 0;

  in_session=0;
  auto_commit=0;
  return 1;
 }

 int auto_commit_on(void)
 {
  auto_commit=1;
  return 1;
 }

 int auto_commit_off(void)
 {
  auto_commit=0;
  return 1;
 }

 int rlogon(const char* connect_str,const int aauto_commit)
 {int status;
  char username[256];
  char passwd[256];
  char tnsname[256];
  char* tnsname_ptr=0;
  char* c=OTL_CCAST(char*,connect_str);
  char* username_ptr=username;
  char* passwd_ptr=passwd;

  auto_commit=aauto_commit;

  username[0]=0;
  passwd[0]=0;
  tnsname[0]=0;

  while(*c&&*c!='/'){
   *username_ptr=*c;
   ++c;
   ++username_ptr;
  }
  *username_ptr=0;

  if(*c=='/')++c;
  while(*c&&*c!='@'){
   *passwd_ptr=*c;
   ++c;
   ++passwd_ptr;
  }
  *passwd_ptr=0;

  if(*c=='@'){
   ++c;
   tnsname_ptr=tnsname;
   while(*c){
    *tnsname_ptr=*c;
    ++c;
    ++tnsname_ptr;
   }
   *tnsname_ptr=0;
  }

  envhp=0;
  srvhp=0;
  errhp=0;
  svchp=0;
  authp=0;
  extern_lda=0;
  attached=0;
  in_session=0;

  status=server_attach(tnsname);
  if(!status)return 0;

  status=session_begin(username,passwd,aauto_commit);
  if(!status)return 0;

  return 1;

 }

 int ext_logon(OCIEnv *a_envhp,OCISvcCtx *a_svchp,const int aauto_commit=0)
 {int status;

  envhp=a_envhp;
  svchp=a_svchp;
  errhp=0;
  srvhp=0;
  authp=0;
  extern_lda=1;
  auto_commit=aauto_commit;

  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
    OTL_RCAST(dvoid**,&errhp),
    OCI_HTYPE_ERROR,
    0,
    0);
  if(status)return 0;

  return 1;

 }

 int logoff(void)
 {
  int rc;
  if(extern_lda){
   OCIHandleFree(OTL_RCAST(dvoid*,errhp), OTL_SCAST(ub4,OCI_HTYPE_ERROR));
   envhp=0;
   svchp=0;
   errhp=0;
   extern_lda=0;
  }else{
   rc=session_end();
   if(!rc)return 0;
   rc=server_detach();
   if(!rc)return 0;
  }
  auto_commit=0;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {sb4 errcode;
  int len;
  OCIErrorGet
   (OTL_RCAST(dvoid*,errhp),
    OTL_SCAST(ub4,1),
    0,
    &errcode,
    OTL_RCAST(text*,exception_struct.msg),
    OTL_SCAST(ub4,sizeof(exception_struct.msg)),
    OCI_HTYPE_ERROR);
  exception_struct.code=errcode;
  len=strlen(OTL_RCAST(char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

 int commit(void)
 {
  return !OCITransCommit(svchp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
 }

 int rollback(void)
 {
  return !OCITransRollback(svchp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
 }

};

class otl_cur0{
public:
};

class otl_var{
public:

 ub1* p_v;
 sb2* p_ind;
 ub2* p_rlen;
 ub2* p_rcode;
 int ftype;
 int array_size;
 int elem_size;
 OCILobLocator** lob;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  OCIDateTime** timestamp;
#endif
 OCIStmt* cda;
 otl_conn* connect;
 ub1* buf;
 int buf_len;
 int ext_buf_flag;
 int act_elem_size;
 ub4 max_tab_len;
 ub4 cur_tab_len;
 int pl_tab_flag;
 int lob_stream_flag;
 int vparam_type;
 int lob_len;
 int lob_pos;
 int lob_ftype;
 int otl_adapter;
 bool lob_stream_mode;
 sb4 unicode_var_len;
 ub2 csid; 
 ub1 csfrm;

 otl_var()
 {
  otl_adapter=otl_ora8_adapter;
  lob_stream_mode=false;
  lob_stream_flag=0;
  unicode_var_len=0;
  csid=0;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  timestamp=0;
#endif
  csfrm=SQLCS_IMPLICIT;
  cda=0;
  p_v=0;
  p_ind=0;
  p_rlen=0;
  p_rcode=0;
  ftype=0;
  lob=0;
  array_size=0;
  connect=0;
  elem_size=0;
  buf=0;
  buf_len=0;
  ext_buf_flag=0;
  act_elem_size=0;
  max_tab_len=0;
  cur_tab_len=0;
  pl_tab_flag=0;
  vparam_type=-1;
  lob_len=0;
  lob_pos=0;
  lob_ftype=0;
 }

 virtual ~otl_var()
 {int i;
  if(ftype==otl_var_refcur&&cda!=0){
    OCIHandleFree(OTL_RCAST(dvoid*,cda),OCI_HTYPE_STMT);
    cda=0;
  }
  if(ftype==otl_var_blob||ftype==otl_var_clob&&lob!=0){
   for(i=0;i<array_size;++i)
    OCIDescriptorFree(OTL_RCAST(dvoid*,lob[i]),
                      OTL_SCAST(ub4,OCI_DTYPE_LOB));
  }
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  if((ftype==otl_var_timestamp ||
      ftype==otl_var_tz_timestamp ||
      ftype==otl_var_ltz_timestamp)&&
      timestamp!=0){
    ub4 dtype=0;
    switch(ftype){
    case otl_var_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP;
      break;
    case otl_var_ltz_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP_LTZ;
      break;
    case otl_var_tz_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP_TZ;
      break;
    }
   for(i=0;i<array_size;++i)
    OCIDescriptorFree(OTL_RCAST(dvoid*,timestamp[i]),dtype);
  }
#endif
  delete[] p_v;
  delete[] p_ind;
  delete[] p_rlen;
  delete[] p_rcode;
  if(!ext_buf_flag)
   delete[] buf;
 }

  int write_dt(void* trg, const void* src, const int sz)
  {
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    OCIDateTime* trg_ptr=OTL_RCAST(OCIDateTime*,trg);
    otl_datetime* src_ptr=OTL_RCAST(otl_datetime*,OTL_CCAST(void*,src));
    int rc=0;
    if(ftype!=otl_var_tz_timestamp){
      rc=OCIDateTimeConstruct 
        (connect->envhp,
         connect->errhp,
         trg_ptr,
         OTL_SCAST(sb2,src_ptr->year),
         OTL_SCAST(ub1,src_ptr->month),
         OTL_SCAST(ub1,src_ptr->day),
         OTL_SCAST(ub1,src_ptr->hour),
         OTL_SCAST(ub1,src_ptr->minute),
         OTL_SCAST(ub1,src_ptr->second),
         OTL_SCAST
         (ub4,otl_to_fraction(src_ptr->fraction,
                              src_ptr->frac_precision)),
         0,
         0);
    }else{
      int tz_hour=src_ptr->tz_hour;
      int tz_minute=src_ptr->tz_minute;
      char tz_str[60];
      char* tzc=otl_itoa(tz_hour,tz_str);
      *tzc=':';
      ++tzc;
      otl_itoa(tz_minute,tzc);
      rc=OCIDateTimeConstruct 
        (connect->envhp,
         connect->errhp,
         trg_ptr,
         OTL_SCAST(sb2,src_ptr->year),
         OTL_SCAST(ub1,src_ptr->month),
         OTL_SCAST(ub1,src_ptr->day),
         OTL_SCAST(ub1,src_ptr->hour),
         OTL_SCAST(ub1,src_ptr->minute),
         OTL_SCAST(ub1,src_ptr->second),
         OTL_SCAST
         (ub4,otl_to_fraction(src_ptr->fraction,
                              src_ptr->frac_precision)),
         OTL_RCAST(text*,tz_str),
         OTL_SCAST(size_t,(tzc-tz_str)));
    }
    if(rc!=0)return 0;
    return 1;
#else
    memcpy(trg,src,sz);
    return 1;
#endif
  }

  int read_dt(void* trg, const void* src, const int sz)
  {
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    OCIDateTime* src_ptr=OTL_RCAST(OCIDateTime*,OTL_CCAST(void*,src));
    otl_datetime* trg_ptr=OTL_RCAST(otl_datetime*,OTL_CCAST(void*,trg));
    sb2 year;
    ub1 month, day, hour, minute, sec;
    ub4 fsec;
    sb1 tz_hour;
    sb1 tz_minute;
    int rc=OCIDateTimeGetDate 
      (connect->envhp,
       connect->errhp, 
       src_ptr,
       &year,
       &month,
       &day);
    if(rc!=0)return 0;
    rc=OCIDateTimeGetTime
      (connect->envhp,
       connect->errhp, 
       src_ptr,
       &hour,
       &minute,
       &sec,
       &fsec);
    if(rc!=0)return 0;
    trg_ptr->year=year;
    trg_ptr->month=month;
    trg_ptr->day=day;
    trg_ptr->hour=hour;
    trg_ptr->minute=minute;
    trg_ptr->second=sec;
    trg_ptr->fraction=otl_from_fraction(fsec,trg_ptr->frac_precision);
    trg_ptr->tz_hour=0;
    trg_ptr->tz_minute=0;
    if(ftype==otl_var_tz_timestamp ||
       ftype==otl_var_ltz_timestamp){
      rc=OCIDateTimeGetTimeZoneOffset
        (connect->envhp,
         connect->errhp, 
         src_ptr,
         &tz_hour,
         &tz_minute);
      if(rc!=0)return 0;
      trg_ptr->tz_hour=tz_hour;
      trg_ptr->tz_minute=tz_minute;
    }
    return 1;
#else
    memcpy(trg,src,sz);
    return 1;
#endif
  }


 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const int aftype,
  int& aelem_size,
  const short aarray_size,
  const void* connect_struct=0,
  const int apl_tab_flag=0)
 {
  int i;
  ub4 lobEmpty=0;

  connect=OTL_RCAST(otl_conn*,OTL_CCAST(void*,connect_struct));
  ftype=aftype;
  pl_tab_flag=apl_tab_flag;
  act_elem_size=aelem_size;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  if((aftype==otl_var_timestamp ||
      aftype==otl_var_tz_timestamp ||
      aftype==otl_var_ltz_timestamp) &&
      apl_tab_flag)
    act_elem_size=sizeof(otl_oracle_date);
#endif
  if(ftype==otl_var_refcur){
   array_size=aarray_size;
   elem_size=1;
   OCIHandleAlloc
    (OTL_RCAST(dvoid*,connect->envhp),
     OTL_RCAST(dvoid**,&cda),
     OCI_HTYPE_STMT,
     0,
     0);
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  }else if((ftype==otl_var_timestamp ||
            ftype==otl_var_tz_timestamp ||
            ftype==otl_var_ltz_timestamp) &&
           !apl_tab_flag){
   array_size=aarray_size;
   elem_size=sizeof(OCIDateTime*);
   act_elem_size=elem_size;
   timestamp=new OCIDateTime*[array_size];
   p_v=(ub1*)timestamp;
   p_ind=new sb2[array_size];
   p_rlen=new ub2[array_size];
   p_rcode=new ub2[array_size];
   for(i=0;i<array_size;++i){
     p_ind[i]=(short)elem_size;
     p_rlen[i]=(short)elem_size;
     p_rcode[i]=0;
   }
   if(connect!=0){
     otl_datetime dt;
     ub4 dtype=0;
     switch(ftype){
     case otl_var_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP;
       break;
     case otl_var_ltz_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP_LTZ;
       break;
     case otl_var_tz_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP_TZ;
       break;
     }
     for(i=0;i<array_size;++i){
       OCIDescriptorAlloc
         (OTL_RCAST(dvoid*,connect->envhp),
          OTL_RCAST(dvoid**,&timestamp[i]),
          dtype,
          0,
          0);
       write_dt(timestamp[i],&dt,1);
    }
   }else
    timestamp=0;
#endif
  }else if(ftype==otl_var_blob||ftype==otl_var_clob){
   array_size=aarray_size;
   elem_size=aelem_size;
   lob=new OCILobLocator*[array_size];
   p_v=(ub1*)lob;
   p_ind=new sb2[array_size];
   p_rlen=0;
   p_rcode=0;
   if(connect!=0){
    for(i=0;i<array_size;++i){
     OCIDescriptorAlloc
      (OTL_RCAST(dvoid*,connect->envhp),
       OTL_RCAST(dvoid**,&lob[i]),
       OTL_SCAST(ub4,OCI_DTYPE_LOB),
       0,
       0);
     lobEmpty=0;
     OCIAttrSet
      (OTL_RCAST(dvoid*,lob[i]),
       OCI_DTYPE_LOB,
       OTL_RCAST(dvoid*,&lobEmpty),
       0,
       OCI_ATTR_LOBEMPTY,
       OTL_RCAST(OCIError*,connect->errhp));
    }
   }else
    lob=0;
  }else{
   if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long){
    elem_size=aelem_size+sizeof(sb4);
    array_size=1;
   }else{
    elem_size=aelem_size;
    array_size=aarray_size;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    if((aftype==otl_var_timestamp ||
        aftype==otl_var_tz_timestamp ||
        aftype==otl_var_ltz_timestamp) && 
       apl_tab_flag){
      elem_size=sizeof(otl_oracle_date);
      aelem_size=elem_size; // sending feedback back to the template class
    }
#endif
   }
#ifdef OTL_UNICODE
   if(ftype==otl_var_char){
     unsigned unicode_buffer_size=
       elem_size*OTL_SCAST(unsigned,array_size)*sizeof(OTL_WCHAR);
     p_v=new ub1[unicode_buffer_size];
     memset(p_v,0,unicode_buffer_size);
   } else if(ftype==otl_var_varchar_long){
     unsigned unicode_buffer_size=elem_size;
     p_v=new ub1[unicode_buffer_size];
     memset(p_v,0,unicode_buffer_size);
   }else{
     p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
     memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
   }
#else
   p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
   memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
#endif
   p_ind=new sb2[array_size];
   p_rlen=new ub2[array_size];
   p_rcode=new ub2[array_size];
   if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long){
    if(aelem_size>32767)
     p_ind[0]=0;
    else
    p_ind[0]=OTL_SCAST(short,aelem_size);
    p_rcode[0]=0;
   }else{
    for(i=0;i<array_size;++i){
#ifdef OTL_UNICODE
      if(ftype==otl_var_char){
        p_ind[i]=(short)elem_size*sizeof(OTL_WCHAR);
        p_rlen[i]=(short)elem_size*sizeof(OTL_WCHAR);
        p_rcode[i]=0;
      }else{
        p_ind[i]=(short)elem_size;
        p_rlen[i]=(short)elem_size;
        p_rcode[i]=0;
      }
#else
      p_ind[i]=(short)elem_size;
      p_rlen[i]=(short)elem_size;
      p_rcode[i]=0;
#endif
    }
   }
  }
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=0;
 }

 void set_pl_tab_len(const int apl_tab_len)
 {
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=OTL_SCAST(ub4,apl_tab_len);
 }

 int get_pl_tab_len(void)
 {
  return (int)cur_tab_len;
 }

 int get_max_pl_tab_len(void)
 {
  return OTL_SCAST(int,max_tab_len);
 }

 int get_blob_len(const int ndx,int& alen)
 {ub4 blen;
  int rc;
  alen=0;
  rc=OCILobGetLength
   (connect->svchp,
    connect->errhp,
    lob[ndx],
    &blen);
  alen=OTL_SCAST(int,blen);
  if(rc!=0)return 0;
  return 1;
 }

 int get_blob
 (const int ndx,
  unsigned char* abuf,
  const int buf_size,
  int& len)
 {
  int byte_buf_size=buf_size;
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
   byte_buf_size=buf_size*sizeof(OTL_CHAR);
#endif  
  ub4 amt=byte_buf_size;
  ub4 offset=1;
  int rc;
  memset(OTL_RCAST(void*,abuf),0,OTL_SCAST(size_t,buf_size));
  int is_init=0;
  rc=OCILobLocatorIsInit
     (connect->envhp,
      connect->errhp,
      lob[ndx],
      &is_init);
  if (rc!=0) return 0;
  if (!is_init){
   len=0;
   return 1;
  }
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
   rc=OCILobRead
   (connect->svchp,
    connect->errhp,
    lob[ndx],
    &amt,
    offset,
    OTL_RCAST(dvoid*,abuf),
    OTL_SCAST(ub4,byte_buf_size),
    0,
    0,
    csid,
    OTL_SCAST(ub1,connect->char_set_));
  len=amt;
  if(rc!=0)return 0;
  return 1;
 }

 void set_lob_stream_flag(const int flg=1)
 {
  lob_stream_flag=flg;
 }

  int close_lob(void)
  {
#if defined(OTL_ORA8I)||defined(OTL_ORA9I)
    int rc;
    boolean flag=0;
    rc=OCILobIsOpen
      (connect->svchp,
       connect->errhp,
       lob[0],
       &flag);
    if(rc!=OCI_SUCCESS)return 0;
    if(flag!=TRUE)return 1;
    rc=OCILobClose
      (connect->svchp,
       connect->errhp,
       lob[0]);
    if(rc!=OCI_SUCCESS)return 0;
#endif
    return 1;
  }

 int put_blob(void)
 {
  if(ftype!=otl_var_clob&&ftype!=otl_var_blob||
     lob_stream_flag||buf==0||buf_len==0)return 1;

  int byte_buf_len=buf_len;
#ifdef OTL_UNICODE
  if(otl_var_clob)
   byte_buf_len=buf_len*sizeof(OTL_CHAR);
#endif
  ub4 amt=OTL_SCAST(ub4,buf_len);
  ub4 offset=1;
  int rc;
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  rc=OCILobWrite
   (connect->svchp,
    connect->errhp,
    lob[0],
    &amt,
    offset,
    OTL_RCAST(dvoid*,buf),
    OTL_SCAST(ub4,byte_buf_len),
    OCI_ONE_PIECE,
    0,
    0,
    csid,
    OTL_SCAST(ub1,connect->char_set_));
  if(rc!=0)return 0;
  return 1;
 }

 int read_blob
 (otl_long_string& s,
  const int andx,
  int& aoffset,
  int alob_len)
 {
  ub4 byte_buf_size=s.buf_size;
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
   byte_buf_size=byte_buf_size*sizeof(OTL_CHAR);
#else
#endif

  ub4 amt=0;
  ub4 offset=aoffset;
  int rc;
  int is_init=0;
  rc=OCILobLocatorIsInit
     (connect->envhp,
      connect->errhp,
      lob[andx],
      &is_init);
  if(rc!=0)return 0;
  if(!is_init){
   s.set_len(0);
   return 1;
  }
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  rc=OCILobRead
   (connect->svchp,
    connect->errhp,
    lob[andx],
    &amt,
    offset,
    OTL_RCAST(dvoid*,s.v),
    OTL_SCAST(ub4,byte_buf_size),
    0,
    0,
    csid,
    OTL_SCAST(ub1,connect->char_set_));
  switch(rc){
  case OCI_SUCCESS:
   if(aoffset==1)
    s.set_len(alob_len);
   else
    s.set_len(alob_len-aoffset+1);
   break;
  case OCI_NEED_DATA:
   s.set_len(s.buf_size);
   break;
  case OCI_ERROR:
   s.set_len(0);
   break;
  }
  if(rc==OCI_NEED_DATA||rc==OCI_SUCCESS){
   aoffset+=s.length;
   return 1;
  }else
   return 0;
 }

 int write_blob
 (const otl_long_string& s,
  const int alob_len,
  int& aoffset,
  otl_cur0& /* cur */)
 {
  if(!lob_stream_flag)return 1;
  if(alob_len==0)return 1;

  int byte_s_length=s.length;
#ifdef OTL_UNICODE
  int byte_lob_len;
  if(otl_var_clob){
   byte_lob_len=alob_len*sizeof(OTL_CHAR);
   byte_s_length=s.length*sizeof(OTL_CHAR);
  }
#endif
  ub4 offset=aoffset;
  ub4 amt=alob_len;
  int rc;
  ub1 mode;
  if(aoffset==1&&alob_len>s.length)
   mode=OCI_FIRST_PIECE;
  else if(aoffset==1&&alob_len<=s.length){
   mode=OCI_ONE_PIECE;
   amt=s.length;
  }else if((aoffset-1)+s.length<alob_len)
   mode=OCI_NEXT_PIECE;
  else
   mode=OCI_LAST_PIECE;
#ifdef OTL_UNICODE
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  if(mode==OCI_FIRST_PIECE){
    rc=OCILobTrim 
      (connect->svchp, 
       connect->errhp, 
       lob[0], 
       0);
    if(rc!=OCI_SUCCESS)
      return 0;
  }
  rc=OCILobWrite
   (connect->svchp,
    connect->errhp,
    lob[0],
    OTL_RCAST(ub4*,&amt),
    offset,
    OTL_RCAST(dvoid*,s.v),
    OTL_SCAST(ub4,byte_s_length),
    mode,
    0,
    0,
    csid,
    OTL_SCAST(ub1,connect->char_set_));
  if(rc==OCI_NEED_DATA||
     rc==OCI_SUCCESS||
     rc==OCI_SUCCESS_WITH_INFO){
   aoffset+=s.length;
   return 1;
  }
  return 0;
 }

 int save_blob
 (const unsigned char* abuf,
  const int len,
  const int extern_buffer_flag)
 {
  if(extern_buffer_flag){
   ext_buf_flag=1;
   buf_len=len;
   buf=OTL_CCAST(unsigned char*,abuf);
  }else{
   if(buf!=0&&!ext_buf_flag){
    delete[] buf;
    buf=0;
   }
   ext_buf_flag=0;
   buf_len=len;
#ifdef OTL_UNICODE
   buf=new ub1[buf_len*sizeof(OTL_CHAR)];
   memcpy(buf,abuf,buf_len*sizeof(OTL_CHAR));
#else
   buf=new ub1[buf_len];
   memcpy(buf,abuf,buf_len);
#endif
  }
  return 1;
 }

 void set_null(int ndx)
 {
  p_ind[ndx]=-1;
 }

 void set_not_null(int ndx, int pelem_size)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   p_ind[0]=0;
  else if(ftype==otl_var_clob||ftype==otl_var_blob){
   if(lob_stream_flag==0){
    ub4 lobEmpty=0;

    OCIAttrSet
     (OTL_RCAST(dvoid*,lob[ndx]),
      OCI_DTYPE_LOB,
      OTL_RCAST(dvoid*,&lobEmpty),
      0,
      OCI_ATTR_LOBEMPTY,
      OTL_RCAST(OCIError*,connect->errhp));
   }
  }else
   p_ind[ndx]=OTL_SCAST(short,pelem_size);
 }

 void set_len(int len, int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long){
#ifdef OTL_UNICODE
   if(ftype==otl_var_varchar_long)
    *OTL_RCAST(sb4*,p_v)=len*sizeof(OTL_CHAR);
   else
    *OTL_RCAST(sb4*,p_v)=len;
#else
   *OTL_RCAST(sb4*,p_v)=len;
#endif
  }else
   p_rlen[ndx]=OTL_SCAST(short,len);
 }

 int get_len(int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long){
   if(p_ind[0]==-1)
    return 0;
   else{
#ifdef OTL_UNICODE
    if(ftype==otl_var_varchar_long)
     return (*(sb4*)p_v)/sizeof(OTL_CHAR);
    else
     return *(sb4*)p_v;
#else
    return *(sb4*)p_v;
#endif
   }
  }else
   return p_rlen[ndx];
 }

 int is_null(int ndx)
 {
  return p_ind[ndx]==-1;
 }

 void* val(int ndx,int pelem_size)
 {
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  if((ftype==otl_var_timestamp ||
      ftype==otl_var_tz_timestamp ||
      ftype==otl_var_ltz_timestamp) &&
     !pl_tab_flag)
   return OTL_RCAST(void*,timestamp[ndx]);
#endif
#ifdef OTL_UNICODE
   if(ftype==otl_var_char)
     return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*pelem_size*sizeof(OTL_WCHAR)]);
   else if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
     return OTL_RCAST(void*,(p_v+sizeof(sb4)));
   else
     return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*pelem_size]);
#else
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   return OTL_RCAST(void*,p_v+sizeof(sb4));
  else
    return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*pelem_size]);
#endif
 }

 static int int2ext(int int_type)
 {
  switch(int_type){
  case inVarChar2: return extCChar;
  case inNumber:   return extFloat;
  case inLong:     return extLongVarChar;
  case inRowId:    return extCChar;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case inDate:     return extTimestamp;
  case inTimestamp:return extTimestamp;
  case inTimestamp_TZ:return extTimestamp_TZ;
  case inTimestamp_LTZ:return extTimestamp_LTZ;
#else
  case inDate:     return extDate;
#endif
  case inRaw:      return extLongVarRaw;
  case inLongRaw:  return extLongVarRaw;
  case inChar:     return extCChar;
  case inCLOB:     return extCLOB;
  case inBLOB:     return extBLOB;
  default:
   return -1;
  }
 }

 static int datatype_size(int ftype,int maxsz,int int_type,int max_long_size)
 {
  switch(ftype){
  case extCChar:
   switch(int_type){
   case inRowId:
    return 30;
   case inDate:
    return otl_oracle_date_size;
   case inRaw:
    return max_long_size;
   default:
    return maxsz+1;
   }
  case extLongVarChar:
   return max_long_size;
  case extLongVarRaw:
   return max_long_size;
  case extCLOB:
   return max_long_size;
  case extBLOB:
   return max_long_size;
  case extFloat:
   return sizeof(double);
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case extDate:
   return sizeof(OCIDateTime*);
  case extTimestamp:
  case extTimestamp_TZ:
  case extTimestamp_LTZ:
   return sizeof(OCIDateTime*);
#else
  case extDate:
   return otl_oracle_date_size;
#endif
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& ftype,
  int& elem_size,
  otl_select_struct_override& override,
  const int column_ndx)
 {int ndx=override.find(column_ndx);
  if(ndx==-1){
   ftype=int2ext(desc.dbtype);
   elem_size=datatype_size(ftype,desc.dbsize,desc.dbtype,max_long_size);
   switch(ftype){
   case extCChar:
    ftype=otl_var_char;
    break;
   case extFloat:
    if(override.all_mask&otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_double;
    break;
   case extLongVarChar:
    ftype=otl_var_varchar_long;
    break;
   case extLongVarRaw:
    ftype=otl_var_raw_long;
    break;
   case extCLOB:
    ftype=otl_var_clob;
    break;
   case extBLOB:
    ftype=otl_var_blob;
    break;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   case extDate:
   case extTimestamp:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
      ftype=otl_var_timestamp;
    break;
   case extTimestamp_TZ:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
      ftype=otl_var_tz_timestamp;
    break;
   case extTimestamp_LTZ:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
      ftype=otl_var_ltz_timestamp;
    break;
#else
   case extDate:
    if(override.all_mask&otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
     ftype=otl_var_timestamp;
    break;
#endif
   }
  }else{
   ftype=override.col_type[ndx];
   switch(ftype){
   case otl_var_char:
    elem_size=override.col_size[ndx];
    break;
   case otl_var_double:
    elem_size=sizeof(double);
    break;
   case otl_var_float:
    elem_size=sizeof(float);
    break;
   case otl_var_int:
    elem_size=sizeof(int);
    break;
   case otl_var_unsigned_int:
    elem_size=sizeof(unsigned);
    break;
   case otl_var_short:
    elem_size=sizeof(short);
    break;
   case otl_var_long_int:
    elem_size=sizeof(double);
    break;
   default:
    elem_size=override.col_size[ndx];
    break;
   }
  }
  desc.otl_var_dbtype=ftype;
 }

};

class otl_cur: public otl_cur0{
public:

 OCIStmt* cda; // Statement handle
 OCIError* errhp; // Error handle
 bool extern_cda;
 int status;
 int eof_status;
 otl_conn* db;
 int straight_select;
 int pos_nbr;
 int commit_on_success;
 int last_param_data_token;
 int last_sql_param_data_status;
 int sql_param_data_count;
  bool canceled;

 otl_cur()
 {
  cda=0;
  errhp=0;
  db=0;
  straight_select=1;
  pos_nbr=0;
  commit_on_success=0;
  last_param_data_token=0;
  last_sql_param_data_status=0;
  sql_param_data_count=0;
  extern_cda=false;
  canceled=false;
 }

 virtual ~otl_cur(){}

  void set_direct_exec(const int /* flag */){}

 ub4 rpc(void)
 {sb4 rpc;
  status=OCIAttrGet
   (OTL_RCAST(dvoid *,cda),
    OTL_SCAST(ub4,OCI_HTYPE_STMT),
    OTL_RCAST(dvoid *,&rpc),
    0,
    OTL_SCAST(ub4,OCI_ATTR_ROW_COUNT),
    errhp);
  if(status)return 0;
  return rpc;
 }

 int open(otl_conn& connect,otl_var* var=0)
 {
  db=&connect;
  commit_on_success=db->auto_commit;
  if(var!=0){
   extern_cda=true;
   cda=var->cda;
   status=OCI_SUCCESS;
  }else{
   status=OCIHandleAlloc
    (OTL_RCAST(dvoid *,db->envhp),
     OTL_RCAST(dvoid **,&cda),
     OCI_HTYPE_STMT,
     0,
     0);
   if(status)return 0;
  }
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid *,db->envhp),
    OTL_RCAST(dvoid **,&errhp),
    OCI_HTYPE_ERROR,
    0,
    0);
  if(status)return 0;
  straight_select=1;
  pos_nbr=0;
  return 1;
 }

 int close(void)
 {
  if(!extern_cda)
   status=OCIHandleFree(OTL_RCAST(dvoid*,cda),OCI_HTYPE_STMT);
  status=OCIHandleFree(OTL_RCAST(dvoid*,errhp),OCI_HTYPE_ERROR);
  cda=0;
  errhp=0;
  commit_on_success=0;
  return 1;
 }

 int parse(const char* stm_text)
 {
  status=OCIStmtPrepare
   (cda,
    errhp,
    OTL_RCAST(text*,OTL_CCAST(char*,stm_text)),
    OTL_SCAST(ub4,strlen(stm_text)),
    OTL_SCAST(ub4,OCI_NTV_SYNTAX),
    OTL_SCAST(ub4,OCI_DEFAULT));
  if(status)return 0;
  return 1;
 }

 int exec(const int iters, const int rowoff)
 {ub4 mode;
  if(commit_on_success)
   mode=OCI_COMMIT_ON_SUCCESS;
  else
   mode=OCI_DEFAULT;
  status=OCIStmtExecute
   (db->svchp,
    cda,
    errhp,
    OTL_SCAST(ub4,iters),
    OTL_SCAST(ub4,rowoff),
    0,
    0,
    mode);
  if(status!=OCI_SUCCESS)
   return 0;
  return 1;
 }

 long get_rpc()
 {
  return rpc();
 }

 int fetch(const short iters,int& eof_data)
 {
  eof_data=0;
  status=OCIStmtFetch
   (cda,
    errhp,
    OTL_SCAST(ub4,iters),
    OTL_SCAST(ub4,OCI_FETCH_NEXT),
    OTL_SCAST(ub4,OCI_DEFAULT));
  eof_status=status;
  if(status!=OCI_SUCCESS&&
     status!=OCI_SUCCESS_WITH_INFO&&
     status!=OCI_NO_DATA)
   return 0;
  if(status==OCI_NO_DATA){
   eof_data=1;
   return 1;
  }
  return 1;
 }

 int tmpl_ftype2ora_ftype(const int ftype)
 {
  switch(ftype){
  case otl_var_char:
   return extCChar;
  case otl_var_double:
   return extFloat;
  case otl_var_float:
   return extFloat;
  case otl_var_int:
   return extInt;
  case otl_var_unsigned_int:
   return extUInt;
  case otl_var_short:
   return extInt;
  case otl_var_long_int:
   return extInt;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case otl_var_timestamp:
   return extTimestamp;
  case otl_var_tz_timestamp:
   return extTimestamp_TZ;
  case otl_var_ltz_timestamp:
   return extTimestamp_LTZ;
#else
  case otl_var_timestamp:
   return extDate;
#endif
 case otl_var_varchar_long:
   return extLongVarChar;
  case otl_var_raw_long:
   return extLongVarRaw;
  case otl_var_clob:
   return SQLT_CLOB;
  case otl_var_blob:
   return SQLT_BLOB;
  default:
   return 0;
  }
 }

 int bind
 (const char* name,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /*param_type*/,
  const int /*name_pos*/,
  const int apl_tab_flag)
 {OCIBind* bindpp;

  int db_ftype=0;
    
   if(ftype==otl_var_refcur){
    status=OCIBindByName
     (cda,
      &bindpp,
      errhp,
      OTL_RCAST(text*,OTL_CCAST(char*,name)),
      strlen(name),
      OTL_RCAST(dvoid*,&v.cda),
      0,
      SQLT_RSET,
      0,
      0,
      0,
      0,
      0,
      OCI_DEFAULT);
   }else if(ftype!=otl_var_clob&&
            ftype!=otl_var_blob){
     int var_elem_size;
#ifdef OTL_UNICODE
     if(ftype==otl_var_char)
      var_elem_size=elem_size*sizeof(OTL_WCHAR);
     else if(ftype==otl_var_varchar_long)
      var_elem_size=elem_size;
     else
      var_elem_size=elem_size;
#else
     var_elem_size=elem_size;
#endif
     db_ftype=tmpl_ftype2ora_ftype(ftype);
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
     if(ftype==otl_var_timestamp || 
        ftype==otl_var_tz_timestamp ||
        ftype==otl_var_ltz_timestamp){
       if(!apl_tab_flag)
         var_elem_size=sizeof(OCIDateTime*);
       else if(db_ftype==extTimestamp)
         db_ftype=extDate;
     }
#endif
#ifdef OTL_UNICODE
     if(ftype==otl_var_char)
       db_ftype=SQLT_VCS;
#endif
    if(apl_tab_flag)
     status=OCIBindByName
      (cda,
       &bindpp,
       errhp,
       OTL_RCAST(text*,OTL_CCAST(char*,name)),
       strlen(name),
       OTL_RCAST(dvoid*,v.p_v),
       var_elem_size,
       OTL_SCAST(ub2,db_ftype),
       OTL_RCAST(dvoid*,v.p_ind),
       0,
       0,
       OTL_SCAST(ub4,v.max_tab_len),
       OTL_RCAST(ub4*,&v.cur_tab_len),
       OCI_DEFAULT);
    else
     status=OCIBindByName
      (cda,
       &bindpp,
       errhp,
       OTL_RCAST(text*,OTL_CCAST(char*,name)),
       strlen(name),
       OTL_RCAST(dvoid*,v.p_v),
       var_elem_size,
       OTL_SCAST(ub2,db_ftype),
       OTL_RCAST(dvoid*,v.p_ind),
       0,
       0,
       0,
       0,
       OCI_DEFAULT);
    if(status)return 0;
#ifdef OTL_UNICODE
    if(ftype==otl_var_char||ftype==otl_var_varchar_long){
      if(ftype!=otl_var_varchar_long){
        v.csfrm=db->char_set_;
        status=OCIAttrSet
          (bindpp, 
           OCI_HTYPE_BIND, 
           &v.csfrm, 
           0, 
           OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
           errhp);
        if(status)return 0;
      }
      v.csid=OTL_UNICODE_ID;
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csid, 
         0, 
         OCI_ATTR_CHARSET_ID,
         errhp);
      if(status)return 0;
      v.unicode_var_len=var_elem_size;
      if(ftype==otl_var_varchar_long)
       v.unicode_var_len=elem_size-sizeof(sb4);
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.unicode_var_len, 
         0, 
         OCI_ATTR_MAXDATA_SIZE, 
         errhp);
      if(status)return 0;
    }
#endif
    return 1;
   }else{
    status=OCIBindByName
     (cda,
      &bindpp,
      errhp,
      OTL_RCAST(text*,OTL_CCAST(char*,name)),
      strlen(name),
      OTL_RCAST(dvoid*,v.p_v),
      OTL_SCAST(sb4,-1),
      OTL_SCAST(ub2,tmpl_ftype2ora_ftype(ftype)),
      OTL_RCAST(dvoid*,v.p_ind),
      0,
      0,
      0,
      0,
     OCI_DEFAULT);
    if(status)return 0;
#ifdef OTL_UNICODE
    if(ftype==otl_var_clob){
      v.csid=OTL_UNICODE_ID;
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csid, 
         0, 
         OCI_ATTR_CHARSET_ID,
         errhp);
      if(status)return 0;
        v.csfrm=db->char_set_;
        status=OCIAttrSet
          (bindpp, 
           OCI_HTYPE_BIND, 
           &v.csfrm, 
           0, 
           OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
           errhp);
        if(status)return 0;
    }
#endif
    return 1;
   }
  if(status)return 0;
  return 1;

 }

 int bind
 (const int column_num,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /*param_type*/)
 {OCIDefine *defnp;
  int db_ftype=0;

  if(ftype!=otl_var_clob&&ftype!=otl_var_blob){
    int var_elem_size;
#ifdef OTL_UNICODE
    if(ftype==otl_var_char)
     var_elem_size=elem_size*sizeof(OTL_WCHAR);
    else if(ftype==otl_var_varchar_long)
     var_elem_size=elem_size+sizeof(sb4);
    else
     var_elem_size=elem_size;
#else
    var_elem_size=elem_size;
#endif
    db_ftype=tmpl_ftype2ora_ftype(ftype);
#ifdef OTL_UNICODE
     if(ftype==otl_var_char)
       db_ftype=SQLT_VCS;
#endif
   status=OCIDefineByPos
    (cda,
     &defnp,
     errhp,
     OTL_SCAST(ub4,column_num),
     OTL_RCAST(dvoid*,v.p_v),
     OTL_SCAST(sb4,var_elem_size),
     OTL_SCAST(ub2,db_ftype),
     OTL_RCAST(dvoid*,v.p_ind),
     OTL_RCAST(ub2*,v.p_rlen),
     OTL_RCAST(ub2*,v.p_rcode),
     OCI_DEFAULT);
   if(status)return 0;
#ifdef OTL_UNICODE
   if(ftype==otl_var_char||ftype==otl_var_varchar_long){
     v.csid=OTL_UNICODE_ID;
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        &v.csid, 
        0, 
        OCI_ATTR_CHARSET_ID, 
        errhp);
     if(status)return 0;
   }
#endif
   return 1;
  }else{
   status=OCIDefineByPos
    (cda,
     &defnp,
     errhp,
     OTL_SCAST(ub4,column_num),
     OTL_RCAST(dvoid*,v.p_v),
     OTL_SCAST(sb4,-1),
     OTL_SCAST(ub2,tmpl_ftype2ora_ftype(ftype)),
     OTL_RCAST(dvoid*,v.p_ind),
     OTL_RCAST(ub2*,v.p_rlen),
     OTL_RCAST(ub2*,v.p_rcode),
     OCI_DEFAULT);
   if(status)return 0;
#ifdef OTL_UNICODE
    if(ftype==otl_var_clob){
      v.csid=OTL_UNICODE_ID;
      status=OCIAttrSet
        (defnp, 
         OCI_HTYPE_DEFINE, 
         &v.csid, 
         0, 
         OCI_ATTR_CHARSET_ID, 
         errhp);
      if(status)return 0;
    }
#endif
   return 1;
  }
 }

 void set_select_type(const int select_type)
 {
  straight_select=select_type;
 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  OCIParam* pard;
  ub2 dtype;
  ub2 dbsize;
  sb2 prec;

#ifdef OTL_ORA8_8I_DESC_COLUMN_SCALE
  ub1 scale;
#else
  sb2 scale;
#endif

  ub1 nullok;
  text* col_name;
  ub4 col_name_len;
  ub4 pos_num;

  eof_desc=0;
  if(straight_select&&pos_nbr==0){
   status=OCIStmtExecute
    (db->svchp,
     cda,
     errhp,
     0,
     0,
     0,
     0,
     OCI_DESCRIBE_ONLY);
   if(status!=OCI_SUCCESS)return 0;
   status=OCIAttrGet
    (cda,
     OCI_HTYPE_STMT,
     OTL_RCAST(ub4*,&pos_num),
     0,
     OTL_SCAST(ub4,OCI_ATTR_PARAM_COUNT),
     errhp);
   if(status!=OCI_SUCCESS)return 0;
   pos_nbr=OTL_SCAST(int,pos_num);
  }
  if(!straight_select&&pos_nbr==0){
   status=OCIAttrGet
    (cda,
     OCI_HTYPE_STMT,
     OTL_RCAST(ub4*,&pos_num),
     0,
     OTL_SCAST(ub4,OCI_ATTR_PARAM_COUNT),
     errhp);
   if(status!=OCI_SUCCESS)return 0;
   pos_nbr=OTL_SCAST(int,pos_num);
  }
  if(column_num<1||column_num>pos_nbr){
   eof_desc=1;
   return 0;
  }
  status=OCIParamGet
   (cda,
    OCI_HTYPE_STMT,
    errhp,
    OTL_RCAST(void**,&pard),
    OTL_SCAST(ub4,column_num));
  if(status!=OCI_SUCCESS&&status!=OCI_NO_DATA)
   return 0;
  if(status==OCI_NO_DATA){
   eof_desc=1;
   return 1;
  }
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&dtype),
    0,
    OTL_SCAST(ub4,OCI_ATTR_DATA_TYPE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.dbtype=dtype;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid**,&col_name),
    OTL_RCAST(ub4*,&col_name_len),
    OTL_SCAST(ub4,OCI_ATTR_NAME),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  strncpy(OTL_RCAST(char*,col.name),OTL_RCAST(char*,col_name),sizeof(col.name));
  col.name[sizeof(col.name)-1]=0;
  col.name[col_name_len]=0;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&dbsize),
    OTL_RCAST(ub4*,0),
    OTL_SCAST(ub4,OCI_ATTR_DATA_SIZE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.dbsize=dbsize;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&prec),
    0,
    OTL_SCAST(ub4,OCI_ATTR_PRECISION),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.prec=prec;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&scale),
    0,
    OTL_SCAST(ub4,OCI_ATTR_SCALE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.scale=scale;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&nullok),
    0,
    OTL_SCAST(ub4,OCI_ATTR_IS_NULL),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.nullok=nullok;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {sb4 errcode;
  int len;

  strcpy(OTL_RCAST(char*,exception_struct.msg),"123456789");
  OCIErrorGet
   (OTL_RCAST(dvoid*,errhp),
    OTL_SCAST(ub4,1),
    0,
    &errcode,
    OTL_RCAST(text*,exception_struct.msg),
    OTL_SCAST(ub4,sizeof(exception_struct.msg)),
    OCI_HTYPE_ERROR);
  exception_struct.code=errcode;
  len=strlen(OTL_RCAST(char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

};

class otl_sel{
public:

 int implicit_cursor;

 int close_select(otl_cur& /*cur*/)
 {
  int i=1;
  return i;
 }

 otl_sel()
 {
  implicit_cursor=0;
 }

 virtual ~otl_sel(){}

  void set_select_type(const int /*atype*/)
 {
  implicit_cursor=0;
 }

  void init(const int /*array_size*/){}

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  eof_data=0;
  cur_row=-1;
  cur.commit_on_success=0;
  rc=cur.exec(0,0);
  if(rc==0)return 0;
  rc=cur.fetch(OTL_SCAST(short,array_size),eof_data);
  if(rc==0)return 0;
  row_count=cur.rpc();
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    return 1;
   }
   cur.commit_on_success=0;
   rc=cur.fetch(OTL_SCAST(short,array_size),eof_data);
   if(rc==0)return 0;
   cur_size=cur.rpc()-row_count;
   row_count=cur.rpc();
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }


};


typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_ora8_connect;


typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_lob_stream: public otl_lob_stream_generic{
public:

  typedef otl_tmpl_exception
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct> otl_exception;

 typedef otl_tmpl_variable<TVariableStruct>* p_bind_var;
 typedef otl_tmpl_connect
         <TExceptionStruct,
         TConnectStruct,
         TCursorStruct>* p_connect;

 typedef otl_tmpl_cursor
         <TExceptionStruct,
          TConnectStruct,
          TCursorStruct,
          TVariableStruct>* p_cursor;

 p_bind_var bind_var;
 p_connect connect;
 p_cursor cursor;

 void init
 (void* avar,void* aconnect,void* acursor,
  int andx,
  int amode,
  const int alob_is_null=0) OTL_NO_THROW
 {
  connect=OTL_RCAST(p_connect,aconnect);
  bind_var=OTL_RCAST(p_bind_var,avar);
  cursor=OTL_RCAST(p_cursor,acursor);
  mode=amode;
  retcode=0;
  lob_is_null=alob_is_null;
  ndx=andx;
  offset=0;
  lob_len=0;
  eof_flag=0;
  in_destructor=0;
  if(bind_var)
   bind_var->var_struct.set_lob_stream_flag();
 }

 void set_len(const int new_len=0) OTL_NO_THROW
 {
  lob_len=new_len;
 }

 otl_tmpl_lob_stream() OTL_NO_THROW
   : otl_lob_stream_generic(true)
 {
  init(0,0,0,0,otl_lob_stream_zero_mode);
 }

  virtual ~otl_tmpl_lob_stream()
    OTL_THROWS_OTL_EXCEPTION
 {in_destructor=1;
  close();
 }

 otl_lob_stream_generic& operator<<(const otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(mode!=otl_lob_stream_write_mode){
   char* stm=0;
   char var_info[256];
   var_info[0]=0;
   if(cursor!=0){
     if(cursor->stm_label)
       stm=cursor->stm_label;
     else
       stm=cursor->stm_text;
   }
   if(bind_var!=0){
    otl_var_info_var
     (bind_var->name,
      bind_var->ftype,
      otl_var_long_string,
      var_info);
   }
   char* vinfo=0;
   if(var_info[0]!=0)
    vinfo=&var_info[0];
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
     (otl_error_msg_9,
      otl_error_code_9,
      stm,
      vinfo);
  }
  if(offset==0)offset=1; 
  if((offset-1)+s.length>lob_len){
   char var_info[256];
   otl_var_info_var
    (bind_var->name,
     bind_var->ftype,
     otl_var_long_string,
     var_info);
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_7,
     otl_error_code_7,
     cursor->stm_label?cursor->stm_label:
     cursor->stm_text,
     var_info);
  }
  retcode=bind_var->var_struct.write_blob
   (s,lob_len,offset,cursor->cursor_struct);
  if(retcode){
   if((offset-1)==lob_len)
    close();
   return *this;
  }
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 otl_lob_stream_generic& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(mode!=otl_lob_stream_read_mode){
   char* stm=0;
   char var_info[256];
   var_info[0]=0;
   if(cursor!=0){
     if(cursor->stm_label)
       stm=cursor->stm_label;
     else
       stm=cursor->stm_text;
   }
   if(bind_var!=0){
    otl_var_info_var
     (bind_var->name,
      bind_var->ftype,
      otl_var_long_string,
      var_info);
   }
   char* vinfo=0;
   if(var_info[0]!=0)
    vinfo=&var_info[0];
   if(this->connect)this->connect->throw_count++;
   if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
   throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>
    (otl_error_msg_10,
     otl_error_code_10,
     stm,
     vinfo);
  }
  if(offset==0&&lob_len==0)
   lob_len=len();
  if(lob_len==0||(offset-1)==lob_len){
   s.set_len(0);
   eof_flag=1;
   return *this;
  }
  if(offset==0)offset=1;
  retcode=bind_var->var_struct.read_blob(s,ndx,offset,lob_len);
  if((offset-1)==lob_len)eof_flag=1;
  if(retcode){
   if(eof()){
    close();
    eof_flag=1;
   }
   return *this;
  }
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 int eof(void) OTL_NO_THROW
 {
  if(lob_is_null)return 1;
  return eof_flag;
 }

 int len(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(cursor==0||connect==0||bind_var==0||lob_is_null)return 0;
  int alen;
  retcode=bind_var->var_struct.get_blob_len(ndx,alen);
  if(retcode)return alen;
  if(this->connect)this->connect->throw_count++;
  if(this->connect&&this->connect->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->connect_struct,
                    cursor->stm_label?cursor->stm_label:
                    cursor->stm_text);
 }

 void close(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(in_destructor){
   if(mode==otl_lob_stream_read_mode){
    bind_var->var_struct.set_lob_stream_flag(0);
    bind_var->set_not_null(0);
   }
   return;
  }
  if(mode==otl_lob_stream_zero_mode)return;
  if(mode==otl_lob_stream_read_mode){
    if(offset<lob_len-1)
      bind_var->var_struct.close_lob();
    bind_var->var_struct.set_lob_stream_flag(0);
    bind_var->set_not_null(0);
    init(0,0,0,0,otl_lob_stream_zero_mode);
  }else{
   // write mode
   if(!(offset==0&&lob_len==0)&&(offset-1)!=lob_len){
     bind_var->var_struct.close_lob();     
     char var_info[256];
     char msg_buf[1024];
     strcpy(msg_buf,otl_error_msg_8);
     otl_var_info_var
       (bind_var->name,
        bind_var->ftype,
        otl_var_long_string,
        var_info);
     if(this->connect)this->connect->throw_count++;
     if(this->connect&&this->connect->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (msg_buf,
        otl_error_code_8,
        cursor->stm_label?cursor->stm_label:
        cursor->stm_text,
        var_info);
   }
   bind_var->var_struct.set_lob_stream_flag(0);
   bind_var->set_not_null(0);
  }
 }

};


typedef otl_tmpl_lob_stream
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_lob_stream;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time0> otl_ora8_inout_stream;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time0> otl_select_stream;


typedef otl_tmpl_ext_hv_decl
 <otl_var,
  otl_time0,
  otl_exc,
  otl_conn,
  otl_cur> otl_ext_hv_decl;

const int otl_no_stream_type=0;
const int otl_inout_stream_type=1;
const int otl_refcur_stream_type=2;
const int otl_select_stream_type=3;
const int otl_constant_sql_type=4;

class otl_connect: public otl_ora8_connect{
public:

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 otl_stream_pool sc;

 void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
 {
  sc.init(max_size);
 }

#endif

public:

 otl_connect() OTL_NO_THROW
   :otl_ora8_connect(){}

#ifdef OTL_UNICODE
 void set_character_set(const int char_set=SQLCS_IMPLICIT)
   OTL_THROWS_OTL_EXCEPTION
 {
  connect_struct.set_char_set(char_set);
 }
#endif 

 otl_connect(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
   : otl_ora8_connect(connect_str, aauto_commit){}

 virtual ~otl_connect() OTL_THROWS_OTL_EXCEPTION
  {}

 static int otl_terminate(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if defined(OTL_ORA8)&&!defined(OTL_ORA8I)&&!defined(OTL_ORA9I)
   return 1;
#else
  return OCITerminate(OCI_DEFAULT)==OCI_SUCCESS;
#endif
 }

 void cancel(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  retcode=connect_struct.cancel();
  if(!retcode){
   throw_count++;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

#if defined(OTL_ORA8I) || defined(OTL_ORA9I)
  void change_password
  (const char* user_name,
   const char* old_password,
   const char* new_password)
    OTL_THROWS_OTL_EXCEPTION
  {
    throw_count=0;
    retcode=connect_struct.change_password
      (user_name,
       old_password,
       new_password);
    if(!retcode){
      throw_count++;
      if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
      if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
      throw otl_exception(connect_struct);
    }
  }
#endif

  void auto_commit_off(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_ora8_connect::auto_commit_off();
  }

  void auto_commit_on(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_ora8_connect::auto_commit_on();
  }

 void rlogon(OCIEnv *envhp,OCISvcCtx *svchp)
   OTL_THROWS_OTL_EXCEPTION
 {
  connected=0;
  long_max_size=32760;
  retcode=connect_struct.ext_logon(envhp,svchp,0);
  if(retcode)
   connected=1;
  else{
   connected=0;
   throw_count++;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void rlogon(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_ora8_connect::rlogon(connect_str,aauto_commit);
 }

 void logoff(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
   sc.init(sc.max_size);
#endif
  if(!connected){
   connect_struct.session_end();
   connect_struct.server_detach();
  }else{
   if(connect_struct.extern_lda)
    connect_struct.logoff();
   else{
    session_end();
    server_detach();
   }
   connected=0;
  }
 }

 void server_attach(const char* tnsname=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  connected=0;
  long_max_size=32760;
  throw_count=0;
  retcode=connect_struct.server_attach(tnsname);
  if(!retcode){
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void server_detach(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  retcode=connect_struct.server_detach();
  if(!retcode){
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void session_begin
  (const char* username,
   const char* password,
   const int auto_commit=0,
   const int session_mode=OCI_DEFAULT
   // OCI_SYSDBA -- in this mode, the user is authenticated for SYSDBA
   // access.  
   // OCI_SYSOPER -- in this mode, the user is authenticated
   // for SYSOPER access.
  ) OTL_THROWS_OTL_EXCEPTION
 {
  throw_count=0;
  retcode=connect_struct.session_begin
   (username,password,auto_commit,session_mode);
  if(retcode)
   connected=1;
  else{
   connected=0;
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void session_reopen(const int auto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  throw_count=0;
  if(connect_struct.session_begin_count==0){
   connected=0;
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(otl_error_msg_11,otl_error_code_11);
  }
  retcode=connect_struct.session_begin(auto_commit);
  if(retcode)
   connected=1;
  else{
   connected=0;
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

 void session_end(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
   sc.init(sc.max_size);
#endif
  connected=0;
  retcode=connect_struct.session_end();
  if(!retcode){
   ++throw_count;
   if(throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(connect_struct);
  }
 }

};

typedef otl_tmpl_variable<otl_var> otl_generic_variable;
typedef otl_generic_variable* otl_p_generic_variable;

class otl_refcur_base_cursor: public
 otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> {
public:

 int cur_row;
 int cur_size;
 int row_count;
 int array_size;

 otl_refcur_base_cursor
 (otl_connect& db,
  otl_var* var,
  const char* master_plsql_block,
  const short arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db,var)
 {
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  stm_text=new char[strlen(master_plsql_block)+1];
  strcpy(stm_text,master_plsql_block);
 }

 otl_refcur_base_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>()
 {
 }

 virtual ~otl_refcur_base_cursor()
 {
  delete[] stm_text;
  stm_text=0;
 }

 void open
 (otl_connect& db,
  otl_var* var,
  const char* master_plsql_block,
  const short arr_size=1)
 {
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db,var);
  stm_text=new char[strlen(master_plsql_block)+1];
  strcpy(stm_text,master_plsql_block);
 }

 void close(void)
 {
  delete[] stm_text;
  stm_text=0;
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::close();
 }

 int first(void)
 {int rc;

  cur_row=-1;
  rc=cursor_struct.fetch(OTL_SCAST(short,array_size),eof_data);
  if(rc==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  row_count=cursor_struct.rpc();
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
   if(otl_tmpl_cursor<otl_exc,otl_conn,otl_cur,otl_var>::eof()){
    cur_row=-1;
    return 0;
   }
   rc=cursor_struct.fetch(OTL_SCAST(short,array_size),eof_data);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
    throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
   }
   cur_size=cursor_struct.rpc()-row_count;
   row_count=cursor_struct.rpc();
   if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind_col
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  v.pos=column_num;
  otl_refcur_base_cursor::bind(column_num,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i;
  desc_len=0;
  cursor_struct.straight_select=0;
  for(i=1;describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }

};

class otl_refcur_stream: public otl_refcur_base_cursor{
public:

  int delay_next;
  int same_sl_flag;
  otl_select_struct_override override;

  void set_column_type(const int column_ndx,
                       const int col_type,
                       const int col_size=0)
    OTL_NO_THROW
  {
    override.add_override(column_ndx,col_type,col_size);
  }
  
  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    override.set_all_column_types(mask);
  }

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

 otl_refcur_stream() OTL_NO_THROW
   :otl_refcur_base_cursor()
 {
  init();
 }

 otl_refcur_stream
 (const short arr_size,
  const char* master_plsql_block,
  otl_var* var,
  otl_connect& db)
   OTL_THROWS_OTL_EXCEPTION
  :otl_refcur_base_cursor(db,var,master_plsql_block,arr_size)
 {
  init();
  try{
   rewind();
   null_fetched=0;
  }catch(otl_exception){
   cleanup();
   if(this->adb)this->adb->throw_count++;
   throw;
  }
 }

 virtual ~otl_refcur_stream()
   OTL_THROWS_OTL_EXCEPTION
 {
  cleanup();
  close();
 }

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  ret_code=first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
  delay_next=0;
 }

 int is_null(void) OTL_NO_THROW
 {
  return null_fetched;
 }

 int eof(void) OTL_NO_THROW
 {
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !ret_code;
 }

 int eof_intern(void)
 {
  return !ret_code;
 }

 void check_if_executed(void){}

 void open
 (otl_connect& db,
  otl_var* var,
  const char* master_plsql_block,
  const short arr_size=1)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_refcur_base_cursor::open(db,var,master_plsql_block,arr_size);
  get_select_list();
  rewind();
 }

  void close(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    override.reset();
    otl_refcur_base_cursor::close();
  }

 otl_refcur_stream& operator>>(otl_time0& t)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].var_struct.read_dt(&t,tm,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception(adb->connect_struct,stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
#endif
   look_ahead();
  }
  return *this;
 }

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared
#else
 otl_refcur_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time0 tmp;
  (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
    s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
    s.month=tmp.month;
    s.day=tmp.day;
    s.hour=tmp.hour-1;
    s.minute=tmp.minute-1;
    s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  return *this;
 }
#endif

 otl_refcur_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {

  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_refcur_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  switch(sl[cur_col].ftype){
  case otl_var_char:
    if(!eof_intern()){
      s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
      if((*this).is_null())
        s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif
      look_ahead();
    }
    break;
#if defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if defined(OTL_STL)
      s.assign((char*)c,len);
#elif defined(OTL_ACE)
      s.set((char*)c,len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.ptr;
    
      int rc=sl[cur_col].var_struct.get_blob
        (cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
        throw otl_exception(adb->connect_struct,
                            stm_label?stm_label:stm_text);
      }
#if defined(OTL_STL)
      s.assign((char*)temp_buf,len);
#elif defined(OTL_ACE)
      s.set((char*)temp_buf,len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

 otl_refcur_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)));
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(OTL_RCAST(char*,s),
            OTL_RCAST(const char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
           );
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     n);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(int,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_unsigned
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     u);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_unsigned_int))
      u=OTL_PCONV(unsigned,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_short
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     sh);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_short))
      sh=OTL_PCONV(short,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_long_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     l);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_long_int))
      l=OTL_PCONV(long int,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_float
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     f);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_float))
      f=OTL_PCONV(float,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_double
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     d);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_double))
     d=*OTL_RCAST(double*,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_varchar_long||
      sl[cur_col].ftype==otl_var_raw_long)&&
     !eof_intern()){
   unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   int len=sl[cur_col].get_len(cur_row);
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,sl[cur_col].ftype);
   s.null_terminate_string(len);
   s.set_len(len);
   look_ahead();
  }else if((sl[cur_col].ftype==otl_var_blob||
            sl[cur_col].ftype==otl_var_clob)&&
           !eof_intern()){
   int len;
   int rc=sl[cur_col].var_struct.get_blob(cur_row,s.v,s.buf_size,len);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception(adb->connect_struct,
                        stm_label?stm_label:stm_text);
   }
   s.set_len(len);
   s.null_terminate_string(len);
   look_ahead();
  }
  return *this;
 }

 otl_refcur_stream& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_blob||
      sl[cur_col].ftype==otl_var_clob)&&
     !eof_intern()){
   s.init
    (&sl[cur_col],
     adb,
     OTL_RCAST(otl_refcur_base_cursor*,this),
     cur_row,
     otl_lob_stream_read_mode,
     this->is_null());
   delay_next=1;
  }
  return *this;
 }

 int select_list_len(void) OTL_NO_THROW
 {
  return sl_len;
 }

 int column_ftype(int ndx=0) OTL_NO_THROW
 {
  return sl[ndx].ftype;
 }

 int column_size(int ndx=0) OTL_NO_THROW
 {
  return sl[ndx].elem_size;
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  desc_len=sl_len;
  return sl_desc;
 }

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;

protected:

 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

 void init(void)
 {
  same_sl_flag=0;
  sl=0;
  sl_len=0;
  null_fetched=0;
  ret_code=0;
  sl_desc=0;
  executed=0;
  cur_in=0;
  cur_col=-1;
  executed=1;
  stm_text=0;
  delay_next=0;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

 int check_type(int type_code, int actual_data_type=0)
 {int out_type_code;
  if(actual_data_type!=0)
   out_type_code=actual_data_type;
  else
   out_type_code=type_code;
  if(type_code==otl_var_timestamp &&
     (sl[cur_col].ftype==otl_var_timestamp ||
      sl[cur_col].ftype==otl_var_tz_timestamp ||
      sl[cur_col].ftype==otl_var_ltz_timestamp))
    return 1;
  if(sl[cur_col].ftype!=type_code){
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     out_type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception
    (otl_error_msg_0,
     otl_error_code_0,
     stm_label?stm_label:stm_text,
     var_info);
  }else
   return 1;
 }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
   ret_code=next();
   cur_col=-1;
  }
 }

 void get_select_list(void)
 {int i,j;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.ptr;
  int sld_tmp_len=0;
  int ftype,elem_size;

   sld_tmp_len=0;
   cursor_struct.straight_select=0;
   for(i=1;describe_column(sl_desc_tmp[i-1],i);++i)
    ++sld_tmp_len;
   sl_len=sld_tmp_len;
   if(sl){
    delete[] sl;
    sl=0;
   }
   sl=new otl_generic_variable[sl_len==0?1:sl_len];
   int max_long_size=this->adb->get_max_long_size();
   for(j=0;j<sl_len;++j){
    otl_generic_variable::map_ftype
     (sl_desc_tmp[j],max_long_size,ftype,elem_size,override,j+1);
    sl[j].copy_pos(j+1);
    sl[j].init(ftype,
               elem_size,
               OTL_SCAST(short,array_size),
               &adb->connect_struct
               );
   }
   if(sl_desc){
    delete[] sl_desc;
    sl_desc=0;
   }
   sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
   memcpy(sl_desc,sl_desc_tmp,sizeof(otl_column_desc)*sl_len);
   for(i=0;i<sl_len;++i)bind_col(i+1,sl[i]);
 }

};

class otl_inout_stream: public otl_ora8_inout_stream{
public:

 otl_inout_stream
 (short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=0)
  : otl_ora8_inout_stream(arr_size,sqlstm,db,
                          alob_stream_mode,sqlstm_label)
 {
  adb2=&db;
 } 

 otl_inout_stream& operator>>(otl_refcur_stream& str)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_refcur,1)){
   if(str.connected)str.close();
   str.open(*adb2,
            &(in_vl[cur_in_x]->var_struct),
            stm_text,
            OTL_SCAST
            (const short, 
             in_vl[cur_in_x]->var_struct.array_size)
           );
   null_fetched=0;
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)

 otl_inout_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

#endif

 otl_inout_stream& operator<<(const otl_null&)
 {
  otl_ora8_inout_stream::operator<<(otl_null());
  return *this;
 }


#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_inout_stream& operator>>(otl_pl_vec_generic& tab)
 {
  otl_ora8_inout_stream::operator>>(tab);
  return *this;
 }

 otl_inout_stream& operator<<(otl_pl_vec_generic& tab)
 {
  otl_ora8_inout_stream::operator<<(tab);
  return *this;
 }

#endif

 otl_inout_stream& operator>>(otl_pl_tab_generic& tab)
 {
  otl_ora8_inout_stream::operator>>(tab);
  return *this;
 }

 otl_inout_stream& operator<<(otl_pl_tab_generic& tab)
 {
  otl_ora8_inout_stream::operator<<(tab);
  return *this;
 }


 otl_inout_stream& operator>>(otl_time0& s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const otl_time0& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

 otl_inout_stream& operator>>(char& c)
 {
  otl_ora8_inout_stream::operator>>(c);
  return *this;
 }

 otl_inout_stream& operator<<(const char c)
 {
  otl_ora8_inout_stream::operator<<(c);
  return *this;
 }


 otl_inout_stream& operator>>(unsigned char& c)
 {
  otl_ora8_inout_stream::operator>>(c);
  return *this;
 }

 otl_inout_stream& operator<<(const unsigned char c)
 {
  otl_ora8_inout_stream::operator<<(c);
  return *this;
 }


 otl_inout_stream& operator>>(char* s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const char* s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

 otl_inout_stream& operator>>(unsigned char* s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const unsigned char* s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }


 otl_inout_stream& operator>>(int& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const int n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }


 otl_inout_stream& operator>>(float& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const float n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }

 otl_inout_stream& operator>>(double& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const double n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }


 otl_inout_stream& operator>>(short int& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const short int n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }

 otl_inout_stream& operator>>(unsigned int& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const unsigned int n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }


 otl_inout_stream& operator>>(long int& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const long int n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }

 otl_inout_stream& operator>>(otl_long_string& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const otl_long_string& n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }

 otl_inout_stream& operator>>(otl_lob_stream& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(otl_lob_stream& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

protected:

 otl_connect* adb2;

};


// ============ OTL Reference Cursor Streams for Oracle 8 =================

class otl_ref_cursor: public
 otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> {
public:

 int cur_row;
 int cur_size;
 int row_count;
 int array_size;

 otl_ref_cursor
 (otl_connect& db,
  const char* cur_placeholder_name,
  const short arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db)
 {int i;
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=0;
  strcpy(cur_placeholder,cur_placeholder_name);
 }

 otl_ref_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
  sel_cur()
 {
 }

 virtual ~otl_ref_cursor()
 {
  delete[] rvl;
  rvl=0;
 }

 void open
 (otl_connect& db,
  const char* cur_placeholder_name,
  const short arr_size=1)
 {int i;
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=0;
  strcpy(cur_placeholder,cur_placeholder_name);
  if(!sel_cur.connected)sel_cur.open(db);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db);
 }

 void release_sel_cur(void)
 {
#ifdef OTL_ORA8_8I_REFCUR
  return;
#else
  char tmp_buf[256];
  OCIBind* bindpp;
  int rc;

  if(!sel_cur.connected)return;
  strcpy(tmp_buf,"begin close ");
  strcat(tmp_buf,cur_placeholder);
  strcat(tmp_buf,"; end;");
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::parse(tmp_buf);
  rc=OCIBindByName
   (cursor_struct.cda,
    &bindpp,
    cursor_struct.errhp,
    OTL_RCAST(text*,cur_placeholder),
    strlen(cur_placeholder),
    OTL_RCAST(dvoid*,&sel_cur.cursor_struct.cda),
    0,
    SQLT_RSET,
    0,
    0,
    0,
    0,
    0,
    OCI_DEFAULT);
  if(rc!=0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception(cursor_struct,
                       stm_label?stm_label:stm_text);
  }
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::exec();
#endif
 }

 void close(void)
 {
  delete[] rvl;
  rvl=0;
  release_sel_cur();
  sel_cur.close();
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::close();
 }

 int first(void)
 {int i,rc;
  OCIBind* bindpp;

  if(!sel_cur.connected){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     strlen(cur_placeholder),
     OTL_RCAST(dvoid*,&sel_cur.cursor_struct.cda),
     0,
     SQLT_RSET,
     0,
     0,
     0,
     0,
     0,
     OCI_DEFAULT);
   if(rc!=0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
    throw otl_exception(cursor_struct,
                        stm_label?stm_label:stm_text);
   }
  }

  if(cur_row==-2)
   ; // Special case -- calling describe_select() between parse() and first()
  else{
   exec(1); // Executing the PLSQL master block
   sel_cur.connected=1;
  }
  cur_row=-1;
  for(i=0;i<vl_cur_len;++i)
   sel_cur.bind(i+1,*rvl[i]);
  rc=sel_cur.cursor_struct.fetch(OTL_SCAST(short,array_size),sel_cur.eof_data);
  if(rc==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception(sel_cur.cursor_struct,
                       stm_label?stm_label:stm_text);
  }
  row_count=sel_cur.cursor_struct.rpc();
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
   if(sel_cur.eof()){
     cur_row=-1;
     return 0;
   }
   rc=sel_cur.cursor_struct.fetch(OTL_SCAST(short,array_size),sel_cur.eof_data);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
    throw otl_exception(sel_cur.cursor_struct,
                        stm_label?stm_label:stm_text);
   }
   cur_size=sel_cur.cursor_struct.rpc()-row_count;
   row_count=sel_cur.cursor_struct.rpc();
   if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  ++vl_cur_len;
  rvl[vl_cur_len-1]=&v;
  v.pos=column_num;
 }

 void bind(otl_generic_variable& v)
 {
  if(v.pos)
   bind(v.pos,v);
  else if(v.name)
   otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(v);
 }

 void bind
 (const char* name,
  otl_generic_variable& v)
 {
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(name,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i,rc;
  OCIBind* bindpp;

  if(!sel_cur.connected){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     strlen(cur_placeholder),
     OTL_RCAST(dvoid*,&sel_cur.cursor_struct.cda),
     0,
     SQLT_RSET,
     0,
     0,
     0,
     0,
     0,
     OCI_DEFAULT);
   if(rc!=0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
    throw otl_exception(cursor_struct,
                        stm_label?stm_label:stm_text);
   }
  }
  exec(1); // Executing the PLSQL master block
  sel_cur.connected=1;
  cur_row=-2; // Special case -- describe_select() before first() or next()
  desc_len=0;
  sel_cur.cursor_struct.straight_select=0;
  for(i=1;sel_cur.describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }

public:

 otl_cursor sel_cur;

protected:

 int rvl_len;
 otl_p_generic_variable* rvl;
 int vl_cur_len;
 char cur_placeholder[64];

};

class otl_ref_select_stream: public otl_ref_cursor{
public:

 otl_select_struct_override *override;
 int delay_next;
 int same_sl_flag;
 long _rfc;

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

 otl_ref_select_stream
 (otl_select_struct_override* aoverride,
  const short arr_size,
  const char* sqlstm,
  const char* cur_placeholder,
  otl_connect& db,
  const char* sqlstm_label=0)
  :otl_ref_cursor(db,cur_placeholder,arr_size)
 {
   if(sqlstm_label!=0){
     if(stm_label!=0){
       delete[] stm_label;
       stm_label=0;
     }
     stm_label=new char[strlen(sqlstm_label)+1];
     strcpy(stm_label,sqlstm_label);
   }

  _rfc=0;
  init();

  override=aoverride;
  {
   int len=strlen(sqlstm)+1;
   stm_text=new char[len];
   strcpy(stm_text,sqlstm);
   otl_ext_hv_decl hvd(stm_text,1);
   hvd.alloc_host_var_list(vl,vl_len,*adb);
  }

  try{

   parse();
   if(vl_len==0){
    rewind();
    null_fetched=0;
   }
  }catch(otl_exception){
   cleanup();
   if(this->adb)this->adb->throw_count++;
   throw;
  }

 }

 virtual ~otl_ref_select_stream()
 {
  cleanup();
  close();
 }

 void rewind(void)
 {
  _rfc=0;
  get_select_list();
  ret_code=first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
  delay_next=0;
 }
  
  void clean(void)
  {
    _rfc=0;
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
    delay_next=0;
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !ret_code;
 }

 int eof_intern(void)
 {
  return !ret_code;
 }


 otl_ref_select_stream& operator>>(otl_time0& t)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].var_struct.read_dt(&t,tm,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception(adb->connect_struct,
                        stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator<<(const otl_time0& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,otl_oracle_date_size)){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,vl[cur_in]->val());
   int rc=vl[cur_in]->var_struct.write_dt(tm,&t,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception(adb->connect_struct,
                        stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,vl[cur_in]->val());
   memcpy(tm,OTL_RCAST(void*,OTL_CCAST(otl_time0*,&t)),otl_oracle_date_size);
#endif
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


 otl_ref_select_stream& operator>>(char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  switch(sl[cur_col].ftype){
  case otl_var_char:
    if(!eof_intern()){
      s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
      look_ahead();
    }
    break;
#if defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if defined(OTL_STL)
      s.assign((char*)c,len);
#elif defined(OTL_ACE)
      s.set((char*)c,len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.ptr;
    
      int rc=sl[cur_col].var_struct.get_blob
        (cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->throw_count++;
        if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
        if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
        throw otl_exception(adb->connect_struct,
                            stm_label?stm_label:stm_text);
      }
#if defined(OTL_STL)
      s.assign((char*)temp_buf,len);
#elif defined(OTL_ACE)
      s.set((char*)temp_buf,len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

 otl_ref_select_stream& operator>>(char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(int& n)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     n);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(int,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned& u)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_unsigned
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     u);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_unsigned_int))
      u=OTL_PCONV(unsigned,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(short& sh)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_short
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     sh);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_short))
      sh=OTL_PCONV(short,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(long int& l)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_long_int
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     l);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_long_int))
      l=OTL_PCONV(long int,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(float& f)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_float
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     f);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_float))
      f=OTL_PCONV(float,double,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(double& d)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
   int match_found=otl_numeric_convert_double
    (sl[cur_col].ftype,
     sl[cur_col].val(cur_row),
     d);
   if(!match_found){
    if(check_type(otl_var_double,otl_var_double))
     d=*OTL_RCAST(double*,sl[cur_col].val(cur_row));
   }
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(otl_long_string& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_varchar_long||
      sl[cur_col].ftype==otl_var_raw_long)&&
     !eof_intern()){
   unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   int len=sl[cur_col].get_len(cur_row);
   if(len>s.buf_size)len=s.buf_size;
   otl_memcpy(s.v,c,len,sl[cur_col].ftype);
   s.v[len]=0;
   s.set_len(len);
   look_ahead();
  }else if((sl[cur_col].ftype==otl_var_blob||
            sl[cur_col].ftype==otl_var_clob)&&
           !eof_intern()){
   int len;
   int rc=sl[cur_col].var_struct.get_blob(cur_row,s.v,s.buf_size,len);
   if(rc==0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception(adb->connect_struct,
                        stm_label?stm_label:stm_text);
   }
   s.set_len(len);
   s.null_terminate_string(len);
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(otl_lob_stream& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].ftype==otl_var_blob||
      sl[cur_col].ftype==otl_var_clob)&&
     !eof_intern()){
   s.init
    (&sl[cur_col],
     adb,
     OTL_RCAST(otl_ref_cursor*,this),
     cur_row,
     otl_lob_stream_read_mode,
     this->is_null());
   delay_next=1;
  }
  return *this;
 }

  otl_ref_select_stream& operator<<(const otl_null& /*n*/)
  {
    check_in_var();
    this->vl[cur_in]->set_null(0);
    get_in_next();
    return *this;
  }

 otl_ref_select_stream& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=OTL_RCAST(unsigned char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }
#endif

 otl_ref_select_stream& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     vl[cur_in]->elem_size
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
     (vl[cur_in]->name,
      vl[cur_in]->ftype,
      otl_var_char,
      temp_var_info);
    if(this->adb)this->adb->throw_count++;
  if(this->adb&&this->adb->throw_count>1)return *this;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(STD_NAMESPACE_PREFIX uncaught_exception())return *this; 
#endif
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const int n)
 {
  check_in_var();
  if(check_in_type(otl_var_int,sizeof(int))){
   *OTL_RCAST(int*,vl[cur_in]->val())=n;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned u)
 {
  check_in_var();
  if(check_in_type(otl_var_unsigned_int,sizeof(unsigned))){
   *OTL_RCAST(unsigned*,vl[cur_in]->val())=u;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const short sh)
 {
  check_in_var();
  if(check_in_type(otl_var_short,sizeof(short))){
   *OTL_RCAST(short*,vl[cur_in]->val())=sh;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const long int l)
 {
  check_in_var();
  if(check_in_type(otl_var_long_int,sizeof(long))){
   *OTL_RCAST(long*,vl[cur_in]->val())=l;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const float f)
 {
  check_in_var();
  if(check_in_type(otl_var_float,sizeof(float))){
   *OTL_RCAST(float*,vl[cur_in]->val())=f;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const double d)
 {
  check_in_var();
  if(check_in_type(otl_var_double,sizeof(double))){
   *OTL_RCAST(double*,vl[cur_in]->val())=d;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 int select_list_len(void)
 {
  return sl_len;
 }

 int column_ftype(int ndx=0)
 {
  return sl[ndx].ftype;
 }

 int column_size(int ndx=0)
 {
  return sl[ndx].elem_size;
 }

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;


protected:


 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

 void init(void)
 {
  same_sl_flag=0;
  sl=0;
  sl_len=0;
  null_fetched=0;
  ret_code=0;
  sl_desc=0;
  executed=0;
  cur_in=0;
  stm_text=0;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

 int check_type(int type_code, int actual_data_type=0)
 {int out_type_code;
  if(actual_data_type!=0)
   out_type_code=actual_data_type;
  else
   out_type_code=type_code;
  if(type_code==otl_var_timestamp &&
     (sl[cur_col].ftype==otl_var_timestamp ||
      sl[cur_col].ftype==otl_var_tz_timestamp ||
      sl[cur_col].ftype==otl_var_ltz_timestamp))
    return 1;
  if(sl[cur_col].ftype!=type_code){
   otl_var_info_col
    (sl[cur_col].pos,
     sl[cur_col].ftype,
     out_type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception
    (otl_error_msg_0,
     otl_error_code_0,
     stm_label?stm_label:stm_text,
     var_info);
  }else
   return 1;
 }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
   ret_code=next();
   cur_col=-1;
   ++_rfc;
  }
 }

 void get_select_list(void)
 {int i,j,rc;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.ptr;
  int sld_tmp_len=0;
  int ftype,elem_size;
  OCIBind* bindpp;

  if(!sel_cur.connected){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     strlen(cur_placeholder),
     OTL_RCAST(dvoid*,&sel_cur.cursor_struct.cda),
     0,
     SQLT_RSET,
     0,
     0,
     0,
     0,
     0,
     OCI_DEFAULT);
   if(rc!=0){
    if(this->adb)this->adb->throw_count++;
    if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
    if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
    throw otl_exception(cursor_struct,
                        stm_label?stm_label:stm_text);
   }
  }

  for(i=0;i<vl_len;++i)
   otl_tmpl_cursor
    <otl_exc,
    otl_conn,
    otl_cur,
    otl_var>::bind(*vl[i]);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::exec(1); // Executing the PLSQL master block
  sel_cur.connected=1;
  cur_row=-2;
  if(same_sl_flag && sl){
   // assuming that ref.cur's select list is the same as
   // in previous executions of the master block.
   for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
   return;
  }else{
   sld_tmp_len=0;
   sel_cur.cursor_struct.straight_select=0;
   for(i=1;sel_cur.describe_column(sl_desc_tmp[i-1],i);++i)
    ++sld_tmp_len;
   sl_len=sld_tmp_len;
   if(sl){
    delete[] sl;
    sl=0;
   }
   sl=new otl_generic_variable[sl_len==0?1:sl_len];
   int max_long_size=this->adb->get_max_long_size();
   for(j=0;j<sl_len;++j){
    otl_generic_variable::map_ftype
     (sl_desc_tmp[j],max_long_size,ftype,elem_size,*override,j+1);
    sl[j].copy_pos(j+1);
    sl[j].init(ftype,
               elem_size,
               OTL_SCAST(short,array_size),
               &adb->connect_struct
               );
   }
   if(sl_desc){
    delete[] sl_desc;
    sl_desc=0;
   }
   sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
   memcpy(sl_desc,sl_desc_tmp,sizeof(otl_column_desc)*sl_len);
   for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
  }
 }

 void get_in_next(void)
 {
  if(cur_in==vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

 int check_in_type(int type_code,int tsize)
 {
  if(vl[cur_in]->ftype==otl_var_char&&type_code==otl_var_char)
   return 1;
  if((vl[cur_in]->ftype==otl_var_timestamp ||
      vl[cur_in]->ftype==otl_var_tz_timestamp ||
      vl[cur_in]->ftype==otl_var_ltz_timestamp) && 
     type_code==otl_var_timestamp)
    return 1;
  if(vl[cur_in]->ftype!=type_code||vl[cur_in]->elem_size!=tsize){
   otl_var_info_var
    (vl[cur_in]->name,
     vl[cur_in]->ftype,
     type_code,
     var_info);
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return 0;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return 0; 
#endif
   throw otl_exception
    (otl_error_msg_0,
     otl_error_code_0,
     stm_label?stm_label:stm_text,
     var_info);
  }else
   return 1;
 }

 void check_in_var(void)
 {
  if(vl_len==0){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception
    (otl_error_msg_1,
     otl_error_code_1,
     stm_label?stm_label:stm_text,
     0);
  }
 }

 void check_if_executed(void)
 {
  if(!executed){
   if(this->adb)this->adb->throw_count++;
   if(this->adb&&this->adb->throw_count>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception())return; 
#endif
   throw otl_exception
    (otl_error_msg_2,
     otl_error_code_2,
     stm_label?stm_label:stm_text,
     0);
  }
 }

};

class otl_stream_shell: public otl_stream_shell_generic{
public:

 otl_ref_select_stream* ref_ss;
 otl_select_stream* ss;
 otl_inout_stream* io;
 otl_connect* adb;


 int auto_commit_flag;

 otl_var_desc* iov;
 int iov_len;
 int next_iov_ndx;

 otl_var_desc* ov;
 int ov_len;
 int next_ov_ndx;

 bool flush_flag;

 otl_select_struct_override override;

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 OTL_STRING_CONTAINER orig_sql_stm;
#endif


 otl_stream_shell()
 {
  should_delete=0;
 }

 otl_stream_shell(const int ashould_delete)
 {
  should_delete=0;
  iov=0; iov_len=0;
  ov=0; ov_len=0;
  next_iov_ndx=0;
  next_ov_ndx=0;
  override.len=0;
  ss=0; io=0; ref_ss=0;
  adb=0;
  flush_flag=true;
  should_delete=ashould_delete;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=0; iov_len=0;
   ov=0; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override.len=0;
   flush_flag=true;

   delete ss;
   delete io;
   delete ref_ss;
   ss=0; io=0; ref_ss=0;
   adb=0;
  }
 }

};

class otl_sp_parm_desc{
public:

 int position;
 char arg_name[40];
 char in_out[20];
 char data_type[40];
 char bind_var[128];

 otl_sp_parm_desc()
 {
  position=-1;
  arg_name[0]=0;
  in_out[0]=0;
  data_type[0]=0;
  bind_var[0]=0;
 }

 otl_sp_parm_desc(const otl_sp_parm_desc& r)
 {
  copy(r);
 }

 otl_sp_parm_desc& operator=(const otl_sp_parm_desc& r)
 {
  copy(r);
  return *this;
 }

 ~otl_sp_parm_desc(){}

protected:

 void copy(const otl_sp_parm_desc& r)
 {
  position=r.position;
  strcpy(arg_name,r.arg_name);
  strcpy(in_out,r.in_out);
  strcpy(data_type,r.data_type);
  strcpy(bind_var,r.bind_var);
 }

};

class otl_stream{
public:
  
  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_ref_select_stream** ref_ss;
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;

  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;
  int end_marker;
  int oper_int_called;
  int last_eof_rc;

  otl_select_struct_override* override;

protected:

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

 static void convert_bind_var_datatype
 (char* out_buf,
  const char* datatype,
  const int varchar_size,
  const int all_num2type)
 {
  out_buf[0]=0;
  if(strcmp(datatype,"BINARY_INTEGER")==0||
     strcmp(datatype,"NATIVE INTEGER")==0||
     strcmp(datatype,"FLOAT")==0||
     strcmp(datatype,"NUMBER")==0){
   switch(all_num2type){
   case otl_var_char:
    strcpy(out_buf,"char[50]");
    break;
   case otl_var_double:
    strcpy(out_buf,"double");
    break;
   case otl_var_float:
    strcpy(out_buf,"float");
    break;
   case otl_var_long_int:
    strcpy(out_buf,"long");
    break;
   case otl_var_int:
    strcpy(out_buf,"int");
    break;
   case otl_var_unsigned_int:
    strcpy(out_buf,"unsigned");
    break;
   case otl_var_short:
    strcpy(out_buf,"short");
    break;
   default:
    break;
   }
  }else if(strcmp(datatype,"DATE")==0)
   strcpy(out_buf,"timestamp");
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
else if(strcmp(datatype,"TIMESTAMP")==0)
   strcpy(out_buf,"timestamp");
#endif
  else if(strcmp(datatype,"VARCHAR2")==0)
   sprintf(out_buf,"char[%d]",varchar_size);
  else if(strcmp(datatype,"CHAR")==0)
   sprintf(out_buf,"char[%d]",varchar_size);
 }
 
public:

  operator int(void) OTL_NO_THROW
  {
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

 static void create_stored_proc_call
 (otl_connect& db,
  otl_stream& args_strm,
  char* sql_stm,
  int& stm_type,
  char* refcur_placeholder,
  const char* proc_name,
  const char* package_name,
  const char* schema_name=0,
  const bool schema_name_included=false,
  const int varchar_size=2001,
  const int all_num2type=otl_var_double)
 {
  sql_stm[0]=0;
  stm_type=otl_no_stream_type;
  refcur_placeholder[0]=0;

  char full_name[1024];
  char temp_buf[1024];
  char temp_buf2[1024];
  int i;

  if(package_name==0)
   sprintf(full_name,"%s",proc_name);
  else
   sprintf(full_name,"%s.%s",package_name,proc_name);
  if(schema_name_included&&schema_name!=0){
    sprintf(temp_buf,"%s.%s",schema_name,full_name);
    strcpy(full_name,temp_buf);
  }

  if(!args_strm.good()){
   args_strm.open
    (50,
     "select position, "
     "       lower(':'||nvl(argument_name,'rc__')) argument_name, "
     "       in_out, "
     "       nvl(data_type,'*') data_type, "
     "       ':'||lower(nvl(argument_name,'rc__'))|| "
     "       decode(data_type,'REF CURSOR',' ', "
     "              '<'||'%s,'|| "
     "              decode(in_out,'IN','in', "
     "                     'OUT','out', "
     "                     'IN/OUT','inout', "
     "                     'xxx') "
     "              ||'>' "
     "             ) || decode(position,0,' := ',' ') "
     "       bind_var "
     "from all_arguments "
     "where object_name=upper(:obj_name<char[50]>) "
     "  and (:pkg_name<char[50]> is null and package_name is null or "
     "       :pkg_name is not null and package_name=upper(:pkg_name)) "
     "  and (:owner<char[50]> is null and owner=USER or "
     "       :owner is not null and owner=upper(:owner)) "
     "  and data_level=0 "
     "order by position",
     db);
  }
  
  otl_auto_array_ptr< otl_Tptr<otl_sp_parm_desc> > desc(otl_var_list_size);
  int desc_len=0;
  otl_sp_parm_desc parm;
  
  args_strm<<proc_name;
  if(package_name==0)
   args_strm<<otl_null();
  else
   args_strm<<package_name;
  if(schema_name==0)
   args_strm<<otl_null();
  else
   args_strm<<schema_name;
  while(!args_strm.eof()){
   args_strm>>parm.position;
   args_strm>>parm.arg_name;
   args_strm>>parm.in_out;
   args_strm>>parm.data_type;
   args_strm>>parm.bind_var;
   ++desc_len;
   desc.ptr[desc_len-1].assign(new otl_sp_parm_desc(parm));
  }

  if(desc_len==0){
   sprintf(temp_buf,"procedure %s",full_name);
   throw otl_exception
    (otl_error_msg_13,
     otl_error_code_13,
     temp_buf);
  }

  if(desc_len==1){
   if(desc.ptr[0].ptr->position==1 && desc.ptr[0].ptr->data_type[0]=='*'){
    // procedure without any parameters
    sprintf(sql_stm,"BEGIN %s; END;",full_name);
    stm_type=otl_constant_sql_type;
    return;
   }if(desc.ptr[0].ptr->position==1 && desc.ptr[0].ptr->data_type[0]!='*'){
    // procedure with one parameter
    if(strcmp(desc.ptr[0].ptr->data_type,"REF CURSOR")==0){
     // procedure with one parameter of refcur type
     if(strcmp(desc.ptr[0].ptr->in_out,"IN")==0){
      // refcur parameter should be either OUT or IN OUT, not IN.
      sprintf(temp_buf,"procedure %s",full_name);
      throw otl_exception
       (otl_error_msg_15,
        otl_error_code_15,
        temp_buf,0);
     }
     sprintf(sql_stm,
             "BEGIN %s(%s); END;",
             full_name,
             desc.ptr[0].ptr->bind_var);
     stm_type=otl_refcur_stream_type;
     strcpy(refcur_placeholder,desc.ptr[0].ptr->arg_name);
     return;
    }else{
     // procedure with one scalar parameter
     convert_bind_var_datatype
      (temp_buf,desc.ptr[0].ptr->data_type,
       varchar_size,all_num2type);
     if(temp_buf[0]==0){
      sprintf(temp_buf,"procedure %s, parameter %s",
             full_name,desc.ptr[0].ptr->arg_name);
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,0);
     }
     sprintf(temp_buf2,desc.ptr[0].ptr->bind_var,temp_buf);
     sprintf(sql_stm,"BEGIN %s(%s); END;",full_name,temp_buf2);
     stm_type=otl_inout_stream_type;
     refcur_placeholder[0]=0;
     return;
    }
   }else if(desc.ptr[0].ptr->position==0){
    if(strcmp(desc.ptr[0].ptr->data_type,"REF CURSOR")==0){
     // refcur function without any parameters
     sprintf(sql_stm,
             "BEGIN %s %s; END;",
             desc.ptr[0].ptr->bind_var,
             full_name);
     stm_type=otl_refcur_stream_type;
     strcpy(refcur_placeholder,desc.ptr[0].ptr->arg_name);
     return;
    }else{
     // scalar function without any parameters
     convert_bind_var_datatype
      (temp_buf,desc.ptr[0].ptr->data_type,
       varchar_size,all_num2type);
     if(temp_buf[0]==0){
      sprintf(temp_buf,"procedure %s, parameter %s",
             full_name,desc.ptr[0].ptr->arg_name);
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,0);
     }
     sprintf(temp_buf2,desc.ptr[0].ptr->bind_var,temp_buf);
     sprintf(sql_stm,"BEGIN %s %s; END;",temp_buf2,full_name);
     stm_type=otl_inout_stream_type;
     refcur_placeholder[0]=0;
     return;
    }
   }
  }
  
  // Checking if the procedure is of the "refcur" type
  bool refcur_flag=false;
  bool refcur_outpar=false;
  int refcur_count=0;
  bool inpar_only=true;
  for(i=0;i<desc_len;++i){
   if(inpar_only && 
      strcmp(desc.ptr[i].ptr->in_out,"IN")!=0 &&
      strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")!=0)
    inpar_only=false;
   if(strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")==0){
    ++refcur_count;
    refcur_flag=true;
    refcur_outpar=strcmp(desc.ptr[i].ptr->in_out,"IN")!=0;
   }
  }
  if(refcur_flag){
   if(!inpar_only||refcur_count>1||!refcur_outpar){
    sprintf(temp_buf,"procedure %s",full_name);
    throw otl_exception
     (otl_error_msg_15,
      otl_error_code_15,
      temp_buf,0);
   }
   stm_type=otl_refcur_stream_type;
   refcur_placeholder[0]=0;
   sql_stm[0]=0;
   bool full_name_printed=false;
   for(i=0;i<desc_len;++i){

    if(i==0)strcat(sql_stm,"BEGIN ");
    if(strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")==0)
     strcpy(refcur_placeholder,desc.ptr[i].ptr->arg_name);

    // in case of a function, function's return code
    if(desc.ptr[i].ptr->position==0){
     convert_bind_var_datatype
      (temp_buf,desc.ptr[i].ptr->data_type,
       varchar_size,all_num2type);
     if(temp_buf[0]==0&&
        strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")!=0){
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.ptr[i].ptr->arg_name);
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,0);
     }
     if(strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")==0)
      strcpy(temp_buf2,desc.ptr[i].ptr->bind_var);
     else
      sprintf(temp_buf2,desc.ptr[i].ptr->bind_var,temp_buf);
     strcat(sql_stm,temp_buf2);
    }
    
    // procedure/function's name
    if(!full_name_printed){
     strcat(sql_stm,full_name);
     strcat(sql_stm,"(");
     full_name_printed=true;
    }

    if(desc.ptr[i].ptr->position!=0){
     // normal parameters
     convert_bind_var_datatype
      (temp_buf,desc.ptr[i].ptr->data_type,
       varchar_size,all_num2type);
     if(temp_buf[0]==0&&
        strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")!=0){
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.ptr[i].ptr->arg_name);
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,0);
     }
     if(strcmp(desc.ptr[i].ptr->data_type,"REF CURSOR")==0)
      strcpy(temp_buf2,desc.ptr[i].ptr->bind_var);
     else
      sprintf(temp_buf2,desc.ptr[i].ptr->bind_var,temp_buf);
     strcat(sql_stm,temp_buf2);
    }

    if(i<desc_len-1&&desc.ptr[i].ptr->position!=0)
     strcat(sql_stm,",");
    else if(i==desc_len-1)
     strcat(sql_stm,"); ");

   }
   strcat(sql_stm," END;");
   return;
  }

  // The procedure is of the "general" type
  stm_type=otl_inout_stream_type;
  refcur_placeholder[0]=0;
  sql_stm[0]=0;
  bool full_name_printed=false;
  for(i=0;i<desc_len;++i){
    if(i==0)strcat(sql_stm,"BEGIN ");
    // in case of a function, function's return code
    if(desc.ptr[i].ptr->position==0){
      convert_bind_var_datatype
        (temp_buf,desc.ptr[i].ptr->data_type,
         varchar_size,all_num2type);
      if(temp_buf[0]==0){
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.ptr[i].ptr->arg_name);
        throw otl_exception
          (otl_error_msg_14,
           otl_error_code_14,
           temp_buf,0);
      }
      sprintf(temp_buf2,desc.ptr[i].ptr->bind_var,temp_buf);
      strcat(sql_stm,temp_buf2);
    }
    // procedure/function's name
    if(!full_name_printed){
      strcat(sql_stm,full_name);
      strcat(sql_stm,"(");
      full_name_printed=true;
    }
    if(desc.ptr[i].ptr->position!=0){
      // normal parameters
      convert_bind_var_datatype
        (temp_buf,desc.ptr[i].ptr->data_type,
         varchar_size,all_num2type);
      if(temp_buf[0]==0){
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.ptr[i].ptr->arg_name);
        throw otl_exception
          (otl_error_msg_14,
           otl_error_code_14,
           temp_buf,0);
      }
      sprintf(temp_buf2,desc.ptr[i].ptr->bind_var,temp_buf);
      strcat(sql_stm,temp_buf2);
    }
    if(i<desc_len-1&&desc.ptr[i].ptr->position!=0)
      strcat(sql_stm,",");
    else if(i==desc_len-1)
      strcat(sql_stm,"); ");
  }
  strcat(sql_stm," END;");
  
 }

 int get_stream_type(void) OTL_NO_THROW
 {
  if(shell==0)return otl_no_stream_type;
  if(shell->ss!=0)return otl_select_stream_type;
  if(shell->io!=0)return otl_inout_stream_type;
  if(shell->ref_ss!=0)return otl_refcur_stream_type;
  return otl_no_stream_type;
 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0) OTL_NO_THROW
 {
   if(shell==0){
     init_stream();
     shell->flush_flag=true;
   }
   override->add_override(column_ndx,col_type,col_size);
 }

 
  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    if(shell==0){
      init_stream();
      shell->flush_flag=true;
    }
    override->set_all_column_types(mask);
 }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==0)init_stream();
   shell->flush_flag=flush_flag;
 }

  void set_lob_stream_mode(const bool /*lob_stream_flag*/=false)
    OTL_NO_THROW
  {
  }
 
 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->iov==0)return 0;
  return &(shell->iov[shell->next_iov_ndx]);
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==0)return 0;
  if(shell->ov==0)return 0;
  return &(shell->ov[shell->next_ov_ndx]);
 }
 
 long get_rpc() OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->get_rpc();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->_rfc;
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   return (*ref_ss)->_rfc;
  }else
   return 0;
 }
 
 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  if((*ss)){
   if((*ss)->vl_len>0){
    (*iov)=new otl_var_desc[(*ss)->vl_len];
    (*iov_len)=(*ss)->vl_len;
    for(i=0;i<(*ss)->vl_len;++i)
     (*ss)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*ss)->sl_len>0){
    (*ov)=new otl_var_desc[(*ss)->sl_len];
    (*ov_len)=(*ss)->sl_len;
    for(i=0;i<(*ss)->sl_len;++i){
     (*ss)->sl[i].copy_var_desc((*ov)[i]);
     (*ov)[i].copy_name((*ss)->sl_desc[i].name);
    }
   }
  }else if((*io)){
   if((*io)->vl_len>0){
    (*iov)=new otl_var_desc[(*io)->vl_len];
    (*iov_len)=(*io)->vl_len;
    for(i=0;i<(*io)->vl_len;++i)
     (*io)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*io)->iv_len>0){
    (*ov)=new otl_var_desc[(*io)->iv_len];
    (*ov_len)=(*io)->iv_len;
    for(i=0;i<(*io)->iv_len;++i)
     (*io)->in_vl [i]->copy_var_desc((*ov)[i]);
   }
  }else if((*ref_ss)){
   if((*ref_ss)->vl_len>0){
    (*iov)=new otl_var_desc[(*ref_ss)->vl_len];
    (*iov_len)=(*ref_ss)->vl_len;
    for(i=0;i<(*ref_ss)->vl_len;++i)
     (*ref_ss)->vl[i]->copy_var_desc((*iov)[i]);
   }
   if((*ref_ss)->sl_len>0){
    (*ov)=new otl_var_desc[(*ref_ss)->sl_len];
    (*ov_len)=(*ref_ss)->sl_len;
    for(i=0;i<(*ref_ss)->sl_len;++i){
     (*ref_ss)->sl[i].copy_var_desc((*ov)[i]);
     (*ov)[i].copy_name((*ref_ss)->sl_desc[i].name);
    }
   }
  }
 }

 void init_stream(void)
 {
  shell=0;
  shell=new otl_stream_shell(0);
  shell_pt.assign(&shell);
  connected=0;

  ref_ss=&(shell->ref_ss);
  ss=&(shell->ss);
  io=&(shell->io);
  adb=&(shell->adb);
  auto_commit_flag=&(shell->auto_commit_flag);
  iov=&(shell->iov);
  iov_len=&(shell->iov_len);
  next_iov_ndx=&(shell->next_iov_ndx);
  ov=&(shell->ov);
  ov_len=&(shell->ov_len);
  next_ov_ndx=&(shell->next_ov_ndx);
  override=&(shell->override);
  
  (*ref_ss)=0;
  (*io)=0;
  (*ss)=0;
  (*adb)=0;
  (*ov)=0; 
  (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*auto_commit_flag)=1;
  (*iov)=0; 
  (*iov_len)=0;
 }

 otl_stream
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=0,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  init_stream();

  (*io)=0; (*ss)=0; (*ref_ss)=0;
  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,ref_cur_placeholder,sqlstm_label);
 }
 
 otl_stream() OTL_NO_THROW
 {
  init_stream();
  shell->flush_flag=true;
 }
 
 virtual ~otl_stream() 
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  try{
   if((*io)!=0&&shell->flush_flag==false)
    (*io)->flush_flag2=false;
   close();
   if(shell!=0){
    if((*io)!=0)
     (*io)->flush_flag2=true;
   }
  }catch(otl_exception&){
   if(shell!=0){
   if((*io)!=0)
    (*io)->flush_flag2=true;
   }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=0)
    shell->should_delete=1;
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif
   throw;
  }
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  if(adb && (*adb) && (*adb)->throw_count>0
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  || STD_NAMESPACE_PREFIX uncaught_exception()
#endif
     ){
   //
  }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void) OTL_NO_THROW
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->eof();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->eof();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   return (*ref_ss)->eof();
  }else
   return 1;
 }
 
 void flush(const int rowoff=0,const bool force_flush=false)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush(rowoff,force_flush);
  }
 }
 
 void clean(const int clean_up_error_flag=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->clean(clean_up_error_flag);
  }else if(*ss){
   (*adb)->reset_throw_count();
   (*ss)->clean();
  }else if(*ref_ss){
   (*adb)->reset_throw_count();
   (*ref_ss)->clean();
  }
 }

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->rewind();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   (*ss)->rewind();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   (*ref_ss)->rewind();
  }
 }
 
 int is_null(void) OTL_NO_THROW
 {
  if((*io))
   return (*io)->is_null();
  else if((*ss))
   return (*ss)->is_null();
  else if((*ref_ss))
   return (*ref_ss)->is_null();
  else
   return 0;
 }

 void set_commit(int auto_commit=0) OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }
 
 void open
 (const short arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=0,
  const char* sqlstm_label=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
   if(shell==0)
    init_stream();
#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)
  char temp_buf[128];
  otl_itoa(arr_size,temp_buf);
  OTL_STRING_CONTAINER sql_stm=
    OTL_STRING_CONTAINER(temp_buf)+
    OTL_STRING_CONTAINER("===>")+sqlstm;
  otl_stream_shell_generic* temp_shell=db.sc.find(sql_stm);
  if(temp_shell){
    if(shell!=0)
      shell_pt.destroy();
   shell=OTL_RCAST(otl_stream_shell*,temp_shell);
   ref_ss=&(shell->ref_ss);
   ss=&(shell->ss);
   io=&(shell->io);
   if((*io)!=0)(*io)->flush_flag2=true;
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
   override=&(shell->override);
   if((*iov_len)==0)
    this->rewind();
   connected=1;
   return;
  }
  shell->orig_sql_stm=sql_stm;
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  while(isspace(*c)||(*c)=='(')++c;
  strncpy(tmp,c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==0)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
    if((strncmp(tmp,"SELECT",6)==0||
        strncmp(tmp,"WITH",4)==0)&&
       ref_cur_placeholder==0){
    (*ss)=new otl_select_stream(override,arr_size,
                                sqlstm,db,otl_explicit_select,
                                sqlstm_label);
   }else if(ref_cur_placeholder!=0){
    (*ref_ss)=new otl_ref_select_stream
     (override,arr_size,sqlstm,ref_cur_placeholder,
      db,sqlstm_label);
   }else {
    (*io)=new otl_inout_stream(arr_size,sqlstm,db,false,sqlstm_label);
    (*io)->flush_flag=shell->flush_flag;
   }
  }catch(otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=0; (*iov_len)=0;
  (*ov)=0; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override->len=0;

  if((*ref_ss)){
   try{
    (*ref_ss)->close();
   }catch(otl_exception&){
    delete (*ref_ss);
    (*ref_ss)=0;
    throw;
   }
   delete (*ref_ss);
   (*ref_ss)=0;
  }

  if((*ss)){
   try{
    (*ss)->close();
   }catch(otl_exception&){
    delete (*ss);
    (*ss)=0;
    throw;
   }
   delete (*ss);
   (*ss)=0;
  }
  
  if((*io)){
   try{
    (*io)->flush();
   }catch(otl_exception&){
    clean(1);
    delete (*io);
    (*io)=0;
    throw;
   }
   delete (*io);
   (*io)=0;
  }

  (*ss)=0; (*io)=0; (*ref_ss)=0;
  if(adb!=0)(*adb)=0; 
  adb=0;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
#else
 void close(void)
#endif
 {
  if(shell==0)return;
#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool&&(*adb)&&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(STD_NAMESPACE_PREFIX uncaught_exception())&&
#endif
     (*adb)->throw_count==0){
   try{
    this->flush();
    this->clean(1);
   }catch(otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->throw_count>0){
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(STD_NAMESPACE_PREFIX uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#endif
   (*adb)->sc.add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if((*ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ss)->sl_len;
   return (*ss)->sl_desc;
  }
  if((*ref_ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ref_ss)->sl_len;
   return (*ref_ss)->sl_desc;
  }
  return 0;
 }

 int good(void) OTL_NO_THROW
 {
  if(!connected)return 0;
  if((*io)||(*ss)||(*ref_ss)){
   (*adb)->reset_throw_count();
   return 1;
  }else
   return 0;
 }

 otl_stream& operator>>(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(tab);
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator>>(otl_refcur_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(s);
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(tab);
   inc_next_iov();
  }
  return *this;
 }

#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_stream& operator>>(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(vec);
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(vec);
   inc_next_iov();
  }
  return *this;
 }

#endif

 otl_stream& operator<<(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(s);
   inc_next_iov();
  }
  return *this;
 }

 otl_stream& operator>>(otl_time0& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if(*ref_ss){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
   return *this;
 }

 otl_stream& operator<<(const otl_time0& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss))
   (*ss)->operator<<(n);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(n);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  return *this;
 }

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared 
#else
 otl_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   otl_time0 tmp;
   (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
   s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
   s.month=tmp.month;
   s.day=tmp.day;
   s.hour=tmp.hour-1;
   s.minute=tmp.minute-1;
   s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  inc_next_ov();
  return *this;
 }
#endif

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared
#else
 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time0 tmp;
  reset_end_marker();
  tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
  tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
  tmp.month=OTL_SCAST(unsigned char, s.month);
  tmp.day=OTL_SCAST(unsigned char, s.day);
  tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
  tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
  tmp.second=OTL_SCAST(unsigned char, (s.second+1));
  (*this)<<tmp;
  inc_next_iov();
  return *this;
 }
#endif


 otl_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(s,OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     strcpy(OTL_RCAST(char*,s),
            OTL_RCAST(const char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
           );
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(n);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(n);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(u);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(u);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(u);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(sh);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(sh);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(sh);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(l);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(l);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(l);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(f);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(f);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(f);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(d);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(d);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(d);
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
   d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     (*io)->operator>>(s);
   }else if((*ss)){
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     (*ref_ss)->operator>>(s);
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
   }else if((*ss)){
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
   }else if((*ref_ss)){
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss))
   (*ss)->operator<<(c);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(c);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(c);
  else if((*ss))
   (*ss)->operator<<(c);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(c);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }
#endif

 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(s);
  else if((*ss))
   (*ss)->operator<<(s);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(s);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(n);
  else if((*ss))
   (*ss)->operator<<(n);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(n);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(u);
  else if((*ss))
   (*ss)->operator<<(u);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(u);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(sh);
  else if((*ss))
   (*ss)->operator<<(sh);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(sh);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(l);
  else if((*ss))
   (*ss)->operator<<(l);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(l);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(f);
  else if((*ss))
   (*ss)->operator<<(f);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(f);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))
   (*io)->operator<<(d);
  else if((*ss))
   (*ss)->operator<<(d);
  else if((*ref_ss)){
   (*ref_ss)->operator<<(d);
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

  otl_stream& operator<<(const otl_null& /*n*/)
    OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io))(*io)->operator<<(otl_null());
  if((*ss))(*ss)->operator<<(otl_null());
  if((*ref_ss)){
   (*ref_ss)->operator<<(otl_null());
   if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
  }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(d);
   inc_next_iov();
  }
  return *this;
 }

};

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) \
    && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

template <OTL_TYPE_NAME TData>
otl_refcur_stream& operator>>(otl_refcur_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}


template <OTL_TYPE_NAME TData>
STD_NAMESPACE_PREFIX ostream& operator<<
  (STD_NAMESPACE_PREFIX ostream& s, 
   const otl_value<TData>& var)
{
 if(var.ind)
  s<<"NULL";
 else
  s<<var.v;
 return s;
}

inline STD_NAMESPACE_PREFIX ostream& operator<<
  (STD_NAMESPACE_PREFIX ostream& s, 
   const otl_value<otl_datetime>& var)
{
 if(var.ind)
  s<<"NULL";
 else{
  s<<var.v.month<<"/"<<var.v.day<<"/"<<var.v.year<<" "
   <<var.v.hour<<":"<<var.v.minute<<":"<<var.v.second;
 }
 return s;
}

#endif

typedef otl_tmpl_nocommit_stream
<otl_stream,
 otl_connect,
 otl_exception> otl_nocommit_stream;

OTL_ORA8_NAMESPACE_END
#ifndef __STDC__DEFINED
#undef __STDC__
#endif
#endif

#if defined(OTL_STL) && !defined(OTL_STLPORT)

#define STL_INPUT_ITERATOR_TO_DERIVE_FROM 
#define STL_OUTPUT_ITERATOR_TO_DERIVE_FROM 

#elif defined(OTL_STLPORT)

#define STL_INPUT_ITERATOR_TO_DERIVE_FROM       \
  : public STD_NAMESPACE_PREFIX iterator        \
    <STD_NAMESPACE_PREFIX input_iterator_tag,   \
     T,Distance,T*,T&>

#define STL_OUTPUT_ITERATOR_TO_DERIVE_FROM              \
  : public STD_NAMESPACE_PREFIX iterator                \
         <STD_NAMESPACE_PREFIX output_iterator_tag,     \
           T,void,void,void>

#endif

#if defined(OTL_STL) || defined(OTL_STLPORT)

#define OTL_ITERATORS                                                   \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance=ptrdiff_t>            \
class otl_input_iterator STL_INPUT_ITERATOR_TO_DERIVE_FROM {            \
public:                                                                 \
                                                                        \
  typedef STD_NAMESPACE_PREFIX input_iterator_tag iterator_category;    \
  typedef T                  value_type;                                \
  typedef Distance           difference_type;                           \
  typedef const T*           pointer;                                   \
  typedef const T&           reference;                                 \
                                                                        \
 otl_stream* stream;                                                    \
 T value;                                                               \
 int end_marker;                                                        \
                                                                        \
 void read()                                                            \
 {                                                                      \
  if(!stream){                                                          \
   end_marker=-1;                                                       \
   return;                                                              \
  }                                                                     \
  if(stream->eof()){                                                    \
   end_marker=-1;                                                       \
   return;                                                              \
  }                                                                     \
  end_marker=stream->eof();                                             \
  if(!end_marker)*stream>>value;                                        \
  if(stream->eof())end_marker=1;                                        \
 }                                                                      \
                                                                        \
 otl_input_iterator() : stream(0), end_marker(-1){}                     \
 otl_input_iterator(otl_stream& s) : stream(&s), end_marker(0){read();} \
                                                                        \
 const T& operator*() const { return value; }                           \
                                                                        \
 otl_input_iterator<T, Distance>& operator++(){read(); return *this;}   \
                                                                        \
 otl_input_iterator<T, Distance> operator++(int)                        \
 {                                                                      \
  otl_input_iterator<T, Distance> tmp = *this;                          \
  read();                                                               \
  return tmp;                                                           \
 }                                                                      \
                                                                        \
};                                                                      \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline STD_NAMESPACE_PREFIX input_iterator_tag iterator_category(       \
  const otl_input_iterator<T, Distance>&                                \
)                                                                       \
{                                                                       \
  return STD_NAMESPACE_PREFIX input_iterator_tag();                     \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline T* value_type(const otl_input_iterator<T, Distance>&)            \
{                                                                       \
 return 0;                                                              \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline Distance* distance_type(const otl_input_iterator<T, Distance>&)  \
{                                                                       \
 return 0;                                                              \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
bool operator==(const otl_input_iterator<T, Distance>& x,               \
                const otl_input_iterator<T, Distance>& y)               \
{                                                                       \
 return x.stream == y.stream && x.end_marker == y.end_marker ||         \
        x.end_marker == -1 && y.end_marker == -1;                       \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
bool operator!=(const otl_input_iterator<T, Distance>& x,               \
                const otl_input_iterator<T, Distance>& y)               \
{                                                                       \
 return !(x==y);                                                        \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T>                                              \
class otl_output_iterator STL_OUTPUT_ITERATOR_TO_DERIVE_FROM {          \
protected:                                                              \
 otl_stream* stream;                                                    \
public:                                                                 \
                                                                        \
  typedef STD_NAMESPACE_PREFIX output_iterator_tag iterator_category;   \
  typedef void       value_type;                                        \
  typedef void       difference_type;                                   \
  typedef void       pointer;                                           \
  typedef void       reference;                                         \
                                                                        \
 otl_output_iterator(otl_stream& s) : stream(&s){}                      \
 otl_output_iterator<T>& operator=(const T& value)                      \
 {                                                                      \
  *stream << value;                                                     \
  return *this;                                                         \
 }                                                                      \
 otl_output_iterator<T>& operator*() { return *this; }                  \
 otl_output_iterator<T>& operator++() { return *this; }                 \
 otl_output_iterator<T>& operator++(int) { return *this; }              \
                                                                        \
};                                                                      \
                                                                        \
template <OTL_TYPE_NAME T>                                              \
inline STD_NAMESPACE_PREFIX output_iterator_tag                         \
iterator_category(const otl_output_iterator<T>&) {                      \
  return STD_NAMESPACE_PREFIX output_iterator_tag();                    \
}

#ifdef OTL_ORA7
OTL_ORA7_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ORA7_NAMESPACE_END
#endif

#ifdef OTL_ORA8
OTL_ORA8_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ORA8_NAMESPACE_END
#endif

#ifdef OTL_ODBC
OTL_ODBC_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ODBC_NAMESPACE_END
#endif

#ifdef OTL_IBASE
OTL_IBASE_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_IBASE_NAMESPACE_END
#endif

#endif

#if defined(OTL_ORA_TEXT_ON)&&defined(text)
#undef text
#endif

#endif
