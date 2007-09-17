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

// $Id: strong.cpp 805 2007-01-13 01:47:23Z rich_sposato $


// ----------------------------------------------------------------------------

#include <loki/StrongPtr.h>

#include <iostream>
#include <cassert>

#include "base.h"


// ----------------------------------------------------------------------------

using namespace std;
using namespace Loki;


// ----------------------------------------------------------------------------

/// Used to check if SmartPtr can be used with a forward-reference.
class Thingy;

typedef Loki::StrongPtr< Thingy, true, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    Thingy_DeleteSingle_ptr;

typedef Loki::StrongPtr< Thingy, true, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteUsingFree, DontPropagateConst >
    Thingy_DeleteUsingFree_ptr;

typedef Loki::StrongPtr< Thingy, true, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteNothing, DontPropagateConst >
    Thingy_DeleteNothing_ptr;


// ----------------------------------------------------------------------------

class Earth;
class Moon;

typedef Loki::StrongPtr< Earth, false, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    Earth_WeakPtr;

typedef Loki::StrongPtr< Earth, true, TwoRefCounts, DisallowConversion,
    AssertCheck, AllowReset, DeleteSingle, DontPropagateConst >
    Earth_StrongPtr;

typedef Loki::StrongPtr< Moon, false, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    Moon_WeakPtr;

typedef Loki::StrongPtr< Moon, true, TwoRefCounts, DisallowConversion,
    AssertCheck, AllowReset, DeleteSingle, DontPropagateConst >
    Moon_StrongPtr;

// ----------------------------------------------------------------------------

class Earth
{
public:

    Earth( void ) : m_moon()
    {
        s_constructions++;
    }

    ~Earth( void )
    {
        s_destructions++;
    }

    void SetMoon( const Moon_StrongPtr & p );

    void SetMoon( const Moon_WeakPtr & p );

    static inline bool AllDestroyed( void )
    {
        return ( s_constructions == s_destructions );
    }

    static inline bool ExtraConstructions( void )
    {
        return ( s_constructions > s_destructions );
    }

    static inline bool ExtraDestructions( void )
    {
        return ( s_constructions < s_destructions );
    }

    static inline unsigned int GetCtorCount( void )
    {
        return s_constructions;
    }

    static inline unsigned int GetDtorCount( void )
    {
        return s_destructions;
    }

private:
    /// Not implemented.
    Earth( const Earth & );
    /// Not implemented.
    Earth & operator = ( const Earth & );

    static unsigned int s_constructions;
    static unsigned int s_destructions;

    Moon_WeakPtr m_moon;
};

unsigned int Earth::s_constructions = 0;
unsigned int Earth::s_destructions = 0;

// ----------------------------------------------------------------------------

class Moon
{
public:

    Moon( void ) : m_earth()
    {
        s_constructions++;
    }

    ~Moon( void )
    {
        s_destructions++;
    }

    void SetEarth( const Earth_WeakPtr & p );

    void SetEarth( const Earth_StrongPtr & p );

    static inline bool AllDestroyed( void )
    {
        return ( s_constructions == s_destructions );
    }

    static inline bool ExtraConstructions( void )
    {
        return ( s_constructions > s_destructions );
    }

    static inline bool ExtraDestructions( void )
    {
        return ( s_constructions < s_destructions );
    }

    static inline unsigned int GetCtorCount( void )
    {
        return s_constructions;
    }

    static inline unsigned int GetDtorCount( void )
    {
        return s_destructions;
    }

private:
    /// Not implemented.
    Moon( const Moon & );
    /// Not implemented.
    Moon & operator = ( const Moon & );

    static unsigned int s_constructions;
    static unsigned int s_destructions;
    Earth_WeakPtr m_earth;
};

unsigned int Moon::s_constructions = 0;
unsigned int Moon::s_destructions = 0;

// ----------------------------------------------------------------------------

