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


#ifndef TROTL_CONVERTOR_H_
#define TROTL_CONVERTOR_H_

#include "trotl_export.h"
#include "trotl_stat.h"
#include "trotl_string.h"
#include "trotl_int.h"
#include "trotl_lob.h"
#include "trotl_date.h"
#include "trotl_collection.h"

#ifdef ORACLE_HAS_XML
#include "trotl_xml.h"
#endif

#include "loki/MultiMethods.h"

namespace trotl
{

struct TROTL_EXPORT Convertor
{
	void OnError(const BindPar &BP, SqlValue &SV);
	void OnError(const SqlValue &SV, BindPar &BP);
};

struct TROTL_EXPORT ConvertorForRead: public Convertor
{
	/*
	 * This the only way, how can I pass row argument
	 */
	ConvertorForRead(unsigned int row) : _row(row) {};

	void Fire(const BindParNumber &BP, SqlInt<int> &SV);
	void Fire(const BindParNumber &BP, SqlInt<unsigned int> &SV);
	void Fire(const BindParNumber &BP, SqlInt<long> &SV);
	void Fire(const BindParNumber &BP, SqlInt<unsigned long> &SV);
	void Fire(const BindParNumber &BP, SqlInt<double> &SV);
	void Fire(const BindParNumber &BP, SqlInt<float> &SV);
	void Fire(const BindParNumber &BP, SqlNumber &SV);
	void Fire(const BindParNumber &BP, SqlCollection &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParNumber &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParNumber &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParNumber &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
#ifdef ORACLE_HAS_XML
	void Fire(const BindParNumber &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	};
#endif

	void Fire(const BindParDate &BP, SqlDateTime &SV);
	void Fire(const BindParDate &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<unsigned int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<unsigned long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<double> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlInt<float> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParDate &BP, SqlCollection &SV)
	{
		OnError(BP, SV);
	};
#ifdef ORACLE_HAS_XML
	void Fire(const BindParDate &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	};
#endif

	void Fire(const BindParClob &BP, SqlClob &SV);
	void Fire(const BindParClob &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<unsigned int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<unsigned long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<double> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlInt<float> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParClob &BP, SqlCollection &SV)
	{
		OnError(BP, SV);
	};
#ifdef ORACLE_HAS_XML
	void Fire(const BindParClob &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	}; // TODO is any convesion possible here?
#endif

	void Fire(const BindParBlob &BP, SqlBlob &SV);
	void Fire(const BindParBlob &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<unsigned int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<unsigned long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<double> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlInt<float> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParBlob &BP, SqlCollection &SV)
	{
		OnError(BP, SV);
	};
#ifdef ORACLE_HAS_XML
	void Fire(const BindParBlob &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	};
#endif

	void Fire(const BindParCollectionTabNum &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabNum &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabNum &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabNum &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabNum &BP, SqlCollection &SV);
#ifdef ORACLE_HAS_XML
	void Fire(const BindParCollectionTabNum &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	};
#endif

	void Fire(const BindParCollectionTabVarchar &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabVarchar &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabVarchar &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabVarchar &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParCollectionTabVarchar &BP, SqlCollection &SV);
#ifdef ORACLE_HAS_XML
	void Fire(const BindParCollectionTabVarchar &BP, SqlXML &SV)
	{
		OnError(BP, SV);
	};
#endif

#ifdef ORACLE_HAS_XML
	void Fire(const BindParXML &BP, SqlDateTime &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlBlob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlClob &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<unsigned int> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<unsigned long> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<double> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlInt<float> &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlNumber &SV)
	{
		OnError(BP, SV);
	};
	void Fire(const BindParXML &BP, SqlXML &SV);
	void Fire(const BindParXML &BP, SqlCollection &SV)
	{
		OnError(BP, SV);
	};
#endif


private:
	ConvertorForRead();
	unsigned int _row;
};

typedef LOKI_TYPELIST_6(const BindParNumber,
                        const BindParClob,
                        const BindParBlob,
                        const BindParDate,
                        const BindParCollectionTabNum,
                        const BindParCollectionTabVarchar)
TL_DispatcherForReadInput;

typedef LOKI_TYPELIST_5(SqlClob,
                        SqlBlob,
                        SqlDateTime,
                        SqlNumber,
                        SqlCollection)
TL_DispatcherForReadOutput;

#ifdef ORACLE_HAS_XML
typedef ::Loki::TL::Append<TL_DispatcherForReadInput, const BindParXML>::Result TL_DispatcherForReadInputX;
typedef ::Loki::TL::Append<TL_DispatcherForReadOutput, SqlXML>::Result TL_DispatcherForReadOutputX;
#else
typedef TL_DispatcherForReadInput TL_DispatcherForReadInputX;
typedef TL_DispatcherForReadOutput TL_DispatcherForReadOutputX;
#endif

typedef ::Loki::StaticDispatcher <
ConvertorForRead,
const BindPar,
TL_DispatcherForReadInputX,
false,
SqlValue,
TL_DispatcherForReadOutputX,
void
> DispatcherForRead;

struct TROTL_EXPORT ConvertorForWrite: public Convertor
{
	/*
	 * This the only way, how can I pass row argument
	 */
	ConvertorForWrite(unsigned int row) : _row(row) {};

