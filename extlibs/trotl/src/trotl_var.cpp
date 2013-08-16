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
#include "trotl_var.h"
#include "trotl_stat.h"
#include "trotl_describe.h"

namespace trotl
{

// ColumnType::ColumnType(SqlStatement &stat, dvoid* handle)
// 	: _data_type(0)
// 	, _width(0)
// 	, _char_semantics(0)
// 	, _scale(-127)
// 	, _precision(0)
// 	, _nullable(false)
// 	, _utf16(false)
// 	, _typecode(0)
// 	, _collection_typecode(0)
// 	, _collection_dschp(NULL)
// 	, _tdo(NULL)
// 	, _oref(NULL)
// {
// 	describe(stat, handle);
// }

// void ColumnType::describe(SqlStatement &stat, dvoid* handle)
// {
// 	// get column name
// 	text* pcol_name = NULL;
// 	text* ptype_name = NULL;
// 	text* pschema_name = NULL;
// 	ub4 size;
// 	ub1 cform;
// 	sword res;

// 	//OCIDescribe *dschp = (OCIDescribe *) 0;
// 	OciDescribe dschp(stat._env);
// 	OCIParam *parmp;
// 	ub1 fsprecision, lfprecision;

// 	size = sizeof(pcol_name);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &pcol_name, &size, OCI_ATTR_NAME, stat._errh));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);
// 	_column_name.assign((const char*)pcol_name, size);

// 	// TODO use this attribute
// 	size = sizeof(fsprecision);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&fsprecision, &size, OCI_ATTR_FSPRECISION, stat._errh));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);

// 	// TODO use this attribute
// 	size = sizeof(lfprecision);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&lfprecision, &size, OCI_ATTR_LFPRECISION, stat._errh));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);

// 	// get column data type
// 	size = sizeof(_data_type);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_data_type, &size, OCI_ATTR_DATA_TYPE, stat._errh));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);

// 	// get NULL-ability flag
// 	size = sizeof(_nullable);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_nullable, &size, OCI_ATTR_IS_NULL, stat._errh));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);

// #if ORACLE_MAJOR_VERSION>=9
// 	// Oracle 9 ->
// 	// retrieve the length semantics for the column
// 	size = sizeof(_char_semantics);
// 	res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_char_semantics, &size, OCI_ATTR_CHAR_USED, stat._errh));
// 	if (res != OCI_SUCCESS)
// 		_char_semantics = 0;
// 	// <- Oracle 9

// 	size = sizeof(_width);
// 	if (_char_semantics)
// 	{
// 		// get the column width in characters
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_width, &size, OCI_ATTR_CHAR_SIZE, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 	}
// 	else
// #endif
// 	{
// 		// get the column width in bytes
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_width, &size, OCI_ATTR_DATA_SIZE, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 	}

// 	switch(_data_type)
// 	{
// 	default: //case OCI_TYPECODE_VARCHAR:
// 		_precision = 0;
// 		_scale = -127;
// 		break;

// 	case SQLT_NUM:
// 		size = sizeof(_precision);
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_precision, &size, OCI_ATTR_PRECISION, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		size = sizeof(_scale);
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &_scale, &size, OCI_ATTR_SCALE, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		break;
// 	case SQLT_DAT:
// #if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
// 	case SQLT_TIME:
// 	case SQLT_TIME_TZ:
// 	case SQLT_TIMESTAMP: // TODO TIMESTAMP(%i)
// 	case SQLT_TIMESTAMP_TZ:
// 	case SQLT_TIMESTAMP_LTZ:
// #endif
// 		_precision = 0;
// 		_scale = -127;

// 		break;
// 	case SQLT_NTY:
// 	{
// 		OCIType *tdo = 0;
// 		OCIRef *oref = 0;
// 		ub4 ssize, nsize;

// 		size = sizeof(oref);
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&oref, &size, OCI_ATTR_REF_TDO, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		_oref = oref;

