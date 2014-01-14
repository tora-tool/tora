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



#include "trotl_describe.h"
#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_error.h"

#include <vector>

namespace trotl
{

Describe* Describe::createDescription(OciConnection &con, const tstring &objectName)
{
	sword res;
	bool isPublic = false;
	OciDescribe desc(con._env);

	try
	{
		res = OCICALL(OCIDescribeAny(con._svc_ctx, con._env._errh,
		                             (dvoid*)objectName.c_str(),  // objptr (IN) (string containing the name of the object
		                             (ub4)objectName.size(),  // objnm_len (IN)
		                             OCI_OTYPE_NAME,   // objptr_typ (IN) points to the name of a schema object
		                             (ub1)OCI_DEFAULT, // info_level (IN) for future extensions. Pass OCI_DEFAULT.
		                             OCI_PTYPE_UNK,    // OCI_PTYPE_TABLE,  //
		                             desc
		                            ));
		oci_check_error(__TROTL_HERE__, con._env._errh, res);
	}
	catch(OciException const &e)
	{
		if(e.get_code() == 4043)
		{
			sb4 val = -1; // ub2 val = 1; /* size of OCI_ATTR_DESC_PUBLIC is undocumented. */
			desc.set_attribute(OCI_ATTR_DESC_PUBLIC, &val);
			isPublic = true;

			res = OCICALL(OCIDescribeAny(con._svc_ctx, con._env._errh,
			                             (dvoid*)objectName.c_str(),  // objptr (IN) (string containing the name of the object
			                             (ub4)objectName.size(),  // objnm_len (IN)
			                             OCI_OTYPE_NAME,   // objptr_typ (IN) points to the name of a schema object
			                             (ub1)OCI_DEFAULT, // info_level (IN) for future extensions. Pass OCI_DEFAULT.
			                             OCI_PTYPE_UNK,    // OCI_PTYPE_TABLE,  //
			                             desc
			                            ));
			oci_check_error(__TROTL_HERE__, con._env._errh, res);
		}
		else
		{
			throw e;
		}
	};

	OciParam param = desc.get_param();
	ub1 obj_type = param.get_attribute<ub1>(OCI_ATTR_PTYPE);
	switch(obj_type)
	{
	case OCI_PTYPE_SYN:
		return new DescribeSynonym(con, desc, objectName, isPublic);
	case OCI_PTYPE_VIEW:
		return new DescribeView(con, desc, objectName);
	case OCI_PTYPE_TABLE:
		return new DescribeTable(con, desc, objectName);
	case OCI_PTYPE_TYPE:
		return new DescribeType(con, desc, objectName);
	case OCI_PTYPE_SEQ:
		return new DescribeSequence(con, desc, objectName);
	case OCI_PTYPE_PROC:
	case OCI_PTYPE_FUNC:
	case OCI_PTYPE_PKG:
	case OCI_PTYPE_TYPE_ATTR:
	case OCI_PTYPE_TYPE_COLL:
	case OCI_PTYPE_TYPE_METHOD:
	case OCI_PTYPE_COL: // - column of a table or view
	case OCI_PTYPE_ARG: // - argument of a function or procedure
	case OCI_PTYPE_TYPE_ARG: // - argument of a type method
	case OCI_PTYPE_TYPE_RESULT: // - results of a method
	case OCI_PTYPE_LIST: // - column list for tables and views, argument list for functions and procedures, or subprogram list for packages
	case OCI_PTYPE_SCHEMA: // - schema
	case OCI_PTYPE_DATABASE: // - database
	case OCI_PTYPE_UNK: // - unknown schema object
		throw_oci_exception(OciException(__TROTL_HERE__, "Error describing object: %d\n").arg(obj_type));
		//no break here
	default:
		throw_oci_exception(OciException(__TROTL_HERE__, "Error describing object: %d\n").arg(obj_type));
		//no break here
	}
};

Describe::objectType TROTL_EXPORT Describe::getObjectType(OciConnection &con, const tstring &objectName)
{
	sword res;
	OciDescribe desc(con._env);

	try
	{
		res = OCICALL(OCIDescribeAny(con._svc_ctx, con._env._errh,
		                             (dvoid*)objectName.c_str(),  // objptr (IN) (string containing the name of the object, pointer to a REF to the TDO, pointer to a TDO
		                             (ub4)objectName.size(),  // objnm_len (IN)
		                             OCI_OTYPE_NAME,   // objptr_typ (IN) points to the name of a schema object
		                             (ub1)OCI_DEFAULT, // info_level (IN) for future extensions. Pass OCI_DEFAULT.
		                             OCI_PTYPE_UNK,    // OCI_PTYPE_TABLE,  //
		                             desc
		                            ));
		oci_check_error(__TROTL_HERE__, con._env._errh, res);
	}
	catch(OciException const &e)
	{
		if(e.get_code() == 4043)
		{
			sb4 val = -1; // ub2 val = 1; /* size of OCI_ATTR_DESC_PUBLIC is undocumented. */
			desc.set_attribute(OCI_ATTR_DESC_PUBLIC, &val);

			res = OCICALL(OCIDescribeAny(con._svc_ctx, con._env._errh,
			                             (dvoid*)objectName.c_str(),  // objptr (IN) (string containing the name of the object, pointer to a REF to the TDO, pointer to a TDO
			                             (ub4)objectName.size(),  // objnm_len (IN)
			                             OCI_OTYPE_NAME,   // objptr_typ (IN) points to the name of a schema object
			                             (ub1)OCI_DEFAULT, // info_level (IN) for future extensions. Pass OCI_DEFAULT.
			                             OCI_PTYPE_UNK,    // OCI_PTYPE_TABLE,  //
			                             desc
			                            ));
			oci_check_error(__TROTL_HERE__, con._env._errh, res);
		}
		else
		{
			throw e;
		}
	}

	OciParam param = desc.get_param();
	ub1 obj_type = param.get_attribute<ub1>(OCI_ATTR_PTYPE);
	switch(obj_type)
	{
	case OCI_PTYPE_SYN:
		return O_SYNONYM;
	case OCI_PTYPE_TABLE:
		return O_TABLE;
	case OCI_PTYPE_VIEW:
		return O_VIEW;
	case OCI_PTYPE_PROC:
		return O_PROCEDURE;
	case OCI_PTYPE_FUNC:
		return O_FUNCTION;
	case OCI_PTYPE_PKG:
		return O_PACKAGE;
	case OCI_PTYPE_TYPE:
		return O_TYPE;
	case OCI_PTYPE_SEQ:
		return O_SEQUENCE;
	case OCI_PTYPE_TYPE_ATTR:
	case OCI_PTYPE_TYPE_COLL:
	case OCI_PTYPE_TYPE_METHOD:
	case OCI_PTYPE_COL: // - column of a table or view
	case OCI_PTYPE_ARG: // - argument of a function or procedure
	case OCI_PTYPE_TYPE_ARG: // - argument of a type method
	case OCI_PTYPE_TYPE_RESULT: // - results of a method
	case OCI_PTYPE_LIST: // - column list for tables and views, argument list for functions and procedures, or subprogram list for packages
	case OCI_PTYPE_SCHEMA: // - schema
	case OCI_PTYPE_DATABASE: // - database
	case OCI_PTYPE_UNK: // - unknown schema object
		throw_oci_exception(OciException(__TROTL_HERE__, "Error describing object: %d\n").arg(obj_type));
		//no break here
	default:
		throw_oci_exception(OciException(__TROTL_HERE__, "Error describing object: %d\n").arg(obj_type));
		//no break here
	}
};


Describe::Describe(OciConnection& con, OciDescribe &desc, const tstring &objectName)
	: _con(con)
	, _desc(desc)
	, _objName(objectName)
{
	//_desc.alloc(conn._env);
}

Describe::Describe(OciConnection& con, const tstring &objectName)
	: _con(con)
	, _desc(_con._env) // NOTE: this descriptor is un-initialized and should be never used
	, _objName(objectName)
{
	//_desc.alloc(conn._env);
}

Describe::~Describe()
{
}

DescribeSynonym::DescribeSynonym(OciConnection &con, OciDescribe &desc, const tstring &objectName, bool isPublic)
	: Describe(con, desc, objectName)
	, _public(isPublic)
{
	OciParam param = _desc.get_param();

	// OCI_ATTR_OBJID Object ID ub4
	_objid = param.get_attribute<ub4>(OCI_ATTR_OBJID);

	// OCI_ATTR_SCHEMA_NAME A string containing the schema name of the synonym translation OraText *
	_schema = param.get_attribute<tstring>(OCI_ATTR_SCHEMA_NAME);

	// OCI_ATTR_NAME A NULL-terminated string containing the object name of the synonym translation OraText *
	_name = param.get_attribute<tstring>(OCI_ATTR_NAME);

	// OCI_ATTR_LINK A NULL-terminated string containing the database link name of the synonym translation OraText *
	_link = param.get_attribute<tstring>(OCI_ATTR_LINK);
};

tstring DescribeSynonym::toString() const
{
	tostream retval;
	if(_public)
		retval << "PUBLIC SYNONYM: " << _objName << std::endl;
	else
		retval << "SYNONYM: " << objectName() << std::endl;
	retval << "target schema: " << _schema << std::endl
	       << "target name: " << _name << std::endl
	       << "link: " << _link << std::endl;
	return retval.str();
};

tstring DescribeSynonym::_synonym("SYNONYM");
tstring DescribeSynonym::_public_synonym("PUBLIC SYNONYM");


DescribeView::DescribeView(OciConnection &con, OciDescribe &desc, const tstring &objectName)
	: Describe(con, desc, objectName)
{
	OciParam param = _desc.get_param();

	// OCI_ATTR_OBJID Object ID ub4
	_objid = param.get_attribute<ub4>(OCI_ATTR_OBJID);

	// OCI_ATTR_NUM_COLS Number of columns ub2
	_num_cols = param.get_attribute<ub2>(OCI_ATTR_NUM_COLS);

	// OCI_ATTR_LIST_COLUMNS Column list (type OCI_PTYPE_LIST) void *
	//OciParam list_columns(param.get_param(OCI_ATTR_LIST_COLUMNS));
	_list_columns = param.get_param(OCI_ATTR_LIST_COLUMNS);

	// OCI_ATTR_REF_TDO REF to the type description object (TDO) of the base type for extent tables OCIRef*
	//OCIRef* ref_tdo;
	//param.get_attribute(&ref_tdo, OCI_ATTR_REF_TDO);
	// throws: ORA-22804: remote operations not permitted on object tables or user-defined type columns

	// OCI_ATTR_IS_TEMPORARY Indicates that the table is temporary ub1
	//ub1 is_temporary;
	//param.get_attribute(&is_temporary, OCI_ATTR_IS_TEMPORARY);
	//throws: ORA-24328: illegal attribute value

	// OCI_ATTR_IS_TYPED Indicates that the table is typed ub1
	_is_typed = param.get_attribute<ub1>(OCI_ATTR_IS_TYPED);

	// OCI_ATTR_DURATION Duration of a temporary table. OCIDuration
	//  Values can be:
	//  OCI_DURATION_SESSION - session
	//  OCI_DURATION_TRANS - transaction
	//  OCI_DURATION_NULL - table not temporary
	//OCIDuration duration;
	//param.get_attribute(&duration, OCI_ATTR_DURATION);
	//throws: ORA-24328: illegal attribute value

	for(ub4 i=1; i<=_num_cols; i++)
	{
		OciParam col_param = _list_columns.get_sub_param(i);
		DescribeColumn *col = new DescribeColumn(_con, col_param, _objName);
		_columns.push_back(col);
	}
}

tstring DescribeView::toString() const
{
	tostream retval;
	retval << "objid: "        << _objid         << std::endl
	       << "num_cols: "     << _num_cols      << std::endl
	       << "is_typed: "     << (unsigned)_is_typed << std::endl;

	for(std::vector<DescribeColumn*>::const_iterator i = _columns.begin(); i != _columns.end(); i++)
	{
		retval << (*i)->toString();
	}

	return retval.str();
}

tstring DescribeView::_view("VIEW");


DescribeTable::DescribeTable(OciConnection &con, OciDescribe &desc, const tstring &objectName)
	: Describe(con, desc, objectName)
{
	OciParam param = _desc.get_param();

	// OCI_ATTR_OBJID Object ID ub4
	_objid = param.get_attribute<ub4>(OCI_ATTR_OBJID);

	// OCI_ATTR_NUM_COLS Number of columns ub2
	_num_cols = param.get_attribute<ub2>(OCI_ATTR_NUM_COLS);

	// OCI_ATTR_LIST_COLUMNS Column list (type OCI_PTYPE_LIST) void *
	_list_columns =	param.get_param(OCI_ATTR_LIST_COLUMNS);

	// OCI_ATTR_REF_TDO REF to the type description object (TDO) of the base type for extent tables OCIRef*
	//OCIRef* ref_tdo;
	//param.get_attribute(&ref_tdo, OCI_ATTR_REF_TDO);
	// throws: ORA-22804: remote operations not permitted on object tables or user-defined type columns

	// OCI_ATTR_IS_TEMPORARY Indicates that the table is temporary ub1
	_is_temporary = param.get_attribute<ub1>(OCI_ATTR_IS_TEMPORARY);

	// OCI_ATTR_IS_TYPED Indicates that the table is typed ub1
	_is_typed = param.get_attribute<ub1>(OCI_ATTR_IS_TYPED);

	// OCI_ATTR_DURATION Duration of a temporary table. OCIDuration
	//  Values can be:
	//  OCI_DURATION_SESSION - session
	//  OCI_DURATION_TRANS - transaction
	//  OCI_DURATION_NULL - table not temporary
	_duration = param.get_attribute<OCIDuration>(OCI_ATTR_DURATION);

	// OCI_ATTR_RDBA Data block address of the segment header ub4
	_rdba = param.get_attribute<ub4>(OCI_ATTR_RDBA);

	// OCI_ATTR_TABLESPACE Tablespace that the table resides in word
	// word?? huh what is it?
	_tablespace = param.get_attribute<uword>(OCI_ATTR_TABLESPACE);

	// OCI_ATTR_CLUSTERED Indicates that the table is clustered ub1
	_clustered =	param.get_attribute<ub1>(OCI_ATTR_CLUSTERED);

	// OCI_ATTR_PARTITIONED Indicates that the table is partitioned ub1
	_partitioned = param.get_attribute<ub1>(OCI_ATTR_PARTITIONED);

	// OCI_ATTR_INDEX_ONLY Indicates that the table is index-only ub1
	_index_only = param.get_attribute<ub1>(OCI_ATTR_INDEX_ONLY);

	for(ub4 i=1; i<=_num_cols; i++)
	{
		OciParam col_param = _list_columns.get_sub_param(i);
		DescribeColumn *col = new DescribeColumn(_con, col_param, _objName);
		_columns.push_back(col);
	}
}

tstring DescribeTable::toString() const
{
	tostream retval;
	retval << "objid: "        << _objid        << std::endl
	       << "num_cols: "     << _num_cols     << std::endl
	       << "is_temporary: " << (unsigned)_is_temporary << std::endl
	       << "is_typed: "     << (unsigned)_is_typed     << std::endl;
	switch(_duration)
	{
	case OCI_DURATION_SESSION:
		retval << "duration: "     << "DURATION_SESSION"     << std::endl;
		break;
	case OCI_DURATION_TRANS:
		retval << "duration: "     << "DURATION_TRANS"     << std::endl;
		break;
	case OCI_DURATION_NULL:
		retval << "duration: "     << "DURATION_NULL"     << std::endl;
		break;
	}
	retval << "rdba: "         << _rdba         << std::endl
	       << "tablespace: "   << _tablespace   << std::endl
	       << "clustered: "    << (unsigned)_clustered    << std::endl
	       << "partitioned: "  << (unsigned)_partitioned  << std::endl
	       << "index_only: "   << (unsigned)_index_only   << std::endl
	       ;

	for(std::vector<DescribeColumn*>::const_iterator i = _columns.begin(); i != _columns.end(); i++)
	{
		retval << (*i)->toString();
	}

	return retval.str();
}

tstring DescribeTable::_table("TABLE");

DescribeColumn::DescribeColumn(OciConnection &con, OciParam &param, const tstring &tableName)
	: Describe(con, "")
	, _tableName("")
	, _name("")
	, _schema_name("")
	, _type_name("")
	, _data_type(0)
	, _typecode(0)
	, _collection_typecode(0)
	, _collection_data_type(0)
	, _precision(0)
	, _scale(0)
	, _charset_id(0)
	, _charset_form(0)
	, _char_used(0)
	, _char_size(0)
	, _data_size(0)
	, _fprecision(0)
	, _lfprecision(0)
	, _is_null(0)
	, _ref_tdo(NULL)
	, _type(NULL)
	, _utf16(false)
	, _reg_name("")
{
	describe(con, param);
	// OCI_ATTR_PRECISION The precision of numeric columns. If the precision is nonzero and scale is -127,
	// then it is a FLOAT, else it is a NUMBER(precision, scale).
	// For the case when precision is 0, NUMBER(precision, scale) can be represented simply as NUMBER.
	// ub1 for explicit describe
	// sb2 for implicit describe
	ub1 precision;
	precision = param.get_attribute<ub1>(OCI_ATTR_PRECISION);
	_precision = precision;
};

DescribeColumn::DescribeColumn(OciConnection &con, OciHandle<OCIStmt> &stmt, unsigned columnPosition, const tstring &sql)
	: Describe(con, "")
	, _tableName("")
	, _name("")
	, _schema_name("")
	, _type_name("")
	, _data_type(0)
	, _typecode(0)
	, _collection_typecode(0)
	, _collection_data_type(0)
	, _precision(0)
	, _scale(0)
	, _charset_id(0)
	, _charset_form(0)
	, _char_used(0)
	, _char_size(0)
	, _data_size(0)
	, _fprecision(0)
	, _lfprecision(0)
	, _is_null(0)
	, _ref_tdo(NULL)
	, _type(NULL)
	, _utf16(false)
	, _reg_name("")
{
		OciParam param(con._env, NULL);
		sword res = OCICALL(OCIParamGet((OCIStmt*)stmt, OciHandleID<OCIStmt>::get_type_id(), con._env._errh, (dvoid**)(OCIParam**)param, columnPosition));

		describe(con, param);
		// OCI_ATTR_PRECISION The precision of numeric columns. If the precision is nonzero and scale is -127,
		// then it is a FLOAT, else it is a NUMBER(precision, scale).
		// For the case when precision is 0, NUMBER(precision, scale) can be represented simply as NUMBER.
		// ub1 for explicit describe
		// sb2 for implicit describe
		sb2 precision;
		precision = param.get_attribute<sb2>(OCI_ATTR_PRECISION);
		_precision = precision;
};

void DescribeColumn::describe(OciConnection &con, OciParam &param)
{
	// OciParam param = _desc.get_param();

	_name = param.get_attribute<tstring>(OCI_ATTR_NAME);

	_schema_name = param.get_attribute<tstring>(OCI_ATTR_SCHEMA_NAME);

	_type_name = param.get_attribute<tstring>(OCI_ATTR_TYPE_NAME);

	_data_type = param.get_attribute<ub2>(OCI_ATTR_DATA_TYPE);

	// OCI_ATTR_SCALE The scale of numeric columns. If the precision is nonzero and scale is -127,
	// then it is a FLOAT, else it is a NUMBER(precision, scale).
	// For the case when precision is 0, NUMBER(precision, scale) can be represented simply as NUMBER. sb1
	_scale = param.get_attribute<sb1>(OCI_ATTR_SCALE);

	// OCI_ATTR_REF_TDO Returns the in-memory REF of the TDO for the type, if the column type is an object type.
	// If space has not been reserved for the OCIRef, then it is allocated implicitly in the cache. The caller can then pin the TDO with OCIObjectPin().
	// OCIRef*

	// OCI_ATTR_CHARSET_ID The character set id, if the type attribute is of a string/character type ub2
	_charset_id = param.get_attribute<ub2>(OCI_ATTR_CHARSET_ID);

	// OCI_ATTR_CHARSET_FORM The character set form, if the type attribute is of a string/character type ub1
	_charset_form = param.get_attribute<ub1>(OCI_ATTR_CHARSET_FORM);
	_utf16 = (_charset_form == SQLCS_NCHAR);

	// OCI_ATTR_CHAR_USED Returns the type of length semantics of the column. 0 means byte-length semantics and 1 means character-length semantics.
	// See "Character Length Semantics Support in Describing". ub4 (well docs says ub4 but it is ub1 in reality)
	_char_used = param.get_attribute<ub1>(OCI_ATTR_CHAR_USED);

	// OCI_ATTR_CHAR_SIZE Returns the column character length which is the number of characters allowed in the column.
	// It is the counterpart of OCI_ATTR_DATA_SIZE which gets the byte length. See "Character Length Semantics Support in Describing". ub2
	_char_size = param.get_attribute<ub2>(OCI_ATTR_CHAR_SIZE);

	// NOTE: older OCI docs refer OCI_ATTR_DATA_SIZE as ub2.
	// In 11gR2 it is ub4, but only ub2 works
	// OCI_ATTR_DATA_SIZE The maximum size of the column. This length is returned in bytes and not characters for strings and raws.
	// It returns 22 for NUMBERs. ub4
	// see metalink note 332084.1
	_data_size = param.get_attribute<ub2>(OCI_ATTR_DATA_SIZE);

	// OCI_ATTR_FSPRECISION The fractional seconds precision of a datetime or interval. ub1
	_fprecision = param.get_attribute<ub1>(OCI_ATTR_FSPRECISION);

	// OCI_ATTR_LFPRECISION The leading field precision of an interval. ub1
	_lfprecision = param.get_attribute<ub1>(OCI_ATTR_LFPRECISION);

	// get NULL-ability flag
	// OCI_ATTR_IS_NULL Returns 0 if null values are not permitted for the column ub1
	_is_null = param.get_attribute<ub1>(OCI_ATTR_IS_NULL);

	if(_data_type == SQLT_NTY)
	{
		sword res;
		// OCI_ATTR_REF_TDO Returns the REF of the TDO for the type, if the arguemnt type is an object OCIRef *
		_ref_tdo = param.get_attribute<OCIRef*>(OCI_ATTR_REF_TDO);

		OciDescribe type_desc(con._env);

		res = OCIDescribeAny(con._svc_ctx, con._env._errh,
		                     _ref_tdo,			// objptr (IN) pointer to a REF to the TDO
		                     sizeof(_ref_tdo),	// objnm_len (IN)
		                     OCI_OTYPE_REF,		// objptr_typ (IN) points to the name of a schema object
		                     (ub1)OCI_DEFAULT,	// info_level (IN) for future extensions. Pass OCI_DEFAULT.
		                     OCI_PTYPE_TYPE,
		                     type_desc
		                    );
		oci_check_error(__TROTL_HERE__, con._env._errh, res);

		_type = new DescribeType(con, type_desc, _name);

		_reg_name = _type->_reg_name;

		// throws ORA-24328: illegal attribute value on number
		_typecode = _type->_typecode;
		_collection_typecode = _type->_collection_typecode;
		_collection_data_type = _type->_collection_data_type;
	}
}

tstring DescribeColumn::toString() const
{
	tostream retval;
	retval << "name: " << _name << '\t'
	       << "schema_name: " << _schema_name << '\t'
	       << "type_name: " << _type_name << '\t'
	       << "data_type: " << _data_type << '\t'
	       << "scale: " << (int)_scale << '\t'
	       << "precision: " << (unsigned)_precision << '\t'
	       << "data_size: " << (unsigned)_data_size << '\t'
	       << "is_null: " << (unsigned)_is_null << '\t'
	       << "type: " << typeName()
	       << std::endl;

	// if(_type != NULL)
	// 	retval << _type->toString();
	return retval.str();
}

tstring DescribeColumn::name() const
{
	return _name;
}

tstring DescribeColumn::typeName() const
{
	tostream str;

	switch(_data_type)
	{
	case SQLT_AFC: // OCI_TYPECODE_CHAR:
		str << (_utf16 ? "N" : "")
		    << "CHAR("
		    << (_char_used ? (ub4)_char_size : (ub4)_data_size)
		    << ' '
		    << (_char_used ? "CHAR" : "BYTES")
		    << ")";
		break;
	case SQLT_CHR: // OCI_TYPECODE_VARCHAR:
		// NOTE: uncomment this on older Oracle versions datatype VARCHAR is no more present on 11g
		// str << (_utf16 ? "N" : "") << "VARCHAR(" << (_char_used ? (ub4)_char_size : (ub4)_data_size) << ")";
		// break;
	case SQLT_VCS: // OCI_TYPECODE_VARCHAR2:
		str << (_utf16 ? "N" : "")
		    << "VARCHAR2("
		    << (_char_used ? (ub4)_char_size : (ub4)_data_size)
		    << ' '
		    << (_char_used ? "CHAR" : "BYTES")
		    << ")";
		break;
	case SQLT_BIN:
		str << "RAW("<< _data_size << ")";
		break;
	case SQLT_NUM: // OCI_TYPECODE_NUMBER:
		// When precision is 0, NUMBER(precision, scale) can be represented simply as NUMBER.
		// If the precision is nonzero and scale is -127, then it is a FLOAT;
		// otherwise, it is a NUMBER(precision, scale).
		if (_precision == 0)
			str << "NUMBER";
		else if(_precision == 38 && _scale == 0)
			str << "INTEGER";
		else if (_scale == -127)
		{
			if (_precision == 126)
				str << "FLOAT";
			else if (_precision == 63)
				str << "REAL";
			else
				str << "FLOAT(" << (int)_precision << ")";
		}
		else if (_precision == 38 && _scale)
			str << "NUMBER(" << '*' << "," << (int)_scale << ")";
		else if (_precision != 38 && _scale)
			str << "NUMBER(" << (int)_precision << "," << (int)_scale << ")";
		else
			str << "NUMBER(" << (int)_precision << ")";

		break;
	case SQLT_DAT: // OCI_TYPECODE_DATE:
		str << "DATE";
		break;
	case SQLT_BLOB: // OCI_TYPECODE_BLOB:
		str << "BLOB";
		break;
	case SQLT_BFILE: // OCI_TYPECODE_BFILE:
		str << "BFILE";
		break;
	case SQLT_CLOB: // OCI_TYPECODE_CLOB:
		str << (_utf16 ? "N" : "") << "CLOB";
		break;
	case SQLT_CFILE: // OCI_TYPECODE_CFILE:
		str << "CFILE";
		break;
	case SQLT_LNG:
		str << "LONG";
		break;
	case SQLT_NTY:
		if (!_schema_name.empty())
			str << _schema_name << '.';
		str << _type_name;
		break;
	case SQLT_LBI:
		str << "LONG RAW"; /* !! TODO unsupported yet */
		break;
	case SQLT_RDD:
	case 208: // 208 = 2 * SQLT_RDD (mentioned only in catalog.sql)
		// _width == 4 for ROWID datatype
		// _width == 4000 for UROWID datatype
		if(_data_size == sizeof(OCIRowid*))
			str << "ROWID";
		else
			str << "UROWID";
		break;
	case SQLT_IBFLOAT:
		str << "BINARY_FLOAT";
		break;
	case SQLT_IBDOUBLE:
		str << "BINARY_DOUBLE";
		break;
	case SQLT_TIMESTAMP:
		str << "TIMESTAMP";
		if(_scale != 6)
			str << '(' << (int)_scale << ')';
		break;
	case SQLT_TIMESTAMP_TZ:
		str << "TIMESTAMP";
		if(_scale != 6)
			str << '(' << (int)_scale << ')';
		str << " WITH TIME ZONE";
		break;
	case SQLT_TIMESTAMP_LTZ:
		str << "TIMESTAMP";
		if(_scale != 6)
			str << '(' << (int)_scale << ')';
		str << " WITH LOCAL TIME ZONE";
		break;
	case SQLT_INTERVAL_YM:
		str << "INTERVAL YEAR TO MONTH";
		break;
	case SQLT_INTERVAL_DS:
		str << "INTERVAL DAY TO SECOND";
		break;
	case SQLT_LAB:
		str << "MLSLABEL";
		break;
	case SQLT_RSET:
		str << "CURSOR";
		break;
	default:
		str << "unknown datatype:" << _data_type;
		break;
	}

	return str.str();
}

// OCIType* DescribeType::getOCIType()
// {
// 	OCIType *retval;
// 	res = OCICALL(OCITypeByRef(stat._env, stat._errh, _type_r, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &tdo));
// 	oci_check_error(__TROTL_HERE__, stat._errh, res);
// 	return retval;
// }

bool DescribeColumn::nullable() const
{
	return !!_is_null;
}

DescribeType* DescribeColumn::getDescribeType()
{
	return _type;
}

DescribeColumn::~DescribeColumn()
{
	if(_type)
		delete _type;
	_type = 0;
}

tstring DescribeColumn::_column("COLUMN");

DescribeType::DescribeType(OciConnection &con, OciDescribe &desc, const tstring &tableName)
	: Describe(con, "")
	, _collection_typecode(0)
	, _reg_name("")
{
	OciParam param = desc.get_param();

	//OCI_ATTR_REF_TDO Returns the in-memory REF of the type descriptor object (TDO) for the type, if the column type is an object type.
	//If space has not been reserved for the OCIRef, then it is allocated implicitly in the cache. The caller can then pin the TDO with OCIObjectPin(). OCIRef *
	//Throws ORA-22804: remote operations not permitted on object tables or user-defined type columns
	//OCIRef *ref_tdo = param.get_attribute<OCIRef*>(OCI_ATTR_REF_TDO);

	//OCI_ATTR_TYPECODE Typecode. See "Data Type Codes". Currently can be only OCI_TYPECODE_OBJECT or OCI_TYPECODE_NAMEDCOLLECTION. OCITypeCode
	_typecode = param.get_attribute<OCITypeCode>(OCI_ATTR_TYPECODE);

	//OCI_ATTR_IS_INCOMPLETE_TYPE Indicates that this is an incomplete type ub1
	_is_incomplete_type = param.get_attribute<ub1>(OCI_ATTR_IS_INCOMPLETE_TYPE);

	//OCI_ATTR_IS_SYSTEM_TYPE Indicates that this is a system type ub1
	_is_system_type = param.get_attribute<ub1>(OCI_ATTR_IS_SYSTEM_TYPE);

	//OCI_ATTR_IS_PREDEFINED_TYPE Indicates that this is a predefined type ub1
	_is_predefined_type = param.get_attribute<ub1>(OCI_ATTR_IS_PREDEFINED_TYPE);

	//OCI_ATTR_IS_TRANSIENT_TYPE Indicates that this is a transient type ub1
	_is_transient_type = param.get_attribute<ub1>(OCI_ATTR_IS_TRANSIENT_TYPE);

	//OCI_ATTR_IS_SYSTEM_GENERATED_TYPE Indicates that this is a system-generated type ub1
	_is_system_generated_type = param.get_attribute<ub1>(OCI_ATTR_IS_SYSTEM_GENERATED_TYPE);

	//OCI_ATTR_HAS_NESTED_TABLE This type contains a nested table attribute. ub1
	_has_nested_table = param.get_attribute<ub1>(OCI_ATTR_HAS_NESTED_TABLE);

	//OCI_ATTR_HAS_LOB This type contains a LOB attribute. ub1
	_has_lob = param.get_attribute<ub1>(OCI_ATTR_HAS_LOB);

	//OCI_ATTR_HAS_FILE This type contains a BFILE attribute. ub1
	_has_file = param.get_attribute<ub1>(OCI_ATTR_HAS_FILE);

	//OCI_ATTR_COLLECTION_ELEMENT Handle to collection element. See "Collection Attributes". void *

	//OCI_ATTR_NUM_TYPE_ATTRS Number of type attributes ub2
	_num_type_attrs = param.get_attribute<ub2>(OCI_ATTR_NUM_TYPE_ATTRS);

	//OCI_ATTR_LIST_TYPE_ATTRS List of type attributes. See "List Attributes". void *

	//OCI_ATTR_NUM_TYPE_METHODS Number of type methods ub2
	_num_type_methods = param.get_attribute<ub2>(OCI_ATTR_NUM_TYPE_METHODS);

	//OCI_ATTR_LIST_TYPE_METHODS List of type methods. See "List Attributes". void *
	////_list_type_methods = param.get_param(OCI_ATTR_LIST_TYPE_METHODS);

	//OCI_ATTR_MAP_METHOD Map method of type. See "Type Method Attributes". void *

	//OCI_ATTR_ORDER_METHOD Order method of type. See "Type Method Attributes". void *

	//OCI_ATTR_IS_INVOKER_RIGHTS Indicates that the type has invoker's rights  ub1
	_is_invoker_rights = param.get_attribute<ub1>(OCI_ATTR_IS_INVOKER_RIGHTS);

	//OCI_ATTR_NAME A pointer to a string that is the type attribute name  OraText *
	_name = param.get_attribute<tstring>(OCI_ATTR_NAME);

	//OCI_ATTR_SCHEMA_NAME A string with the schema name where the type has been created  OraText *
	_schema_name = param.get_attribute<tstring>(OCI_ATTR_SCHEMA_NAME);

	//OCI_ATTR_IS_FINAL_TYPE Indicates that this is a final type ub1
	_is_final_type = param.get_attribute<ub1>(OCI_ATTR_IS_FINAL_TYPE);

	//OCI_ATTR_IS_INSTANTIABLE_TYPE Indicates that this is an instantiable type ub1
	_is_instantiable_type = param.get_attribute<ub1>(OCI_ATTR_IS_INSTANTIABLE_TYPE);

	//OCI_ATTR_IS_SUBTYPE Indicates that this is a subtype ub1
	_is_subtype = param.get_attribute<ub1>(OCI_ATTR_IS_SUBTYPE);

	//OCI_ATTR_SUPERTYPE_SCHEMA_NAME Name of the schema that contains the supertype OraText *
	_supertype_schema_name = param.get_attribute<tstring>(OCI_ATTR_SUPERTYPE_SCHEMA_NAME);

	//OCI_ATTR_SUPERTYPE_NAME Name of the supertype OraText *
	_supertype_name = param.get_attribute<tstring>(OCI_ATTR_SUPERTYPE_NAME);

	if (_typecode == OCI_TYPECODE_NAMEDCOLLECTION)
	{
		//OCI_ATTR_COLLECTION_TYPECODE Typecode of collection if type is collection; invalid otherwise. See "Data Type Codes".
		//Currently can be only OCI_TYPECODE_VARRAY or OCI_TYPECODE_TABLE.
		// If this attribute is queried for a type that is not a collection, an error is returned.
		//OCITypeCode
		_collection_typecode = param.get_attribute<OCITypeCode>(OCI_ATTR_COLLECTION_TYPECODE);

		OciParam collection_param = param.get_param(OCI_ATTR_COLLECTION_ELEMENT);

		/* get the data size */
		_collection_data_size = collection_param.get_attribute<ub2>(OCI_ATTR_DATA_SIZE);

		/* get the name of the collection */
		_collection_type_name = collection_param.get_attribute<tstring>(OCI_ATTR_TYPE_NAME);

		/* get the name of the schema */
		_collection_schema_name = collection_param.get_attribute<tstring>(OCI_ATTR_SCHEMA_NAME);

		/* get the data type */
		_collection_data_type = collection_param.get_attribute<ub2>(OCI_ATTR_DATA_TYPE);

		if (_collection_typecode == OCI_TYPECODE_VARRAY)
		{
			/* get the number of elements */
			_collection_num_elems = collection_param.get_attribute<ub4>(OCI_ATTR_NUM_ELEMS);
			_reg_name = "VARARRAY OF " + _collection_type_name;
		}
		else     // _collection_typecode == OCI_TYPECODE_TABLE
		{
			_reg_name = "TABLE OF " + _collection_type_name;
		}
	}
	else
	{
		_reg_name = _schema_name + '.' + _name;
	}

//	if(_typecode == SQLT_NTY)
//	{
//		// OCI_ATTR_REF_TDO Returns the REF of the TDO for the type, if the arguemnt type is an object OCIRef *
//		OCIRef *_ref_tdo = param.get_attribute<OCIRef*>(OCI_ATTR_REF_TDO);
//	}
};

tstring DescribeType::toString() const
{
	tostream retval;
	retval
	                << "typecode: "             << _typecode             << std::endl
	                << "collection_typecode: "  << _collection_typecode  << std::endl
	                << "is_incomplete_type: "   << (unsigned)_is_incomplete_type   << std::endl
	                << "is_system_type: "       << (unsigned)_is_system_type       << std::endl
	                << "is_predefined_type: "   << (unsigned)_is_predefined_type   << std::endl
	                << "is_transient_type: "    << (unsigned)_is_transient_type    << std::endl
	                << "is_system_generated_type: " << (unsigned)_is_system_generated_type << std::endl
	                << "has_nested_table: "     << (unsigned)_has_nested_table     << std::endl
	                << "has_lob: "              << (unsigned)_has_lob              << std::endl
	                << "has_file: "             << (unsigned)_has_file             << std::endl
	                << "num_type_attrs: "       << _num_type_attrs       << std::endl
	                << "num_type_methods: "     << _num_type_methods     << std::endl
	                << "is_invoker_rights: "    << (unsigned)_is_invoker_rights    << std::endl
	                << "is_final_type: "        << (unsigned)_is_final_type        << std::endl
	                << "is_instantiable_type: " << (unsigned)_is_instantiable_type << std::endl
	                << "is_subtype: "           << (unsigned)_is_subtype	    << std::endl;
	// TODO handle collection here
	return retval.str();
};

tstring DescribeType::_type("TYPE");


DescribeSequence::DescribeSequence(OciConnection &con, OciDescribe &desc, const tstring &tableName)
	: Describe(con, "")
	, _min(con._env)
	, _max(con._env)
	, _incr(con._env)
	, _cache(con._env)
	, _hw_marks(con._env)
{
	OciParam param = desc.get_param();

	//OCI_ATTR_OBJID Object ID ub4
	_objid = param.get_attribute<ub4>(OCI_ATTR_OBJID);

	//OCI_ATTR_MIN  Minimum value (in Oracle NUMBER format) ub1 *
	_min = param.get_attribute<OCINumber>(OCI_ATTR_MIN);

	//OCI_ATTR_MAX Maximum value (in Oracle NUMBER format) ub1 *
	_max = param.get_attribute<OCINumber>(OCI_ATTR_MAX);

	//OCI_ATTR_INCR Increment (in Oracle NUMBER format) ub1 *
	_incr = param.get_attribute<OCINumber>(OCI_ATTR_INCR);

	//OCI_ATTR_CACHE Number of sequence numbers cached; zero if the sequence is not a cached sequence (in Oracle NUMBER format) ub1 *
	_cache = param.get_attribute<OCINumber>(OCI_ATTR_CACHE);

	//OCI_ATTR_ORDER Whether the sequence is ordered ub1
	_order = param.get_attribute<ub1>(OCI_ATTR_ORDER);

	//OCI_ATTR_HW_MARK High-water mark (in NUMBER format) ub1 *
	_hw_marks = param.get_attribute<OCINumber>(OCI_ATTR_HW_MARK);
}

tstring DescribeSequence::toString() const
{
	tostream retval;
	retval
	                << "Min: " << _min.toString() << std::endl
	                << "Max: " << _max.toString() << std::endl
	                << "Increment: " << _incr.toString() << std::endl
	                << "Cache: " << _cache.toString() << std::endl
	                << "Order: " << (unsigned)_order << std::endl
	                << "High water mark: " << _hw_marks.toString() << std::endl
	                ;
	return retval.str();
};

tstring DescribeSequence::_seq("SEQUENCE");
};

