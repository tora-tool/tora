///////////////////////////////////////////////////////////////////////////////
// Unit Test for Loki
//
// Copyright (C) 2002 Terje Slettebø
// Copyright (C) 2002 Pavel Vozenilek
// Copyright (C) 2005 Peter Kümmel

// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FACTORYPARMTEST_H
#define FACTORYPARMTEST_H

// $Id: FactoryParmTest.h 761 2006-10-17 20:48:18Z syntheticpp $


#ifdef ENABLE_NEW_FACTORY_CODE

#include <loki/Factory.h>

using Loki::Factory;
using Loki::SingletonHolder;

///////////////////////////////////////////////////////////////////////////////
// FactoryPArmTest
///////////////////////////////////////////////////////////////////////////////

namespace FactoryTestParmPrivate
{
    class AbstractProduct
    {
    };

    class Product : public AbstractProduct
    {
    public:
        Product()
        {}
        Product(int)
        {}
        Product(int, int)
        {}
        Product(int, int, int)
        {}
        Product(int, int, int, int)
        {}
        Product(int, int, int, int, int)
        {}
        Product(int, int, int, int, int,
                int)
        {}
        Product(int, int, int, int, int,
                int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int,
                int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int,
                int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int,
                int, int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int,
                int, int, int, int)
        {}
        Product(int, int, int, int, int,
                int, int, int, int, int,
                int, int, int, int, int)
        {}

    };


