////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 Peter Kümmel
// Copyright (c) 2005 Richard Sposato
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: SmallObjBench.cpp 808 2007-02-25 13:08:45Z syntheticpp $


// ----------------------------------------------------------------------------

#define LOKI_SMALL_OBJECT_USE_NEW_ARRAY

#ifndef LOKI_CLASS_LEVEL_THREADING
#define LOKI_CLASS_LEVEL_THREADING
#endif

#include <loki/SmallObj.h>
#include "timer.h"

#include <iostream>
#include <string>

//#define COMPARE_BOOST_POOL 
#ifdef COMPARE_BOOST_POOL
    #include <boost\pool\object_pool.hpp>
#endif

using namespace std;


// ----------------------------------------------------------------------------

template<unsigned int N>
class ThisIsASmallObject
{
    char data[N];
};

template<unsigned int N, class T>        
struct Base : public ThisIsASmallObject<N>, public T 
{};

template<unsigned int N>        
struct Base<N, void> : public ThisIsASmallObject<N> 
{};


// ----------------------------------------------------------------------------

#ifdef COMPARE_BOOST_POOL

template<unsigned int N>
class BoostPoolNew : public Base<N,void>
{
private:
    static boost::object_pool< BoostPoolNew<N> > BoostPool;

public:
    /// Throwing single-object new throws bad_alloc when allocation fails.
#ifdef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    static void * operator new ( std::size_t )
#else
    static void * operator new ( std::size_t ) throw ( std::bad_alloc )
#endif
    {
        return BoostPool.malloc();
    }

    /// Non-throwing single-object new returns NULL if allocation fails.
    static void * operator new ( std::size_t, const std::nothrow_t & ) throw ()
    {
        return BoostPool.malloc();
    }

    /// Placement single-object new merely calls global placement new.
    inline static void * operator new ( std::size_t size, void * place )
    {
        return ::operator new( size, place );
    }

    /// Single-object delete.
    static void operator delete ( void * p ) throw ()
    {
        BoostPool.free( reinterpret_cast< BoostPoolNew * >( p ) );
    }

    /** Non-throwing single-object delete is only called when nothrow
        new operator is used, and the constructor throws an exception.
        */
    static void operator delete ( void * p, const std::nothrow_t & ) throw()
    {
        BoostPool.free( reinterpret_cast< BoostPoolNew * >( p ) );
    }

    /// Placement single-object delete merely calls global placement delete.
    inline static void operator delete ( void * p, void * place )
    {
        ::operator delete ( p, place );
    }

    /** @note This class does not provide new [] and delete [] operators since
     the Boost.Pool allocator only works for memory requests of the same size.
     */
};

template<unsigned int N>
boost::object_pool< BoostPoolNew<N> > BoostPoolNew<N>::BoostPool;

#endif

// ----------------------------------------------------------------------------


int array_test_nr = 0;
double t100_new = 0;
double t100_delete = 0;

#define LOKI_SMALLOBJ_BENCH(FUNC, CODE_LOOP)                                 \
template<class T, int TN>                                                    \
int FUNC(void**, const int N, int loop, Timer& t, const char* s)             \
{                                                                            \
    t.start();                                                               \
    /****************************************************************/       \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_LOOP                                                            \
    }                                                                        \
    /****************************************************************/       \
    t.stop();                                                                \
    if(array_test_nr==0)                                                     \
        t.t100 = t.t();                                                      \
    array_test_nr++;                                                         \
    t.print(t.t(),s);                                                        \
    return t.t();                                                            \
}
#ifdef COMPARE_BOOST_POOL
    #define LOKI_BOOST_TEST_NR 3
#else
    #define LOKI_BOOST_TEST_NR -1
#endif

// ----------------------------------------------------------------------------

