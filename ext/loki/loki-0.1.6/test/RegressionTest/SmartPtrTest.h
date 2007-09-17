///////////////////////////////////////////////////////////////////////////////
// Unit Test for Loki
//
// Copyright Terje Slettebø and Pavel Vozenilek 2002.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SMARTPTRTEST_H
#define SMARTPTRTEST_H

// $Id: SmartPtrTest.h 760 2006-10-17 20:36:13Z syntheticpp $


#include "UnitTest.h"
#include <loki/SmartPtr.h>
#include <cassert>
#include <algorithm>


#if 0 // throw assert on failed test
#define LOKI_assert(x) assert(x)
#else
#define LOKI_assert(x) 
#endif

using namespace Loki;

// friend injection
// see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=28597
class TestClass;
typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, AssertCheck, DefaultSPStorage> Class;

bool Compare( const Class& a, const Class& b )
{
    return true; 
}

void  friend_injection()
{
    std::vector<Class> vec;
    std::sort( vec.begin(), vec.end(), Compare );
    std::nth_element( vec.begin(), vec.begin(), vec.end(), Compare );
    std::search( vec.begin(), vec.end(),
        vec.begin(), vec.end(), Compare );
    Class a, b;
    Compare( a, b );
}

///////////////////////////////////////////////////////////////////////////////
// SmartPtrTest
///////////////////////////////////////////////////////////////////////////////

class TestClass
{
public:
  TestClass() : references(1)
  {
    ++instances;
  }

  ~TestClass()
  {
    --instances;
  }

  void AddRef()
  {
    ++references;
  }

  void Release()
  {
  --references;

  if (references <= 0)
    delete this;
  }

  TestClass* Clone()
  {
    return new TestClass(*this);
  }



public:
  static int instances;

  int references;
};

int TestClass::instances = 0;

class SmartPtrTest : public Test
{
public:
  SmartPtrTest() : Test("SmartPtr.h") {}

  virtual void execute(TestResult &result)
    {
    printName(result);

    using namespace Loki;

    { SmartPtr<TestClass> p = new TestClass; }

    bool test1=TestClass::instances == 0;

    { p0 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p1 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p2 p(new TestClass); p2 pp(p); } LOKI_assert(TestClass::instances==0);
    { p3 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p4 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p5 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p6 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p7 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p8 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p9 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p10 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p11 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p12 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p13 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p14 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p15 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p16 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p17 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p18 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p19 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p20 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p21 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p22 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p23 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p24 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p25 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p26 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p27 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p28 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p29 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p30 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p31 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p40 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p41 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p42 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p43 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p44 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p45 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p46 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p47 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p48 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p49 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p50 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p51 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p52 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p53 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p54 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p55 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p56 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p57 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p58 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p59 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p60 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p61 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p62 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p63 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p64 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p65 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p66 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p67 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p68 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p69 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p70 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p71 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p72 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p73 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p74 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p75 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p76 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p77 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p78 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p79 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p80 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p81 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p82 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p83 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p84 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p85 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p86 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p87 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p88 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p89 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p90 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p91 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p92 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p93 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p94 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p95 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p96 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p97 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p98 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p99 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p100 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p101 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p102 p(new TestClass); } LOKI_assert(TestClass::instances==0);
    { p103 p(new TestClass); }

    bool test2=TestClass::instances==0;

    bool r=test1 && test2;

    testAssert("SmartPtr",r,result);

    std::cout << '\n';
    }

private:
  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, AssertCheck, DefaultSPStorage> p0;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, AssertCheck, DefaultSPStorage> p1;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, AssertCheck, DefaultSPStorage> p2;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, AssertCheck, DefaultSPStorage> p3;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, AssertCheck, DefaultSPStorage> p4;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, AssertCheck, DefaultSPStorage> p5;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, AssertCheck, DefaultSPStorage> p6;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, AssertCheck, DefaultSPStorage> p7;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, AssertCheck, DefaultSPStorage> p8;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, AssertCheck, DefaultSPStorage> p9;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, AssertCheck, DefaultSPStorage> p10;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, AssertCheck, DefaultSPStorage> p11;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, AssertCheck, DefaultSPStorage> p12;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, AssertCheck, DefaultSPStorage> p13;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, AssertCheck, DefaultSPStorage> p14;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, AssertCheck, DefaultSPStorage> p15;

  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p16;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p17;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, AssertCheckStrict, DefaultSPStorage> p18;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p19;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p20;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p21;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p22;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, AssertCheckStrict, DefaultSPStorage> p23;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, AssertCheckStrict, DefaultSPStorage> p24;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, AssertCheckStrict, DefaultSPStorage> p25;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, AssertCheckStrict, DefaultSPStorage> p26;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, AssertCheckStrict, DefaultSPStorage> p27;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, AssertCheckStrict, DefaultSPStorage> p28;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, AssertCheckStrict, DefaultSPStorage> p29;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, AssertCheckStrict, DefaultSPStorage> p30;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, AssertCheckStrict, DefaultSPStorage> p31;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, RejectNullStatic, DefaultSPStorage> p40;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, RejectNullStatic, DefaultSPStorage> p41;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, RejectNullStatic, DefaultSPStorage> p42;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, RejectNullStatic, DefaultSPStorage> p43;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, RejectNullStatic, DefaultSPStorage> p44;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, RejectNullStatic, DefaultSPStorage> p45;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNullStatic, DefaultSPStorage> p46;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNullStatic, DefaultSPStorage> p47;

  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage> p48;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, RejectNullStatic, DefaultSPStorage> p49;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, RejectNullStatic, DefaultSPStorage> p50;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, RejectNullStatic, DefaultSPStorage> p51;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, RejectNullStatic, DefaultSPStorage> p52;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage> p53;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNullStatic,DefaultSPStorage> p54;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNullStatic,DefaultSPStorage> p55;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, RejectNull, DefaultSPStorage> p56;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, RejectNull, DefaultSPStorage> p57;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, RejectNull, DefaultSPStorage> p58;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, RejectNull, DefaultSPStorage> p59;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, RejectNull, DefaultSPStorage> p60;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, RejectNull, DefaultSPStorage> p61;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNull, DefaultSPStorage> p62;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNull, DefaultSPStorage> p63;

  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, RejectNull,  DefaultSPStorage> p64;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, RejectNull,  DefaultSPStorage> p65;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, RejectNull,  DefaultSPStorage> p66;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, RejectNull,  DefaultSPStorage> p67;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, RejectNull,  DefaultSPStorage> p68;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, RejectNull,  DefaultSPStorage> p69;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNull,  DefaultSPStorage> p70;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNull,  DefaultSPStorage> p71;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, RejectNullStrict, DefaultSPStorage> p72;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, RejectNullStrict, DefaultSPStorage> p73;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, RejectNullStrict, DefaultSPStorage> p74;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, RejectNullStrict, DefaultSPStorage> p75;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, RejectNullStrict, DefaultSPStorage> p76;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, RejectNullStrict, DefaultSPStorage> p77;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNullStrict, DefaultSPStorage> p78;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, RejectNullStrict, DefaultSPStorage> p79;

  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, RejectNullStrict, DefaultSPStorage> p80;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, RejectNullStrict, DefaultSPStorage> p81;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, RejectNullStrict, DefaultSPStorage> p82;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, RejectNullStrict, DefaultSPStorage> p83;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, RejectNullStrict, DefaultSPStorage> p84;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, RejectNullStrict, DefaultSPStorage> p85;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNullStrict, DefaultSPStorage> p86;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, RejectNullStrict, DefaultSPStorage> p87;

