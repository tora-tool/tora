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
#include "trotl_convertor.h"

namespace trotl
{

void ConvertorForRead::Fire(const BindParDate &BP, SqlDateTime &DT)
{
//	std::cout << "Convert: void Fire(BindParDate, SqlDateTime)\n";
	DT._ind = BP.indp[_row];
	if(DT.is_not_null())
	{
		OCIDate const &d = (((OCIDate*)BP.valuep)[_row]);

		DT._date.year = d.OCIDateYYYY;
		DT._date.month = d.OCIDateMM ? d.OCIDateMM : 0;
		DT._date.day = d.OCIDateDD ? d.OCIDateDD : 0;
		DT._date.hour = d.OCIDateTime.OCITimeHH ? d.OCIDateTime.OCITimeHH : 0;
		DT._date.minute = d.OCIDateTime.OCITimeMI ? d.OCIDateTime.OCITimeMI : 0;
		DT._date.second = d.OCIDateTime.OCITimeSS ? d.OCIDateTime.OCITimeSS : 0;
	}
};

/*
 *  I am not sure here, OCILobAssign or just simple pointer assign?
 */
void ConvertorForRead::Fire(const BindParClob &BP, SqlClob &CL)
{
	CL._ind = BP.indp[_row];
	if(CL.is_not_null())
	{
		if(BP.isTemporary(_row))
		{
			sword res = OCICALL(OCILobLocatorAssign(BP._stmt._conn._svc_ctx, BP._env._errh, ((OCILobLocator**)BP.valuep)[_row], &CL._loc));
			oci_check_error(__TROTL_HERE__, BP._env._errh, res);
		}
		else
		{
			sword res = OCICALL(OCILobAssign(BP._env, BP._env._errh, ((OCILobLocator**)BP.valuep)[_row], &CL._loc));
			oci_check_error(__TROTL_HERE__, BP._env._errh, res);
		}
		//CL._loc = ((OCILobLocator**)BP.valuep)[_row];
		//((OCILobLocator**)BP.valuep)[_row] = NULL;
		//CL._ind = BP.indp[_row];

//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//		res = OCICALL(OCILobLocatorAssign(_conn._svc_ctx, _conn._env._errh, other._loc, &_loc));
//#else
//		res = OCICALL(OCILobAssign(_conn._env, _conn._env._errh, other._loc, &_loc));	// no support for temporary LOBs
//#endif
	}
};

void ConvertorForRead::Fire(const BindParBlob &BP, SqlBlob &BL)
{
	BL._ind = BP.indp[_row];
	if(BL.is_not_null())
	{
		if(BP.isTemporary(_row))
		{
			sword res = OCICALL(OCILobLocatorAssign(BP._stmt._conn._svc_ctx, BP._env._errh, ((OCILobLocator**)BP.valuep)[_row], &BL._loc));
			oci_check_error(__TROTL_HERE__, BP._env._errh, res);
		}
		else
		{
			sword res = OCICALL(OCILobAssign(BP._env, BP._env._errh, ((OCILobLocator**)BP.valuep)[_row], &BL._loc));
			oci_check_error(__TROTL_HERE__, BP._env._errh, res);
		}

		//BL._loc = ((OCILobLocator**)BP.valuep)[_row];
		//((OCILobLocator**)BP.valuep)[_row] = NULL;
		//BL._ind = BP.indp[_row];

//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//		res = OCICALL(OCILobLocatorAssign(_conn._svc_ctx, _conn._env._errh, other._loc, &_loc));
//#else
//		res = OCICALL(OCILobAssign(_conn._env, _conn._env._errh, other._loc, &_loc));	// no support for temporary LOBs
//#endif
	}
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<int> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<int> implemented elsewhere"));
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<unsigned int> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<unsigned int> implemented elsewhere"));
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<long> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<long> implemented elsewhere"));
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<unsigned long> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<unsigned long> implemented elsewhere"));
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<float> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<float> implemented elsewhere"));
};

void ConvertorForRead::Fire(const BindParNumber &BP, SqlInt<double> &BL)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from BindParCInt to SqlInt<double> implemented elsewhere"));
};

// todo - this relies on the fact that BindParNumber is BindParNumber now
void ConvertorForRead::Fire(const BindParNumber &BP, SqlNumber &SN)
{
	SN._ind = BP.indp[_row];
	if(SN.is_not_null())
	{
		sword res = OCICALL(OCINumberAssign(SN._env._errh, BP.get_ref(_row), &SN._val));
		oci_check_error(__TROTL_HERE__, SN._env._errh, res);
	}
	// TODO raise exception here, if STMT.type==PL/SQL and BP._cnt > 1
	//if(BP._cnt != 1)
	//std::cerr << "Warning assigning vector into scalar(" << BP._cnt << ")" << std::endl;
};

#ifdef ORACLE_HAS_XML
void ConvertorForRead::Fire(const BindParXML &BP, SqlXML &BL)
{
	// TODO add copy of NULL indocator here
	BL._ind = BP.indp[_row];
	if(BL.is_not_null())
		BL.xml = BP._xmlvaluep[_row];
	//BP.xml[_row] = NULL; TODO const_cast here?
}
#endif

