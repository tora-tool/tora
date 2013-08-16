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
#include "trotl_spatial.h"
#include "trotl_stat.h"
#include "trotl_describe.h"


namespace trotl
{

// TODO
//Util::RegisterInFactory<BindParSpatial, BindParFactTwoParmSing> regBindXML("MDSYS.SDO_GEOMETRY");

Util::RegisterInFactory<BindParSpatial, CustDefineParFactTwoParmSing> regCustDefineNTY_SPATIAL("MDSYS.SDO_GEOMETRY");

BindParSpatial::BindParSpatial(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
	, _global_geom_obj(NULL)
	, _global_geom_ind(NULL)
{
	dty =  SQLT_NTY;
	_type_name = ct->typeName();
	value_sz = sizeof(SDO_GEOMETRY_TYPE*);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub2*)rlenp)[i] = (ub2) value_sz;
	}

	init();
}

BindParSpatial::BindParSpatial(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindPar(pos, stmt, decl)
	, _global_geom_obj(NULL)
	, _global_geom_ind(NULL)
{
	dty =  SQLT_NTY;
	_type_name = "MDSYS.SDO_GEOMETRY";
	value_sz = sizeof(SDO_GEOMETRY_TYPE*);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub4*)rlenp)[i] = (ub4) value_sz;
	}

	init();
}

void BindParSpatial::init()
{
	sword res;
	_spatialtdo = NULL;
	_global_geom_obj = (SDO_GEOMETRY_TYPE**) calloc(_cnt, sizeof(SDO_GEOMETRY_TYPE*));
	_global_geom_ind = (SDO_GEOMETRY_ind**) calloc(_cnt, sizeof(SDO_GEOMETRY_ind*));

	res = OCICALL(OCITypeByName(_stmt._env, _stmt._errh, _stmt._conn._svc_ctx,
	                            (const oratext*)"MDSYS", (ub4)strlen("MDSYS"),
	                            (const oratext*)"SDO_GEOMETRY", (ub4)strlen("SDO_GEOMETRY"),
	                            0, 0,
	                            OCI_DURATION_SESSION, OCI_TYPEGET_ALL,
	                            (OCIType**) &_spatialtdo));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);

	if(_spatialtdo == NULL)
		throw_oci_exception(OciException(__TROTL_HERE__, "Unknown datatype in the database: MDSYS.SDO_GEOMETRY"));
}

void BindParSpatial::define_hook()
{
	sword res = OCICALL(OCIDefineObject(defnpp, _stmt._errh,
	                                    _spatialtdo,
	                                    (dvoid **) &(_global_geom_obj[0]),
	                                    (ub4 *) 0,
	                                    (dvoid **) &(_global_geom_ind[0]),
	                                    (ub4 *) 0));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
// 	// TODO OCIDefineArrayOfStruct here ??
}

void BindParSpatial::bind_hook()
{
	//TODO
	throw_oci_exception(OciException(__TROTL_HERE__, "Not implemented yet"));
}

// TODO
tstring BindParSpatial::get_string(unsigned int row) const
{
	sword res;
	int global_nelems;
	int global_nords;
	int gtype;
	tostream _stringrepres;
	double x, y, z;
	
	if(_global_geom_ind[row]->_atomic == OCI_IND_NULL)
	{
		return "NULL";
	}

	
	/* Get the size of the sdo_elem_info array */
	res = OCICALL(OCICollSize(_stmt._env,
				  _stmt._errh,
				  (OCIColl *)(_global_geom_obj[row]->sdo_elem_info),
				  &global_nelems));
	_stringrepres << " sdo_elem_info size: " << global_nelems;

	/* Get the size of the ordinates array */
	res = OCICALL(OCICollSize(_stmt._env,
				  _stmt._errh,
				  (OCIColl *)(_global_geom_obj[row]->sdo_ordinates),
				  &global_nords));
	_stringrepres << " sdo_ordinates size: " << global_nords;

	/* sc_ops_get_gtype */
	res = OCICALL(OCINumberToInt(_stmt._errh,
				     &(_global_geom_obj[row]->sdo_gtype),
				     (uword)sizeof(int), OCI_NUMBER_SIGNED,
				     (dvoid *)&gtype));
	_stringrepres << " -- gtype: " << gtype;

	/* sc_ops_get_sdo_point */
	if (_global_geom_ind[row]->sdo_point._atomic == OCI_IND_NOTNULL)
	{
		if (_global_geom_ind[row]->sdo_point.x == OCI_IND_NOTNULL)
		{
			res = OCICALL(OCINumberToReal(_stmt._errh,
						      &(_global_geom_obj[row]->sdo_point.x),
						      (uword)sizeof(double),
						      (dvoid *)&x));
			
			_stringrepres << " -- sdo_point.X: " << x;
		}

		if (_global_geom_ind[row]->sdo_point.y == OCI_IND_NOTNULL)
		{
			res = OCICALL(OCINumberToReal(_stmt._errh,
							&(_global_geom_obj[row]->sdo_point.y),
							(uword)sizeof(double),
							(dvoid *)&y));

			_stringrepres << " -- sdo_point.Y: " << y;
		}

		if (_global_geom_ind[row]->sdo_point.z == OCI_IND_NOTNULL)
		{
			res = OCICALL(OCINumberToReal(_stmt._errh,
							&(_global_geom_obj[row]->sdo_point.z),
							(uword)sizeof(double),
							(dvoid *)&z));

			_stringrepres << " -- sdo_point.Z: " << z;
		}
	}
	else
		_stringrepres << "-- sdo_point IS NULL";

	/* Loop through and print all the elements for this geometry */
	for(unsigned i=0; i<global_nelems; i+=3)
	{
		boolean        exists;
		OCINumber      *oci_number;
		ub4            element_type;

		res = OCICALL(OCICollGetElem(_stmt._env,
					     _stmt._errh,
					     (OCIColl *)(_global_geom_obj[row]->sdo_elem_info),
					     (sb4)(i), // +1 ?? TODO
					     (boolean *)&exists,
					     (dvoid **)&oci_number, (dvoid **)0));
			
		res = OCICALL(OCINumberToInt(_stmt._errh,
					     oci_number,
					     (uword)sizeof(ub4), OCI_NUMBER_UNSIGNED,
					     (dvoid *)&element_type));
			
		_stringrepres << "-- element type: " << element_type;

		switch (element_type)
		{
		case 1:
			//sc_ops_get_type1();
			break;
		case 2:
			//sc_ops_get_type2();
			break;
		case 3:
			//sc_ops_get_type3();
			break;
		case 4:
			//sc_ops_get_type4();
			break;
		case 5:
			//sc_ops_get_type5();
			break;
		}		
	}
	
	return _stringrepres.str();
};

};
