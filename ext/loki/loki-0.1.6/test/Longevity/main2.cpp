////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 by Peter Kuemmel
//
// Code covered by the MIT License
// The author make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main2.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#include <iostream>

#include <loki/Singleton.h>


struct L1
{
    L1(){std::cout << "create L1: " << this << "\n";}
    ~L1(){std::cout << "delete L1: " << this <<" \n";}
};

struct L2
{
    L2(){std::cout << "create L2 \n";}
    ~L2(){std::cout << "delete L2 \n";}
};

struct L3
{
    L3(){std::cout << "create L3 \n";}
    ~L3(){std::cout << "delete L3 \n";}
};

int main()
{
    Loki::SetLongevity
                (new L1, 1);
    Loki::SetLongevity<L1,    void (*)(L1*)>
                (new L1, 1, Loki::Private::Deleter<L1>::Delete);    
    Loki::SetLongevity<L1,    Loki::Private::Deleter<L1>::Type>
                (new L1, 1, Loki::Private::Deleter<L1>::Delete);
    Loki::SetLongevity(new L2, 2);
    Loki::SetLongevity(new L1, 1);    
    Loki::SetLongevity(new L3, 3);    
    Loki::SetLongevity(new L1, 1);

    std::cout << "\n";
}

