////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: Dependencies.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#include <iostream>

#include <loki/SmallObj.h>
#include <loki/Function.h>


using namespace Loki;


////////////////////////////////////////////////////////////////////////////////////
//
//    Data object for all singletons
//
////////////////////////////////////////////////////////////////////////////////////

template<template <class> class T, int Nr>
struct SingletonDataObject
{
    SingletonDataObject() {std::cout<<"new SingletonDataObject"<<Nr<<"\n\n";}
    ~SingletonDataObject(){std::cout<<"delete SingletonDataObject"<<Nr<<"\n\n";}
    
    int i[Nr];
};



////////////////////////////////////////////////////////////////////////////////////
//
//    How to use LongevityLifetime policies 
//    DieAsSmallObjectParent/DieAsSmallObjectChild with SmallObjects?
//
////////////////////////////////////////////////////////////////////////////////////

//  LongevityLifetime::DieAsSmallObjectParent is the default lifetime 
//  of SmallObject template:
typedef Loki::SmallObject<> 
SmallObject_DieAs;

class MySmallObject_DieAs : public SmallObject_DieAs
{};

typedef SingletonHolder
<
    MySmallObject_DieAs, 
    CreateUsingNew, 
    LongevityLifetime::DieAsSmallObjectChild
>
Singleton_with_MySmallObject_DieAs;



////////////////////////////////////////////////////////////////////////////////////
//
//    How to use LongevityLifetime policies 
//    DieAsSmallObjectParent/DieAsSmallObjectChild with 
//    classes containing a Functor/Function?
//
////////////////////////////////////////////////////////////////////////////////////


struct MyFunctionObject_DieAs
{
    MyFunctionObject_DieAs()
    {
        functor  = Functor<void>    (this, &MyFunctionObject_DieAs::f);
        function = Function< void()>(this, &MyFunctionObject_DieAs::f);
    }

    void f(){}
    Functor<void> functor;
    Function<void()> function;
    
};

typedef SingletonHolder
<
    MyFunctionObject_DieAs, 
    CreateUsingNew, 
    LongevityLifetime::DieAsSmallObjectChild
>
Singleton_MyFunctionObject_DieAs;


////////////////////////////////////////////////////////////////////////////////////
//
//    Test code for LongevityLifetime Policy
//
////////////////////////////////////////////////////////////////////////////////////

template<template <class> class Lifetime>
struct Master_die_first
{
    Master_die_first() 
    {
        data = &Singleton::Instance();
    }

    virtual ~Master_die_first(){}

    typedef Master_die_first<Lifetime> ThisType;
    typedef SingletonDataObject<Lifetime,888> MasterSingleton;
    typedef SingletonHolder<MasterSingleton,CreateUsingNew,Lifetime> Singleton;

    MasterSingleton* data;
};

template<template <class> class Lifetime>
struct Master_die_last
{
    Master_die_last() 
    {
        data = &Singleton::Instance();
    }

    virtual ~Master_die_last(){}

    typedef Master_die_last<Lifetime> ThisType;
    typedef SingletonDataObject<Lifetime,555> MasterSingleton;
    typedef SingletonHolder<MasterSingleton,CreateUsingNew,Lifetime> Singleton;

    MasterSingleton* data;
};

typedef Master_die_first<LongevityLifetime::DieFirst>
Master1_die_first;

typedef Master_die_last<LongevityLifetime::DieLast>
Master1_die_last;

class B1_die_first;
class B1_die_last;

typedef SingletonHolder<B1_die_first,CreateUsingNew, 
LongevityLifetime::DieFirst
> Follower1_Singleton_B1_die_first;

typedef SingletonHolder<B1_die_last,CreateUsingNew, 
LongevityLifetime::DieLast
> Follower1_Singleton_B1_die_last;

class B1_die_first : public Master1_die_last
{
public:
    B1_die_first(){std::cout<<"new B1_die_first, look for SingletonDataObject555\n\n";}
    ~B1_die_first(){std::cout<<"delete B1_die_first, look for SingletonDataObject555\n\n";}
};

class B1_die_last : public Master1_die_first
{
public:
    B1_die_last(){std::cout<<"new B1_die_last,  look for SingletonDataObject888\n\n";}
    ~B1_die_last(){std::cout<<"delete B1_die_last,  look for SingletonDataObject888\n\n";}
};

// test of FollowIntoDeath policy, not supported by msvc 7.1 compiler
#if !defined(_MSC_VER) || (_MSC_VER>=1400)

////////////////////////////////////////////////////////////////////////////////////
//
//    How to use FollowIntoDeath with SmallObjects?
//
////////////////////////////////////////////////////////////////////////////////////