    template< class T>
    class CreatorClass
    {
    public:
        CreatorClass()
        {
        }
        T* create0()
        {
            return new T;
        }
        T* create1( int)
        {
            return new T(1);
        }
        T* create2( int, int)
        {
            return new T(1,2);
        }
        T* create3( int, int, int)
        {
            return new T(1,2,3);
        }
        T* create4( int, int, int, int)
        {
            return new T(1,2,3,4);
        }
        T* create5( int, int, int, int, int)
        {
            return new T(1,2,3,4,5);
        }
        T* create6( int, int, int, int, int,
                    int)
        {
            return new T(1,2,3,4,5,6);
        }
        T* create7( int, int, int, int, int,
                    int, int)
        {
            return new T(1,2,3,4,5,6,7);
        }
        T* create8( int, int, int, int, int,
                    int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8);
        }
        T* create9( int, int, int, int, int,
                    int, int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9);
        }
        T* create10(int, int, int, int, int,
                    int, int, int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10);
        }
        T* create11(int, int, int, int, int,
                    int, int, int, int, int,
                    int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10,11);
        }
        T* create12(int, int, int, int, int,
                    int, int, int, int, int,
                    int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10,11,12);
        }
        T* create13(int, int, int, int, int,
                    int, int, int, int, int,
                    int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10,11,12,13);
        }
        T* create14(int, int, int, int, int,
                    int, int, int, int, int,
                    int, int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10,11,12,13,14);
        }
        T* create15(int, int, int, int, int,
                    int, int, int, int, int,
                    int, int, int, int, int)
        {
            return new T(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        }
    };

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int >
    >Factory0;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_1( int ) >
    >Factory1;

    typedef SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_2( int, int ) >
    >Factory2;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_3( int, int, int ) >
    >Factory3;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_4( int, int, int, int ) >
    >Factory4;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_5( int, int, int, int, int ) >
    >Factory5;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_6(  int, int, int, int, int,
                                                int ) >
    >Factory6;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_7(  int, int, int, int, int,
                                                int, int ) >
    >Factory7;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_8(  int, int, int, int, int,
                                                int, int, int ) >
    >Factory8;


    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_9(  int, int, int, int, int,
                                                int, int, int, int ) >
    >Factory9;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_10( int, int, int, int, int,
                                                int, int, int, int, int ) >
    >Factory10;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_11( int, int, int, int, int,
                                                int, int, int, int, int,
                                                int ) >
    >Factory11;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_12( int, int, int, int, int,
                                                int, int, int, int, int,
                                                int, int ) >
    >Factory12;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_13( int, int, int, int, int,
                                                int, int, int, int, int,
                                                int, int, int ) >
    >Factory13;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_14( int, int, int, int, int,
                                                int, int, int, int, int,
                                                int, int, int, int ) >
    >Factory14;

    typedef 
    SingletonHolder<
    Factory< AbstractProduct, int, LOKI_TYPELIST_15( int, int, int, int, int,
                                                int, int, int, int, int,
                                                int, int, int, int, int ) >
    >Factory15;


    CreatorClass<Product> creaClass;

    bool registerAll(){

        bool const o0 = Factory0::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create0 );
        bool const o1 = Factory1::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create1 );
        bool const o2 = Factory2::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create2 );
        bool const o3 = Factory3::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create3 );
        bool const o4 = Factory4::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create4 );
        bool const o5 = Factory5::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create5 );
        bool const o6 = Factory6::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create6 );
        bool const o7 = Factory7::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create7 );
        bool const o8 = Factory8::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create8 );
        bool const o9 = Factory9::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create9 );
        bool const o10 = Factory10::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create10 );
        bool const o11 = Factory11::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create11 );
        bool const o12 = Factory12::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create12 );
        bool const o13 = Factory13::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create13 );
        bool const o14 = Factory14::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create14 );
        bool const o15 = Factory15::Instance().Register( 1, &creaClass, &CreatorClass<Product>::create15 );
        
        return o0 && o1 && o2 && o3 && o4 && o5 && o6 && o7 && o8 && o9 && o10 && o11 && o12 && o13 && o14 && o15;
    }

    bool testFactoryParm()
    {
        bool reg = registerAll();

        AbstractProduct* p;
        
        p = Factory0::Instance().CreateObject(1);
        delete p;
        bool test0=p!=NULL;
        
        p = Factory1::Instance().CreateObject(1,64);
        delete p;
        bool test1=p!=NULL;

        p = Factory2::Instance().CreateObject(1,64,64);
        delete p;
        bool test2=p!=NULL;

        p = Factory3::Instance().CreateObject(1,64,64,64);
        delete p;
        bool test3=p!=NULL;

        p = Factory4::Instance().CreateObject(1,64,64,64,64);
        delete p;
        bool test4=p!=NULL;

        p = Factory5::Instance().CreateObject(1,64,64,64,64,64);
        delete p;
        bool test5=p!=NULL;

        p = Factory6::Instance().CreateObject(1,    64,64,64,64,64,
                                                    64);
        delete p;
        bool test6=p!=NULL;

        p = Factory7::Instance().CreateObject(1,    64,64,64,64,64,
                                                    64,64);
        delete p;
        bool test7=p!=NULL;    

        p = Factory8::Instance().CreateObject(1,    64,64,64,64,64,
                                                    64,64,64);    
        delete p;
        bool test8=p!=NULL;
        
        p = Factory9::Instance().CreateObject(1,    64,64,64,64,64,
                                                    64,64,64,64);
        delete p;
        bool test9=p!=NULL;

        p = Factory10::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64);
        delete p;
        bool test10=p!=NULL;

        p = Factory11::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64,
                                                    64);
        delete p;
        bool test11=p!=NULL;

        p = Factory12::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64,
                                                    64,64);
        delete p;
        bool test12=p!=NULL;

        p = Factory13::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64,
                                                    64,64,64);
        delete p;
        bool test13=p!=NULL;

        p = Factory14::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64,
                                                    64,64,64,64);
        delete p;
        bool test14=p!=NULL;

        p = Factory15::Instance().CreateObject(1,   64,64,64,64,64,
                                                    64,64,64,64,64,
                                                    64,64,64,64,64);
        delete p;
        bool test15=p!=NULL;

        return  reg     && test0  &&
                test1   && test2  && test3  && test4  && test5 &&
                test6   && test7  && test8  && test9  && test10 &&
                test11  && test12 && test13 && test14 && test15;
    }


}

class FactoryParmTest : public Test
{
public:
    FactoryParmTest() : Test("FactoryParm.h") {}

    virtual void execute(TestResult &result)
    {
        printName(result);

        bool test1=FactoryTestParmPrivate::testFactoryParm();

        bool r=test1;

        testAssert("FactoryParm",r,result);

        std::cout << '\n';
    }
    
} factoryParmTest;


#endif

#endif