void Moon::SetEarth( const Earth_WeakPtr & p )
{
    m_earth = p;
}

// ----------------------------------------------------------------------------

void Moon::SetEarth( const Earth_StrongPtr & p )
{
    m_earth = p;
}

// ----------------------------------------------------------------------------

void Earth::SetMoon( const Moon_WeakPtr & p )
{
    m_moon = p;
}

// ----------------------------------------------------------------------------

void Earth::SetMoon( const Moon_StrongPtr & p )
{
    m_moon = p;
}

// ----------------------------------------------------------------------------

typedef Loki::StrongPtr< BaseClass, false, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr;

typedef Loki::StrongPtr< BaseClass, true, TwoRefCounts, DisallowConversion,
    NoCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr;

typedef Loki::StrongPtr< BaseClass, false, TwoRefLinks, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    NonConstBase_WeakLink_NoConvert_Assert_NoPropagate_ptr;

typedef Loki::StrongPtr< BaseClass, true, TwoRefLinks, DisallowConversion,
    NoCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst >
    NonConstBase_StrongLink_NoConvert_NoCheck_NoPropagate_ptr;

typedef Loki::StrongPtr< BaseClass, false, TwoRefCounts, DisallowConversion,
    AssertCheck, AllowReset, DeleteSingle, DontPropagateConst >
    NonConstBase_WeakCount_NoConvert_Assert_Reset_NoPropagate_ptr;

typedef Loki::StrongPtr< BaseClass, true, TwoRefCounts, DisallowConversion,
    NoCheck, AllowReset, DeleteSingle, DontPropagateConst >
    NonConstBase_StrongCount_NoConvert_NoCheck_Reset_NoPropagate_ptr;

/// @note Used for const propagation tests.
typedef Loki::StrongPtr< const BaseClass, true, TwoRefCounts, DisallowConversion,
    NoCheck, CantResetWithStrong, DeleteSingle, PropagateConst >
    ConstBase_StrongCount_NoConvert_NoCheck_Propagate_ptr;

typedef Loki::StrongPtr< const BaseClass, false, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, PropagateConst >
    ConstBase_WeakCount_NoConvert_Assert_Propagate_ptr;

// ----------------------------------------------------------------------------

