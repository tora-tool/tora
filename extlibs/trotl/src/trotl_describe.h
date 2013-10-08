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

#ifndef TROTL_DESCRIBE_H_
#define TROTL_DESCRIBE_H_

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_handle.h"
#include "trotl_conn.h"
#include "trotl_int.h"

namespace trotl
{

struct OciDescribe;

struct OciParam : public OciSimpleHandle<OCIParam>
{
	//template <class TYPE> friend struct OciHandle;
	friend struct OciDescribe;
	friend class SqlStatement;
	friend class DescribeColumn;

	OciParam() : OciSimpleHandle<OCIParam>() {};
	~OciParam()
	{
		if(_handle)
		{
			sword res = OCICALL(OCIDescriptorFree(_handle, OCI_DTYPE_PARAM));
			oci_check_error(__TROTL_HERE__, _envhp, res);
			// Will be freed while destructing OCIDescriptor
			// and should not be freed by parent calling OCIHandleFree
			_handle = 0;
		}
	};

	template<class RETTYPE>
	inline
	RETTYPE get_attribute(ub4 attrtype)
	{
		if( Loki::TypeTraits<RETTYPE>::isIntegral)
		{
			RETTYPE retval;
			sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIParam>::get_type_id(), &retval, 0, attrtype, _errh));
			oci_check_error(__TROTL_HERE__, _errh, res);
			return retval;
		}
		else if( Loki::TypeTraits<RETTYPE>::isPointer)
		{
			RETTYPE retval;
			sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIParam>::get_type_id(), &retval, 0, attrtype, _errh));
			oci_check_error(__TROTL_HERE__, _errh, res);
			return retval;
		} // TODO throw something else here
		throw_oci_exception(OciException(__TROTL_HERE__, "Unsupported datatype: %s").arg(typeid(RETTYPE).name()));
	}

	OciParam get_param(ub4 attrtype)
	{
		OciParam retval(_envhp, NULL);
		sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIParam>::get_type_id(), (OCIParam**)retval, 0, attrtype, _errh));
		oci_check_error(__TROTL_HERE__, _errh, res);
		return retval;
	}

	OciParam get_sub_param(ub4 pos)
	{
		OciParam retval(_envhp, NULL);
		sword res = OCICALL(OCIParamGet(_handle, OciHandleID<OCIParam>::get_type_id(), _errh, (dvoid**)(OCIParam**)retval, pos));
		oci_check_error(__TROTL_HERE__, _errh, res);
		return retval;
	}

	OciParam(const OciParam& other) : OciSimpleHandle<OCIParam>(other._envhp, other._handle)
	{
		_errh.alloc(_envhp); // TODO we SHOULD can destructively copy _errh here
		const_cast<OciParam&>(other)._handle = NULL;
	};

	OciParam& operator=(const OciParam &other)
	{
		_envhp = other._envhp;
		_handle = other._handle;
		_errh.alloc(_envhp); // TODO we can destructively copy _errh here
		const_cast<OciParam&>(other)._handle = NULL;
		return *this;
	};
private:
	operator HandleType**()
	{
		return &_handle;
	}

	OciError _errh;
protected:
	OciParam(OCIEnv* envhp, HandleType* handle)
		: OciSimpleHandle<OCIParam>(envhp, handle)
	{
		_errh.alloc(_envhp);
	};

	OciParam(OCIEnv* envh); // : OciSimpleHandle<OCIParam>(envh) {};
};

template<>
inline
tstring OciParam::get_attribute <tstring> (ub4 attrtype)
{
	OraText *retval;
	ub4 retval_len;
	sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIParam>::get_type_id(), &retval, &retval_len, attrtype, _errh));
	oci_check_error(__TROTL_HERE__, _errh, res);
	return tstring((const char*)retval, retval_len);
}

template<>
inline
OCINumber OciParam::get_attribute <OCINumber> (ub4 attrtype)
{
	void *retvalp = NULL;
	ub4 retval_len = 0;
	SqlNumber::OracleNumber retval;

	sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIParam>::get_type_id(), &retvalp, &retval_len, attrtype, _errh));
	oci_check_error(__TROTL_HERE__, _errh, res);

	/** Note this OCIAttrGet returns "real" SQLT_NUM, but no OCI functions work
	    with this datatype. Therefore I use union(see trotl_int.h) and do prepend
	    one byte for length in order to convert it into SQLT_VNU.
	*/
	retval.OracleNumberFormat.len = (ub1) retval_len;
	memcpy(&retval.OracleNumberFormat.exp, retvalp, retval_len);

	return retval.ocinumber;
}