#define LOKI_SMALLOBJ_BENCH_ARRAY(FUNC, CODE_DECL, CODE_NEW, CODE_DELETE)    \
template<class T, int TN>                                                    \
int FUNC(void** arrv, const int N, int loop, Timer& t, const char* s)        \
{                                                                            \
                                                                             \
    CODE_DECL;                                                               \
    T** arr = reinterpret_cast<T**>(arrv);                                   \
    t.start();                                                               \
    /****************************************************************/       \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_NEW                                                             \
    }                                                                        \
    /****************************************************************/       \
    t.stop();                                                                \
    cout << "1. ";                                                           \
    if(array_test_nr==0)                                                     \
    {                                                                        \
        t.t100 = t.t();                                                      \
        t100_new = t.t100;                                                   \
    }                                                                        \
    else                                                                     \
        t.t100 = t100_new;                                                   \
    t.print(t.t(),s);                                                        \
                                                                             \
    if(array_test_nr==LOKI_BOOST_TEST_NR)                                    \
    {                                                                        \
        cout <<                                                              \
        "2. boost    :\tboost::object_pool is not tested because it's too slow"\
        << endl << endl;                                                     \
        array_test_nr++;                                                     \
        return t.t();                                                        \
    }                                                                        \
    t.start();                                                               \
    /****************************************************************/       \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_DELETE                                                          \
    }                                                                        \
    /****************************************************************/       \
    t.stop();                                                                \
    cout << "2. ";                                                           \
    if(array_test_nr==0)                                                     \
    {                                                                        \
        t.t100 = t.t();                                                      \
        t100_delete = t.t100;                                                \
    }                                                                        \
    else                                                                     \
        t.t100 = t100_delete;                                                \
    t.print(t.t(),s);                                                        \
    array_test_nr++;                                                         \
    cout << endl;                                                            \
    return t.t();                                                            \
}

// ----------------------------------------------------------------------------


LOKI_SMALLOBJ_BENCH(delete_new        ,delete new T;)
LOKI_SMALLOBJ_BENCH(delete_new_mal    ,std::free(std::malloc(sizeof(T)));)
LOKI_SMALLOBJ_BENCH(delete_new_all    ,std::allocator<T> st;st.deallocate(st.allocate(1), 1);)

LOKI_SMALLOBJ_BENCH(delete_new_array    ,delete[] new T[N];)
LOKI_SMALLOBJ_BENCH(delete_new_array_mal,std::free(std::malloc(sizeof(T[TN])));)
LOKI_SMALLOBJ_BENCH(delete_new_array_all,std::allocator<T[TN]> st;st.deallocate(st.allocate(1), 1);)

LOKI_SMALLOBJ_BENCH_ARRAY(new_del_on_arr    , , arr[i] = new T; , 
                                                delete arr[i];)
LOKI_SMALLOBJ_BENCH_ARRAY(new_del_on_arr_mal, , arr[i] = static_cast<T*>(std::malloc(sizeof(T))); , 
                                                std::free(arr[i]);)
LOKI_SMALLOBJ_BENCH_ARRAY(new_del_on_arr_all,    std::allocator<T> st , 
                                                arr[i]=st.allocate(1); , 
                                                st.deallocate(arr[i], 1);)

LOKI_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a    , , arr[i] = new T[TN]; , 
                                                delete[] arr[i];)
LOKI_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a_mal, , arr[i] = static_cast<T*>(std::malloc(sizeof(T[TN]))); , 
                                                std::free(arr[i]);)
LOKI_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a_all,std::allocator<T[TN]> st , 
                                                arr[i]=reinterpret_cast<T*>(st.allocate(1)); , 
                                                st.deallocate(reinterpret_cast<T(*)[TN]>(arr[i]), 1);)


// ----------------------------------------------------------------------------


#ifndef COMPARE_BOOST_POOL
#define LOKI_SMALL_OBJECT_BENCH_ABCD(FUNC,N,LOOP,TIMER,MESSAGE)              \
    array_test_nr = 0;                                                 \
    cout << MESSAGE << endl;                                           \
    FUNC<A,N>(a,N,LOOP,TIMER,"new      :");                            \
    FUNC<B,N>(a,N,LOOP,TIMER,"SmallObj :");                            \
    FUNC<C,N>(a,N,LOOP,TIMER,"ValueObj :");                            \
    FUNC##_all<A,N>(a,N,LOOP,TIMER,"allocator:");                      \
    FUNC##_mal<A,N>(a,N,LOOP,TIMER,"malloc   :");                      \
    cout << endl << endl;    
#else
#define LOKI_SMALL_OBJECT_BENCH_ABCD(FUNC,N,LOOP,TIMER,MESSAGE)              \
    array_test_nr = 0;                                                 \
    cout << MESSAGE << endl;                                           \
    FUNC<A,N>(a,N,LOOP,TIMER,"new      :");                            \
    FUNC<B,N>(a,N,LOOP,TIMER,"SmallObj :");                            \
    FUNC<C,N>(a,N,LOOP,TIMER,"ValueObj :");                            \
    FUNC<D,N>(a,N,LOOP,TIMER,"boost    :");                            \
    FUNC##_all<A,N>(a,N,LOOP,TIMER,"allocator:");                      \
    FUNC##_mal<A,N>(a,N,LOOP,TIMER,"malloc   :");                      \
    cout << endl << endl;
#endif

// ----------------------------------------------------------------------------

