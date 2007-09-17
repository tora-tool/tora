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
#ifndef TYPELISTTEST_H
#define TYPELISTTEST_H

// $Id: TypelistTest.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/Typelist.h>
#include <loki/Sequence.h>

///////////////////////////////////////////////////////////////////////////////
// TypelistTest
///////////////////////////////////////////////////////////////////////////////

class TypelistTest : public Test
{
public:
  TypelistTest() : Test("Typelist.h") {}

  virtual void execute(TestResult &result)
    {
    printName(result);

    using namespace Loki;
    using namespace Loki::TL;

#ifndef LOKI_DISABLE_TYPELIST_MACROS
    typedef LOKI_TYPELIST_1(char) CharList;
    typedef LOKI_TYPELIST_3(char,int,double) CharIntDoubleList;
    typedef LOKI_TYPELIST_4(char,int,double,char) CharIntDoubleCharList;
    typedef LOKI_TYPELIST_3(Base,Derived1,Derived2) BaseDerived1Derived2List;
    typedef LOKI_TYPELIST_3(Derived2,Derived1,Base) Derived2Derived1BaseList;
    typedef LOKI_TYPELIST_4(Base,Derived1,Base,Derived2) BaseDerived1BaseDerived2List;
    typedef LOKI_TYPELIST_4(Derived1,Base,Derived1,Derived2) Derived1BaseDerived1Derived2List;
#else
    typedef Seq<char>::Type CharList;
    typedef Seq<char,int,double>::Type CharIntDoubleList;
    typedef Seq<char,int,double,char>::Type CharIntDoubleCharList;
    typedef Seq<Base,Derived1,Derived2>::Type BaseDerived1Derived2List;
    typedef Seq<Derived2,Derived1,Base>::Type Derived2Derived1BaseList;
    typedef Seq<Base,Derived1,Base,Derived2>::Type BaseDerived1BaseDerived2List;
    typedef Seq<Derived1,Base,Derived1,Derived2>::Type Derived1BaseDerived1Derived2List;
#endif

    bool r;

    r=Length<NullType>::value==0 &&
      Length<CharList>::value==1 &&
      Length<CharIntDoubleList>::value==3;

    testAssert("Length",r,result);

    r=SameType<TypeAt<CharList,0>::Result,char>::value &&
      SameType<TypeAt<CharIntDoubleList,2>::Result,double>::value;

    testAssert("TypeAt",r,result);

    #if !(defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__MWERKS__) && _MSC_VER < 1300)

    // TypeAtNonStrict works like TypeAt on MSVC 6.0

    r=SameType<TypeAtNonStrict<NullType,0>::Result,NullType>::value &&
      SameType<TypeAtNonStrict<CharList,0>::Result,char>::value &&
      SameType<TypeAtNonStrict<CharIntDoubleList,2>::Result,double>::value &&
      SameType<TypeAtNonStrict<CharIntDoubleList,3>::Result,NullType>::value &&
      SameType<TypeAtNonStrict<CharList,1,long>::Result,long>::value;

    testAssert("TypeAtNonStrict",r,result);

    #else

    testAssert("TypeAtNonStrict",false,result,false);

    #endif

    r=IndexOf<NullType,char>::value==-1 &&
      IndexOf<CharList,char>::value==0 &&
      IndexOf<CharIntDoubleList,double>::value==2 &&
      IndexOf<CharIntDoubleList,long>::value==-1;

    testAssert("IndexOf",r,result);

    #if !(defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__MWERKS__) && _MSC_VER < 1300)

    // Append, Erase, EraseAll, NoDuplicates, Replace, ReplaceAll, Reverse,
    // MostDerived and DerivedToFront doesn't work on MSVC 6.0

#ifndef LOKI_DISABLE_TYPELIST_MACROS

    r=SameType<Append<NullType,NullType>::Result,NullType>::value &&
      SameType<Append<NullType,char>::Result,LOKI_TYPELIST_1(char)>::value &&
      SameType<Append<NullType,CharList>::Result,CharList>::value &&
      SameType<Append<CharList,NullType>::Result,CharList>::value &&
      SameType<Append<CharList,int>::Result,LOKI_TYPELIST_2(char,int)>::value &&
      SameType<Append<CharList,CharIntDoubleList>::Result,LOKI_TYPELIST_4(char,char,int,double)>::value;

    testAssert("Append",r,result);

    r=SameType<Erase<NullType,char>::Result,NullType>::value &&
      SameType<Erase<CharList,char>::Result,NullType>::value &&
      SameType<Erase<CharList,long>::Result,CharList>::value &&
      SameType<Erase<CharIntDoubleList,int>::Result,LOKI_TYPELIST_2(char,double)>::value &&
      SameType<Erase<CharIntDoubleList,double>::Result,LOKI_TYPELIST_2(char,int)>::value;

    testAssert("Erase",r,result);

    r=SameType<EraseAll<NullType,char>::Result,NullType>::value &&
      SameType<EraseAll<CharList,char>::Result,NullType>::value &&
      SameType<EraseAll<CharList,long>::Result,CharList>::value &&
      SameType<EraseAll<CharIntDoubleList,int>::Result,LOKI_TYPELIST_2(char,double)>::value &&
      SameType<EraseAll<CharIntDoubleList,double>::Result,LOKI_TYPELIST_2(char,int)>::value &&
      SameType<EraseAll<CharIntDoubleCharList,char>::Result,LOKI_TYPELIST_2(int,double)>::value &&
      SameType<EraseAll<CharIntDoubleCharList,int>::Result,LOKI_TYPELIST_3(char,double,char)>::value &&
      SameType<EraseAll<CharIntDoubleCharList,double>::Result,LOKI_TYPELIST_3(char,int,char)>::value;

    testAssert("EraseAll",r,result);

    r=SameType<NoDuplicates<NullType>::Result,NullType>::value &&
      SameType<NoDuplicates<CharList>::Result,CharList>::value &&
      SameType<NoDuplicates<CharIntDoubleList>::Result,CharIntDoubleList>::value &&
      SameType<NoDuplicates<CharIntDoubleCharList>::Result,CharIntDoubleList>::value;

    testAssert("NoDuplicates",r,result);

    r=SameType<Replace<NullType,char,long>::Result,NullType>::value &&
      SameType<Replace<CharList,char,long>::Result,LOKI_TYPELIST_1(long)>::value &&
      SameType<Replace<CharList,int,long>::Result,CharList>::value &&
      SameType<Replace<CharIntDoubleList,char,long>::Result,LOKI_TYPELIST_3(long,int,double)>::value &&
      SameType<Replace<CharIntDoubleList,long,char[16]>::Result,CharIntDoubleList>::value &&
      SameType<Replace<CharIntDoubleCharList,char,long>::Result,LOKI_TYPELIST_4(long,int,double,char)>::value;

    testAssert("Replace",r,result);

    r=SameType<ReplaceAll<NullType,char,long>::Result,NullType>::value &&
      SameType<ReplaceAll<CharList,char,long>::Result,LOKI_TYPELIST_1(long)>::value &&
      SameType<ReplaceAll<CharList,int,long>::Result,CharList>::value &&
      SameType<ReplaceAll<CharIntDoubleList,char,long>::Result,LOKI_TYPELIST_3(long,int,double)>::value &&
      SameType<ReplaceAll<CharIntDoubleList,long,char[16]>::Result,CharIntDoubleList>::value &&
      SameType<ReplaceAll<CharIntDoubleCharList,char,long>::Result,LOKI_TYPELIST_4(long,int,double,long)>::value;

    testAssert("ReplaceAll",r,result);

    r=SameType<Reverse<NullType>::Result,NullType>::value &&
      SameType<Reverse<CharList>::Result,CharList>::value &&
      SameType<Reverse<CharIntDoubleList>::Result,LOKI_TYPELIST_3(double,int,char)>::value;

    testAssert("Reverse",r,result);

    r=SameType<MostDerived<NullType,Base>::Result,Base>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Base>::Result,Derived2>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Derived1>::Result,Derived2>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Derived2>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Base>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Derived1>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Derived2>::Result,Derived2>::value;

    testAssert("MostDerived",r,result);

    r=SameType<DerivedToFront<NullType>::Result,NullType>::value &&
      SameType<DerivedToFront<CharList>::Result,CharList>::value &&
      SameType<DerivedToFront<CharIntDoubleList>::Result,CharIntDoubleList>::value &&
      SameType<DerivedToFront<CharIntDoubleCharList>::Result,CharIntDoubleCharList>::value &&
      SameType<DerivedToFront<BaseDerived1Derived2List>::Result,Derived2Derived1BaseList>::value &&
      SameType<DerivedToFront<Derived2Derived1BaseList>::Result,Derived2Derived1BaseList>::value &&
      SameType<DerivedToFront<BaseDerived1BaseDerived2List>::Result,LOKI_TYPELIST_4(Derived2,Derived1,Base,Base)>::value &&
      SameType<DerivedToFront<Derived1BaseDerived1Derived2List>::Result,LOKI_TYPELIST_4(Derived2,Derived1,Derived1,Base)>::value;

      testAssert("DerivedToFront",r,result);

#else //LOKI_DISABLE_TYPELIST_MACROS

    r=SameType<Append<NullType,NullType>::Result,NullType>::value &&
        SameType<Append<NullType,char>::Result,Seq<char>::Type >::value &&
      SameType<Append<NullType,CharList>::Result,CharList>::value &&
      SameType<Append<CharList,NullType>::Result,CharList>::value &&
      SameType<Append<CharList,int>::Result,Seq<char,int>::Type >::value &&
      SameType<Append<CharList,CharIntDoubleList>::Result,Seq<char,char,int,double>::Type >::value;

    testAssert("Append",r,result);

    r=SameType<Erase<NullType,char>::Result,NullType>::value &&
      SameType<Erase<CharList,char>::Result,NullType>::value &&
      SameType<Erase<CharList,long>::Result,CharList>::value &&
      SameType<Erase<CharIntDoubleList,int>::Result,Seq<char,double>::Type >::value &&
      SameType<Erase<CharIntDoubleList,double>::Result,Seq<char,int>::Type >::value;

    testAssert("Erase",r,result);

    r=SameType<EraseAll<NullType,char>::Result,NullType>::value &&
      SameType<EraseAll<CharList,char>::Result,NullType>::value &&
      SameType<EraseAll<CharList,long>::Result,CharList>::value &&
      SameType<EraseAll<CharIntDoubleList,int>::Result,Seq<char,double>::Type >::value &&
      SameType<EraseAll<CharIntDoubleList,double>::Result,Seq<char,int>::Type >::value &&
      SameType<EraseAll<CharIntDoubleCharList,char>::Result,Seq<int,double>::Type >::value &&
      SameType<EraseAll<CharIntDoubleCharList,int>::Result,Seq<char,double,char>::Type >::value &&
      SameType<EraseAll<CharIntDoubleCharList,double>::Result,Seq<char,int,char>::Type >::value;

    testAssert("EraseAll",r,result);

    r=SameType<NoDuplicates<NullType>::Result,NullType>::value &&
      SameType<NoDuplicates<CharList>::Result,CharList>::value &&
      SameType<NoDuplicates<CharIntDoubleList>::Result,CharIntDoubleList>::value &&
      SameType<NoDuplicates<CharIntDoubleCharList>::Result,CharIntDoubleList>::value;

    testAssert("NoDuplicates",r,result);

    r=SameType<Replace<NullType,char,long>::Result,NullType>::value &&
      SameType<Replace<CharList,char,long>::Result,Seq<long>::Type >::value &&
      SameType<Replace<CharList,int,long>::Result,CharList>::value &&
      SameType<Replace<CharIntDoubleList,char,long>::Result,Seq<long,int,double>::Type >::value &&
      SameType<Replace<CharIntDoubleList,long,char[16]>::Result,CharIntDoubleList>::value &&
      SameType<Replace<CharIntDoubleCharList,char,long>::Result,Seq<long,int,double,char>::Type >::value;

    testAssert("Replace",r,result);

    r=SameType<ReplaceAll<NullType,char,long>::Result,NullType>::value &&
      SameType<ReplaceAll<CharList,char,long>::Result,Seq<long>::Type >::value &&
      SameType<ReplaceAll<CharList,int,long>::Result,CharList>::value &&
      SameType<ReplaceAll<CharIntDoubleList,char,long>::Result,Seq<long,int,double>::Type >::value &&
      SameType<ReplaceAll<CharIntDoubleList,long,char[16]>::Result,CharIntDoubleList>::value &&
      SameType<ReplaceAll<CharIntDoubleCharList,char,long>::Result,Seq<long,int,double,long>::Type >::value;

    testAssert("ReplaceAll",r,result);

    r=SameType<Reverse<NullType>::Result,NullType>::value &&
      SameType<Reverse<CharList>::Result,CharList>::value &&
      SameType<Reverse<CharIntDoubleList>::Result,Seq<double,int,char>::Type >::value;

    testAssert("Reverse",r,result);

    r=SameType<MostDerived<NullType,Base>::Result,Base>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Base>::Result,Derived2>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Derived1>::Result,Derived2>::value &&
      SameType<MostDerived<BaseDerived1Derived2List,Derived2>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Base>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Derived1>::Result,Derived2>::value &&
      SameType<MostDerived<Derived2Derived1BaseList,Derived2>::Result,Derived2>::value;

    testAssert("MostDerived",r,result);

    r=SameType<DerivedToFront<NullType>::Result,NullType>::value &&
      SameType<DerivedToFront<CharList>::Result,CharList>::value &&
      SameType<DerivedToFront<CharIntDoubleList>::Result,CharIntDoubleList>::value &&
      SameType<DerivedToFront<CharIntDoubleCharList>::Result,CharIntDoubleCharList>::value &&
      SameType<DerivedToFront<BaseDerived1Derived2List>::Result,Derived2Derived1BaseList>::value &&
      SameType<DerivedToFront<Derived2Derived1BaseList>::Result,Derived2Derived1BaseList>::value &&
      SameType<DerivedToFront<BaseDerived1BaseDerived2List>::Result,Seq<Derived2,Derived1,Base,Base>::Type >::value &&
      SameType<DerivedToFront<Derived1BaseDerived1Derived2List>::Result,Seq<Derived2,Derived1,Derived1,Base>::Type >::value;

      testAssert("DerivedToFront",r,result);

#endif //LOKI_DISABLE_TYPELIST_MACROS



    #else

    testAssert("Append",false,result,false);
    testAssert("Erase",false,result,false);
    testAssert("EraseAll",false,result,false);
    testAssert("NoDuplicates",false,result,false);
    testAssert("Replace",false,result,false);
    testAssert("Reverse",false,result,false);
    testAssert("MostDerived",false,result,false);
    testAssert("DerivedToFront",false,result,false);

    #endif

    std::cout << '\n';
    }

private:
  struct Base { char c; };
  struct Derived1 : Base { char c; };
  struct Derived2 : Derived1 { char c; };
} typelistTest;
#endif
