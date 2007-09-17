////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 by Peter Kuemmel
//
// Code covered by the MIT License
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: Factory.cpp 761 2006-10-17 20:48:18Z syntheticpp $


//#define CLASS_LEVEL_THERADING

#define USE_SEQUENCE

#include <iostream>
#include <string>
#include <loki/Factory.h>
#include <loki/Functor.h>

#ifdef LOKI_DISABLE_TYPELIST_MACROS
#define USE_WQUENCE
#endif

#ifdef USE_SEQUENCE
#include <loki/Sequence.h>

#endif

using namespace Loki;
using std::cout;
using std::endl;

 
////////////////////////////////////////////
// Object to create: Product
// Constructor with 0 and 2 arguments
////////////////////////////////////////////
 
class AbstractProduct{};
 
class Product : public AbstractProduct
{
public:
    Product(){}
    Product( int, int ){}
};
 
///////////////////////////////////////////////////////////
// Factory for creation a Product object without parameters
///////////////////////////////////////////////////////////
 
typedef SingletonHolder 
<
    Factory< AbstractProduct, int >,
    CreateUsingNew,
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
PFactoryNull;
 
/////////////////////////////////////////////////////////////
// Factory for creation a Product object with 2 parameters
/////////////////////////////////////////////////////////////
 
typedef SingletonHolder
<
#ifndef USE_SEQUENCE
Factory< AbstractProduct, std::string, LOKI_TYPELIST_2( int, int ) >,
#else
Factory< AbstractProduct, std::string, Seq< int, int > >,
#endif
    CreateUsingNew,
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
PFactory;
 
////////////////////////////////////////////////////
// Creator functions with different names
////////////////////////////////////////////////////
 
Product* createProductNull()             
{ 
    cout << "createProductNull()" << endl;
    return new Product;     
}
Product* createProductParm( int a, int b ) 
{ 
    cout << "createProductParm( int a, int b ) " << endl;
    return new Product( a, b ); 
}

///////////////////////////////////////////////////
// Overloaded creator functions 
///////////////////////////////////////////////////
Product* createProductOver()             
{ 
    cout << "createProductOver()" << endl;
    return new Product;     
}
Product* createProductOver( int a, int b ) 
{ 
    cout << "createProductOver( int a, int b )" << endl;
    return new Product( a, b ); 
}

 
 
///////////////////////////////////////////////////
// Creator functions are polymorphic member functions
///////////////////////////////////////////////////
 
class AbstractCreator{
public:
    virtual ~AbstractCreator() {}
    virtual AbstractProduct* create() = 0;
    virtual AbstractProduct* createParm( int, int ) = 0;
};

class Creator : public AbstractCreator{
public:
    Creator(){};
    AbstractProduct* create()                 
    {
        cout << "Creator::create()" << endl;
        return new Product;     
    }
    AbstractProduct* createParm( int a, int b ) 
    {
        cout << "Creator::create( int a, int b )" << endl;
        return new Product( a,b );
    }
};
 
///////////////////////////////////////////////////////////////
// Creator functions are member functions of a template class
///////////////////////////////////////////////////////////////
 
template< class T>
class CreatorT{
public:
    CreatorT(){};
    T* create()                 
    {
        cout << "CreatorT<T>::create()" << endl;
        return new T;     
    }
    T* createParm( int a, int b ) 
    {
        cout << "CreatorT<T>::create( int a, int b )" << endl;
        return new T( a,b );
    }
};
 
 ///////////////////////////////////////////////////////////////
// get creator functions on runntime
///////////////////////////////////////////////////////////////

#ifndef USE_SEQUENCE
typedef Functor<Product*,LOKI_TYPELIST_2(int,int)> CreateFunctor;
#else
typedef Functor<Product*,Seq<int,int> > CreateFunctor;
#endif

typedef 
SingletonHolder
<
#ifndef USE_SEQUENCE
    Factory< AbstractProduct, int,LOKI_TYPELIST_3(CreateFunctor,int,int) >,
