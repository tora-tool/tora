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


#ifdef _MSC_VER
#pragma warning (disable: 4512)
#endif


#define LOKI_INHERITED_PIMPL_NAME d
#define LOKI_INHERITED_RIMPL_NAME d

#include "type.h"
#include "type2.h"

#include <loki/SafeFormat.h>



/////////////////////////////////////////
// Definition of ImplOf<A>
/////////////////////////////////////////
namespace Loki // gcc!!
{
    template<>
    struct ImplOf<A> : public SmallObject<> // inherit SmallObj for speed up
    {
        ImplOf() : data(0) {Printf("A created\n");}
        ~ImplOf(){Printf("A destroyed, data=%d\n")(data);}
        int data;
    };
}
/////////////////////////////////////////
// class A definition
/////////////////////////////////////////
A::A()
{}

void A::foo()
{
    (*d).data = 1;
    d->data = 111;
}


/////////////////////////////////////////
// Definition  of ImplOf<B>
/////////////////////////////////////////
namespace Loki // gcc!!
{
    template<>
    struct ImplOf<B> : public SmallObject<> // inherit SmallObj for speed up
    {
        ImplOf() : data(0) {Printf("B created\n");}
        ~ImplOf(){Printf("B destroyed, data=%d\n")(data);}
        int data;
    };
}
/////////////////////////////////////////
// class B definition
/////////////////////////////////////////
B::B() : Loki::PimplOf<B>::Owner()
{}

void B::foo()
{
    (*d).data = 2;
    d->data = 222;
}


/////////////////////////////////////////
// Definition  of ImplOf<C>
/////////////////////////////////////////
namespace Loki // gcc!!
{
    template<>
    struct ImplOf<C> : public SmallObject<> // inherit SmallObj for speed up
    {
        ImplOf(): data(0) {Printf("C created\n");}
        ~ImplOf(){Printf("C destroyed, data=%d\n")(data);}
        int data;
    };
}
/////////////////////////////////////////
// class C definition
/////////////////////////////////////////
C::C() : p(), d(*p)
{}

void C::foo()
{
    d.data = 333;
}



/////////////////////////////////////////
// Definition  of ImplOf<D>
/////////////////////////////////////////
namespace Loki // gcc!!
{
    template<>
    struct ImplOf<D> : public SmallObject<> // inherit SmallObj for speed up
    {
        ImplOf(): data(0) {Printf("D created\n");}
        ~ImplOf(){Printf("D destroyed, data=%d\n")(data);}
        int data;
    };
}

/////////////////////////////////////////
// class D definition
/////////////////////////////////////////
D::D() : Loki::RimplOf<D>::Owner()
{}

void D::foo()
{
    d.data = 444;
}


/////////////////////////////////////////
// main
/////////////////////////////////////////

void test_more();

int main()
{
    A* a = new A;
    B* b = new B;
    C* c = new C;
    D* d = new D;
    a->foo();
    b->foo();
    c->foo();
    d->foo();
    delete a;
    delete b;
    delete c;
    delete d;

    test_more();

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    return 0;
}


////////////////////
// more test code
////////////////////

// incomplete type test
// this must give errors

//Incomplete1 ii; // compiler error
//Incomplete2 i2; // linker error
//Incomplete4 i4; // compiler error

/////////////////////////////////////////
// Definition  of ImplOf<E>
/////////////////////////////////////////
namespace Loki // gcc!!
{
    template<>
    struct ImplOf<E> : public SmallObject<> // inherit SmallObj for speed up
    {
        ImplOf() : data(0) {Printf("E created\n");}
        ~ImplOf(){Printf("E destroyed, data=%d\n")(data);}
        int data;

        void foo()       {Printf("E  foo() \n");}
        void foo() const {Printf("E  foo() const \n");}
    };
}



P1::P1(){d->data = 1;}
P2::P2(){d->data = 2;}
P3::P3(){d->data = 3;}
P4::P4(){d->data = 4;}
P5::P5(){d->data = 5;}

PO1::PO1(){d->data = 6;}
PO2::PO2(){d->data = 7;}
PO3::PO3(){d->data = 8;}
PO4::PO4(){d->data = 9;}
PO5::PO5(){d->data = 10;}

void P1::f(){d->foo();}
void P2::f(){d->foo();}
void P3::f(){d->foo();}
void P4::f(){d->foo();}
void P5::f(){d->foo();}

void PO1::f(){d->foo();}
void PO2::f(){d->foo();}
void PO3::f(){d->foo();}
void PO4::f(){d->foo();}
void PO5::f(){d->foo();}

