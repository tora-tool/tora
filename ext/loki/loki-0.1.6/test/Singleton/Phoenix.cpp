////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 by Curtis Krauskopf
// Copyright (c) 2005 by Peter Kuemmel
//
// Code covered by the MIT License
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: Phoenix.cpp 760 2006-10-17 20:36:13Z syntheticpp $


// A singleton LogClass object that resurrects itself after
// it has been automatically destroyed during program
// termination.  When a dead-reference to the LogClass
// singleton is detected, it is resurrected automatically.
//
// Expected output:
//   Example c'tor
//   LogClass c'tor
//   LogClass now instantiated.
//   LogClass d'tor
//   Example d'tor starting
//   LogClass c'tor
//   LogClass: inside Example d'tor
//   Example d'tor finished
//
// The last line of the output only appears when this
// program is compiled with the ATEXIT_FIXED symbol
// defined (see the Loki library and the CUJ article).
//

#include <iostream>
#include <loki/Singleton.h>   // for Loki::SingletonHolder

using namespace std;   // okay for small programs
using namespace Loki;  // okay for small programs

class LogClass
{
public:
    LogClass()
    {
        echo("LogClass c'tor");
    }
    ~LogClass()
    {
        echo("LogClass d'tor");
    }
    void echo(const char *s)
    {
        cout << s << endl;
    }
};

typedef SingletonHolder<LogClass, CreateUsingNew, PhoenixSingleton> LogBook;


// A generic example class that stores and echoes a const char.
//
class Example
{
public:
    Example()
    {
        echo("Example c'tor");
    }
    ~Example()
    {
        echo("Example d'tor starting");
        LogBook::Instance().echo("LogClass: inside Example d'tor");
        echo("Example d'tor finished");
    }
    void echo(const char *s)
    {
        cout << s << endl;
    }
};


int main(int argc, char* argv[])
{
    Example *example = new Example();
     SetLongevity<Example, void (*)(Example*)>(example, 1, &Loki::Private::Deleter<Example>::Delete);
    LogBook::Instance().echo("LogClass now instantiated.");
    
#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    return 0;
}

