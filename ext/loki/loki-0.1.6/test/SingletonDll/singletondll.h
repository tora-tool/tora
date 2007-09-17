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
#ifndef LOKI_SINGLETONDLL_H
#define LOKI_SINGLETONDLL_H

// $Id: singletondll.h 760 2006-10-17 20:36:13Z syntheticpp $


#include "singletondll_export.h"

class Foo;

// Use the predefined Loki::Singleton
// of loki/Singleton.h and use the export 
// specifier of the current library
#define LOKI_SINGLETON_EXPORT SINGLETONDLL_EXPORT
#include <loki/Singleton.h>


// declare the Singleton template by yourself
// and export Singleton<Foo>, so the singleton 
// is not in the Loki namespace
template<class T>
class SINGLETONDLL_EXPORT Singleton
{
public:
    static T& Instance();
};



#endif