// 		res = OCICALL(OCITypeByRef(stat._env, stat._errh, oref, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &tdo));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		_tdo = tdo;

// 		// size = sizeof(tdo2);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&tdo2, &size, OCI_ATTR_TDO, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(_typecode);
// 		// res = OCICALL(OCIAttrGet(parmp, OCI_DTYPE_PARAM, (dvoid*)&_typecode, &size, OCI_ATTR_TYPECODE, stat._errh));
// 		// //oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		// checkerr(stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_IS_INCOMPLETE_TYPE, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_IS_SYSTEM_TYPE, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_IS_PREDEFINED_TYPE, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_IS_TRANSIENT_TYPE, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_IS_SYSTEM_GENERATED_TYPE, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(boolean);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &boolean, &size, OCI_ATTR_HAS_NESTED_TABLE, stat._errh));
// 		//oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(obj_id);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&obj_name, &size, OCI_ATTR_OBJ_ID, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(obj_name);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&obj_name, &size, OCI_ATTR_OBJ_NAME, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		// size = sizeof(obj_schema);
// 		// res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, (dvoid*)&obj_name, &size, OCI_ATTR_OBJ_SCHEMA, stat._errh));
// 		// oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		ssize = sizeof(pschema_name);
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &pschema_name, &ssize, OCI_ATTR_SCHEMA_NAME, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		nsize = sizeof(ptype_name);
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &ptype_name, &nsize, OCI_ATTR_TYPE_NAME, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		_reg_name = _data_type_name = tstring((char*)pschema_name, ssize) + "." + tstring((char*)ptype_name,nsize);

// 		//res = OCICALL(OCIHandleAlloc((dvoid *) stat._env, (dvoid **) &dschp, (ub4) OCI_HTYPE_DESCRIBE, (size_t) 0, (dvoid **) 0));
// 		//oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		res = OCICALL(OCIDescribeAny(stat._conn._svc_ctx, stat._errh,
// 		                             (dvoid *) oref, 0, OCI_OTYPE_REF, (ub1)OCI_DEFAULT, (ub1) OCI_PTYPE_TYPE, dschp));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		res = OCICALL(OCIAttrGet((dvoid *) dschp, (ub4) OCI_HTYPE_DESCRIBE,	(dvoid *)&parmp, (ub4 *)0, (ub4)OCI_ATTR_PARAM, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		size = sizeof(_typecode);
// 		res = OCICALL(OCIAttrGet(parmp, OCI_DTYPE_PARAM, (dvoid*)&_typecode, &size, OCI_ATTR_TYPECODE, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);

// 		if (_typecode == OCI_TYPECODE_NAMEDCOLLECTION)
// 		{
// 			ub4 ssize, tsize;
// 			ub2 len;
// 			text* coll_type_name = NULL;
// 			text* coll_schema_name = NULL;
// 			res = OCICALL(OCIAttrGet((dvoid *) parmp, (ub4) OCI_DTYPE_PARAM,
// 			                         (dvoid *)&_collection_typecode, (ub4 *)0, (ub4)OCI_ATTR_COLLECTION_TYPECODE, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			res = OCICALL(OCIAttrGet((dvoid *) parmp, (ub4) OCI_DTYPE_PARAM,
// 			                         (dvoid *)&_collection_dschp, (ub4 *)0,	(ub4)OCI_ATTR_COLLECTION_ELEMENT, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			/* get the data size */
// 			res = OCICALL(OCIAttrGet((dvoid*) _collection_dschp, (ub4) OCI_DTYPE_PARAM, (dvoid*) &len, (ub4 *) 0, (ub4) OCI_ATTR_DATA_SIZE, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			/* get the name of the collection */
// 			res = OCICALL(OCIAttrGet((dvoid*) _collection_dschp, (ub4) OCI_DTYPE_PARAM, (dvoid*) &coll_type_name, (ub4 *) &tsize, (ub4) OCI_ATTR_TYPE_NAME, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			/* get the name of the schema */
// 			res = OCICALL(OCIAttrGet((dvoid*) _collection_dschp, (ub4) OCI_DTYPE_PARAM, (dvoid*) &coll_schema_name, (ub4 *) &ssize, (ub4) OCI_ATTR_SCHEMA_NAME, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			/* get the data type */
// 			res = OCICALL(OCIAttrGet((dvoid*) _collection_dschp, (ub4) OCI_DTYPE_PARAM, (dvoid*) &_collection_typecode, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE, stat._errh));
// 			oci_check_error(__TROTL_HERE__, stat._errh, res);