void P1::f()const{d->foo();}
void P2::f()const{d->foo();}
void P3::f()const{d->foo();}
void P4::f()const{d->foo();}
void P5::f()const{d->foo();}

void PO1::f()const{d->foo();}
void PO2::f()const{d->foo();}
void PO3::f()const{d->foo();}
void PO4::f()const{d->foo();}
void PO5::f()const{d->foo();}




R1::R1():d(*p){d.data = 11;}
R2::R2():d(*p){d.data = 22;}
R3::R3():d(*p){d.data = 33;}
R4::R4():d(*p){d.data = 44;}
R5::R5():d(*p){d.data = 55;}

void R1::f(){d.foo();}
void R2::f(){d.foo();}
void R3::f(){d.foo();}
void R4::f(){d.foo();}
void R5::f(){d.foo();}

void R1::f()const{d.foo();}
void R2::f()const{d.foo();}
void R3::f()const{d.foo();}
void R4::f()const{d.foo();}
void R5::f()const{d.foo();}



RO1::RO1(){d.data = 66;}
RO2::RO2(){d.data = 77;}
RO3::RO3(){d.data = 88;}
RO4::RO4(){d.data = 99;}
RO5::RO5(){d.data = 1010;}

void RO1::f(){d.foo();}
void RO2::f(){d.foo();}
void RO3::f(){d.foo();}
void RO4::f(){d.foo();}
void RO5::f(){d.foo();}

void RO1::f()const{d.foo();}
void RO2::f()const{d.foo();}
void RO3::f()const{d.foo();}
void RO4::f()const{d.foo();}
void RO5::f()const{d.foo();}


void test_more()
{
    Loki::Printf("\n\nMore tests:\n");

    Loki::Printf("\nCreating Pimpls\n");
    P1* p1 =  new P1;
    P2* p2 =  new P2;
    P3* p3 =  new P3;
    P4* p4 =  new P4;
    P5* p5 =  new P5;
    PO1* p6 =  new PO1;
    PO2* p7 =  new PO2;
    PO3* p8 =  new PO3;
    PO4* p9 =  new PO4;
    PO5* p10 = new PO5;

    Loki::Printf("\nConst check\n");
    p1->f();
    p2->f();
    p3->f();
    p4->f();
    p5->f();
    p6->f();
    p7->f();
    p8->f();
    p9->f();
    p10->f();

    Loki::Printf("\nDeleting Pimpls\n");
    delete p1;
    delete p2;
    delete p3;
    delete p4;
    delete p5;
    delete p6;
    delete p7;
    delete p8;
    delete p9;
    delete p10;
    
    
    Loki::Printf("\nCreating Rimpls\n");
    R1* r1 =  new R1;
    R2* r2 =  new R2;
    R3* r3 =  new R3;
    R4* r4 =  new R4;
    R5* r5 =  new R5;

    RO1* r6 =  new RO1;
    RO2* r7 =  new RO2;
    RO3* r8 =  new RO3;
    RO4* r9 =  new RO4;
    RO5* r10 =  new RO5;

    r1->f();
    r2->f();
    r3->f();
    r4->f();
    r5->f();
    r6->f();
    r7->f();
    r8->f();
    r9->f();
    r10->f();

    Loki::Printf("\nDeleting Rimpls\n");
    delete r1;
    delete r2;
    delete r3;
    delete r4;
    delete r5;
    delete r6;
    delete r7;
    delete r8;
    delete r9;
    delete r10;


    Loki::Printf("\nCreating const Pimpls\n");
    const P1* cp1 =  new P1;
    const P2* cp2 =  new P2;
    const P3* cp3 =  new P3;
    const P4* cp4 =  new P4;
    const P5* cp5 =  new P5;

    const PO1* cp6 =  new PO1;
    const PO2* cp7 =  new PO2;
    const PO3* cp8 =  new PO3;
    const PO4* cp9 =  new PO4;
    const PO5* cp10 =  new PO5;

    Loki::Printf("\nConst check\n");
    cp1->f();
    cp2->f();
    cp3->f();
    cp4->f();
    cp5->f();
    cp6->f();
    cp7->f();
    cp8->f();
    cp9->f();
    cp10->f();

    Loki::Printf("\nDeleting const Pimpls\n");
    delete cp1;
    delete cp2;
    delete cp3;
    delete cp4;
    delete cp5;
    delete cp6;
    delete cp7;
    delete cp8;
    delete cp9;
    delete cp10;
}