/// OCI Describe Handles
struct OciDescribe: public OciHandle<OCIDescribe>
{
	OciDescribe(OciEnv& env) : OciHandle<OCIDescribe>(env) {};
	OciParam get_param()
	{
		OciParam retval(_env, NULL);
		sword res = OCICALL(OCIAttrGet(_handle, OciHandleID<OCIDescribe>::get_type_id(), (OCIParam**)retval, 0, OCI_ATTR_PARAM, _env._errh));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		return retval;
	}
};


// Base class for all Describe* classes
class TROTL_EXPORT Describe
{
public:
	enum objectType
	{
		O_SYNONYM,
		O_PUBLIC_SYNONYM,
		O_VIEW,
		O_TABLE,
		O_COLUMN,
		O_TYPE,
		O_SEQUENCE,
		O_PROCEDURE,
		O_FUNCTION,
		O_PACKAGE
	};

	virtual const tstring& whatIsThisString() const = 0;
	virtual objectType whatIsThis() const = 0;
	virtual tstring toString() const = 0;

	const tstring& objectName() const
	{
		return _objName;
	};

	static objectType getObjectType(OciConnection &con, const tstring &objectName);
	static Describe* createDescription(OciConnection &con, const tstring &objectName);

	virtual ~Describe();
protected:
	Describe(const tstring &objectName);
	Describe(OciConnection& con, const tstring &objectName);
	Describe(OciConnection& con, OciDescribe &desc, const tstring &objectName);
	OciConnection &_con;
	OciDescribe _desc;
	const tstring _objName;
	//static objectType objectTypeFromOCI(const tstring &objectName);
};

class TROTL_EXPORT DescribeSynonym : public Describe
{
public:
	DescribeSynonym(OciConnection& conn, const tstring &objectName);

	virtual const tstring& whatIsThisString() const
	{
		return _public ? _public_synonym :_synonym;
	};
	virtual objectType whatIsThis() const
	{
		return _public ? O_PUBLIC_SYNONYM : O_SYNONYM;
	};
	virtual tstring toString() const;

	inline const tstring& name() const
	{
		return _name;
	};
	inline const tstring& schema() const
	{
		return _schema;
	};
	inline const tstring& link() const
	{
		return _link;
	};
	inline const bool isPublic() const
	{
		return _public;
	};

protected:
	friend class Describe;
	DescribeSynonym(OciConnection &, OciDescribe&, const tstring &objectName, bool);
public:
	tstring _name, _schema, _link;
	bool _public;
	ub4 _objid;
	static tstring _synonym;
	static tstring _public_synonym;
};

class DescribeColumn;

class TROTL_EXPORT DescribeView : public Describe
{
public:
	DescribeView(OciDescribe&, const tstring &objectName);

	virtual const tstring& whatIsThisString() const
	{
		return _view;
	};
	virtual objectType whatIsThis() const
	{
		return O_VIEW;
	};
	virtual tstring toString() const;

	DescribeColumn* getColumn(unsigned);
protected:
	friend class Describe;
	DescribeView(OciConnection &, OciDescribe&, const tstring &objectName);
public:
	ub4 _objid;
	ub2 _num_cols;
	ub1 _is_typed;
	OciParam _list_columns;
	std::vector<DescribeColumn*> _columns;
	static tstring _view;
};


class TROTL_EXPORT DescribeTable : public Describe
{
public:
	DescribeTable(OciDescribe&, const tstring &objectName);

	virtual const tstring& whatIsThisString() const
	{
		return _table;
	};
	virtual objectType whatIsThis() const
	{
		return O_TABLE;
	};
	virtual tstring toString() const;

	DescribeColumn* getColumn(unsigned);
protected:
	friend class Describe;
	DescribeTable(OciConnection& conn, OciDescribe &desc, const tstring &objectName);
public:
	ub4 _objid;
	ub2 _num_cols;
	OciParam _list_columns;
	ub1 _is_temporary;
	ub1 _is_typed;
	OCIDuration _duration;
	ub4 _rdba;
	uword _tablespace;
	ub1 _clustered;
	ub1 _partitioned;
	ub1 _index_only;
	std::vector<DescribeColumn*> _columns;
	static tstring _table;
};