// 			ub4 num_elems = 0;
// 			if (_collection_typecode == OCI_TYPECODE_VARRAY)
// 			{
// 				/* get the number of elements */
// 				res = OCICALL(OCIAttrGet((dvoid*) _collection_dschp, (ub4) OCI_DTYPE_PARAM, (dvoid*) &num_elems, (ub4 *) 0,
// 				                         (ub4) OCI_ATTR_NUM_ELEMS, stat._errh));
// 				oci_check_error(__TROTL_HERE__, stat._errh, res);
// 				_reg_name = tstring("VARARRAY OF ") + tstring((char*)coll_type_name ,tsize);
// 			}
// 			else
// 			{
// 				_reg_name = tstring("TABLE OF ") + tstring((char*)coll_type_name ,tsize);
// 			}
// 			// std::cout << "OCI_ATTR_COLLECTION_TYPECODE: " << _collection_typecode << std::endl
// 			// 	  << "OCI_ATTR_COLLECTION_ELEMENT: " << _collection_dschp << std::endl
// 			// 	  << "OCI_ATTR_DATA_SIZE: " << len << std::endl
// 			// 	  << "OCI_ATTR_TYPE_NAME: " << coll_type_name << std::endl
// 			// 	  << "OCI_ATTR_SCHEMA_NAME: " << coll_schema_name << std::endl
// 			// 	  << "OCI_ATTR_DATA_TYPE: " << _collection_typecode << std::endl
// 			// 	  << "OCI_ATTR_NUM_ELEMS: " << num_elems << std::endl
// 			// 	  << "========================================" << std::endl;
// 			//_data_type = (OCI_TYPECODE_VARRAY << 8) + _collection_typecode;
// 		}
// 		break;
// 	}
// 	case SQLT_LNG:
// 		_width = g_OCIPL_MAX_LONG; //TODO long can have up to 2GB, use callback define
// 		break;
// 	case SQLT_CHR:
// 	case SQLT_STR:
// 	case SQLT_VCS:
// 	case SQLT_CLOB:
// 		res = OCICALL(OCIAttrGet(handle, OCI_DTYPE_PARAM, &cform, 0, OCI_ATTR_CHARSET_FORM, stat._errh));
// 		oci_check_error(__TROTL_HERE__, stat._errh, res);
// 		_utf16 = (cform == SQLCS_NCHAR);
// 		break;
// 	}
// }

// tstring ColumnType::get_type_str(bool show_null) const
// {
// 	tostream str;

