////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// Copyright (c) 2005 by Peter Kuemmel
// This code DOES NOT accompany the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
//
// Code covered by the MIT License
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////
#ifndef LOKI_FACTORYPARM_INC_
#define LOKI_FACTORYPARM_INC_

// $Header: /cvsroot/loki-lib/loki/include/loki/Factory.h,v 1.16 2006/03/08 16:41:38 syntheticpp Exp $ /cvsroot/loki-lib/loki/include/loki/Factory.h,v 1.15 2006/01/19 23:11:55 lfittl Exp $

#include "LokiTypeInfo.h"
#include "Functor.h"
#include "AssocVector.h"
#include "SmallObj.h"
#include "Sequence.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
//unreachable code if OnUnknownType throws an exception
#endif

///  \defgroup FactoryGroup Factory

namespace Loki
{

////////////////////////////////////////////////////////////////////////////////
///  \class DefaultFactoryError
///
///  \ingroup FactoryGroup
///  Manages the "Unknown Type" error in an object factory
////////////////////////////////////////////////////////////////////////////////

    template <typename IdentifierType, class AbstractProduct>
    struct DefaultFactoryError
    {
        struct Exception : public std::exception
        {
            const char* what() const throw() { return "Unknown Type"; }
        };
        
        static AbstractProduct* OnUnknownType(IdentifierType)
        {
            throw Exception();
        }
    };
    
    
#define LOKI_ENABLE_NEW_FACTORY_CODE
#ifdef LOKI_ENABLE_NEW_FACTORY_CODE


////////////////////////////////////////////////////////////////////////////////
// class template FunctorImpl
////////////////////////////////////////////////////////////////////////////////

    struct FactoryImplBase 
    {
        typedef EmptyType Parm1;
        typedef EmptyType Parm2;
        typedef EmptyType Parm3;
        typedef EmptyType Parm4;
        typedef EmptyType Parm5;
        typedef EmptyType Parm6;
        typedef EmptyType Parm7;
        typedef EmptyType Parm8;
        typedef EmptyType Parm9;
        typedef EmptyType Parm10;
        typedef EmptyType Parm11;
        typedef EmptyType Parm12;
        typedef EmptyType Parm13;
        typedef EmptyType Parm14;
        typedef EmptyType Parm15;
    };

    template <typename AP, typename Id, typename TList >
    struct FactoryImpl;

    template<typename AP, typename Id>
    struct FactoryImpl<AP, Id, NullType>
                : public FactoryImplBase
    {
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id & id ) = 0;
    };
template <typename AP, typename Id, typename P1 >
    struct FactoryImpl<AP,Id, Seq<P1> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2 >
    struct FactoryImpl<AP, Id, Seq<P1, P2> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2,typename P3 >
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2,typename P3,typename P4 >
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4 ) = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5 >
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5 ) = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6 )
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7 )
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12,Parm13)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13,typename P14>
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        typedef typename TypeTraits<P14>::ParameterType Parm14;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm8,Parm10,
                                Parm11,Parm12,Parm13,Parm14)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13,typename P14,typename P15 >
    struct FactoryImpl<AP, Id, Seq<P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15> >
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        typedef typename TypeTraits<P14>::ParameterType Parm14;
        typedef typename TypeTraits<P15>::ParameterType Parm15;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12,Parm13,Parm14,Parm15 )
        = 0;
    };

