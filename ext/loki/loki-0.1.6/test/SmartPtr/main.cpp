////////////////////////////////////////////////////////////////////////////////
// Test program for The Loki Library
// Copyright (c) 2006 Richard Sposato
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main.cpp 805 2007-01-13 01:47:23Z rich_sposato $


// ----------------------------------------------------------------------------

#include <loki/SmartPtr.h>

#include <iostream>

#include "base.h"

// ----------------------------------------------------------------------------

using namespace std;
using namespace Loki;

extern void DoStrongRefCountTests( void );
extern void DoStrongRefLinkTests( void );
extern void DoStrongReleaseTests( void );
extern void DoWeakCycleTests( void );
extern void DoStrongConstTests( void );
extern void DoStrongForwardReferenceTest( void );
extern void DoStrongCompareTests( void );

extern void DoLockedPtrTest( void );
extern void DoLockedStorageTest( void );

unsigned int BaseClass::s_constructions = 0;
unsigned int BaseClass::s_destructions = 0;

unsigned int MimicCOM::s_constructions = 0;
unsigned int MimicCOM::s_destructions = 0;


// ----------------------------------------------------------------------------

/// Used to check if SmartPtr can be used with a forward-reference.
class Thingy;

#ifdef __GNUC__
#warning The warnings are by design: Check if SmartPtr can be used with a forward-reference.
#endif

typedef Loki::SmartPtr< Thingy, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, PropagateConst >
    Thingy_DefaultStorage_ptr;

typedef Loki::SmartPtr< Thingy, RefCounted, DisallowConversion,
    AssertCheck, HeapStorage, PropagateConst >
    Thingy_HeapStorage_ptr;


// ----------------------------------------------------------------------------

/// @note Used for testing most policies.
typedef Loki::SmartPtr< BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr;

/// @note These 3 are used for testing const policies.
typedef Loki::SmartPtr< const BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    ConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< const BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, PropagateConst >
    ConstBase_RefCount_NoConvert_Assert_Propagate_ptr;

typedef Loki::SmartPtr< BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, PropagateConst >
    NonConstBase_RefCount_NoConvert_Assert_Propagate_ptr;


// ----------------------------------------------------------------------------

/// @note These 5 are used for testing ownership policies.
typedef Loki::SmartPtr< BaseClass, COMRefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, RefLinked, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, DeepCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, DestructiveCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, NoCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr;


// ----------------------------------------------------------------------------

/// @note These 2 are used for testing inheritance.
typedef Loki::SmartPtr< PublicSubClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< PrivateSubClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr;


// ----------------------------------------------------------------------------

/// @note Used for testing how well SmartPtr works with COM objects.
typedef Loki::SmartPtr< MimicCOM, COMRefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    MimicCOM_ptr;


// ----------------------------------------------------------------------------

void DoConstConversionTests( void )
{

    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
    ConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p2( p1 );
    ConstBase_RefCount_NoConvert_Assert_Propagate_ptr p3( p1 );
    NonConstBase_RefCount_NoConvert_Assert_Propagate_ptr p4( p1 );

//  p1 = p2;  // illegal! converts const to non-const.
//  p1 = p3;  // illegal! converts const to non-const.
    p1 = p4;  // legal, but dubious. Changes const-propagation policy.
    p2 = p1;  // legal.  natural const conversion.
    p2 = p3;  // legal, but dubious. Changes const-propagation policy.
    p2 = p4;  // legal, but dubious. Changes const-propagation policy.
    p3 = p1;  // legal, but dubious. Changes const-propagation policy.
    p3 = p2;  // legal, but dubious. Changes const-propagation policy.
    p3 = p4;  // legal.  natural const conversion.
    p4 = p1;  // legal, but dubious. Changes const-propagation policy.
//  p4 = p2;  // illegal! converts const to non-const.
//  p4 = p3;  // illegal! converts const to non-const.
}

// ----------------------------------------------------------------------------