void DoStrongRefCountTests( void )
{

    BaseClass * pNull = NULL; (void)pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void)ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void)dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w0;
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s0;
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( w0 );

        // Copy from weak to strong is available.
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( w0 );
        // Assignment from weak to strong is available.
        s1 = w1;

        // Converting from strong to weak is available.
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( s0 );
        // Assignment from strong to weak is available.
        w1 = s1;

        assert( !s0 );
        assert( !s1 );
        assert( !w0 );
        assert( !w1 );
        assert( s1 == pNull );
        assert( s0 == pNull );
        assert( w1 == pNull );
        assert( w1 == pNull );
        assert( pNull == s0 );
        assert( pNull == s1 );
        assert( pNull == w0 );
        assert( pNull == w1 );
        assert( s0 == s0 );
        assert( s1 == s1 );
        assert( w0 == w0 );
        assert( w1 == w1 );
        assert( s1 == s0 );
        assert( s0 == s1 );
        assert( w0 == s0 );
        assert( s0 == w0 );
        assert( w0 == w1 );
        assert( w1 == w0 );
        assert( w0 == w1 );
        assert( w1 == s1 );
        assert( s1 == w1 );
        assert( s1 <= s0 );
        assert( s1 >= s0 );
        assert( s0 <= s1 );
        assert( s0 >= s1 );
        assert( w0 <= s0 );
        assert( w0 >= s0 );
        assert( s0 <= w0 );
        assert( s0 >= w0 );
        assert( w1 <= w0 );
        assert( w1 >= w0 );
        assert( w0 <= w1 );
        assert( w0 >= w1 );
        assert( !( s1 < s0 ) );
        assert( !( s1 > s0 ) );
        assert( !( s0 < s1 ) );
        assert( !( s0 > s1 ) );
        assert( !( w0 < s0 ) );
        assert( !( w0 > s0 ) );
        assert( !( s0 < w0 ) );
        assert( !( s0 > w0 ) );
        assert( !( w1 < w0 ) );
        assert( !( w1 > w0 ) );
        assert( !( w0 < w1 ) );
        assert( !( w0 > w1 ) );
        assert( !( w0 < pNull ) );
        assert( !( w0 > pNull ) );
        assert( !( pNull < w0 ) );
        assert( !( pNull > w0 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( new BaseClass );
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( new BaseClass );
        assert( w1 != w2 );
        assert( w1 );
        assert( w2 );
        w1 = w2;
        assert( w1 == w2 );
        assert( w1 );
        assert( w2 );
        assert( dtorCount + 1 == BaseClass::GetDtorCount() );
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );

    {
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( new BaseClass );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s2( new BaseClass );
        assert( s1 != s2 );
        assert( s1 );
        assert( s2 );
        s1 = s2;
        assert( s1 == s2 );
        assert( s1 );
        assert( s2 );
        assert( dtorCount + 3 == BaseClass::GetDtorCount() );
    }
    assert( ctorCount + 4 == BaseClass::GetCtorCount() );
    assert( dtorCount + 4 == BaseClass::GetDtorCount() );
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( new BaseClass );
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( new BaseClass );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( w1 );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s2( w2 );

        // prove basic stuff.
        assert( w1 != w2 );
        assert( s1 != s2 );
        assert( s1 == w1 );
        assert( s2 == w2 );
        assert( s1 );
        assert( s2 );
        assert( w1 );
        assert( w2 );

        // prove a weak pointer can be re-assigned to another without affecting
        // any strong co-pointers. and that no objects were released.
        w1 = w2;  // w1 == w2 == s2   s1
        assert( w1 == w2 );
        assert( s1 != s2 );
        assert( s1 != w1 );
        assert( s1 != w2 );
        assert( s2 == w1 );
        assert( w1 );
        assert( w2 );
        assert( s1 );
        assert( s2 );
        assert( dtorCount + 4 == BaseClass::GetDtorCount() );

        // Prove they all point to same thing.
        s1 = s2;  // w1 == w2 == s2 == s1
        // and prove that one of them released the object.
        assert( dtorCount + 5 == BaseClass::GetDtorCount() );
        assert( w1 == w2 );
        assert( s1 == s2 );
        assert( s1 == w1 );
        assert( s1 == w2 );
        assert( s2 == w1 );
        assert( w1 );
        assert( w2 );
        assert( s1 );
        assert( s2 );
    }
    assert( ctorCount + 6 == BaseClass::GetCtorCount() );
    assert( dtorCount + 6 == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( new BaseClass );
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( new BaseClass );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( w1 );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s2( w2 );

        // prove basic stuff.  w1 == s1   w2 == s2
        assert( w1 != w2 );
        assert( s1 != s2 );
        assert( s1 == w1 );
        assert( s2 == w2 );
        assert( s1 );
        assert( s2 );
        assert( w1 );
        assert( w2 );

        // prove a strong pointer can be re-assigned to another weak pointer,
        // and that any weak co-pointers released the object.
        s1 = w2;  // s1 == w2 == s2   w1
        assert( w1 != w2 );
        assert( s1 == s2 );
        assert( s1 != w1 );
        assert( s1 == w2 );
        assert( s2 != w1 );
        assert( !w1 );
        assert( w2 );
        assert( s1 );
        assert( s2 );
        assert( dtorCount + 7 == BaseClass::GetDtorCount() );

        // Prove that when strong pointer is re-assigned, object
        // is not destroyed if another strong co-pointer exists.
        s1 = w1;  // w1 == s1   w2 == s2
        // and prove that none of them released an object.
        assert( dtorCount + 7 == BaseClass::GetDtorCount() );
        assert( w1 != w2 );
        assert( s1 != s2 );
        assert( s1 == w1 );
        assert( s2 == w2 );
        assert( !s1 );
        assert( s2 );
        assert( !w1 );
        assert( w2 );
    }
    assert( ctorCount + 8 == BaseClass::GetCtorCount() );
    assert( dtorCount + 8 == BaseClass::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoStrongConstTests( void )
{

    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void)ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void)dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( new BaseClass );
        const NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( w1 );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( w1 );
        const NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s2( w2 );

        const BaseClass & cbw1 = *w1;
        cbw1.DoThat();
        const BaseClass & cbw2 = *w2;
        cbw2.DoThat();
        const BaseClass & cbs1 = *s1;
        cbs1.DoThat();
        const BaseClass & cbs2 = *s2;
        cbs2.DoThat();

        BaseClass & bw1 = *w1;
        bw1.DoThat();
        BaseClass & bw2 = *w2;
        bw2.DoThat();
        BaseClass & bs1 = *s1;
        bs1.DoThat();
        BaseClass & bs2 = *s2;
        bs2.DoThat();
    }
    assert( ctorCount + 1 == BaseClass::GetCtorCount() );
    assert( dtorCount + 1 == BaseClass::GetDtorCount() );

    {
        ConstBase_WeakCount_NoConvert_Assert_Propagate_ptr w1( new BaseClass );
        const ConstBase_WeakCount_NoConvert_Assert_Propagate_ptr w2( w1 );
        ConstBase_StrongCount_NoConvert_NoCheck_Propagate_ptr s1( w1 );
        const ConstBase_StrongCount_NoConvert_NoCheck_Propagate_ptr s2( w2 );

        const BaseClass & cbw1 = *w1;
        cbw1.DoThat();
        const BaseClass & cbw2 = *w2;
        cbw2.DoThat();
        const BaseClass & cbs1 = *s1;
        cbs1.DoThat();
        const BaseClass & cbs2 = *s2;
        cbs2.DoThat();

        /** @note These are illegal because constness is propagated by the
         StrongPtr's policy.  Your compiler should produce error messages if
         you attempt to compile these lines.
         */
        //BaseClass & bw1 = *w1;
        //bw1.DoThat();
        //BaseClass & bw2 = *w2;
        //bw2.DoThat();
        //BaseClass & bs1 = *s1;
        //bs1.DoThat();
        //BaseClass & bs2 = *s2;
        //bs2.DoThat();
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoStrongReleaseTests( void )
{

    BaseClass * pNull = NULL; (void)pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void)ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void)dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        // These are tests of pointers that don't allow reset or release if
        // there is at least 1 strong pointer.  Ironically, this means that
        // if only 1 strong pointer holds onto an object, and you call Release
        // using that strong pointer, it can't release itself.

        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w1( new BaseClass );
        NonConstBase_WeakCount_NoConvert_Assert_NoPropagate_ptr w2( new BaseClass );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s1( w1 );
        NonConstBase_StrongCount_NoConvert_NoCheck_NoPropagate_ptr s2( w2 );

        // Prove that neither weak nor strong pointers can be
        // released if any co-pointer is strong.
        bool released = ReleaseAll( w2, pNull );
        assert( !released );

        released = ReleaseAll( w1, pNull );
        assert( !released );

        released = ReleaseAll( s1, pNull );
        assert( !released );

        released = ReleaseAll( s2, pNull );
        assert( !released );

        // Prove that weak and strong pointers can be reset only
        // if stored pointer matches parameter pointer - or there
        // are no strong co-pointers.
        bool reset = ResetAll( w2, pNull );
        assert( !reset );

        reset = ResetAll( w1, pNull );
        assert( !reset );

        reset = ResetAll( s1, pNull );
        assert( !reset );

        reset = ResetAll( s2, pNull );
        assert( !reset );

        s2 = pNull;
        assert( dtorCount + 1 == BaseClass::GetDtorCount() );
        reset = ResetAll( w2, pNull );
        assert( reset );

        reset = ResetAll( w1, pNull );
        assert( !reset );

        reset = ResetAll( s1, pNull );
        assert( !reset );

        reset = ResetAll( s2, pNull );
        assert( reset );
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );

    {
        // These are tests of pointers that do allow reset and release even
        // if a strong pointer exists.

        NonConstBase_WeakCount_NoConvert_Assert_Reset_NoPropagate_ptr w1( new BaseClass );
        NonConstBase_StrongCount_NoConvert_NoCheck_Reset_NoPropagate_ptr w2( new BaseClass );
        NonConstBase_WeakCount_NoConvert_Assert_Reset_NoPropagate_ptr s1( w1 );
        NonConstBase_StrongCount_NoConvert_NoCheck_Reset_NoPropagate_ptr s2( w2 );

        BaseClass * thing = NULL;
        bool released = ReleaseAll( w2, thing );
        assert( released );
        assert( NULL != thing );
        delete thing;
        assert( dtorCount + 3 == BaseClass::GetDtorCount() );

        released = ReleaseAll( s1, thing );
        assert( released );
        assert( NULL != thing );
        delete thing;
        assert( dtorCount + 4 == BaseClass::GetDtorCount() );

        released = ReleaseAll( w1, thing );
        assert( released );
        assert( NULL == thing );

        released = ReleaseAll( s2, thing );
        assert( released );
        assert( NULL == thing );

        // Prove that weak and strong pointers can be reset
        // only if stored pointer matches parameter pointer
        // - even if there are strong co-pointers.
        bool reset = ResetAll( w2, pNull );
        assert( reset );

        reset = ResetAll( w1, pNull );
        assert( reset );

        reset = ResetAll( s1, pNull );
        assert( reset );

        reset = ResetAll( s2, pNull );
        assert( reset );
        assert( ctorCount + 4 == BaseClass::GetCtorCount() );
        assert( dtorCount + 4 == BaseClass::GetDtorCount() );

        s2 = new BaseClass;
        s1 = new BaseClass;
        reset = ResetAll( w2, pNull );
        assert( reset );

        reset = ResetAll( w1, pNull );
        assert( reset );
    }
    assert( ctorCount + 6 == BaseClass::GetCtorCount() );
    assert( dtorCount + 6 == BaseClass::GetDtorCount() );
}

