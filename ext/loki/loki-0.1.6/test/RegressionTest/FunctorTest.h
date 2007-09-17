///////////////////////////////////////////////////////////////////////////////
// Unit Test for Loki
//
// Copyright Terje Slettebø and Pavel Vozenilek 2002.
// Copyright Peter Kümmel, 2006

// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FUNCTORTEST_H
#define FUNCTORTEST_H

// $Id: FunctorTest.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/Functor.h>

///////////////////////////////////////////////////////////////////////////////
// FunctorTest
///////////////////////////////////////////////////////////////////////////////

void free_function(bool &result)
{
    result=true;
}

class FunctorTest : public Test
{
public:
    FunctorTest() : Test("Functor.h")
    {}

    virtual void execute(TestResult &result)
    {
        printName(result);

        using namespace Loki;

        bool r;

        TestFunctor testFunctor;
        TestClass testClass;


#ifndef LOKI_DISABLE_TYPELIST_MACROS

        Functor<void,LOKI_TYPELIST_1(bool &)> function(testFunction);
        Functor<void,LOKI_TYPELIST_1(bool &)> function2(testFunction);
        Functor<void,LOKI_TYPELIST_1(bool &)> functor(testFunctor);
        Functor<void,LOKI_TYPELIST_1(bool &)> functor2(testFunctor);
        Functor<void,LOKI_TYPELIST_1(bool &)> classFunctor(&testClass,&TestClass::member);
        Functor<void,LOKI_TYPELIST_1(bool &)> classFunctor2(&testClass,&TestClass::member);
        Functor<void,LOKI_TYPELIST_1(bool &)> functorCopy(function);
        Functor<void,LOKI_TYPELIST_1(bool &)> functorCopy2(function);

        Functor<void,NullType> bindFunctor(BindFirst(function,testResult));
        Functor<void,NullType> bindFunctor2(BindFirst(function,testResult));

        Functor<void> chainFunctor(Chain(bindFunctor,bindFunctor));
        Functor<void> chainFunctor2(Chain(bindFunctor,bindFunctor));

        Functor<void,LOKI_TYPELIST_1(bool &)> member_func(&testClass,&TestClass::member);
        Functor<void,LOKI_TYPELIST_1(bool &)> free_func(&free_function);
        Functor<void,LOKI_TYPELIST_1(bool &)> NULL_func;
        Functor<void,LOKI_TYPELIST_1(bool &)> NULL_func0;
#else

        Functor<void,Seq<bool &> > function(testFunction);
        Functor<void,Seq<bool &> > function2(testFunction);
        Functor<void,Seq<bool &> > functor(testFunctor);
        Functor<void,Seq<bool &> > functor2(testFunctor);
        Functor<void,Seq<bool &> > classFunctor(&testClass,&TestClass::member);
        Functor<void,Seq<bool &> > classFunctor2(&testClass,&TestClass::member);
        Functor<void,Seq<bool &> > functorCopy(function);
        Functor<void,Seq<bool &> > functorCopy2(function);

        //TODO:
        // BindFirst and Chainer

        Functor<void,Seq<bool &> > member_func(&testClass,&TestClass::member);
        Functor<void,Seq<bool &> > free_func(&free_function);
        Functor<void,Seq<bool &> > NULL_func;
        Functor<void,Seq<bool &> > NULL_func0;
#endif

        testResult=false;
        function(testResult);
        bool functionResult=testResult;

        testResult=false;
        functor(testResult);
        bool functorResult=testResult;

        testResult=false;
        classFunctor(testResult);
        bool classFunctorResult=testResult;

        testResult=false;
        functorCopy(testResult);
        bool functorCopyResult=testResult;

#ifdef LOKI_FUNCTORS_ARE_COMPARABLE

        bool functionCompare = function==function2;
        bool functorCompare = functor!=functor2;  // is this a bug?
        bool classFunctorCompare = classFunctor==classFunctor2;
        bool functorCopyCompare = functorCopy==functorCopy2;

        bool free_mem = free_func!=member_func;
        bool mem_free = member_func!=free_func;

        bool null0 = NULL_func == NULL_func0;
        bool null1 = NULL_func != free_func;
        bool null2 = NULL_func != member_func;
        bool null3 = free_func != NULL_func;
        bool null4 = member_func != NULL_func;


#ifndef LOKI_DISABLE_TYPELIST_MACROS

        bool bindFunctorCompare = bindFunctor==bindFunctor2;
        bool chainFunctorCompare = chainFunctor==chainFunctor2;
#endif

        bool compare =  functionCompare &&
                        functorCompare &&
                        classFunctorCompare &&
                        functorCopyCompare &&
                        mem_free &&
                        free_mem &&
                        null0 &&
                        null1 &&
                        null2 &&
                        null3 &&
                        null4
#ifndef LOKI_DISABLE_TYPELIST_MACROS
                        && bindFunctorCompare
                        && chainFunctorCompare;
#else
                        ;
#endif

#else

        bool compare=true;
#endif //LOKI_FUNCTORS_ARE_COMPARABLE


#ifndef LOKI_DISABLE_TYPELIST_MACROS

        testResult=false;
        bindFunctor();
        bool bindFunctorResult=testResult;

        testResult=false;
        chainFunctor();
        bool chainFunctorResult=testResult;

        r=functionResult && functorResult && classFunctorResult && functorCopyResult && bindFunctorResult &&
          chainFunctorResult && compare;
#else
        //TODO!
        r=functionResult && functorResult && classFunctorResult && functorCopyResult && compare;
#endif

        testAssert("Functor",r,result);

        std::cout << '\n';
    }

private:
    static bool testResult;

    static void testFunction(bool &result)
    {
        result=true;
    }

    class TestFunctor
    {
    public:
        void operator()(bool &result)
        {
            result=true;
        }
        bool operator==(const TestFunctor& rhs) const
        {
            const TestFunctor* p = &rhs;
            return this==p;
        }
    };

    class TestClass
    {
    public:
        void member(bool &result)
        {
            result=true;
        }
    };
}
functorTest;

bool FunctorTest::testResult;

#ifndef SMALLOBJ_CPP
# define SMALLOBJ_CPP
# include "../../src/SmallObj.cpp"
#endif
#endif