typedef Loki::SmallObject
< 
    LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    LOKI_DEFAULT_CHUNK_SIZE, 
    LOKI_MAX_SMALL_OBJECT_SIZE,
    LOKI_DEFAULT_OBJECT_ALIGNMENT, 
    FollowIntoDeath::With<DefaultLifetime>::AsMasterLifetime
>
MySmallObjectBase;

struct MySmallObject : public MySmallObjectBase
{
};

typedef SingletonHolder
<
    MySmallObject, 
    CreateUsingNew, 
    FollowIntoDeath::AfterMaster<MySmallObjectBase::ObjAllocatorSingleton>::IsDestroyed
>
Singleton_of_with_a_MySmallObject;



////////////////////////////////////////////////////////////////////////////////////
//
//    Test code for FollowIntoDeath policy
//
////////////////////////////////////////////////////////////////////////////////////


template<template <class> class Lifetime>
struct Master1
{
    Master1() 
    {
        data = &Singleton::Instance();
    }

    virtual ~Master1(){}

    typedef Master1<Lifetime> ThisType;
    typedef SingletonDataObject<Lifetime,1> MasterSingleton;
    typedef SingletonHolder<MasterSingleton,CreateUsingNew,Lifetime> Singleton;

    MasterSingleton* data;
};

typedef Master1<FollowIntoDeath::With<DefaultLifetime>::AsMasterLifetime>
Master1_DefaultLifetime;

typedef Master1<FollowIntoDeath::With<NoDestroy>::AsMasterLifetime>
Master1_NoDestroy;

typedef Master1<FollowIntoDeath::With<PhoenixSingleton>::AsMasterLifetime>
Master1_PhoenixSingleton;

typedef Master1<FollowIntoDeath::With<DeletableSingleton>::AsMasterLifetime>
Master1_DeletableSingleton;


class B1_DefaultLifetime : public Master1_DefaultLifetime
{
public:
    B1_DefaultLifetime(){std::cout<<"new B1_DefaultLifetime\n";}
    ~B1_DefaultLifetime(){std::cout<<"delete B1_DefaultLifetime\n";}
};

class B1_NoDestroy : public Master1_NoDestroy
{
public:
    B1_NoDestroy(){std::cout<<"new B1_NoDestroy. B1_NoDestroy must not be deleted\n";}
    ~B1_NoDestroy(){std::cout<<"delete B1_NoDestroy\n";};
};

class B1_PhoenixSingleton : public Master1_PhoenixSingleton
{
public:
    B1_PhoenixSingleton(){std::cout<<"new B1_PhoenixSingleton\n";}
    ~B1_PhoenixSingleton(){std::cout<<"delete B1_PhoenixSingleton\n";}
};

class B1_DeletableSingleton : public Master1_DeletableSingleton
{
public:
    B1_DeletableSingleton(){std::cout<<"new B1_DeletableSingleton\n";}
    ~B1_DeletableSingleton(){std::cout<<"delete B1_DeletableSingleton\n";}
};

typedef SingletonHolder<B1_DefaultLifetime,CreateUsingNew, 
FollowIntoDeath::AfterMaster<Master1_DefaultLifetime::MasterSingleton>::IsDestroyed
> Follower1_Singleton_DefaultLifetime;

typedef SingletonHolder<B1_NoDestroy,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master1_NoDestroy::MasterSingleton>::IsDestroyed
> Follower1_Singleton_NoDestroy;

typedef SingletonHolder<B1_PhoenixSingleton,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master1_PhoenixSingleton::MasterSingleton>::IsDestroyed
> Follower1_Singleton_PhoenixSingleton;

typedef SingletonHolder<B1_DeletableSingleton,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master1_DeletableSingleton::MasterSingleton>::IsDestroyed
> Follower1_Singleton_DeletableSingleton;


////////////////////////////////////////////////////////////////////////////////////
//
//    Test code for FollowIntoDeath policy with reverse instantiation
//
////////////////////////////////////////////////////////////////////////////////////

template<template <class> class Lifetime>
struct Master2
{
    Master2() 
    {
        // don't create a MasterSingleton2 object!!
        // to test the FollowIntoDeath policy
    }

    virtual ~Master2(){}

    typedef Master2<Lifetime> ThisType;
    typedef SingletonDataObject<Lifetime,2> MasterSingleton;
    typedef SingletonHolder<MasterSingleton,CreateUsingNew,Lifetime> Singleton;

    MasterSingleton* data;
};

typedef Master2<FollowIntoDeath::With<DefaultLifetime>::AsMasterLifetime>
Master2_DefaultLifetime;

typedef Master2<FollowIntoDeath::With<NoDestroy>::AsMasterLifetime>
Master2_NoDestroy;

typedef Master2<FollowIntoDeath::With<PhoenixSingleton>::AsMasterLifetime>
Master2_PhoenixSingleton;