// ----------------------------------------------------------------------------

void DoStrongRefLinkTests( void )
{

    BaseClass * pNull = NULL; (void)pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void)ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void)dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakLink_NoConvert_Assert_NoPropagate_ptr w0;
        NonConstBase_WeakLink_NoConvert_Assert_NoPropagate_ptr w1;
        NonConstBase_StrongLink_NoConvert_NoCheck_NoPropagate_ptr s0;
        NonConstBase_StrongLink_NoConvert_NoCheck_NoPropagate_ptr s1;
        assert( !s0 );
        assert( !s1 );
        assert( s0 == s1 );
        assert( s1 == pNull );
        assert( s0 == pNull );
        assert( pNull == s0 );
        assert( pNull == s1 );
        assert( s1 == s0 );
        assert( s1 == s1 );
        assert( s0 == s0 );
        assert( s0 == s1 );
        assert( s1 <= s0 );
        assert( s1 >= s0 );
        assert( s0 <= s1 );
        assert( s0 >= s1 );
        assert( !( s1 < s0 ) );
        assert( !( s1 > s0 ) );
        assert( !( s0 < s1 ) );
        assert( !( s0 > s1 ) );
        assert( !w0 );
        assert( !w1 );
        assert( w0 == pNull );
        assert( s0 == pNull );
        assert( pNull == s0 );
        assert( pNull == w0 );
        assert( w0 == s0 );
        assert( w0 == w0 );
        assert( s0 == s0 );
        assert( s0 == w0 );
        assert( w0 <= s0 );
        assert( w0 >= s0 );
        assert( s0 <= w0 );
        assert( s0 >= w0 );
        assert( !( w0 < s0 ) );
        assert( !( w0 > s0 ) );
        assert( !( s0 < w0 ) );
        assert( !( s0 > w0 ) );
        assert( w0 == w1 );
        assert( w1 == pNull );
        assert( w0 == pNull );
        assert( pNull == w0 );
        assert( pNull == w1 );
        assert( w1 == w0 );
        assert( w1 == w1 );
        assert( w0 == w0 );
        assert( w0 == w1 );
        assert( w1 <= w0 );
        assert( w1 >= w0 );
        assert( w0 <= w1 );
        assert( w0 >= w1 );
        assert( !( w1 < w0 ) );
        assert( !( w1 > w0 ) );
        assert( !( w0 < w1 ) );
        assert( !( w0 > w1 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    {
        NonConstBase_WeakLink_NoConvert_Assert_NoPropagate_ptr w3( new BaseClass );
        NonConstBase_WeakLink_NoConvert_Assert_NoPropagate_ptr w4( new BaseClass );
        assert( w3 != w4 );
        assert( w3 );
        assert( w4 );
        w3 = w4;
        assert( w3 == w4 );
        assert( w3 );
        assert( w4 );
        assert( dtorCount + 1 == BaseClass::GetDtorCount() );
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );

    {
        NonConstBase_StrongLink_NoConvert_NoCheck_NoPropagate_ptr s3( new BaseClass );
        NonConstBase_StrongLink_NoConvert_NoCheck_NoPropagate_ptr s4( new BaseClass );
        assert( s3 != s4 );
        assert( s3 );
        assert( s4 );
        s3 = s4;
        assert( s3 == s4 );
        assert( s3 );
        assert( s4 );
        assert( dtorCount + 3 == BaseClass::GetDtorCount() );
    }
    assert( ctorCount + 4 == BaseClass::GetCtorCount() );
    assert( dtorCount + 4 == BaseClass::GetDtorCount() );
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

}

// ----------------------------------------------------------------------------

void DoWeakCycleTests( void )
{

    const unsigned int ctorCountMoon = Moon::GetCtorCount(); (void)ctorCountMoon;
    const unsigned int dtorCountMoon = Moon::GetDtorCount(); (void)dtorCountMoon;
    assert( Moon::AllDestroyed() );
    const unsigned int ctorCountEarth = Earth::GetCtorCount(); (void)ctorCountEarth;
    const unsigned int dtorCountEarth = Earth::GetDtorCount(); (void)dtorCountEarth;
    assert( Earth::AllDestroyed() );

    {
        Earth_WeakPtr ew0;
        Moon_WeakPtr mw0;
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon );
    assert( Moon::GetDtorCount() == dtorCountMoon );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth );
    assert( Earth::GetDtorCount() == dtorCountEarth );

    {
        Earth_WeakPtr ew1( new Earth );
    }
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+1 );
    assert( Earth::GetDtorCount() == dtorCountEarth+1 );

    {
        Moon_WeakPtr mw1( new Moon );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+1 );
    assert( Moon::GetDtorCount() == dtorCountMoon+1 );

    {
        Earth_WeakPtr ew1( new Earth );
        Moon_WeakPtr mw1( new Moon );
        ew1->SetMoon( mw1 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+2 );
    assert( Moon::GetDtorCount() == dtorCountMoon+2 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+2 );
    assert( Earth::GetDtorCount() == dtorCountEarth+2 );

    {
        Earth_WeakPtr ew1( new Earth );
        Moon_WeakPtr mw1( new Moon );
        mw1->SetEarth( ew1 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+3 );
    assert( Moon::GetDtorCount() == dtorCountMoon+3 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+3 );
    assert( Earth::GetDtorCount() == dtorCountEarth+3 );

    {
        Earth_WeakPtr ew1( new Earth );
        Moon_WeakPtr mw1( new Moon );
        ew1->SetMoon( mw1 );
        mw1->SetEarth( ew1 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+4 );
    assert( Moon::GetDtorCount() == dtorCountMoon+4 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+4 );
    assert( Earth::GetDtorCount() == dtorCountEarth+4 );

    {
        Earth_StrongPtr es1( new Earth );
        Moon_StrongPtr ms1( new Moon );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+5 );
    assert( Moon::GetDtorCount() == dtorCountMoon+5 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+5 );
    assert( Earth::GetDtorCount() == dtorCountEarth+5 );

    {
        Earth_StrongPtr es1( new Earth );
        Moon_StrongPtr ms1( new Moon );
        es1->SetMoon( ms1 );
        ms1->SetEarth( es1 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+6 );
    assert( Moon::GetDtorCount() == dtorCountMoon+6 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+6 );
    assert( Earth::GetDtorCount() == dtorCountEarth+6 );

    {
        Earth_StrongPtr es1( new Earth );
        Moon_StrongPtr ms1( new Moon );
        {
            Earth_WeakPtr ew1( es1 );
            Moon_WeakPtr mw1( ms1 );
            ew1->SetMoon( mw1 );
            mw1->SetEarth( ew1 );
        }
        // Note that dtor counts have not changed from previous test.
        assert( Moon::GetCtorCount() == ctorCountMoon+7 );
        assert( Moon::GetDtorCount() == dtorCountMoon+6 );
        assert( Earth::GetCtorCount() == ctorCountEarth+7 );
        assert( Earth::GetDtorCount() == dtorCountEarth+6 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+7 );
    assert( Moon::GetDtorCount() == dtorCountMoon+7 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+7 );
    assert( Earth::GetDtorCount() == dtorCountEarth+7 );

    {
        Earth_StrongPtr es1;
        Moon_StrongPtr ms1;
        {
            Earth_WeakPtr ew1( new Earth );
            Moon_WeakPtr mw1( new Moon );
            ew1->SetMoon( mw1 );
            mw1->SetEarth( ew1 );
            es1 = ew1;
            ms1 = mw1;
        }
        // Note that dtor counts have not changed from previous test.
        assert( Moon::GetCtorCount() == ctorCountMoon+8 );
        assert( Moon::GetDtorCount() == dtorCountMoon+7 );
        assert( Earth::GetCtorCount() == ctorCountEarth+8 );
        assert( Earth::GetDtorCount() == dtorCountEarth+7 );
    }
    assert( Moon::AllDestroyed() );
    assert( Moon::GetCtorCount() == ctorCountMoon+8 );
    assert( Moon::GetDtorCount() == dtorCountMoon+8 );
    assert( Earth::AllDestroyed() );
    assert( Earth::GetCtorCount() == ctorCountEarth+8 );
    assert( Earth::GetDtorCount() == dtorCountEarth+8 );
}

// ----------------------------------------------------------------------------

void DoStrongForwardReferenceTest( void )
{
    /** @note These lines should cause the compiler to make a warning message
     about attempting to delete an undefined type.  They should also cause
     an error message about a negative subscript since 
     */
    //Thingy_DeleteSingle_ptr p1;
    //Thingy_DeleteSingle_ptr p2( p1 );
    //Thingy_DeleteSingle_ptr p3;
    //p3 = p1;

    /** @note These lines should cause the compiler to make an error message
     about attempting to call the destructor for an undefined type.
     */
    //Thingy_DeleteUsingFree_ptr p4;
    //Thingy_DeleteUsingFree_ptr p5( p4 );
    //Thingy_DeleteUsingFree_ptr p6;
    //p6 = p4;

    /** @note These lines should cause the compiler to make neither a warning
     nor an error message even though the type is undefined.
     */
    Thingy_DeleteNothing_ptr p7;
    Thingy_DeleteNothing_ptr p8( p7 );
    Thingy_DeleteNothing_ptr p9;
    p9 = p7;
}

// ----------------------------------------------------------------------------

#include <algorithm>

struct Foo
{
};
typedef Loki::StrongPtr
< 
    BaseClass, false, TwoRefCounts, DisallowConversion,
    AssertCheck, CantResetWithStrong, DeleteSingle, DontPropagateConst 
>
Ptr;

bool Compare( const Ptr&, const Ptr&)
{
    return true; 
}

void friend_handling2()
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
    ReleaseAll( w1, pNull );
}

// ----------------------------------------------------------------------------

void DoStrongCompareTests( void )
{
    Earth * p1 = new Earth;
    Earth * p2 = new Earth;
    Earth_StrongPtr sp1( p1 );
    Earth_StrongPtr sp2( p2 );
    const bool isOneLess = ( p1 < p2 );
    if ( isOneLess )
    {
        assert( sp1 <   p2 );
        assert( sp1 <  sp2 );
        assert( sp1 <=  p2 );
        assert( sp1 <= sp2 );
        assert( sp1 <=  p1 );
        assert( sp1 ==  p1 );
        assert( sp1 == sp1 );
        assert( sp1 !=  p2 );
        assert( sp1 != sp2 );
        assert( sp2 >   p1 );
        assert( sp2 >  sp1 );
        assert( sp2 >=  p1 );
        assert( sp2 >= sp1 );
        assert( sp2 >=  p2 );
        assert( sp2 ==  p2 );
        assert( sp2 == sp2 );
        assert( sp2 !=  p1 );
        assert( sp2 != sp1 );
        assert(  p1 <   p2 );
        assert(  p1 <  sp2 );
        assert(  p1 <=  p2 );
        assert(  p1 <= sp2 );
        assert(  p1 <= sp1 );
        assert(  p1 ==  p1 );
        assert(  p1 == sp1 );
        assert(  p1 !=  p2 );
        assert(  p1 != sp2 );
        assert(  p2 >   p1 );
        assert(  p2 >  sp1 );
        assert(  p2 >=  p1 );
        assert(  p2 >= sp1 );
        assert(  p2 >= sp2 );
        assert(  p2 ==  p2 );
        assert(  p2 == sp2 );
        assert(  p2 !=  p1 );
        assert(  p2 != sp1 );
    }
    else
    {
        assert( sp2 <   p1 );
        assert( sp2 <  sp1 );
        assert( sp2 <=  p1 );
        assert( sp2 <=  p2 );
        assert( sp2 <= sp1 );
        assert( sp2 ==  p2 );
        assert( sp2 == sp2 );
        assert( sp2 !=  p1 );
        assert( sp2 != sp1 );
        assert( sp1 >   p2 );
        assert( sp1 >  sp2 );
        assert( sp1 >=  p2 );
        assert( sp1 >=  p1 );
        assert( sp1 >= sp2 );
        assert( sp1 ==  p1 );
        assert( sp1 == sp1 );
        assert( sp1 !=  p2 );
        assert( sp1 != sp2 );
        assert(  p2 <   p1 );
        assert(  p2 <  sp1 );
        assert(  p2 <=  p1 );
        assert(  p2 <= sp1 );
        assert(  p2 <= sp2 );
        assert(  p2 ==  p2 );
        assert(  p2 == sp2 );
        assert(  p2 !=  p1 );
        assert(  p2 != sp1 );
        assert(  p1 >   p2 );
        assert(  p1 >  sp2 );
        assert(  p1 >=  p2 );
        assert(  p1 >= sp2 );
        assert(  p1 >= sp1 );
        assert(  p1 ==  p1 );
        assert(  p1 == sp1 );
        assert(  p1 !=  p2 );
        assert(  p1 != sp2 );
    }
}

// ----------------------------------------------------------------------------
