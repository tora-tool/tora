////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 Richard Sposato
// Copyright (c) 2005 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: SmallObjSingleton.cpp 761 2006-10-17 20:48:18Z syntheticpp $


#include <loki/SmallObj.h>
#include <loki/Singleton.h>
#include <iostream>

// define DO_EXTRA_LOKI_TESTS in src/SmallObj.cpp to get 
// a message when a SmallObject is created/deleted.

using namespace std;

// ----------------------------------------------------------------------------
//
//            LongevityLifetime Parent/Child policies
//
// ----------------------------------------------------------------------------

typedef Loki::SmallValueObject< LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    512, 32, 4, Loki::LongevityLifetime::DieAsSmallObjectParent
>
SmallObjectParent;

class SmallObjectChild : public SmallObjectParent
{
public:

    typedef Loki::SingletonHolder< SmallObjectChild, Loki::CreateUsingNew,
        Loki::LongevityLifetime::DieAsSmallObjectChild>
        MySmallSingleton;

    /// Returns reference to the singleton.
    inline static SmallObjectChild & Instance( void )
    {
        return MySmallSingleton::Instance();
    }

    SmallObjectChild( void )
    {
        cout << "SmallObjectChild created" << endl;
    }
    ~SmallObjectChild( void )
    {
        cout << "~SmallObjectChild" << endl;
    }
    void DoThat( void )
    {
        cout << "SmallObjectChild::DoThat" << endl << endl;
    }
private:
    char m_stuff[ 16 ];
};


// ----------------------------------------------------------------------------
//
//            SingletonWithLongevity policy
//
// ----------------------------------------------------------------------------

typedef Loki::SmallValueObject< LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    LOKI_DEFAULT_CHUNK_SIZE, LOKI_MAX_SMALL_OBJECT_SIZE,
    LOKI_DEFAULT_OBJECT_ALIGNMENT, 
    Loki::SingletonWithLongevity 
>
LongLivedObject;

class LongLivedSingleton : public LongLivedObject
{
public:

    typedef Loki::SingletonHolder< LongLivedSingleton, Loki::CreateUsingNew,
        Loki::SingletonWithLongevity>
        MySmallSingleton;

    /// Returns reference to the singleton.
    inline static LongLivedSingleton & Instance( void )
    {
        return MySmallSingleton::Instance();
    }

    LongLivedSingleton( void )
    {
        cout << "LongLivedSingleton created" << endl;
    }
    ~LongLivedSingleton( void )
    {
        cout << "~LongLivedSingleton" << endl;
    }
    void DoThat( void )
    {
        cout << "LongLivedSingleton::DoThat" << endl << endl;
    }
private:
    char m_stuff[ 16 ];
};

inline unsigned int GetLongevity( LongLivedSingleton * )
{
    /// @note Must return a longevity level lower than the one in SmallObj.h.
    return 1;
}

#if !defined(_MSC_VER) || (_MSC_VER>=1400)

// ----------------------------------------------------------------------------
//
//            FollowIntoDeath policy
//
// ----------------------------------------------------------------------------

typedef Loki::SmallValueObject< LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    LOKI_DEFAULT_CHUNK_SIZE, LOKI_MAX_SMALL_OBJECT_SIZE,
    LOKI_DEFAULT_OBJECT_ALIGNMENT, 
    Loki::FollowIntoDeath::With<Loki::DefaultLifetime>::AsMasterLifetime 
>
MasterObject;


class FollowerSingleton : public MasterObject
{
public:

    typedef Loki::SingletonHolder< FollowerSingleton, Loki::CreateUsingNew,
        Loki::FollowIntoDeath::AfterMaster<FollowerSingleton::ObjAllocatorSingleton>::IsDestroyed>
        MySmallSingleton;

    /// Returns reference to the singleton.
    inline static FollowerSingleton & Instance( void )
    {
        return MySmallSingleton::Instance();
    }

    FollowerSingleton( void )
    {
        cout << "FollowerSingleton created" << endl;
    }
    ~FollowerSingleton( void )
    {
        cout << "~FollowerSingleton" << endl;
    }
    void DoThat( void )
    {
        cout << "FollowerSingleton::DoThat" << endl << endl;
    }
private:
    char m_stuff[ 16 ];
};


#endif

// ----------------------------------------------------------------------------
//
//            NoDestroy policy
//
// ----------------------------------------------------------------------------

typedef Loki::SmallValueObject< LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    LOKI_DEFAULT_CHUNK_SIZE, LOKI_MAX_SMALL_OBJECT_SIZE,
    LOKI_DEFAULT_OBJECT_ALIGNMENT, Loki::NoDestroy >
ImmortalObject;

class ImmortalSingleton : public ImmortalObject
{
public:

    typedef Loki::SingletonHolder< ImmortalSingleton, Loki::CreateUsingNew,
        Loki::NoDestroy>
        MySmallSingleton;

    /// Returns reference to the singleton.
    inline static ImmortalSingleton & Instance( void )
    {
        return MySmallSingleton::Instance();
    }