typedef Master2<FollowIntoDeath::With<DeletableSingleton>::AsMasterLifetime>
Master2_DeletableSingleton;



class B2_DefaultLifetime : public Master2_DefaultLifetime
{
public:
    B2_DefaultLifetime(){std::cout<<"new B2_DefaultLifetime\n";}
    ~B2_DefaultLifetime(){std::cout<<"delete B2_DefaultLifetime\n";}
};

class B2_NoDestroy : public Master2_NoDestroy
{
public:
    B2_NoDestroy(){std::cout<<"new B2_NoDestroy. B2_NoDestroy must not be deleted\n";}
    ~B2_NoDestroy(){std::cout<<"delete B2_NoDestroy\n";};
};

class B2_PhoenixSingleton : public Master2_PhoenixSingleton
{
public:
    B2_PhoenixSingleton(){std::cout<<"new B2_PhoenixSingleton\n";}
    ~B2_PhoenixSingleton(){std::cout<<"delete B2_PhoenixSingleton\n";}
};

class B2_DeletableSingleton : public Master2_DeletableSingleton
{
public:
    B2_DeletableSingleton(){std::cout<<"new B2_DeletableSingleton\n";}
    ~B2_DeletableSingleton(){std::cout<<"delete B2_DeletableSingleton\n";}
};

typedef SingletonHolder<B2_DefaultLifetime,CreateUsingNew, 
FollowIntoDeath::AfterMaster<Master2_DefaultLifetime::MasterSingleton>::IsDestroyed
> Follower2_Singleton_DefaultLifetime;

typedef SingletonHolder<B2_NoDestroy,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master2_NoDestroy::MasterSingleton>::IsDestroyed
> Follower2_Singleton_NoDestroy;

typedef SingletonHolder<B2_PhoenixSingleton,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master2_PhoenixSingleton::MasterSingleton>::IsDestroyed
> Follower2_Singleton_PhoenixSingleton;

typedef SingletonHolder<B2_DeletableSingleton,CreateUsingNew, 
    FollowIntoDeath::AfterMaster<Master2_DeletableSingleton::MasterSingleton>::IsDestroyed
> Follower2_Singleton_DeletableSingleton;

#endif //#if !defined(_MSC_VER) || (_MSC_VER>=1400)

////////////////////////////////////////////////////////////////////////////////////
//
//    detect memory leaks on MSVC Ide
//
////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

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
#endif

////////////////////////////////////////////////////////////////////////////////////
//
//    main
//
////////////////////////////////////////////////////////////////////////////////////

int main()
{
    
#ifdef _MSC_VER
    heap_debug();
#endif

    void* p;

    p = static_cast<void*>(&Singleton_with_MySmallObject_DieAs::Instance());
    p = static_cast<void*>(&Singleton_MyFunctionObject_DieAs::Instance());

    std::cout<<"\n";

    p = static_cast<void*>(&Follower1_Singleton_B1_die_first::Instance());
    p = static_cast<void*>(&Follower1_Singleton_B1_die_last::Instance());


    // test of FollowIntoDeath policy, not supported by msvc 7.1 compiler
#if !defined(_MSC_VER) || (_MSC_VER>=1400)

    std::cout << "\nMaster1:\n\n";

    p = static_cast<void*>(&Follower1_Singleton_DefaultLifetime::Instance());
    p = static_cast<void*>(&Follower1_Singleton_PhoenixSingleton::Instance());
    p = static_cast<void*>(&Follower1_Singleton_DeletableSingleton::Instance());
    

    std::cout << "\n\nMaster2:\n\n";

    B2_DefaultLifetime *def2 = &Follower2_Singleton_DefaultLifetime::Instance();
    def2->data = &Master2_DefaultLifetime::Singleton::Instance();

    B2_PhoenixSingleton *pho2 = &Follower2_Singleton_PhoenixSingleton::Instance();
    pho2->data = &Master2_PhoenixSingleton::Singleton::Instance();

    B2_DeletableSingleton *del2 = &Follower2_Singleton_DeletableSingleton::Instance();
    del2->data = &Master2_DeletableSingleton::Singleton::Instance();
    
    // memory leak when code is enabled
//#define ENABLE_MEMORY_LEAK
#ifdef ENABLE_MEMORY_LEAK
    p = static_cast<void*>(&Follower1_Singleton_NoDestroy::Instance());
    B2_NoDestroy *no2 = &Follower2_Singleton_NoDestroy::Instance();
    no2->data = &Master2_NoDestroy::Singleton::Instance();
#endif

#endif //#if !defined(_MSC_VER) || (_MSC_VER>=1400)

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    std::cout << "\nnow leaving main \n";
    std::cout << "________________________________\n\n";

    return 0;
}
