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
#ifndef LOKI_FOO_H
#define LOKI_FOO_H

// $Id: foo.h 760 2006-10-17 20:36:13Z syntheticpp $


#include "classlist.h"


struct Foo : Base
{
    Foo();
    virtual ~Foo();
    void foo();
};

struct Boo : Base
{
    Boo();
    virtual ~Boo();
    void foo();
};



#endif