// 	switch(_data_type)
// 	{
// 	case SQLT_AFC: // OCI_TYPECODE_CHAR:
// 		str << (_utf16 ? "N" : "") << "CHAR(" << _width << ")";
// 		break;
// 	case SQLT_CHR: // OCI_TYPECODE_VARCHAR:
// 		str << (_utf16 ? "N" : "") << "VARCHAR(" << _width << ")";
// 		break;
// 	case SQLT_VCS: // OCI_TYPECODE_VARCHAR2:
// 		str << (_utf16 ? "N" : "") << "VARCHAR2(" << _width << ")";
// 		break;
// 	case SQLT_BIN:
// 		str << "RAW("<< _width << ")";
// 		break;
// 	case SQLT_NUM: // OCI_TYPECODE_NUMBER:
// 		// When precision is 0, NUMBER(precision, scale) can be represented simply as NUMBER.
// 		// If the precision is nonzero and scale is -127, then it is a FLOAT;
// 		// otherwise, it is a NUMBER(precision, scale).
// 		if (_precision == 0)
// 			str << "NUMBER";
// 		else if (_scale == -127)
// 		{
// 			if (_precision == 126)
// 				str << "FLOAT";
// 			else
// 				str << "FLOAT(" << (int)_precision << ")";
// 		}
// 		else if (_scale)
// 			str << "NUMBER(" << (int)_precision << "," << (int)_scale << ")";
// 		else
// 			str << "NUMBER(" << (int)_precision << ")";

// 		break;
// 	case SQLT_DAT: // OCI_TYPECODE_DATE:
// 		str << "DATE";
// 		break;
// 	case SQLT_BLOB: // OCI_TYPECODE_BLOB:
// 		str << "BLOB";
// 		break;
// 	case SQLT_BFILE: // OCI_TYPECODE_BFILE:
// 		str << "BFILE";
// 		break;
// 	case SQLT_CLOB: // OCI_TYPECODE_CLOB:
// 		str << (_utf16 ? "N" : "") << "CLOB";
// 		break;
// 	case SQLT_CFILE: // OCI_TYPECODE_CFILE:
// 		str << "CFILE";
// 		break;
// 	case SQLT_LNG:
// 		str << "LONG";
// 		break;
// 	case SQLT_NTY:
// 		str << _data_type_name;
// 		break;
// 	case SQLT_LBI:
// 		str << "LONG RAW"; /* !! TODO unsupported yet */
// 		break;
// 	case SQLT_RDD:
// 	case 208: // 208 = 2 * SQLT_RDD (mentioned only in catalog.sql)
// 		// _width == 4 for ROWID datatype
// 		// _width == 4000 for UROWID datatype
// 		if(_width == sizeof(OCIRowid*))
// 			str << "ROWID";
// 		else
// 			str << "UROWID";
// 		break;
// 	case SQLT_IBFLOAT:
// 		str << "BINARY_FLOAT";
// 		break;
// 	case SQLT_IBDOUBLE:
// 		str << "BINARY_DOUBLE";
// 		break;
// 	case SQLT_TIMESTAMP:
// 		str << "TIMESTAMP";
// 		break;
// 	case SQLT_TIMESTAMP_TZ:
// 		str << "TIMESTAMP WITH TIMEZONE";
// 		break;
// 	case SQLT_INTERVAL_YM:
// 		str << "INTERVAL YEAR TO MONTH";
// 		break;
// 	case SQLT_INTERVAL_DS:
// 		str << "INTERVAL DAY TO SECOND";
// 		break;
// 	case SQLT_TIMESTAMP_LTZ:
// 		str << "TIMESTAMP WITH LOCAL TIME ZONE";
// 		break;

// 	default:
// 		str << "unknown datatype:" << _data_type;
// 		break;
// 	}

// 	if (show_null)
// 	{
// 		if (!_nullable)
// 			str << " NOT NULL";
// 		else
// 			str << " NULL";
// 	}
// 	return str.str();
// }

/* Placeholder for Define operations */
BindPar::BindPar(unsigned int pos, SqlStatement &stmt, DescribeColumn *ct)
	: valuep(NULL)
	, indp(NULL)
	, rlenp(NULL)
	// , rcodep(NULL)
	, alenp(NULL)
	, mode(OCI_DEFAULT)
	, bindp(NULL)
	, defnpp(NULL)
	, _env(stmt._env)
	, _stmt(stmt)
	, _pos(pos)
	, _max_cnt(g_OCIPL_BULK_ROWS)
	, _cnt(g_OCIPL_BULK_ROWS)
	, _bound(false)
	, _type_name("")
	, _reg_name("")
	, _bind_name("")
	, _bind_typename("")
{
	indp = (OCIInd*) calloc(_cnt, sizeof(OCIInd));
	rlenp = calloc(_cnt, sizeof(ub2)); // OciDefineByPos uses ub2* for lenp

	_bind_type = DEFINE_SELECT;
};

