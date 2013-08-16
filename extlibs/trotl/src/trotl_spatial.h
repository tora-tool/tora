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

#ifndef TROTL_SPATIAL_H_
#define TROTL_SPATIAL_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_conn.h"
#include "trotl_var.h"

#define SDO_GEOMETRY               "MDSYS.SDO_GEOMETRY"

namespace trotl
{

	/*************************************************************************/
	/* Type definitions                                                      */
	/*************************************************************************/
	struct sdo_point_type
	{
		OCINumber x;
		OCINumber y;
		OCINumber z;
	};
	typedef struct sdo_point_type sdo_point_type;

	typedef OCIArray sdo_elem_info_array;
	typedef OCIArray sdo_ordinate_array;

	struct sdo_geometry
	{
		OCINumber      sdo_gtype;
		OCINumber      sdo_srid;
		sdo_point_type sdo_point;
		OCIArray       *sdo_elem_info;
		OCIArray       *sdo_ordinates;
	};

	typedef struct sdo_geometry SDO_GEOMETRY_TYPE;

	/***
	 ** Indicator structures for SDO_GEOMETRY_TYPE
	 ***/
	struct sdo_point_type_ind
	{
		OCIInd _atomic;
		OCIInd x;
		OCIInd y;
		OCIInd z;
	};
	typedef struct sdo_point_type_ind sdo_point_type_ind;

	struct SDO_GEOMETRY_ind
	{
		OCIInd                    _atomic;
		OCIInd                    sdo_gtype;
		OCIInd                    sdo_srid;
		struct sdo_point_type_ind sdo_point;
		OCIInd                    sdo_elem_info;
		OCIInd                    sdo_ordinates;
	};
	typedef struct SDO_GEOMETRY_ind SDO_GEOMETRY_ind;

	struct TROTL_EXPORT BindParSpatial: public BindPar
	{
		friend struct ConvertorForRead;
		friend struct ConvertorForWrite;

		// TODO remember OCIConn or at least svcctx in this class
		BindParSpatial(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
		BindParSpatial(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

		virtual ~BindParSpatial()
		{
			// TODO free _any_indp, _oan_buffer
		}

		virtual tstring get_string(unsigned int row) const;

		void init();

		virtual void define_hook();
		virtual void bind_hook();


//private:

		OCIType *_spatialtdo;
		SDO_GEOMETRY_TYPE **_global_geom_obj;/* spatial object buffer */
		SDO_GEOMETRY_ind  **_global_geom_ind; /* Object indicator */
	protected:
		BindParSpatial(const BindParSpatial &other);
	};

};
#endif /*TROTL_SPATIAL_H_*/