void ConvertorForRead::Fire(const BindParCollectionTabNum &BP, SqlCollection &SV)
{
	sword res;
	SV._collection_tdo = BP._collection_tdo;
	SV._collection_typecode = BP._collection_typecode;
	SV._collection_data_type = BP._collection_data_type;
	SV._ind = *(sb2*)(BP._collection_indp[_row]);
	SV._data_type_name = BP._type_name;

	res = OCICALL(OCIObjectNew(BP._stmt._env, BP._stmt._errh, BP._stmt._conn._svc_ctx,
	                           OCI_TYPECODE_TABLE, // TabNum => Table
	                           BP._collection_tdo,
	                           NULL,               // dvoid *table
	                           OCI_DURATION_SESSION,
	                           TRUE,               // boolean value
	                           (dvoid**) &SV._valuep));
	oci_check_error(__TROTL_HERE__, BP._stmt._errh, res);

	res = OCICALL(OCICollAssign(BP._stmt._env, BP._stmt._errh, (OCIColl*)BP.valuep[_row], SV._valuep));
	oci_check_error(__TROTL_HERE__, BP._stmt._errh, res);
}

void ConvertorForRead::Fire(const BindParCollectionTabVarchar &BP, SqlCollection &SV)
{
	sword res;
	SV._collection_tdo = BP._collection_tdo;
	SV._collection_typecode = BP._collection_typecode;
	SV._collection_data_type = BP._collection_data_type;
	SV._ind = *(sb2*)(BP._collection_indp[_row]);
	SV._data_type_name = BP._type_name;

	res = OCICALL(OCIObjectNew(BP._stmt._env, BP._stmt._errh, BP._stmt._conn._svc_ctx,
	                           OCI_TYPECODE_TABLE, // TabNum => Table
	                           BP._collection_tdo,
	                           NULL,               // dvoid *table
	                           OCI_DURATION_SESSION,
	                           TRUE,               // boolean value
	                           (dvoid**) &SV._valuep));
	oci_check_error(__TROTL_HERE__, BP._stmt._errh, res);

	res = OCICALL(OCICollAssign(BP._stmt._env, BP._stmt._errh, (OCIColl*)BP.valuep[_row], SV._valuep));
	oci_check_error(__TROTL_HERE__, BP._stmt._errh, res);
}

// TODO this needs to be tested
void ConvertorForWrite::Fire(const SqlDateTime &DT, BindParDate &BP)
{
//	std::cout << "Convert: void Fire(BindParDate, SqlDateTime)\n";

	OCIDate &d = (((OCIDate*)BP.valuep)[_row]);

	d.OCIDateYYYY = DT._date.year;
	d.OCIDateMM = DT._date.month;
	d.OCIDateDD = DT._date.day;
	d.OCIDateTime.OCITimeHH = DT._date.hour;
	d.OCIDateTime.OCITimeMI = DT._date.minute;
	d.OCIDateTime.OCITimeSS = DT._date.second;
	BP.indp[_row] = DT._ind;
};

void ConvertorForWrite::Fire(const SqlClob &CL, BindParClob &BP)
{
	std::cerr << "void ConvertorForWrite::Fire(const SqlClob &CL, BindParClob &BP), row: " << _row << std::endl;
	/* TODO:
	 * - whose errh should I use? BP's or SqlValue's (race)
	 * - no support for temporary lobs here
	 * - should I call OCILobLocatorIsInit here ??
	 */

	sword res = OCICALL(OCILobAssign(BP._env, BP._env._errh, CL._loc, &((OCILobLocator**)BP.valuep)[_row]));
	oci_check_error(__TROTL_HERE__, BP._env._errh, res);

//	((OCILobLocator**)BP.valuep)[_row] = CL._loc;
//	CL._loc = NULL;

//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//		res = OCICALL(OCILobLocatorAssign(_conn._svc_ctx, _conn._env._errh, other._loc, &_loc));
//#else
//		res = OCICALL(OCILobAssign(_conn._env, _conn._env._errh, other._loc, &_loc));	// no support for temporary LOBs
//#endif
	BP.indp[_row] = CL._ind;
};

void ConvertorForWrite::Fire(const SqlBlob &BL, BindParBlob &BP)
{
	//TODO sword res = OCICALL(OCILobAssign(BP._env, BP._env._errh, ((OCILobLocator**)BP.valuep)[_row], &CL._loc));
	//_conn._env, _conn._env._errh, other._loc, &_loc));	// no support for temporary LOBs
	//oci_check_error(__TROTL_HERE__, BP._env._errh, res);

	((OCILobLocator**)BP.valuep)[_row] = BL._loc;
//	BL._loc = NULL;
	BP.indp[_row] = BL._ind;
};

// void ConvertorForWrite::Fire(const SqlInt<int> &BL, BindParCInt &BP)
// {
// 	throw_oci_exception(OciException(__TROTL_HERE__, "Conversion from SqlInt<int> to BindParCInt implemented elsewhere"));
// };

#ifdef ORACLE_HAS_XML
void ConvertorForWrite::Fire(const SqlXML &BL, BindParXML &BP)
{
//	throw_ocipl_exception(OciException(__TROTL_HERE__, "Not implemented yet\n"));
	BP._xmlvaluep[0] = BL.xml;
//	((OCILobLocator**)BP.valuep)[_row] = NULL;
//	BP.indp[_row] = BL._ind;
}
#endif

void Convertor::OnError(const BindPar &BP, SqlValue &val)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Misssing Conversion(%s%d to SqlValue)\n").arg(BP._type_name).arg(BP.value_sz));
};

void Convertor::OnError(const SqlValue &val, BindPar &BP)
{
	throw_oci_exception(OciException(__TROTL_HERE__, "Misssing Conversion(SqlValue to %s%d)\n").arg(BP._type_name).arg(BP.value_sz));
};

};
