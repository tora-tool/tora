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

// $Id: type2.h 761 2006-10-17 20:48:18Z syntheticpp $


#include <loki/Pimpl.h>

/////////////////////////////////////////
// class A2 declaration
/////////////////////////////////////////

template< class T>
class Impl;

class A2
{
public:
    A2();
    ~A2();
    void foo();

private:
    PimplOf<A2>::Type d;
};


/////////////////////////////////////////
// class B2 declaration
/////////////////////////////////////////

class B2 : private PimplOf<B2>::Owner
{
public:
    B2();
    ~B2();
    void foo();
};



/////////////////////////////////////////
// class C2 declaration
/////////////////////////////////////////

class C2
{
public:
    C2();
    ~C2();
    void foo();

private:
    PimplOf<C2>::Type rint; 
    RimplOf<C2>::Type d; 
};


/////////////////////////////////////////
// class D2 declaration
/////////////////////////////////////////

class D2 : private RimplOf<D2>::Owner
{
public:
    D2();
    ~D2();
    void foo();
};


/////////////////////////////////////////
// incomplete type test
/////////////////////////////////////////
class Incomplete1
{
public:
    Incomplete1();
    void foo();
private:
    PimplOf<Incomplete1>::Type d;
};

class Incomplete2
{
public:
    Incomplete2();
    ~Incomplete2();
    void foo();
private:
    PimplOf<Incomplete2>::Type d;
};


// Test: don't compile with inline destructor 
#if 0

class Incomplete3
{
public:
    Incomplete3();
    ~Incomplete3()
    {
        // inline destructor
    }
    void foo();
private:
    PimplOf<Incomplete3>::Type d;
};
#endif


// Test: don't compile with incomplete type and auto_ptr

#include <memory>

class Impl4;
class Incomplete4
{
public:
    Incomplete4();
    void foo();
private:
    Pimpl<Impl4, std::auto_ptr<Impl4> > d;
};

