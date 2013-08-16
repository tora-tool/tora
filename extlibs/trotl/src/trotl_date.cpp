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
#include "trotl_date.h"
#include "trotl_describe.h"

#include <algorithm>
#include <cctype>       // std::toupper
#include <string>

namespace trotl
{

Util::RegisterInFactory<BindParDate, BindParFactTwoParmSing> regBindDate("date");

Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineDAT(SQLT_DAT);
Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineODT(SQLT_ODT);
Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineDATE(SQLT_DATE);
Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineTimeStamp(SQLT_TIMESTAMP);
Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineTimeStampTZ(SQLT_TIMESTAMP_TZ);
Util::RegisterInFactory<BindParDate, DefineParFactTwoParmSing, int> regDefineTimeStampLTZ(SQLT_TIMESTAMP_LTZ);

BindParDate::BindParDate(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	valuep = (void**) calloc(_cnt, sizeof(OCIDate));

	dty = SQLT_ODT;
	value_sz = sizeof(OCIDate);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub2*)rlenp)[i] = (ub2) value_sz;
	}
	_type_name = ct->typeName();
	
}

BindParDate::BindParDate(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	valuep = (void**) calloc(_cnt, sizeof(OCIDate));

	dty = SQLT_ODT;
	value_sz = sizeof(OCIDate);
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub4*)rlenp)[i] = (ub4) value_sz;
	}
	_type_name = "DATE";
}

tstring BindParDate::get_string(unsigned int row) const
{

	if(!indp[row])
	{
		text str_buf[200];
		ub4 str_len = sizeof(str_buf) / sizeof( *str_buf);

		//const char fmt[] = "YYYY:MM:DD HH24:MI:SS";
		const char lang_fmt[] = "American";

		sword res = OCICALL(OCIDateToText(_env._errh,
		                                  &(((OCIDate*)valuep)[row]),
		                                  (CONST text*) g_TROTL_DEFAULT_DATE_FTM,
		                                  (ub4)strlen(g_TROTL_DEFAULT_DATE_FTM),
		                                  (CONST text*) lang_fmt,
		                                  (ub4) sizeof(lang_fmt)-1,
		                                  (ub4 *)&str_len,
		                                  str_buf
		                                 ));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		str_buf[ min( (str_len+1) , (unsigned)sizeof(str_buf) ) ] = '\0';

		return (const char*)str_buf;
	}
	else
	{
		return "";
	}
}

tstring SqlDateTime::ocidatetime_to_string() const
{
	char buffer[40];

	snprintf(buffer, sizeof(buffer),
	         "%04d.%02d.%02d %02d:%02d:%02d",
	         _date.year, _date.month, _date.day,
	         _date.hour, _date.minute, _date.second);

	return buffer;
}

struct tm* SqlDateTime::to_gmt(void) const
{
	struct tm ltime =
	{
		_date.second,
		_date.minute,
		_date.hour,
		_date.day,
		_date.month,
		_date.year,
		-1,
		-1,
		-1
	};

	time_t time = mktime(&ltime);

	struct tm* gmt = gmtime(&time);
	return gmt;
}

struct tm* SqlDateTime::to_lctime(void) const
{
	struct tm ltime =
	{
		_date.second,
		_date.minute,
		_date.hour,
		_date.day,
		_date.month,
		_date.year,
		-1,
		-1,
		-1
	};

	time_t time = mktime(&ltime);

	struct tm* lct = localtime(&time);
	return lct;
}


// original code starts here