  typedef SmartPtr<TestClass, DeepCopy, AllowConversion, NoCheck, DefaultSPStorage> p88;
  typedef SmartPtr<TestClass, RefCounted, AllowConversion, NoCheck, DefaultSPStorage> p89;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      AllowConversion, NoCheck, DefaultSPStorage> p90;
  typedef SmartPtr<TestClass, COMRefCounted, AllowConversion, NoCheck, DefaultSPStorage> p91;
  typedef SmartPtr<TestClass, RefLinked, AllowConversion, NoCheck, DefaultSPStorage> p92;
  typedef SmartPtr<TestClass, DestructiveCopy, AllowConversion, NoCheck, DefaultSPStorage> p93;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, NoCheck, DefaultSPStorage> p94;
  typedef SmartPtr<TestClass, NoCopy, AllowConversion, NoCheck, DefaultSPStorage> p95;

  typedef SmartPtr<TestClass, DeepCopy, DisallowConversion, NoCheck, DefaultSPStorage> p96;
  typedef SmartPtr<TestClass, RefCounted, DisallowConversion, NoCheck, DefaultSPStorage> p97;
  typedef SmartPtr<TestClass, RefCountedMTAdj<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::RefCountedMT, 
      DisallowConversion, NoCheck, DefaultSPStorage> p98;
  typedef SmartPtr<TestClass, COMRefCounted, DisallowConversion, NoCheck, DefaultSPStorage> p99;
  typedef SmartPtr<TestClass, RefLinked, DisallowConversion, NoCheck, DefaultSPStorage> p100;
  typedef SmartPtr<TestClass, DestructiveCopy, DisallowConversion, NoCheck, DefaultSPStorage> p101;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, NoCheck, DefaultSPStorage> p102;
  typedef SmartPtr<TestClass, NoCopy, DisallowConversion, NoCheck, DefaultSPStorage> p103;
} smartPtrTest;


#ifndef SMARTPTR_CPP
#    define SMARTPTR_CPP
#    include "../../src/SmartPtr.cpp"
#endif

#endif