#ifndef LOKI_DISABLE_TYPELIST_MACROS

    template <typename AP, typename Id, typename P1 >
    struct FactoryImpl<AP,Id, LOKI_TYPELIST_1( P1 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2 >
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_2( P1, P2 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2,typename P3 >
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_3( P1, P2, P3 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3 ) = 0;
    };

    template<typename AP, typename Id, typename P1,typename P2,typename P3,typename P4 >
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_4( P1, P2, P3, P4 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4 ) = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5 >
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_5( P1, P2, P3, P4, P5 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5 ) = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_6( P1, P2, P3, P4, P5, P6 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6 )
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_7( P1, P2, P3, P4, P5, P6, P7 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7 )
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_8( P1, P2, P3, P4, P5, P6, P7, P8 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_9( P1, P2, P3, P4, P5, P6, P7, P8, P9 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_10( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_11( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_12( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_13( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12,Parm13)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13,typename P14>
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_14( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        typedef typename TypeTraits<P14>::ParameterType Parm14;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm8,Parm10,
                                Parm11,Parm12,Parm13,Parm14)
        = 0;
    };

    template<typename AP, typename Id,
    typename P1,typename P2,typename P3,typename P4,typename P5,
    typename P6,typename P7,typename P8,typename P9,typename P10,
    typename P11,typename P12,typename P13,typename P14,typename P15 >
    struct FactoryImpl<AP, Id, LOKI_TYPELIST_15( P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15 )>
                : public FactoryImplBase
    {
        typedef typename TypeTraits<P1>::ParameterType Parm1;
        typedef typename TypeTraits<P2>::ParameterType Parm2;
        typedef typename TypeTraits<P3>::ParameterType Parm3;
        typedef typename TypeTraits<P4>::ParameterType Parm4;
        typedef typename TypeTraits<P5>::ParameterType Parm5;
        typedef typename TypeTraits<P6>::ParameterType Parm6;
        typedef typename TypeTraits<P7>::ParameterType Parm7;
        typedef typename TypeTraits<P8>::ParameterType Parm8;
        typedef typename TypeTraits<P9>::ParameterType Parm9;
        typedef typename TypeTraits<P10>::ParameterType Parm10;
        typedef typename TypeTraits<P11>::ParameterType Parm11;
        typedef typename TypeTraits<P12>::ParameterType Parm12;
        typedef typename TypeTraits<P13>::ParameterType Parm13;
        typedef typename TypeTraits<P14>::ParameterType Parm14;
        typedef typename TypeTraits<P15>::ParameterType Parm15;
        virtual ~FactoryImpl() {}
        virtual AP* CreateObject(const Id& id,Parm1, Parm2, Parm3, Parm4, Parm5,
                                Parm6, Parm7, Parm8, Parm9,Parm10,
                                Parm11,Parm12,Parm13,Parm14,Parm15 )
        = 0;
    };

#endif //LOKI_DISABLE_TYPELIST_MACROS


////////////////////////////////////////////////////////////////////////////////
///  \class Factory
///
///  \ingroup FactoryGroup
///  Implements a generic object factory.     
///  
///  Create functions can have up to 15 parameters.
///    
////////////////////////////////////////////////////////////////////////////////
    template
    <
        class AbstractProduct,
        typename IdentifierType,
        typename CreatorParmTList = NullType,
        template<typename, class> class FactoryErrorPolicy = DefaultFactoryError
    >
    class Factory : public FactoryErrorPolicy<IdentifierType, AbstractProduct>
    {
        typedef FactoryImpl< AbstractProduct, IdentifierType, CreatorParmTList > Impl;

        typedef typename Impl::Parm1 Parm1;
        typedef typename Impl::Parm2 Parm2;
        typedef typename Impl::Parm3 Parm3;
        typedef typename Impl::Parm4 Parm4;
        typedef typename Impl::Parm5 Parm5;
        typedef typename Impl::Parm6 Parm6;
        typedef typename Impl::Parm7 Parm7;
        typedef typename Impl::Parm8 Parm8;
        typedef typename Impl::Parm9 Parm9;
        typedef typename Impl::Parm10 Parm10;
        typedef typename Impl::Parm11 Parm11;
        typedef typename Impl::Parm12 Parm12;
        typedef typename Impl::Parm13 Parm13;
        typedef typename Impl::Parm14 Parm14;
        typedef typename Impl::Parm15 Parm15;

        typedef Functor<AbstractProduct*, CreatorParmTList> ProductCreator;

        typedef AssocVector<IdentifierType, ProductCreator> IdToProductMap;

        IdToProductMap associations_;

    public:

        Factory()
            : associations_()
        {
        }

        ~Factory()
        {
            associations_.erase(associations_.begin(), associations_.end());
        }

        bool Register(const IdentifierType& id, ProductCreator creator)
        {
            return associations_.insert(
                         typename IdToProductMap::value_type(id, creator)).second != 0;
        }

        template <class PtrObj, typename CreaFn>
        bool Register(const IdentifierType& id, const PtrObj& p, CreaFn fn)
        {
            ProductCreator creator( p, fn );
            return associations_.insert(
                typename IdToProductMap::value_type(id, creator)).second != 0;

        }

        bool Unregister(const IdentifierType& id)
        {
            return associations_.erase(id) != 0;
        }

        std::vector<IdentifierType> RegisteredIds()
        {
            std::vector<IdentifierType> ids;
            for(typename IdToProductMap::iterator it = associations_.begin(); 
                it != associations_.end(); ++it)
            {
                ids.push_back(it->first);
            }
            return ids;
        }

        AbstractProduct* CreateObject(const IdentifierType& id)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5,
                                            Parm6 p6)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5,
                                            Parm6 p6, Parm7 p7 )
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5,
                                            Parm6 p6, Parm7 p7, Parm8 p8)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5,
                                            Parm6 p6, Parm7 p7, Parm8 p8, Parm9 p9)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9 );
            return this->OnUnknownType(id);
        }
        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1 p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5 p5,
                                            Parm6 p6, Parm7 p7, Parm8 p8, Parm9 p9,Parm10 p10)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1  p1, Parm2 p2, Parm3 p3, Parm4 p4, Parm5  p5,
                                            Parm6  p6, Parm7 p7, Parm8 p8, Parm9 p9, Parm10 p10,
                                            Parm11 p11)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1  p1,  Parm2  p2, Parm3 p3, Parm4 p4, Parm5  p5,
                                            Parm6  p6,  Parm7  p7, Parm8 p8, Parm9 p9, Parm10 p10,
                                            Parm11 p11, Parm12 p12)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1  p1,  Parm2  p2,  Parm3  p3, Parm4 p4, Parm5  p5,
                                            Parm6  p6,  Parm7  p7,  Parm8  p8, Parm9 p9, Parm10 p10,
                                            Parm11 p11, Parm12 p12, Parm13 p13)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1  p1,  Parm2  p2,  Parm3  p3,  Parm4  p4, Parm5  p5,
                                            Parm6  p6,  Parm7  p7,  Parm8  p8,  Parm9  p9, Parm10 p10,
                                            Parm11 p11, Parm12 p12, Parm13 p13, Parm14 p14)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14 );
            return this->OnUnknownType(id);
        }

        AbstractProduct* CreateObject(const IdentifierType& id,
                                            Parm1  p1,  Parm2  p2,  Parm3  p3,  Parm4  p4,  Parm5  p5,
                                            Parm6  p6,  Parm7  p7,  Parm8  p8,  Parm9  p9,  Parm10 p10,
                                            Parm11 p11, Parm12 p12, Parm13 p13, Parm14 p14, Parm15 p15)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
                return (i->second)( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15 );
            return this->OnUnknownType(id);
        }

    };