	void Fire(const SqlDateTime &SV, BindParDate &BP);
	void Fire(const SqlBlob &SV, BindParBlob &BP);
	void Fire(const SqlClob &SV, BindParClob &BP);
	//void Fire(const SqlInt<int> &SV, BindParCInt &BP);
#ifdef ORACLE_HAS_XML
	void Fire(const SqlXML &SV, BindParXML &BP);
#endif

	void Fire(const SqlDateTime &SV, BindParClob &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlDateTime &SV, BindParBlob &BP)
	{
		OnError(SV, BP);
	}
	//void Fire(const SqlDateTime &SV, BindParCInt &BP) { OnError(SV, BP); };
#ifdef ORACLE_HAS_XML
	void Fire(const SqlDateTime &SV, BindParXML &BP)
	{
		OnError(SV, BP);
	};
#endif

	void Fire(const SqlBlob &SV, BindParDate &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlBlob &SV, BindParClob &BP)
	{
		OnError(SV, BP);
	};
	//void Fire(const SqlBlob &SV, BindParCInt &BP) { OnError(SV, BP); };
#ifdef ORACLE_HAS_XML
	void Fire(const SqlBlob &SV, BindParXML &BP)
	{
		OnError(SV, BP);
	};
#endif

	void Fire(const SqlClob &SV, BindParDate &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlClob &SV, BindParBlob &BP)
	{
		OnError(SV, BP);
	};
	//void Fire(const SqlClob &SV, BindParCInt &BP) { OnError(SV, BP); };
#ifdef ORACLE_HAS_XML
	void Fire(const SqlClob &SV, BindParXML &BP)
	{
		OnError(SV, BP);
	};
#endif

	void Fire(const SqlInt<int> &SV, BindParDate &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlInt<int> &SV, BindParBlob &BP)
	{
		OnError(SV, BP);
	}
	void Fire(const SqlInt<int> &SV, BindParClob &BP)
	{
		OnError(SV, BP);
	};
#ifdef ORACLE_HAS_XML
	void Fire(const SqlInt<int> &SV, BindParXML &BP)
	{
		OnError(SV, BP);
	};
#endif

#ifdef ORACLE_HAS_XML
	void Fire(const SqlXML &SV, BindParDate &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlXML &SV, BindParBlob &BP)
	{
		OnError(SV, BP);
	};
	void Fire(const SqlXML &SV, BindParClob &BP)
	{
		OnError(SV, BP);
	};
	//void Fire(const SqlXML &SV, BindParCInt &BP) { OnError(SV, BP); };
#endif

private:
	ConvertorForWrite();
	unsigned int _row;
};

typedef ::Loki::StaticDispatcher
<
ConvertorForWrite,
const SqlValue,

#ifdef ORACLE_HAS_XML
LOKI_TYPELIST_5(const SqlClob,const SqlBlob,const SqlDateTime,const SqlInt<int>, const SqlXML),
#else
LOKI_TYPELIST_4(const SqlClob,const SqlBlob,const SqlDateTime,const SqlInt<int>),
#endif

false,
BindPar,

#ifdef ORACLE_HAS_XML
LOKI_TYPELIST_4(BindParClob, BindParBlob, BindParDate, BindParXML),
#else
LOKI_TYPELIST_3(BindParClob, BindParBlob, BindParDate),
#endif

void
> DispatcherForWrite;

};

#endif /*TROTL_CONVERTOR_H_*/