void DoOwnershipConversionTests( void )
{
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
    NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p2;
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p3;
    NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p4( new BaseClass );
    NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p5;
    NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p6;

    // legal constructions.  Each should allow copy with same policies.
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p7( p1 );
    NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p8( p2 );
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p9( p3 );
    NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p10( p4 );
    NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p11( p5 );

    // illegal construction!  Can't copy anything with NoCopy policy.
    // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p12( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p13( p2 );
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p14( p3 );
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p15( p4 );
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p16( p5 );
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p17( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p18( p1 );
//  NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p19( p3 );
//  NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p20( p4 );
//  NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p21( p5 );
//  NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p22( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p23( p1 );
//  NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p24( p2 );
//  NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p25( p4 );
//  NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p26( p5 );
//  NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p27( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p28( p1 );
//  NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p29( p2 );
//  NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p30( p3 );
//  NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p31( p5 );
//  NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p32( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p33( p1 );
//  NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p34( p2 );
//  NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p35( p3 );
//  NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p36( p4 );
//  NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p37( p6 );

    // illegal constructions!  Can't convert from one ownership policy to another.
//  NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p38( p1 );
//  NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p39( p2 );
//  NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p40( p3 );
//  NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p41( p4 );
//  NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p42( p5 );

    // illegal assignements!  Can't convert from one ownership policy to another.
//  p1 = p2;
//  p1 = p3;
//  p1 = p4;
//  p1 = p5;
//  p1 = p6;
//  p2 = p1;
//  p2 = p3;
//  p2 = p4;
//  p2 = p5;
//  p2 = p6;
//  p3 = p1;
//  p3 = p2;
//  p3 = p4;
//  p3 = p5;
//  p3 = p6;
//  p4 = p1;
//  p4 = p2;
//  p4 = p3;
//  p4 = p5;
//  p4 = p6;
//  p5 = p1;
//  p5 = p2;
//  p5 = p3;
//  p5 = p4;
//  p5 = p6;
//  p6 = p1;
//  p6 = p2;
//  p6 = p3;
//  p6 = p4;
//  p6 = p5;
}

// ----------------------------------------------------------------------------

void DoInheritanceConversionTests( void )
{
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
    PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p2;
    PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p3;

    p1 = p2;  // legal.  Cast to public base class allowed.
    assert( p1 == p2 );
//  p1 = p3;  // illegal!  Can't assign pointer since base class is private.
//  p2 = p1;  // illegal!  Can't do cast to derived class in pointer assignment.
//  p2 = p3;  // illegal!  Can't assign when types are unrelated.
//  p3 = p1;  // illegal!  Can't do cast to derived class in pointer assignment.
//  p3 = p2;  // illegal!  Can't assign when types are unrelated.

    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p4( p2 );
    assert( p4 == p1 );
    assert( p4 == p2 );
    // These copy-constructions are illegal for reasons shown above.
//  NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p5( p3 );
//  PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p6( p1 );
//  PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p7( p3 );
//  PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p8( p1 );
//  PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p9( p2 );
}

// ----------------------------------------------------------------------------

void DoRefCountSwapTests( void )
{
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1( new BaseClass );
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p2( new BaseClass );

    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p3( p1 );
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p4( p2 );

    // p1 == p3    and    p2 == p4
    assert( p1 == p3 );
    assert( p1 != p2 );
    assert( p1 != p4 );
    assert( p2 == p4 );
    assert( p2 != p3 );
    assert( p2 != p1 );

    // p1 == p4    and    p2 == p3
    p3.Swap( p4 );
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );

    // p1 == p3    and    p2 == p4
    p3.Swap( p4 );
    assert( p1 == p3 );
    assert( p1 != p2 );
    assert( p1 != p4 );
    assert( p2 == p4 );
    assert( p2 != p3 );
    assert( p2 != p1 );

    // p2 == p3    and    p1 == p4
    p1.Swap( p2 );
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );

    // p2 == p3    and    p1 == p4
    p1.Swap( p1 );
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );

    // p2 == p3    and    p4 == p1
    p1.Swap( p4 );
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
}

// ----------------------------------------------------------------------------

