///////////////////////////////////////////////////////////////////////////////
// Unit Test for Loki
//
// Copyright Peter Kümmel 2005
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef THREADSTEST_H
#define THREADSTEST_H

// $Id: ThreadsTest.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/Threads.h>
#include "UnitTest.h"

namespace ThreadsTestPrivate
{
    class SingleLevel : public Loki::SingleThreaded<SingleLevel>
    {
        int i;
    public:
        void test()
        {
            Lock lock0;
            Lock lock(*this);
            Lock lockThis(this);
            i++;
        }
    };

#if defined(LOKI_CLASS_LEVEL_THREADING) || defined(LOKI_OBJECT_LEVEL_THREADING)

    class ClassLevel : public Loki::ClassLevelLockable<ClassLevel>
    {
        int i;
    public:
        void test()
        {
            Lock lock0;
            Lock lock(*this);
            Lock lockThis(this);
            i++;
        }
    };

    class ObjectLevel : public Loki::ObjectLevelLockable<ObjectLevel>
    {
        int i;
    public:
        void test()
        {
            //Lock lock0_must_not_compile;
            Lock lock(*this);
            Lock lockThis(this);
            i++;
        }
    };

#endif

}//namespace Loki


class ThreadsTest : public Test
{
public:
  ThreadsTest() : Test("Threads.h") {}

  virtual void execute(TestResult &result)
    {
    printName(result);

    bool r = true; // TODO some tests

    testAssert("Threads",r,result);

    std::cout << '\n';
    }
} threadsTest;

#endif
