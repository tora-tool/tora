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

//TODO use SQLT_ODT for dates whithout time

#ifndef TROTL_DATE_H_
#define TROTL_DATE_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_var.h"

extern "C" {
#include <time.h>
}

namespace trotl
{

/*
 * This datatype is registered as SQLT_DAT but defined as SQLT_ODT
 * I use SQLT_ODT for SQLT_DAT
 */
struct TROTL_EXPORT BindParDate: public BindPar
{
	BindParDate(unsigned int pos, SqlStatement &stmt, DescribeColumn*);
	BindParDate(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParDate()
	{}

	virtual tstring get_string(unsigned int row) const;

protected:
	BindParDate(const BindParDate &other);
};

/// wrapper for internal Oracle DATE type
struct TROTL_EXPORT SqlDateTime : public SqlValue
{
	struct ocidatetime
	{
//		ub1	century;
		ub2 year;
		ub1	month;
		ub1	day;
		ub1	hour;
		ub1	minute;
		ub1	second;
	};

	SqlDateTime() {}

//	SqlDateTime(const ocidatetime& date)
//	:	_date(date)
//	{
//	}

//	ocidatetime* get_ref() {return &_date;}

	tstring str() const
	{
		if (_ind.is_not_null())
			return ocidatetime_to_string(/*_date*/);
		else
			return tstring();
	}

	tstring to_string(void) const;
	tstring ocidatetime_to_string(void) const;
	struct tm* to_gmt(void) const;
	struct tm* to_lctime(void) const;
//	static :tstring ocidatetime_to_string(const ocidatetime& date);

protected:
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;
	ocidatetime _date;
};


//original code starts here

/// wrapper for OCI DATE type
//struct OciDate : public OCIDate, public SqlValue
//{
//protected:
//	:tstring to_string(void) const;
//	struct tm* to_gmt(void) const;
//	struct tm* to_lctime(void) const;
//	time_t to_time_t(void) const;
//
//public:
//	static :tstring ocidate_to_string(const OCIDate& date);
//	static tm* ocidate_to_gmt(const OCIDate& date);
//	static tm* ocidate_to_lctime(const OCIDate& date);
//	static time_t ocidate_to_time_t(const OCIDate& date);
//
//	OciDate() {}
//	OciDate(const OCIDate& date);
//	OciDate(OCIError* errh, const char* str);
//	OciDate(struct tm* t);
//
//	:tstring str() const
//	{
//		if (_ind.is_not_null())
//			return to_string();
//		else
//			return :tstring();
//	}
//
//	operator :tstring(void) const { return str(); }
//
//	int get_int_date() const
//	{
//		if (_ind.is_not_null())
//			return ((OCIDateYYYY*100) + OCIDateMM)*100 + OCIDateDD;
//		else
//			return 0;
//	}
//
//	operator int(void) const { return get_int_date(); }
//
//	struct tm* get_gmt() const
//	{
//		if (_ind.is_not_null())
//			return to_gmt();
//		else
//			return 0;
//	}
//
//	operator tm*(void) const { return get_gmt(); }
//
//	struct tm* get_lctime() const
//	{
//		if (_ind.is_not_null())
//			return to_lctime();
//		else
//			return 0;
//	}
//
//	time_t get_time_t() const
//	{
//		if (_ind.is_not_null())
//			return to_time_t();
//		else
//			return 0;
//	}
//	operator time_t(void) { return get_time_t(); };
//
//	int compare(const OciDate& other) const;
//
//	bool operator==(const OciDate& other) const { return compare(other); }
//};


/// OCI DATE type bind variable array
//struct OciDateArray : public SqlIndArray
//{
//	OciDateArray(int size=g_OCIPL_BULK_ROWS)
//	:	SqlIndArray(size),
//	_buffers((OCIDate*)malloc(size*sizeof(OCIDate)))
//	{
//	}
//
//	~OciDateArray()
//	{
//		free(_buffers);
//	}
//
//	OCIDate* get_ref(int row) {return &_buffers[row];}
//
//	:tstring str(int row) const
//	{
//		if (_pind[row].is_not_null())
//			return OciDate::ocidate_to_string(_buffers[row]);
//		else
//			return tstring();
//	}
//
//	int get_int_date(int row) const
//	{
//		if (_pind[row].is_not_null()) {
//			const OCIDate& date = _buffers[row];
//			return ((date.OCIDateYYYY*100) + date.OCIDateMM)*100 + date.OCIDateDD;
//		} else
//			return 0;
//	}
//
//	struct tm* get_gmt(int row) const
//	{
//		if (_pind[row].is_not_null())
//			return OciDate::ocidate_to_gmt(_buffers[row]);
//		else
//			return 0;
//	}
//
//	struct tm* get_lctime(int row) const
//	{
//		if (_pind[row].is_not_null())
//			return OciDate::ocidate_to_lctime(_buffers[row]);
//		else
//			return 0;
//	}
//
//	time_t get_time_t(int row) const
//	{
//		if (_pind[row].is_not_null())
//			return OciDate::ocidate_to_time_t(_buffers[row]);
//		else
//			return 0;
//	}
//
//protected:
//	OCIDate* _buffers;
//};


//struct ocidatetime {
//	ub1	century;
//	ub1	year;
//	ub1	month;
//	ub1	day;
//	ub1	hour;
//	ub1	minute;
//	ub1	second;
//};


/// wrapper for internal Oracle DATE type

//struct SqlDateTimeArray : public SqlIndArray
//{
//	SqlDateTimeArray(int size=g_OCIPL_BULK_ROWS)
//	:	SqlIndArray(size),
//	_buffers((ocidatetime*)malloc(size*sizeof(ocidatetime)))
//	{}
//
//	~SqlDateTimeArray()
//	{
//		free(_buffers);
//	}
//
//	ocidatetime* get_ref(int row) {return &_buffers[row];}
//
//	tstring str(int row) const
//	{
//		if (_pind[row].is_not_null())
//			return SqlDateTime::ocidatetime_to_string(_buffers[row]);
//		else
//			return tstring();
//	}
//
//protected:
//	ocidatetime* _buffers;
//};

}

#endif