#else

    template
    <
        class AbstractProduct, 
        typename IdentifierType,
        typename ProductCreator = AbstractProduct* (*)(),
        template<typename, class>
            class FactoryErrorPolicy = DefaultFactoryError
    >
    class Factory 
        : public FactoryErrorPolicy<IdentifierType, AbstractProduct>
    {
    public:
        bool Register(const IdentifierType& id, ProductCreator creator)
        {
            return associations_.insert(
                typename IdToProductMap::value_type(id, creator)).second;
        }
        
        bool Unregister(const IdentifierType& id)
        {
            return associations_.erase(id) == 1;
        }
        
        AbstractProduct* CreateObject(const IdentifierType& id)
        {
            typename IdToProductMap::iterator i = associations_.find(id);
            if (i != associations_.end())
            {
                return (i->second)();
            }
            return this->OnUnknownType(id);
        }
        
    private:
        typedef AssocVector<IdentifierType, ProductCreator> IdToProductMap;
        IdToProductMap associations_;
    };


#endif //#define ENABLE_NEW_FACTORY_CODE

////////////////////////////////////////////////////////////////////////////////
///  \class CloneFactory
///
///  \ingroup FactoryGroup
///  Implements a generic cloning factory
////////////////////////////////////////////////////////////////////////////////

    template
    <
        class AbstractProduct, 
        class ProductCreator = 
            AbstractProduct* (*)(const AbstractProduct*),
        template<typename, class>
            class FactoryErrorPolicy = DefaultFactoryError
    >
    class CloneFactory
        : public FactoryErrorPolicy<TypeInfo, AbstractProduct>
    {
    public:
        bool Register(const TypeInfo& ti, ProductCreator creator)
        {
            return associations_.insert(
                typename IdToProductMap::value_type(ti, creator)).second;
        }
        
        bool Unregister(const TypeInfo& id)
        {
            return associations_.erase(id) == 1;
        }
        
        AbstractProduct* CreateObject(const AbstractProduct* model)
        {
            if (model == 0) return 0;
            
            typename IdToProductMap::iterator i = 
                associations_.find(typeid(*model));
            if (i != associations_.end())
            {
                return (i->second)(model);
            }
            return this->OnUnknownType(typeid(*model));
        }
        
    private:
        typedef AssocVector<TypeInfo, ProductCreator> IdToProductMap;
        IdToProductMap associations_;
    };
} // namespace Loki