/* Placeholder for Bind operations */
BindPar::BindPar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl)
	: valuep(NULL)
	, indp(NULL)
	, rlenp(NULL)
	// , rcodep(NULL)
	, alenp(NULL)
	, mode(OCI_DEFAULT)
	, bindp(NULL)
	, defnpp(NULL)
	, _env(stmt._env)
	, _stmt(stmt)
	, _pos(pos)
	, _max_cnt(decl.bracket[1])
	, _cnt(decl.bracket[1])
	, _bound(false)
	, _type_name("")
	, _reg_name("")
	, _bind_name(decl.bindname)
	, _bind_typename(decl.bindtype)
{
	indp = (OCIInd*) calloc(_cnt, sizeof(OCIInd));
	rlenp = calloc(_cnt, sizeof(ub4)); // OCIBindByPos uses ub4* for lenp

	if(decl.inout == "in")
		_bind_type = BIND_IN;
	else if(decl.inout == "inout")
		_bind_type = BIND_INOUT;
	else if(decl.inout == "out")
		_bind_type = BIND_OUT;
};

};


// TODO This code was pasted from tora/src/tooracleconnection.cpp

// 			for (int i = 0;i < descriptionLen;i++)
// 			{
// 				toQuery::queryDescribe desc;
// 				desc.AlignRight = false;
// 				desc.Name = QString::fromUtf8(description[i].name);
// 				/*
// 				 * http://www.stanford.edu/dept/itss/docs/oracle/10g/server.101/b10758/sqlqr06.htm
// 				 * include ocidfn.h
// 				 */
// 				switch (description[i].dbtype)
// 				{
// 				case 1: /* VARCHAR2, NVARCHAR2 SQLT_CHR */
// 					desc.Datatype = QString::fromLatin1("VARCHAR2(%i)");
// 					datatypearg1 = description[i].char_size;
// #ifdef OTL_ORA_UNICODE
// 					if (description[i].charset_form == 2)
// 					{
// 						desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
// 					}
// #endif
// 					break;

// 				case 2: /* NUMBER SQLT_NUM */
// 					desc.Datatype = QString::fromLatin1("NUMBER");

// 					if (description[i].prec)
// 					{
// 						desc.Datatype = QString::fromLatin1("NUMBER(%i)");
// 						datatypearg1 = description[i].prec;
// 						if (description[i].scale != 0)
// 						{
// 							desc.Datatype = QString::fromLatin1("NUMBER(%i,%i)");
// 							datatypearg2 = description[i].scale;
// 						}
// 					}
// 					desc.AlignRight = true;
// 					break;

// 				case 8: /* LONG SQLT_LNG */
// 					desc.Datatype = QString::fromLatin1("LONG");
// 					break;

// 				case 12: /* DATE SQLT_DAT */
// 					desc.Datatype = QString::fromLatin1("DATE");
// 					desc.AlignRight = true;
// 					break;

// 				case 23: /* RAW SQL_BIN */
// 					desc.Datatype = QString::fromLatin1("RAW(%i)");
// 					datatypearg1 = description[i].dbsize;
// 					break;

// 				case 24: /* LONG RAW SQLT_LBI */
// 					desc.Datatype = QString::fromLatin1("LONG RAW");
// 					break;

// 				case 104: /* ROWID, docu: 69, ocidfn.h: 104  SQLT_RDD */
// 				case 208: /* UROWID */
// 					desc.Datatype = QString::fromLatin1("ROWID");
// 					break;

