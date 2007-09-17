////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 by Curtis Krauskopf
// Copyright (c) 2005 by Peter Kuemmel
//
// Code covered by the MIT License
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main.cpp 760 2006-10-17 20:36:13Z syntheticpp $


// This is an example of using the SetLongevity function for both
// singletons and globally and locally defined dynamically allocated
// objects.
//
// The program defines three classes:  Example, Keyboard and LogClass.
//
// The purpose of the Example class is to send a message to cout
// when an Example object is being destroyed.
//
// The Keyboard class is a singleton.
//
// The LogClass class is also a singleton.
//
// The pGlobal object is deleted using an adapter functor to
// customize Example's destruction (see destGlobal()).
// The glue that binds the adapter functor (above) with Loki's
// SetLongevity function is:
//
// Loki::Private::Adapter<Example>exampleAdapter = {&destGlobal};
// SetLongevity(pGlobal, globalPriority, exampleAdapter);
//
// An alternative Loki-compatible way of destroying pGlobal (without
// defining a global static functor) is to use the default parameter
// on SetLongevity:
//
// Example *pLocal = new Example("Destroying local Example");
// SetLongevity(pLocal, localPriority);
//
// The parameters passed by the user on main define the longevity values
// for (respectively):
//  1)  The global object
//  2)  The local object
//  3)  The Keyboard singleton
//  4)  The LogClass singleton
//
// Examples:
//    longevity 1 2 3 4
//    longevity 40 30 20 10
//

// $Header$

#include <iostream>
#include <loki/Singleton.h>   // for Loki::SingletonHolder

using namespace std;   // okay for small programs
using namespace Loki;  // okay for small programs

// These globals allow the priority for each object to be
// set in main() but used anywhere in the program.
int  globalPriority;
int  localPriority;
int  keyboardPriority;
int  logPriority;



// A generic example class that stores and echoes a const char.
//
class Example
{
public:
    Example(const char * s)
    {
        msg = s;
    };
    virtual ~Example()
    {
        echo(msg);
    }
    void echo(const char *s)
    {
        cout << s << endl;
    }
protected:
    const char *msg;
};


// A singleton Keyboard object derived from the Example class.
// Its longevity is set by the user on the command line.
//
class Keyboard : public Example
{
public:
    Keyboard() : Example("Destroying Keyboard")
    {  }
}
;

inline unsigned int GetLongevity(Keyboard *)
{
    return keyboardPriority;
}

typedef SingletonHolder<Keyboard, CreateUsingNew, SingletonWithLongevity> keyboard;


// A singleton LogClass object derived from the Example class.
// Its longevity is set by the user on the command line.
//
class LogClass : public Example
{
public:
    LogClass() : Example("Destroying LogClass")
    {  }
}
;

inline unsigned int GetLongevity(LogClass *)
{
    return logPriority;
}

typedef SingletonHolder<LogClass, CreateUsingNew, SingletonWithLongevity> LogBook;


// Instantiate a global Example object.  It's not a singleton
// but because it's instantiated with new (and therefore it isn't
// automatically destroyed) it can use the SetLongevity template function.
// Its longevity is determined by the user on the command line.
//
Example* pGlobal( new Example("Destroying global Example") );

// destGlobal() is called when the pGlobal object needs to be destroyed.
static void destGlobal()
{
    cout << "Going to delete pGlobal\n";
    delete pGlobal;
}

void help(const char *s)
{
    cout << "To use:\n";
    cout << s << " par1 par2 par3 par4\n";
    cout << "  where each par is a number that represents the object's ";
    cout << " longevity:\n";
    cout << "    par1: global object\n";
    cout << "    par2: local object\n";
    cout << "    par3: keyboard singleton\n";
    cout << "    par4: LogBook singleton\n";
    cout << "Example:  " << s << " 1 2 3 4" << endl;
}


int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        help(argv[0]);
        return 0;
    }

    globalPriority = atoi(argv[1]);
    localPriority = atoi(argv[2]);
    keyboardPriority = atoi(argv[3]);
    logPriority = atoi(argv[4]);

    // Use an adapter functor to tie the destGlobal function to the
    // destruction priority for pGlobal.
    Loki::Private::Adapter<Example> exampleAdapter = { &destGlobal };
    SetLongevity(pGlobal, globalPriority, exampleAdapter);

    // Use Loki's private Deleter template function to destroy the
    // pLocal object for a user-defined priority.
    Example *pLocal = new Example("Destroying local Example");
    SetLongevity<Example, void (*)(Example*)>(pLocal, localPriority, &Loki::Private::Deleter<Example>::Delete);

    // Make the global and local objects announce their presense.
    pGlobal->echo("pGlobal created during program initialization.");
    pLocal->echo("pLocal created after main() started.");

    // Instantiate both singletons by calling them...
    LogBook::Instance().echo("LogClass singleton instantiated");
    keyboard::Instance().echo("Keyboard singleton instantiated");

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif


    return 0;
}