#else
    Factory< AbstractProduct, int,Seq<CreateFunctor,int,int> >,
#endif
    CreateUsingNew,
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
PFactoryFunctorParm;

Product* createProductRuntime(CreateFunctor func, int a, int b) 
{ 
    Product *p = func( a, b );
    cout << "    called by createProductRuntime(CreateFunctor func, int a, int b) " << endl;
    return p; 
}
 

///////////////////////////////////////
// Register creator functions 
// No additional typdefs are necessary!
//////////////////////////////////////
 
Creator creator;
AbstractCreator* c = &creator;
CreatorT<Product> cT;
 
bool reg()
{
    bool const ok1 = PFactoryNull::Instance().Register( 1, createProductNull );
    bool const ok2 = PFactoryNull::Instance().Register( 2, static_cast<Product*(*)()>(createProductOver) );
    bool const ok3 = PFactoryNull::Instance().Register( 3, c, &AbstractCreator::create );
    bool const ok4 = PFactoryNull::Instance().Register( 4, &cT, &CreatorT<Product>::create );
 
    bool const ok5 = PFactory::Instance().Register( "One", createProductParm );
    bool const ok6 = PFactory::Instance().Register( "Two", static_cast<Product*(*)(int,int)>(createProductOver) );
    bool const ok7 = PFactory::Instance().Register( "Three", c, &AbstractCreator::createParm );
    bool const ok8 = PFactory::Instance().Register( "Four", &cT, &CreatorT<Product>::createParm );

    bool const ok9 = PFactoryFunctorParm::Instance().Register( 1, createProductRuntime );
    
    return ok1 && ok2 && ok3 && ok4 && ok5 && ok6 && ok7 && ok8 && ok9;
}


////////////////////////////////////////////////////////////////////////////////////
//
//    detect memory leaks on MSVC Ide
//
////////////////////////////////////////////////////////////////////////////////////

//#define MSVC_DETECT_MEMORY_LEAKS
#ifdef MSVC_DETECT_MEMORY_LEAKS

#include <crtdbg.h>
#include <cassert>

void heap_debug()
{
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

    //tmpFlag |= _CRTDBG_CHECK_MasterLWMasterYS_DF;

    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );
}
#else
void heap_debug()
{}
#endif

int main()
{
    heap_debug();

    reg();
    
    AbstractProduct* p;
 
    cout << endl << "creator function is a simple function:" << endl;
    p= PFactoryNull::Instance().CreateObject( 1 ); 
    delete p;
    p= PFactory::Instance().CreateObject( "One", 64,64 );
    delete p;
    
    cout << endl << "creator function is a overloaded function:" << endl;
    p= PFactoryNull::Instance().CreateObject( 2 );
    delete p;
    p= PFactory::Instance().CreateObject( "Two", 64,64 );
    delete p;
    
    cout << endl << "creator function is a member function:" << endl;
    p= PFactoryNull::Instance().CreateObject( 3 );
    delete p;
    p= PFactory::Instance().CreateObject( "Three", 64,64 );
    delete p;
    
    cout << endl << "creator function is a template member function" << endl;
    p= PFactoryNull::Instance().CreateObject( 4 );
    delete p;
    p= PFactory::Instance().CreateObject( "Four", 64,64 );
    delete p;

    CreateFunctor func1(createProductParm);
    CreateFunctor func2(&cT, &CreatorT<Product>::createParm);
    cout << endl << "creator function is a functor parameter" << endl;
    p= PFactoryFunctorParm::Instance().CreateObject( 1, func1, 64,64 );
    delete p;
    p= PFactoryFunctorParm::Instance().CreateObject( 1, func2, 64,64 );
    delete p;

    
    cout << endl;
    cout << "Registered ids: \n";

    std::vector<std::string> ids = PFactory::Instance().RegisteredIds();

    for(std::vector<std::string>::iterator it=ids.begin(); it!=ids.end(); ++it)
        cout << *it << "\n";


    cout << endl;
    cout << endl;
    
#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    return EXIT_SUCCESS;
}