// 				case 96: /* CHAR, NCHAR SQLT_AFC */
// 					desc.Datatype = QString::fromLatin1("CHAR(%i)");
// 					datatypearg1 = description[i].char_size;
// #ifdef OTL_ORA_UNICODE
// 					if (description[i].charset_form == 2)
// 					{
// 						desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
// 					}
// #endif
// 					break;
// #ifdef OTL_ORA_NATIVE_TYPES
// 				case 100: /* BINARY_FLOAT SQLT_IBFLOAT */
// 					desc.Datatype = QString::fromLatin1("BINARY_FLOAT");
// 					break;

// 				case 101: /* BINARY_DOUBLE SQLT_IBDOUBLE */
// 					desc.Datatype = QString::fromLatin1("BINARY_DOUBLE");
// 					break;
// #endif
// 				case 112: /* CLOB, NCLOB SQLT_CLOB */
// 					desc.Datatype = QString::fromLatin1("CLOB");
// #ifdef OTL_ORA_UNICODE
// 					if (description[i].charset_form == 2)
// 					{
// 						desc.Datatype = QString::fromLatin1("N") + desc.Datatype;
// 					}
// #endif
// 					break;

// 				case 113: /* BLOB SQLT_BLOB */
// 					desc.Datatype = QString::fromLatin1("BLOB");
// 					break;
// #ifdef OTL_ORA_TIMESTAMP
// 				case 187: /* TIMESTAMP, docu: 180, ocidfn.h: 187 SQLT_TIMESTAMP SQLT_TIMESTAMP */
// 					desc.Datatype = QString::fromLatin1("TIMESTAMP(%i)");
// 					datatypearg1 = description[i].scale;
// 					break;

// 				case 188: /* TIMESTAMP WITH TIME ZONE, docu: 181, ocidfn.h: 188 SQLT_INTERVAL_TZ */
// 					desc.Datatype = QString::fromLatin1("TIMESTAMP(%i) WITH TIME ZONE");
// 					datatypearg1 = description[i].scale;
// 					break;

// 				case 189: /* INTERVAL YEAR TO MONTH, docu: 182, ocidfn.h: 189 SQLT_INTERVAL_YM */
// 					desc.Datatype = QString::fromLatin1("INTERVAL YEAR(%i) TO MONTH");
// 					datatypearg1 = description[i].prec;
// 					break;

// 				case 190: /* INTERVAL DAY TO SECOND, docu: 183, ocidfn.h: 190  SQLT_INTERVAL_DS */
// 					desc.Datatype = QString::fromLatin1("INTERVAL DAY(%i) TO SECOND(%i)");
// 					datatypearg1 = description[i].prec;
// 					datatypearg2 = description[i].scale;
// 					break;

// 				case 232: /* TIMESTAMP WITH LOCAL TIME ZONE docu: 231, ocidfn.h: 232 SQLT_TIMESTAMP_LTZ */
// 					desc.Datatype = QString::fromLatin1("TIMESTAMP(%i) WITH LOCAL TIME ZONE");
// 					datatypearg1 = description[i].scale;
// 					break;
// #endif
// 				default:
// 					desc.Datatype = QString::fromLatin1("UNKNOWN");

// 					/* report unmatched datatypes for adding later */
// 					printf("File a bug report and include the table layout and the following data:\n");
// 					printf("  type=%i, otl_type=%i, size=%i, nullok=%i, charset_form=%i, charset_size=%i, prec=%i, scale=%i\n",
// 					       description[i].dbtype,
// 					       description[i].otl_var_dbtype,
// 					       description[i].dbsize,
// 					       description[i].nullok,
// 					       description[i].charset_form,
// 					       description[i].char_size,
// 					       description[i].prec,
// 					       description[i].scale);
//                 }
//                 desc.Datatype.sprintf(desc.Datatype.toUtf8().constData(), datatypearg1, datatypearg2);

//                 desc.Null = description[i].nullok;

//                 ret.insert(ret.end(), desc);
// 			}