class SqlStatement;
class DescribeType;

class TROTL_EXPORT DescribeColumn: public Describe
{
public:
	virtual const tstring& whatIsThisString() const
	{
		return _column;
	};
	virtual objectType whatIsThis() const
	{
		return O_COLUMN;
	};
	virtual tstring toString() const;

	tstring name() const;
	tstring typeName() const;
	bool nullable() const;
	DescribeType* getDescribeType();

	virtual ~DescribeColumn();

//protected:
	friend class Describe;
	friend class DescribeView;
	friend class DescribeTable;
	friend class SqlStatement;

	DescribeColumn(OciConnection&, const tstring &objectName);
	DescribeColumn(OciConnection&, OciParam&, const tstring &tableName);
	DescribeColumn(OciConnection&, OciHandle<OCIStmt>&, unsigned columnPosition, const tstring &statement);

	void describe(OciConnection &, OciParam&);
	tstring _tableName;
	tstring _name;
	tstring _schema_name;
	tstring _type_name;
	ub2 _data_type;
	//throws ORA-24328: illegal attribute value on number, valid only for SQLT_NTY
	//OCITypeCode typecode = param.get_attribute<OCITypeCode>(OCI_ATTR_TYPECODE);
	OCITypeCode _typecode;
	OCITypeCode _collection_typecode;
	OCITypeCode _collection_data_type;
	int _precision;			// ub1 for explicit describe // sb2 for implicit describe
	sb1 _scale;
	ub2 _charset_id;
	ub1 _charset_form;
	ub1 _char_used;
	ub2 _char_size;
	ub2 _data_size; 		// NOTE: older OCI docs refer OCI_ATTR_DATA_SIZE as ub2. In 11g it is ub4
	ub1 _fprecision;
	ub1 _lfprecision;
	ub1 _is_null;
	OCIRef *_ref_tdo;
	// derived values
	DescribeType *_type;
	bool _utf16;
	tstring  _reg_name;
	static tstring _column;
};


class TROTL_EXPORT DescribeType: public Describe
{
public:
	virtual const tstring& whatIsThisString() const
	{
		return _type;
	};
	virtual objectType whatIsThis() const
	{
		return O_TYPE;
	};
	virtual tstring toString() const;

	friend class Describe;
	friend class DescribeColumn;

protected:
	DescribeType(OciConnection& conn, const tstring &objectName);
	DescribeType(OciConnection &con, OciDescribe &desc, const tstring &objectName);
	DescribeType(OciConnection &con, OciParam &param, const tstring &columnName);
public:
	OCITypeCode _typecode;
	OCITypeCode _collection_typecode;
	ub1 _is_incomplete_type;
	ub1 _is_system_type;
	ub1 _is_predefined_type;
	ub1 _is_transient_type;
	ub1 _is_system_generated_type;
	ub1 _has_nested_table;
	ub1 _has_lob;
	ub1 _has_file;
	ub2 _num_type_attrs;
	ub2 _num_type_methods;
	OciParam _list_type_methods;
	ub1 _is_invoker_rights;
	tstring _name;
	tstring _schema_name;
	ub1 _is_final_type;
	ub1 _is_instantiable_type;
	ub1 _is_subtype;
	tstring _supertype_schema_name;
	tstring _supertype_name;
	ub2 _collection_data_size;
	tstring _collection_type_name;
	tstring _collection_schema_name;
	ub2 _collection_data_type;
	ub4 _collection_num_elems;
	tstring _reg_name;
	static tstring _type;
};

class TROTL_EXPORT DescribeSequence: public Describe
{
public:
	virtual const tstring& whatIsThisString() const
	{
		return _seq;
	};
	virtual objectType whatIsThis() const
	{
		return O_SEQUENCE;
	};
	virtual tstring toString() const;

protected:
	friend class Describe;
	DescribeSequence(OciConnection &con, OciDescribe &desc, const tstring &objectName);
public:
	ub4 _objid;
	SqlNumber _min;
	SqlNumber _max;
	SqlNumber _incr;
	SqlNumber _cache;
	ub1 _order;
	SqlNumber _hw_marks;
	static tstring _seq;
};

};

#endif