void DoRefLinkSwapTests( void )
{

    BaseClass * pBaseClass = new BaseClass;
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p1( pBaseClass );
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p2( new BaseClass );
    p1->DoThat();
    p2->DoThat();

    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p3( p1 );
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p4( p2 );

    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p5;
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p6( new BaseClass );

    // p1 <---> p3    and    p2 <---> p4   and   p5   and   p6
    assert( p1 == p3 );
    assert( p1 != p2 );
    assert( p1 != p4 );
    assert( p2 == p4 );
    assert( p2 != p3 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p3.Swap( p4 );  // p1 <---> p4    and    p2 <---> p3   and   p5   and   p6
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p3.Swap( p4 );   // p1 <---> p3    and    p2 <---> p4   and   p5   and   p6
    assert( p1 == p3 );
    assert( p1 != p2 );
    assert( p1 != p4 );
    assert( p2 == p4 );
    assert( p2 != p3 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p1.Swap( p2 );  // p2 <---> p3    and    p1 <---> p4   and   p5   and   p6
    assert( p1 != pBaseClass );
    assert( p2 == pBaseClass );
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p1.Swap( p1 );  // p2 <---> p3    and    p1 <---> p4   and   p5   and   p6
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p1.Swap( p4 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p4.Swap( p1 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p5.Swap( p5 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );

    p5.Swap( p1 );  // p2 <---> p3    and    p4 <---> p5   and   p1   and   p6
    assert( p5 == p4 );
    assert( p5 != p2 );
    assert( p5 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p5 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p4 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p6.Swap( p1 );  // p2 <---> p3    and    p4 <---> p5   and   p1   and   p6
    assert( p5 == p4 );
    assert( p5 != p2 );
    assert( p5 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p5 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p4 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );

    p5.Swap( p1 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
    assert( p1 == p4 );
    assert( p1 != p2 );
    assert( p1 != p3 );
    assert( p2 == p3 );
    assert( p2 != p4 );
    assert( p2 != p1 );
    assert( p1 != p5 );
    assert( p2 != p5 );
    assert( p3 != p5 );
    assert( p4 != p5 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );

    p6 = p2;  // p6 <---> p2 <---> p3    and    p4 <---> p1   and   p5
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p5 != p3 );
    assert( p2 != p4 );
    assert( p2 != p5 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );

    p5 = p3;  // p6 <---> p2 <---> p3 <---> p5   and    p4 <---> p1
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );

    p5.Swap( p3 );  // p6 <---> p2 <---> p5 <---> p3   and    p4 <---> p1
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );

    p2.Swap( p3 );  // p6 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );

    bool merged = false;
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p7( pBaseClass );
    assert( p7 == p7 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    merged = p7.Merge( p6 );
    // p7 <---> p6 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( merged );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );

    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p8( pBaseClass );
    assert( p6 == p8 );
    assert( p1 != p8 );
    merged = p6.Merge( p8 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );

    merged = p6.Merge( p6 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );

    merged = p6.Merge( p3 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );

    merged = p5.Merge( p1 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
    assert( !merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );

    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p9( pBaseClass );
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr pA( p9 );
    assert( p9 == pA );
    assert( p9 == p8 );
    assert( p1 != p8 );
    merged = p9.Merge( p1 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2
    //   and    p4 <---> p1   and   p9 <---> pA
    assert( !merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );
    assert( p9 == p9 );
    assert( pA == pA );

    merged = p9.Merge( p2 );
    // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2 <---> p9 <---> pA
    //   and    p4 <---> p1
    assert( merged );
    assert( p6 == p8 );
    assert( p6 == p7 );
    assert( p1 != p7 );
    assert( p6 == p5 );
    assert( p6 == p2 );
    assert( p6 == p3 );
    assert( p5 == p3 );
    assert( p2 == p3 );
    assert( p1 == p4 );
    assert( p2 != p4 );
    assert( p1 != p5 );
    assert( p2 != p1 );
    assert( p3 != p1 );
    assert( p1 == p1 );
    assert( p2 == p2 );
    assert( p3 == p3 );
    assert( p4 == p4 );
    assert( p5 == p5 );
    assert( p6 == p6 );
    assert( p7 == p7 );
    assert( p8 == p8 );
    assert( p9 == p9 );
    assert( pA == pA );
}

// ----------------------------------------------------------------------------

void DoRefLinkTests( void )
{

    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w0;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w1;
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w3( new BaseClass );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w4( new BaseClass );
        assert( w3 != w4 );
        assert( w3 );
        assert( w4 );
        w3 = w4;
        assert( w3 == w4 );
        assert( w3 );
        assert( w4 );
        assert( dtorCount + 1 == BaseClass::GetDtorCount() );
        w3->DoThat();
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

}

// ----------------------------------------------------------------------------

void DoRefCountNullPointerTests( void )
{
    BaseClass * pNull = NULL; (void) pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p0;
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p2( p0 );

        assert( !p0 );
        assert( !p1 );
        assert( !p2 );
        assert( p1 == pNull );
        assert( p0 == pNull );
        assert( pNull == p0 );
        assert( pNull == p1 );
        assert( pNull == p2 );
        assert( p0 == p0 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p1 == p0 );
        assert( p0 == p1 );
        assert( p2 == p0 );
        assert( p0 == p2 );
        assert( p1 <= p0 );
        assert( p1 >= p0 );
        assert( p0 <= p1 );
        assert( p0 >= p1 );
        assert( p2 <= p0 );
        assert( p2 >= p0 );
        assert( p0 <= p2 );
        assert( p0 >= p2 );
        assert( !( p1 < p0 ) );
        assert( !( p1 > p0 ) );
        assert( !( p0 < p1 ) );
        assert( !( p0 > p1 ) );
        assert( !( p2 < p0 ) );
        assert( !( p2 > p0 ) );
        assert( !( p0 < p2 ) );
        assert( !( p0 > p2 ) );
        assert( !( p0 < pNull ) );
        assert( !( p0 > pNull ) );
        assert( !( pNull < p0 ) );
        assert( !( pNull > p0 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoRefLinkNullPointerTests( void )
{
    BaseClass * pNull = NULL; (void) pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p0;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p1;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p2( p0 );

        assert( !p0 );
        assert( !p1 );
        assert( !p2 );
        assert( p1 == pNull );
        assert( p0 == pNull );
        assert( pNull == p0 );
        assert( pNull == p1 );
        assert( pNull == p2 );
        assert( p0 == p0 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p1 == p0 );
        assert( p0 == p1 );
        assert( p2 == p0 );
        assert( p0 == p2 );
        assert( p1 <= p0 );
        assert( p1 >= p0 );
        assert( p0 <= p1 );
        assert( p0 >= p1 );
        assert( p2 <= p0 );
        assert( p2 >= p0 );
        assert( p0 <= p2 );
        assert( p0 >= p2 );
        assert( !( p1 < p0 ) );
        assert( !( p1 > p0 ) );
        assert( !( p0 < p1 ) );
        assert( !( p0 > p1 ) );
        assert( !( p2 < p0 ) );
        assert( !( p2 > p0 ) );
        assert( !( p0 < p2 ) );
        assert( !( p0 > p2 ) );
        assert( !( p0 < pNull ) );
        assert( !( p0 > pNull ) );
        assert( !( pNull < p0 ) );
        assert( !( pNull > p0 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoComRefTest( void )
{

    const unsigned int ctorCount = MimicCOM::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = MimicCOM::GetDtorCount(); (void) dtorCount;
    assert( MimicCOM::AllDestroyed() );
    {
        MimicCOM_ptr p1;
    }
    assert( MimicCOM::AllDestroyed() );
    assert( ctorCount == MimicCOM::GetCtorCount() );
    assert( dtorCount == MimicCOM::GetDtorCount() );

    {
        MimicCOM_ptr p1( MimicCOM::QueryInterface() );
    }
    assert( ctorCount+1 == MimicCOM::GetCtorCount() );
    assert( dtorCount+1 == MimicCOM::GetDtorCount() );

    {
        MimicCOM_ptr p2( MimicCOM::QueryInterface() );
        MimicCOM_ptr p3( p2 );
        MimicCOM_ptr p4;
        p4 = p2;
    }
    assert( ctorCount+2 == MimicCOM::GetCtorCount() );
    assert( dtorCount+2 == MimicCOM::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoForwardReferenceTest( void )
{
    /** @note These lines should cause the compiler to make a warning message
     about attempting to delete an undefined type.  But it should not produce
     any error messages.
     */
    Thingy_DefaultStorage_ptr p1;
    Thingy_DefaultStorage_ptr p2( p1 );
    Thingy_DefaultStorage_ptr p3;
    p3 = p2;

    /** @note These lines should cause the compiler to make an error message
     about attempting to call the destructor for an undefined type.
     */
    //Thingy_HeapStorage_ptr p4;
    //Thingy_HeapStorage_ptr p5( p4 );
    //Thingy_HeapStorage_ptr p6;
    //p6 = p5;
}


int main( unsigned int argc, const char * argv[] )
{
    bool doThreadTest = false;
    for ( unsigned int ii = 1; ii < argc; ++ii )
    {
        if ( ::strcmp( argv[ii], "-t" ) == 0 )
            doThreadTest = true;
    }

    DoRefLinkTests();
    DoStrongRefCountTests();
    DoStrongReleaseTests();
    DoStrongReleaseTests();
    DoWeakCycleTests();
    DoStrongCompareTests();

    DoForwardReferenceTest();
    DoStrongForwardReferenceTest();

    DoRefCountNullPointerTests();
    DoRefLinkNullPointerTests();

    DoRefCountSwapTests();
    DoRefLinkSwapTests();

    DoComRefTest();

    DoStrongConstTests();
    DoConstConversionTests();
    DoOwnershipConversionTests();
    DoInheritanceConversionTests();

#if defined (LOKI_OBJECT_LEVEL_THREADING) || defined (LOKI_CLASS_LEVEL_THREADING)
    if ( doThreadTest )
    {
        DoLockedStorageTest();
        DoLockedPtrTest();
    }
#endif

    // Check that nothing was leaked.
    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );

    // Check that no destructor called too often.
    assert( !BaseClass::ExtraDestructions() );

    cout << "All SmartPtr tests passed!" << endl;
    return 0;
}


// ----------------------------------------------------------------------------

#include <algorithm>

struct Foo
{
};

typedef Loki::SmartPtr
< 
    BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst 
> 
Ptr;

bool Compare( const Ptr&, const Ptr&)
{
    return true; 
}

void friend_handling()
{
    // http://sourceforge.net/tracker/index.php?func=detail&aid=1570582&group_id=29557&atid=396644

    // friend injection
    // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=28597
    std::vector<Ptr> vec;
    std::sort( vec.begin(), vec.end(), Compare );
    std::nth_element( vec.begin(), vec.begin(), vec.end(), Compare );
    std::search( vec.begin(), vec.end(),
        vec.begin(), vec.end(), Compare );
    Ptr a, b;
    Compare( a, b );

    // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=29486
    BaseClass * pNull ;
    Ptr w1( new BaseClass );
    Release( w1, pNull );

 }

// ----------------------------------------------------------------------------

// $Log$
// Revision 1.14  2006/10/17 10:36:08  syntheticpp
// change line ending
//
// Revision 1.13  2006/10/17 10:09:37  syntheticpp
// add test code for template friends with template template parameters
//
// Revision 1.12  2006/10/16 11:48:13  syntheticpp
// by default Loki is compiled without thread support, so we must disable the dependency on thread classes (StrongPtr) to avaoid linker errors when compiling with the default build process. Should  we change the default threading of Loki?
//
// Revision 1.11  2006/10/13 23:59:42  rich_sposato
// Added check for -t command line parameter to do lock-thread test.
// Changed ending chars of some lines from LF to CR-LF to be consistent.
//
// Revision 1.10  2006/10/11 11:17:53  syntheticpp
// test injected friends. Thanks to Sigoure Benoit
//
// Revision 1.9  2006/05/30 14:17:05  syntheticpp
// don't confuse with warnings
//
// Revision 1.8  2006/05/18 05:05:21  rich_sposato
// Added QueryInterface function to MimicCOM class.
//
// Revision 1.7  2006/04/28 00:34:21  rich_sposato
// Added test for thread-safe StrongPtr policy.
//
// Revision 1.6  2006/04/16 14:05:39  syntheticpp
// remove warnings
//
// Revision 1.5  2006/04/05 22:53:12  rich_sposato
// Added StrongPtr class to Loki along with tests for StrongPtr.
//
// Revision 1.4  2006/03/21 20:50:22  syntheticpp
// fix include error
//
// Revision 1.3  2006/03/17 22:52:56  rich_sposato
// Fixed bugs 1452805 and 1451835.  Added Merge ability for RefLink policy.
// Added more tests for SmartPtr.
//
// Revision 1.2  2006/03/01 02:08:11  rich_sposato
// Fixed bug 1440694 by adding check if rhs is previous neighbor.
//
// Revision 1.1  2006/02/25 01:53:20  rich_sposato
// Added test project for Loki::SmartPtr class.
//