    ImmortalSingleton( void )
    {
        cout << "ImmortalSingleton created" << endl;
    }
    ~ImmortalSingleton( void )
    {
        cout << "~ImmortalSingleton destructor should never get called!" << endl;
    }
    void DoThat( void )
    {
        cout << "ImmortalSingleton::DoThat" << endl << endl;
    }
private:
    char m_stuff[ 16 ];
};

// ----------------------------------------------------------------------------
//
//            NoDestroy and SingletonWithLongevity policy combination
//
// ----------------------------------------------------------------------------

class MortalSingleton : public ImmortalObject
{
public:

    typedef Loki::SingletonHolder< MortalSingleton, Loki::CreateUsingNew,
        Loki::SingletonWithLongevity>
        MySmallSingleton;

    /// Returns reference to the singleton.
    inline static MortalSingleton & Instance( void )
    {
        return MySmallSingleton::Instance();
    }

    MortalSingleton( void )
    {
        cout << "MortalSingleton created" << endl;
    }
    ~MortalSingleton( void )
    {
        cout << "~MortalSingleton" << endl;
    }
    void DoThat( void )
    {
        cout << "MortalSingleton::DoThat" << endl << endl;
    }
private:
    char m_stuff[ 16 ];
};

inline unsigned int GetLongevity( MortalSingleton * )
{
    /// @note Must return a longevity level lower than the one in SmallObj.h.
    return 1;
}

// ----------------------------------------------------------------------------
//
//    detect memory leaks on MSVC Ide
//
// ----------------------------------------------------------------------------

//#define LOKI_MSVC_CHECK_FOR_MEMORY_LEAKS
#ifdef LOKI_MSVC_CHECK_FOR_MEMORY_LEAKS

#include <crtdbg.h>
#include <cassert>

void heap_debug()
{
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

    //tmpFlag |= _CRTDBG_CHECK_MasterLWMasterYS_DF;

    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );
}

#else

void heap_debug()
{}

#endif


// ----------------------------------------------------------------------------
//
//    main
//
// ----------------------------------------------------------------------------

int main()
{
    heap_debug();

    cout << endl
         << "This program tests the lifetime policies for Loki's " << endl
         << "Small-Object Allocator and singleton objects that are derived " << endl
         << "from SmallObject or SmallValueObject." << endl 
         << endl
         << "Use one of the following lifetime policies" << endl
         << "to manage the lifetime dependency:" << endl
         << endl 
         << "-  LongevityLifetime Parent/Child: SmallObject has" << endl
         << "       LongevityLifetime::DieAsSmallObjectParent" << endl
         << "   policy and the derived Singleton has " << endl
         << "       LongevityLifetime::DieAsSmallObjectChild" << endl
         << "   This is tested by the SmallObjectChild class." << endl
         << endl
         << "-  Both SmallObject and derived Singleton use" << endl
         << "       SingletonWithLongevity" << endl
         << "   policy. This is tested by the LongLivedSingleton class." << endl
         << endl
#if !defined(_MSC_VER) || (_MSC_VER>=1400)         
         << "-  FollowIntoDeath: SmallObject has" << endl
         << "       FollowIntoDeath::With<LIFETIME>::AsMasterLiftime" << endl
         << "   policy and the derived Singleton has " << endl
         << "       FollowIntoDeath::AfterMaster<MASTERSINGLETON>::IsDestroyed" << endl
         << "   policy. This is tested by the FollowerSingleton class." << endl
         << endl
#endif
         << "-  Both SmallObject and derived Singleton use" << endl
         << "       NoDestroy" << endl
         << "   policy. This is tested by the ImmortalSingleton class." << endl
         << "   Note: yow will get memory leaks" << endl
         << endl
         << "-  SmallObject has"<< endl
         << "       NoDestroy" << endl
         << "   policy but the derived Singleton has" << endl
         << "       SingletonWithLongevity" << endl
         << "   policy. This is tested by the MortalSingleton class." << endl
         << "   Note: yow will get memory leaks" << endl << endl

         << endl << endl
         << "If this program executes without crashing or asserting" << endl
         << "at exit time, then all policies work." << endl << endl;
    
    
    SmallObjectChild::Instance().DoThat();
    LongLivedSingleton::Instance().DoThat();
    
#if !defined(_MSC_VER) || (_MSC_VER>=1400)         
    FollowerSingleton::Instance().DoThat();
#endif
    
#define ENABLE_MEMORY_LEAK
#ifdef ENABLE_MEMORY_LEAK
    ImmortalSingleton::Instance().DoThat();
    MortalSingleton::Instance().DoThat();
#else
    cout << endl;
    cout << "ImmortalSingleton and MortalSingleton" << endl;
    cout << "are disabled due to the test on memory leaks.";
    cout << endl << endl;
#endif
    
#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    cout << endl<< endl << "now leaving main" << endl;
    cout << "________________________________" << endl << endl;

    return 0;
}

// ----------------------------------------------------------------------------