template<
    unsigned int Size,
    int loop,
    template <class, class> class ThreadingModel,
    std::size_t chunkSize,
    std::size_t maxSmallObjectSize,
    std::size_t objectAlignSize,
    template <class> class LifetimePolicy,
    class MutexPolicy
>
void testSize()
{

//#define LOKI_ALLOCATOR_PARAMETERS ::Loki::SingleThreaded, 4096, 128, 4, Loki::NoDestroy

    typedef Base<Size, void> A;
    typedef Base<Size, Loki::SmallObject< ThreadingModel, chunkSize,
        maxSmallObjectSize, objectAlignSize, LifetimePolicy, MutexPolicy > > B;
    typedef Base<Size, Loki::SmallValueObject< ThreadingModel, chunkSize,
        maxSmallObjectSize, objectAlignSize, LifetimePolicy, MutexPolicy > > C;
    typedef Loki::AllocatorSingleton< ThreadingModel, chunkSize,
        maxSmallObjectSize, objectAlignSize, LifetimePolicy, MutexPolicy > AllocatorSingleton;

#ifdef COMPARE_BOOST_POOL
    typedef BoostPoolNew<Size> D;
#endif

    assert( (!AllocatorSingleton::IsCorrupted()) );
    cout << endl << endl;
    cout << "Allocator Benchmark Tests with " << Size << " bytes big objects " << endl;
    cout << endl;
    cout << "new      = global new/delete     \tsizeof(A) = " << sizeof(A) << endl;
    cout << "SmallObj = Loki::SmallObject     \tsizeof(B) = " << sizeof(B) << endl;
    cout << "ValueObj = Loki::SmallValueObject\tsizeof(C) = " << sizeof(C) << endl;
#ifdef COMPARE_BOOST_POOL
    cout << "boost    = boost::object_pool    \tsizeof(D) = " << sizeof(D) << endl;
#endif
    cout << "allocator= std::allocator        \tsizeof(A) = " << sizeof(A) << endl;
    cout << "malloc   = std::malloc/free      \tsizeof(A) = " << sizeof(A) << endl;
    cout << endl << endl;

    Timer t;

    const int N = 3;    
    int Narr = 1000*1000;

    void** a= new void*[Narr];

    cout << loop  << " times ";
    LOKI_SMALL_OBJECT_BENCH_ABCD(delete_new        ,0,loop,t,"'delete new T'");
    assert( (!AllocatorSingleton::IsCorrupted()) );
    
    cout << "N=" << N <<" :  " << loop  << " times ";
    LOKI_SMALL_OBJECT_BENCH_ABCD(delete_new_array    ,N,loop,t,"'delete[] new T[N]'");
    assert( (!AllocatorSingleton::IsCorrupted()) );

    cout << "i=0..." << Narr << " :  ";
    LOKI_SMALL_OBJECT_BENCH_ABCD(new_del_on_arr    ,0,Narr,t,"1. 'arr[i] = new T'   2. 'delete arr[i]'");
    assert( (!AllocatorSingleton::IsCorrupted()) );
    
    cout << "i=0..." << Narr << ",  N=" << N <<" :  ";
    LOKI_SMALL_OBJECT_BENCH_ABCD(new_del_a_on_a    ,N,Narr,t,"1. 'arr[i] = new T[N]'   2. 'delete[] arr[i]'");
    assert( (!AllocatorSingleton::IsCorrupted()) );


    delete [] a;
    
    cout << "_________________________________________________________________" << endl;
    assert( (!AllocatorSingleton::IsCorrupted()) );
    AllocatorSingleton::ClearExtraMemory();
}

// ----------------------------------------------------------------------------

void DoSingleThreadTest (void)
{
    const int loop = 1000*1000;
    cout << endl;
    testSize<  2, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  3, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  8, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  9, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize< 16, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize< 17, loop, ::Loki::SingleThreaded, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif
}

// ----------------------------------------------------------------------------

#if defined(LOKI_CLASS_LEVEL_THREADING)

void DoClassLockTest (void)
{
    const int loop = 1000*1000;
    cout << endl;
    testSize<  2, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  3, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  8, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize<  9, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize< 16, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();
    testSize< 17, loop, ::Loki::ClassLevelLockable, 4096, 128, 4, ::Loki::NoDestroy, ::Loki::Mutex >();

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif
}

#endif

// ----------------------------------------------------------------------------

int main()
{
    DoSingleThreadTest();

#if defined(LOKI_CLASS_LEVEL_THREADING)
    DoClassLockTest();
#endif

    return 0;
}

// ----------------------------------------------------------------------------
