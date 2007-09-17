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


#include <iostream>

#include <loki/Factory.h>
#include <loki/Singleton.h>

#include "classlist.h"

typedef Loki::SingletonHolder
<
    Loki::Factory<Base, std::string>
>
BaseFactory;


bool registerClass(std::string key, Base*(*creator)() )
{
    return BaseFactory::Instance().Register(key,creator);
}

Loki::RegisterOnCreateSet<ClassList> registerAllClasses;
Loki::UnRegisterOnDeleteSet<ClassList> unregisterAllClasses;


int main()
{
    Base* foo = BaseFactory::Instance().CreateObject("Foo");
    Base* boo = BaseFactory::Instance().CreateObject("Boo");
    
    foo->foo();
    boo->foo();

    delete foo;
    delete boo;

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    return 0;
}