#ifdef _MSC_VER
#pragma warning( pop ) 
#endif

////////////////////////////////////////////////////////////////////////////////
// Change log:
// June 20,    2001: ported by Nick Thurn to gcc 2.95.3. Kudos, Nick!!!
// May 08,     2002: replaced const_iterator with iterator so that self-modifying
//                   ProductCreators are supported. Also, added a throw()
//                   spec to what(). Credit due to Jason Fischl.
// February 2, 2003: fixed dependent names - credit due to Rani Sharoni
// March 4,    2003: fixed dependent names - credit due to Ruslan Zasukhin and CW 8.3 
// July 26,    2005: parameter support by Peter Kümmel 
////////////////////////////////////////////////////////////////////////////////

#endif // FACTORY_INC_

// $Log: Factory.h,v $
// Revision 1.16  2006/03/08 16:41:38  syntheticpp
// remove second $
//
// Revision 1.15  2006/01/19 23:11:55  lfittl
// - Disabled -Weffc++ flag, fixing these warnings produces too much useless code
// - Enabled -pedantic, -Wold-style-cast and -Wundef for src/ and test/
//
// Revision 1.14  2006/01/16 19:05:09  rich_sposato
// Added cvs keywords.
//
// Revision 1.13  2006/01/04 23:45:07  syntheticpp
// remove gcc 4.0 warnings, Thanks to Lukas Fittl
//
// Revision 1.12  2005/11/12 17:11:55  syntheticpp
// make typedefs private, replace tabs
//
// Revision 1.11  2005/11/12 16:52:36  syntheticpp
// protect private data, add std::vector<IdType> RegisteredIds()
//
// Revision 1.10  2005/11/03 12:43:35  syntheticpp
// more doxygen documentation, modules added
//
// Revision 1.9  2005/10/30 14:03:23  syntheticpp
// replace tabs space
//
// Revision 1.8  2005/10/30 13:49:44  syntheticpp
// make disabling the TYPELIST macros possible
//
// Revision 1.7  2005/10/05 09:57:37  syntheticpp
// move unreachable code warnings
//
// Revision 1.6  2005/09/26 07:33:04  syntheticpp
// move macros into LOKI_ namespace
//
// Revision 1.5  2005/07/31 14:23:24  syntheticpp
// invert new factory code macro logic to be ReferenceTest more compatible with noncc code
//
// Revision 1.4  2005/07/28 14:26:09  syntheticpp
// add cvs Header/Log
//

