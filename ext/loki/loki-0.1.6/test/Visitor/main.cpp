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

// $Id: main.cpp 759 2006-10-17 20:27:29Z syntheticpp $


#include <loki/Visitor.h>
#include <iostream>


class Base : public Loki::BaseVisitable<>
{
public:
    LOKI_DEFINE_VISITABLE()
};

class Type1 : public Base
{
public:
    LOKI_DEFINE_VISITABLE()
};

class VariableVisitor : 
    public Loki::BaseVisitor, 
    //public Loki::Visitor<Base>,
    //public Loki::Visitor<Type1>
#ifndef LOKI_DISABLE_TYPELIST_MACROS
    public Loki::Visitor<LOKI_TYPELIST_2(Base,Type1)>
#else
    public Loki::Visitor<Loki::Seq<Base,Type1>::Type>
#endif
{ 
public: 
    void Visit(Base&){std::cout << "void Visit(Base&)\n";}
    void Visit(Type1&){std::cout << "void Visit(Type1&)\n";}
}; 


class CBase : public Loki::BaseVisitable<void, Loki::DefaultCatchAll, true>
{
public:
    LOKI_DEFINE_CONST_VISITABLE()
};

class CType1 : public CBase
{
public:
    LOKI_DEFINE_CONST_VISITABLE()
};

class CVariableVisitor : 
    public Loki::BaseVisitor, 
    //public Loki::Visitor<CBase,void,true>,
    //public Loki::Visitor<CType1,void,true>
#ifndef LOKI_DISABLE_TYPELIST_MACROS
    public Loki::Visitor<LOKI_TYPELIST_2(CBase,CType1),void,true>
#else
    public Loki::Visitor<Loki::Seq<CBase,CType1>::Type,void,true>
#endif
{ 
public: 
    void Visit(const CBase&){std::cout << "void Visit(CBase&)\n";}
    void Visit(const CType1&){std::cout << "void Visit(CType1&)\n";}
}; 

int main()
{
    VariableVisitor visitor;
    Type1 type1;
    Base* dyn = &type1;
    dyn->Accept(visitor);

    CVariableVisitor cvisitor;
    CType1 ctype1;
    CBase* cdyn = &ctype1;
    cdyn->Accept(cvisitor);

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

}