//  tstring OciDate::to_string (void) const // Usage of OCIDateToText would also be possible.
//  {
//    char buffer[40];
//
//    snprintf(buffer, sizeof(buffer),
//	     "%02d.%02d.%04d %02d:%02d:%02d",
//	     OCIDateDD, OCIDateMM, OCIDateYYYY,
//	     OCIDateTime.OCITimeHH, OCIDateTime.OCITimeMI, OCIDateTime.OCITimeSS);
//    return buffer;
//    //return ocidate_to_string(static_cast<OCIdate>(this);
//  }
//
//  tstring OciDate::ocidate_to_string(const OCIDate& date)
//  {
//    char buffer[40];
//    snprintf(buffer, sizeof(buffer),
//	     "%02d.%02d.%04d %02d:%02d:%02d",
//	     date.OCIDateDD, date.OCIDateMM, date.OCIDateYYYY,
//	     date.OCIDateTime.OCITimeHH, date.OCIDateTime.OCITimeMI, date.OCIDateTime.OCITimeSS);
//    return buffer;
//  }
//
//  struct tm* OciDate::to_gmt(void) const
//  {
//    struct tm ltime = {
//      OCIDateTime.OCITimeSS,
//      OCIDateTime.OCITimeMI,
//      OCIDateTime.OCITimeHH,
//      OCIDateDD,
//      OCIDateMM - 1,
//      OCIDateYYYY - 1900,
//      -1,
//      -1,
//      -1
//    };
//
//    time_t time = mktime(&ltime);
//
//    struct tm* gmt = gmtime(&time);
//    return gmt;
//  }
//
//  tm* OciDate::ocidate_to_gmt(const OCIDate& date)
//  {
//    struct tm ltime = {
//      date.OCIDateTime.OCITimeSS,
//      date.OCIDateTime.OCITimeMI,
//      date.OCIDateTime.OCITimeHH,
//      date.OCIDateDD,
//      date.OCIDateMM - 1,
//      date.OCIDateYYYY - 1900,
//      -1,
//      -1,
//      -1
//    };
//
//    time_t time = mktime(&ltime);
//
//    struct tm* gmt = gmtime(&time);
//    return gmt;
//  }
//
//  struct tm* OciDate::to_lctime(void) const
//  {
//    struct tm ltime = {
//      OCIDateTime.OCITimeSS,
//      OCIDateTime.OCITimeMI,
//      OCIDateTime.OCITimeHH,
//      OCIDateDD,
//      OCIDateMM - 1,
//      OCIDateYYYY - 1900,
//      -1,
//      -1,
//      -1
//    };
//
//    time_t time = mktime(&ltime);
//
//    struct tm* lct = localtime(&time);
//    return lct;
//  }
//
//  tm* OciDate::ocidate_to_lctime(const OCIDate& date)
//  {
//    struct tm ltime = {
//      date.OCIDateTime.OCITimeSS,
//      date.OCIDateTime.OCITimeMI,
//      date.OCIDateTime.OCITimeHH,
//      date.OCIDateDD,
//      date.OCIDateMM - 1,
//      date.OCIDateYYYY - 1900,
//      -1,
//      -1,
//      -1
//    };
//
//    time_t time = mktime(&ltime);
//
//    struct tm* lct = localtime(&time);
//    return lct;
//  }
//
//  time_t OciDate::to_time_t(void) const
//  {
//    time_t time = mktime(ocidate_to_lctime(*this));
//
//    return time;
//  }
//
//  static time_t ocidate_to_time_t(const OCIDate& date)
//  {
//  }
//
//  OciDate::OciDate(const OCIDate& date)
//  {
//    OCIDate::operator=(date);
//    ind().set();
//  }
//
//  OciDate::OciDate(OCIError* errh, const char* str)
//  {
//    //while(str && istspace(*str)) ++str; TODO
//
//    if (str && *str) {
//      sword res = OCICALL(OCIDateFromText(errh, (const text*)str, (ub4)strlen(str), NULL/*fmt*/, 0, NULL/*lang*/, 0, this));
//
//      oci_check_error(__TROTL_HERE__, errh, res);
//
//      _ind.set();
//    } else
//      _ind.clear();
//  }
//
//  OciDate::OciDate(struct tm* t)
//  {
//    OCIDateYYYY = t->tm_year + 1900;
//    OCIDateMM = t->tm_mon + 1;
//    OCIDateDD = t->tm_mday;
//    OCIDateTime.OCITimeHH = t->tm_hour;
//    OCIDateTime.OCITimeMI = t->tm_min;
//    OCIDateTime.OCITimeSS = t->tm_sec;
//
//    ind().set();
//  }
//
//  int OciDate::compare(const OciDate& other) const
//  {
//    if (OCIDateYYYY < other.OCIDateYYYY)
//      return -1;
//    else if (OCIDateYYYY > other.OCIDateYYYY)
//      return 1;
//
//    if (OCIDateMM < other.OCIDateMM)
//      return -1;
//    else if (OCIDateMM > other.OCIDateMM)
//      return 1;
//
//    if (OCIDateDD < other.OCIDateDD)
//      return -1;
//    else if (OCIDateDD > other.OCIDateDD)
//      return 1;
//
//    if (OCIDateTime.OCITimeHH < other.OCIDateTime.OCITimeHH)
//      return -1;
//    else if (OCIDateTime.OCITimeHH > other.OCIDateTime.OCITimeHH)
//      return 1;
//
//    if (OCIDateTime.OCITimeMI < other.OCIDateTime.OCITimeMI)
//      return -1;
//    else if (OCIDateTime.OCITimeMI > other.OCIDateTime.OCITimeMI)
//      return 1;
//
//    if (OCIDateTime.OCITimeSS < other.OCIDateTime.OCITimeSS)
//      return -1;
//    else if (OCIDateTime.OCITimeSS > other.OCIDateTime.OCITimeSS)
//      return 1;
//
//    return 0;
//  }
//
//  tstring SqlDateTime::to_string(void) const
//  {
//    char buffer[40];
//
//    snprintf(buffer, sizeof(buffer),
//	     "%02d.%02d.%04d %02d:%02d:%02d",
//	     _date.day, _date.month, (_date.century-100)*100+(_date.year-100),
//	     _date.hour-1, _date.minute-1, _date.second-1);
//
//    return buffer;
//  }
//

}
