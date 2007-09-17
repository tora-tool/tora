////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#define LOKI_CLASS_LEVEL_THREADING

#ifndef LOKI_CLASS_LEVEL_THREADING
#define LOKI_OBJECT_LEVEL_THREADING
#endif 

#include "Thread.h"

#include <loki/LockingPtr.h>
#include <loki/SafeFormat.h>

using namespace Loki;

int g;
int numThreads = 10;
int loop = 5;


struct A
{
    A(){};

#define  DO for(int i=0; i<10000000; i++) g++;

    void print(void* id) const
    {
        DO;Printf("%p: ----------------\n")(id);
        DO;Printf("%p: ---------------\n")(id);
        DO;Printf("%p: --------------\n")(id);
        DO;Printf("%p: -------------\n")(id);
        DO;Printf("%p: ------------\n")(id);
        DO;Printf("%p: -----------\n")(id);
        DO;Printf("%p: ----------\n")(id);
        DO;Printf("%p: ---------\n")(id);
        DO;Printf("%p: --------\n")(id);
        DO;Printf("%p: -------\n")(id);
        DO;Printf("%p: ------\n")(id);
        DO;Printf("%p: -----\n")(id);
        DO;Printf("%p: ----\n")(id);
        DO;Printf("%p: ---\n")(id);
        DO;Printf("%p: --\n")(id);
        DO;Printf("%p: -\n")(id);
        DO;Printf("%p: \n")(id);
        DO;Printf("%p: \n")(id);
    }
};

typedef Loki::LockingPtr<A,LOKI_DEFAULT_MUTEX,DontPropagateConst> UserLockingPtr;
typedef Loki::LockingPtr<A,LOKI_DEFAULT_MUTEX,PropagateConst> ConstUserLockingPtr;

void* RunLocked(void *id)
{
    volatile A a;
    static Loki::Mutex m;    
    for(int i=0; i<loop; i++)
    {
        UserLockingPtr l(a,m);
        l->print(id);
    }
    return 0;
}

void* RunConstLocked(void *id)
{
    const volatile A a;
    static Loki::Mutex m;    
    for(int i=0; i<loop; i++)
    {
        ConstUserLockingPtr l(a,m);
        l->print(id);
    }
    return 0;
}

void* Run(void *id)
{
    A a;
    for(int i=0; i<loop; i++)
        a.print(id);
    return 0;
}

int main ()
{
    std::vector<Thread*> threads;

    for(int i=0; i<numThreads; i++)
    {
        Printf("Creating thread %d\n")(i);
        threads.push_back(new Thread(RunLocked,reinterpret_cast<void*>(i)));
    }
    for(int i=0; i<numThreads; i++)
        threads.at(i)->start();

    Thread::JoinThreads(threads);
    Thread::DeleteThreads(threads);

    Printf("--------------------------------------------------------------------------------------\n");
    Printf("--------------------------------------------------------------------------------------\n");
    Printf("--------------------------------------------------------------------------------------\n");

    for(int i=0; i<numThreads; i++)
    {
        Printf("Creating thread %d\n")(i);
        threads.push_back(new Thread(Run,reinterpret_cast<void*>(i)));
    }
    for(int i=0; i<numThreads; i++)
        threads.at(i)->start();

    Thread::JoinThreads(threads);
    Thread::DeleteThreads(threads);

    
    // test pair ctor
    volatile A a;
    Loki::Mutex m;
    UserLockingPtr::Pair pair(&a,&m);    
    UserLockingPtr l( pair );
    
    ConstUserLockingPtr::Pair cpair(&a,&m);    
    ConstUserLockingPtr cl( cpair );

}


