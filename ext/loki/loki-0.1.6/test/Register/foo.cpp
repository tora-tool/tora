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

// $Id: foo.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#include "foo.h"
#include <iostream>

Base::~Base(){}

Foo::Foo(){}
Boo::Boo(){}


Foo::~Foo(){}
Boo::~Boo(){}


void Foo::foo(){std::cout << "Foo::foo() called,  this: " << this << "\n"; }
void Boo::foo(){std::cout << "Boo::foo() called,  this: " << this << "\n"; }




// Register code

#include "classlist.h"

LOKI_CHECK_CLASS_IN_LIST( Foo, ClassList )
LOKI_CHECK_CLASS_IN_LIST( Boo, ClassList )

Base* createFoo(){ return new Foo; }
Base* createBoo(){ return new Boo; }

namespace Loki
{
    template<> bool RegisterFunction<Foo>()
    {
        std::cout << "RegisterFunction<Foo>\n";
        return registerClass("Foo", &createFoo);
    }
    template<> bool RegisterFunction<Boo>()
    {
        std::cout << "RegisterFunction<Boo>\n";
        return registerClass("Boo", &createBoo);
    }

    template<> bool UnRegisterFunction<Foo>()
    {
        std::cout << "UnRegisterFunction<Foo>\n";
        return true;
    }
    template<> bool UnRegisterFunction<Boo>()
    {
        std::cout << "UnRegisterFunction<Boo>\n";
        return true;
    }
}
