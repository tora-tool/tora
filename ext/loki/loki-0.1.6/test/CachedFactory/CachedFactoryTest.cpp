////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 by Guillaume Chatelet
//
// Code covered by the MIT License
//
// Permission to use, copy, modify, distribute and sell this software for any 
// purpose is hereby granted without fee, provided that the above copyright 
// notice appear in all copies and that both that copyright notice and this 
// permission notice appear in supporting documentation.
//
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
//
// This code DOES NOT accompany the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
//
////////////////////////////////////////////////////////////////////////////////

// $Id: CachedFactoryTest.cpp 809 2007-02-25 14:34:30Z syntheticpp $


#define USE_SEQUENCE

#include <cassert>
#include <iostream>
#include <loki/Factory.h>

#ifdef LOKI_DISABLE_TYPELIST_MACROS
	#define USE_WQUENCE
#endif

#ifdef USE_SEQUENCE
	#include <loki/Sequence.h>
#endif

#include <loki/CachedFactory.h>

typedef long milliSec;

#if defined(_WIN32) && !defined(__CYGWIN__)
	#include <windows.h>
    milliSec getmilliSeconds()
    {
    	return clock()*1000/CLOCKS_PER_SEC;
    }
#else
	#include <unistd.h>
	void Sleep(unsigned int t) { usleep( 1000 * static_cast<unsigned long>(t));}
    /**
     * Returns the number of milliseconds elapsed from the epoch.
     * This counter might overrun and come back to 0.
     * This might give a false result in the following tests.  
     */
    #include <sys/time.h>
    milliSec getmilliSeconds()
    {
    	struct timeval time;
    	time.tv_usec = 0;
    	time.tv_sec = 0;
    	gettimeofday(&time, NULL);
    	return milliSec(time.tv_usec/1000+time.tv_sec*1000);
    }
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using namespace Loki;

class AbstractProduct{
public:
    virtual ~AbstractProduct(){};
};
 
class Product : public AbstractProduct
{
public:
    Product(){}
    Product( int, int ){}
};

class CostlyProduct : public AbstractProduct
{
public:
    CostlyProduct(){Sleep(100);}
    CostlyProduct( int, int ){Sleep(100);}
};

class DebugProduct : public AbstractProduct
{
public:
    DebugProduct()
    { cout << "Product Ctor()" << endl; }
    DebugProduct( int, int )
    { cout << "Product Ctor(int, int)" << endl; }
    ~DebugProduct()
    { cout << "Product Dtor()" << endl; }
};

CostlyProduct* createCostlyProductNull()
{ 
    return new CostlyProduct;
}

DebugProduct* createDebugProductNull()
{ 
    return new DebugProduct;
}

Product* createProductNull()             
{ 
    return new Product;
}

Product* createProductInt(int a, int b)             
{ 
    return new Product(a,b);
}

const int nullID(0);
const int intID(1);

bool dispResult(const char* message, bool result)
{
    cout << "## " << message << (result?" ...OK":" ...Failed !") << endl;
    return result;
}

template<class CCache>
bool unitTestCacheOverhead(int loop){
	milliSec start(0), elapsedNoCache(0), elapsedCache(0);
    start = getmilliSeconds();
    for(int i=0;i<loop;i++)
        delete createProductNull();
    elapsedNoCache = getmilliSeconds() - start;
    cout << " " << elapsedNoCache << " ms" ;
    CCache CC;
    CC.Register(nullID, createProductNull);
    start = getmilliSeconds();
    for(int i=0;i<loop;i++){
        AbstractProduct *pProduct(CC.CreateObject(nullID));
        CC.ReleaseObject(pProduct);
    }
    elapsedCache = getmilliSeconds() - start;
    cout << " " << elapsedCache << " ms";
    cout << " | average overhead per fetch : " <<((static_cast<double>(elapsedCache-elapsedNoCache)) / CLOCKS_PER_SEC * 1000 / loop)  << " ms" << endl;
    return true;
}

void testCacheOverhead(){
    const int loop(1000000);
	cout << "Starting cache overhead test with " << loop << " loops" << endl;
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictRandom > CRandomEvict;
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictLRU > CLRUEvict;
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictAging > CAgingEvict;
    bool test1 = dispResult("Random policy", unitTestCacheOverhead< CRandomEvict >(loop));(void)test1;
    bool test2 = dispResult("LRU policy", unitTestCacheOverhead< CLRUEvict >(loop));(void)test2;
    bool test3 = dispResult("Aging policy", unitTestCacheOverhead< CAgingEvict >(loop));(void)test3;
}
void unitTestCachePerformance(int loop){
    typedef CachedFactory< AbstractProduct, int > CCache;
	milliSec start(0), elapsedNoCache(0), elapsedCache(0);
    start = getmilliSeconds();
    for(int i=0;i<loop;i++)
        delete createCostlyProductNull();
    elapsedNoCache = getmilliSeconds() - start;
    CCache CC;
    CC.Register(nullID, createCostlyProductNull);
    start = getmilliSeconds();
    for(int i=0;i<loop;i++){
        AbstractProduct *pProduct(CC.CreateObject(nullID));
        CC.ReleaseObject(pProduct);
    }
    elapsedCache = getmilliSeconds() - start;
    cout << "No cache "<<elapsedNoCache<<" ms | Cache "<<elapsedCache<<" ms | Efficiency " << (double(elapsedNoCache)/elapsedCache)-1 << endl;
}

void testCachePerformance()
{
    for(int i=1;i<=5;i++){
        cout << " || => " << i << " iterations" << endl;
        unitTestCachePerformance(i);
    }
}

template< class Cache >
milliSec typicalUse( Cache &CC, unsigned objectKind, unsigned maxObjectCount, unsigned maxIteration)
{
    assert(objectKind>0);
    assert(maxIteration>0);
    assert(maxObjectCount>0);
    vector< AbstractProduct* > fetched;
    fetched.reserve(maxObjectCount);
    srand(0); // initialise the pseudo random operator
    milliSec start(0);
    milliSec end(0);
    try{
        // Registering objects
        for(size_t i=0;i<objectKind;i++)
            CC.Register(i, createProductNull);
        // Simulating real use
        start = getmilliSeconds();
        for(unsigned i=0;i<maxIteration;i++)
        {
            const size_t size(fetched.size());
            if( size == maxObjectCount ){
                CC.ReleaseObject(fetched.back());
                fetched.pop_back();
            } else if(size == 0){
                fetched.push_back(CC.CreateObject(int(objectKind*rand()/(RAND_MAX + 1.0))));
            } else if(rand()<RAND_MAX/2){
                CC.ReleaseObject(fetched.back());
                fetched.pop_back();
            } else {
                fetched.push_back(CC.CreateObject(int(objectKind*rand()/(RAND_MAX + 1.0))));
            }
        }
        end = getmilliSeconds();
    }catch(std::exception &e)
    {
        cout << "Error in executing typicalUse " << endl << e.what() << endl;
    }
    // Cleaning in use objects
    for(std::vector<AbstractProduct*>::iterator itr = fetched.begin(); itr!=fetched.end(); itr++)
        CC.ReleaseObject(*itr);
    fetched.clear();
    return end-start;
}

template< class Cache >
void displayTypicalUse(Cache &CC, unsigned objectKind, unsigned maxObjectCount, unsigned maxIteration)
{
    CC.displayCacheType();
    cout << "====> " << typicalUse<Cache>(CC, objectKind, maxObjectCount, maxIteration) << " ms" << endl;
}

void testTypicalUse()
{
    const unsigned objectKind(10);
    const unsigned maxObjectCount(300);
    const unsigned maxIteration(1000000);
    cout << "# " << objectKind << " objects registered in the Factory" << endl;
    cout << "# Cache contains max " << maxObjectCount << " objects" << endl;
    cout << "# Test performs "<< maxIteration <<" iterations" << endl;
    {
        typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AlwaysCreate, EvictRandom, SimpleStatisticPolicy > CRandomEvict;
        CRandomEvict cache;
        displayTypicalUse<CRandomEvict>( cache, objectKind, maxObjectCount, maxIteration);
    }
    {
        typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AlwaysCreate, EvictLRU, SimpleStatisticPolicy > CLRUEvict;
        CLRUEvict cache;
        displayTypicalUse( cache, objectKind, maxObjectCount, maxIteration);
    }
    {
        typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AlwaysCreate, EvictAging, SimpleStatisticPolicy > CAgingEvict;
        CAgingEvict cache;
        displayTypicalUse( cache, objectKind, maxObjectCount, maxIteration);
    }
}

template< class Cache >
bool testEvictionError()
{
    bool testPassed = false;
    Cache CC;
    CC.Register(nullID, createProductNull);
    CC.setMaxCreation(1);
    AbstractProduct *pProduct1 = NULL, *pProduct2 = NULL;
    try{
        pProduct1 = CC.CreateObject(nullID); // should be OK
        pProduct2 = CC.CreateObject(nullID); // should cast an exception
    } catch(std::exception &e){
        if(strcmp(e.what(), EvictionException().what())==0)
            testPassed = true;
    }
    if(pProduct1!=NULL)
        CC.ReleaseObject(pProduct1);
    if(pProduct2!=NULL)
        CC.ReleaseObject(pProduct2);
    return testPassed;
}

bool testAllEvictionError()
{
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictRandom > CRandomEvict;
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictLRU > CLRUEvict;
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictAging > CAgingEvict;
    bool test1 = dispResult("Random policy", testEvictionError< CRandomEvict >());
    bool test2 = dispResult("LRU policy", testEvictionError< CLRUEvict >());
    bool test3 = dispResult("Aging policy", testEvictionError< CAgingEvict >());
    return test1 && test2 && test3;
}

bool testAmountLimitedCreation()
{
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, AmountLimitedCreation, EvictRandom, SimpleStatisticPolicy > CCache;
    CCache CC;
    CC.Register(nullID, createDebugProductNull);
    CC.Register(intID, createDebugProductNull);
    // CC.setMaxCreation(0); <== would break on assert, such a cache is useless
    CC.setMaxCreation(1);
    AbstractProduct *pProduct1 = NULL, *pProduct2 = NULL;
    pProduct1 = CC.CreateObject(nullID); // should be OK
    CC.ReleaseObject(pProduct1);
    pProduct2 = CC.CreateObject(intID); // should call the evict method
    if(pProduct1!=NULL)
        CC.ReleaseObject(pProduct1);
    if(pProduct2!=NULL)
        CC.ReleaseObject(pProduct2);
    return (CC.getDestroyed()==1);
}

bool testRateLimitedFetchPolicy(bool waitBetweenFetch)
{
    typedef CachedFactory< AbstractProduct, int, NullType, SimplePointer, RateLimitedCreation > CCache;
    CCache CC;
    CC.Register(nullID, createProductNull);
    CC.setRate(2/*max two fetchs*/,100/*within 100 ms*/);
 
    bool exceptionOccured = false;
    const int number(5);
    const int sleepTime(70);
    AbstractProduct *products[number];
    for(int i=0;i<number;i++) products[i]=NULL; // initializing
    try{
        for(int i=0;i<number;i++){
            products[i] = CC.CreateObject(nullID);
            if(waitBetweenFetch && (i!=(number-1))){
                Sleep(sleepTime);
            }
        }
    } catch (std::exception &e){
        exceptionOccured = true;
        cout << "Exception occured" << endl << e.what() <<endl;
    }
    
    // Cleaning things by releasing 
    for(int i=0;i<number;i++)
        if(products[i]!=NULL)
            CC.ReleaseObject(products[i]);

    return exceptionOccured;
}

bool fullTestRateLimitedFetchPolicy()
{
    cout << " || => Setting rate limit to 2 fetch within 100 ms." << endl;
    cout << " || => generating 5 objects " << endl;
    bool test1 = dispResult("Fast creation", testRateLimitedFetchPolicy(false)==true);
    cout << " || => generating 5 objects with 70ms between each Fetch" << endl;
    bool test2 = dispResult("Slow creation", testRateLimitedFetchPolicy(true)==false);
    return test1 && test2;
}

bool testRelease(){
    typedef CachedFactory< AbstractProduct, int > CCache;
	CCache CC;
    CC.Register(nullID, createProductNull);
    AbstractProduct *pProduct = NULL;
    CC.ReleaseObject(pProduct);
    cout << "Caching failed" << endl;
    return false;
}


bool testCache()
{
    typedef CachedFactory< AbstractProduct, int, Seq< int, int > > CCache2Parm;

	CCache2Parm CC2;
    CC2.Register(intID, createProductInt);
	AbstractProduct * pProduct = CC2.CreateObject(intID,5,3);
	AbstractProduct * pSave(pProduct);
	CC2.ReleaseObject(pProduct);
	pProduct = CC2.CreateObject(intID,5,3);
	if(pSave != pProduct)
    {
	   cout << "Caching failed" << endl;
	   return false;
    }
    else
    {
        CC2.ReleaseObject(pProduct);
        return true;
    }
}

#include <loki/SPCachedFactory.h>

template<class T>
class SmartPointer_OneTArg : public SmartPointer<T>
{
};

bool testSmartPointer()
{
	typedef CachedFactory< AbstractProduct, int, NullType, SmartPointer_OneTArg, AlwaysCreate, EvictRandom, SimpleStatisticPolicy > CFactory;
	CFactory factory;
	factory.Register(0, createProductNull);
	for(int i=0;i<500;++i)
	{
		CFactory::ProductReturn ptr(factory.CreateObject(0));
		CFactory::ProductReturn ptr2(ptr); // check that copying the SP won't release the object twice
	}
	// all object should have been returned to the factory
	bool outOk = factory.getOut()==0;
	// one object allocater
	bool allocOk = factory.getAllocated()==1;
	// one missed, the first one
	bool missedOk = factory.getMissed()==1;
	// 500 fetched
	bool fetchedOk = factory.getFetched()==500;
	// 499 hit
	bool hitOk = factory.getHit()==499;
	return outOk && allocOk && missedOk && fetchedOk && hitOk;
}

void dispText(const char* text)
{
    cout << endl;
    cout << "##========================================"<< endl;
    cout << "## " << text << endl;
    cout << "##========================================"<< endl;
}

void dispText(const char* text, const char* comment)
{
    cout << endl;
    cout << "##========================================"<< endl;
    cout << "## " << text << endl;
    cout << "## " << comment << endl;
    cout << "##========================================"<< endl;
}

void separator()
{
    cout << endl << endl;
}

void performanceTests()
{
    dispText(" ==> Performance tests <==");
    separator();
    dispText("Test typical use", "tries different Cache strategies in a typical use configuration");
    testTypicalUse();
    separator();
    dispText("Test efficiency","Comparison between generating costly objects (100ms) and using Cache");
    testCachePerformance();
    separator();
    dispText("Test overhead","Intensive use of Cache to determine the cache time overhead");
    testCacheOverhead();
    separator();
}

void reliabilityTests()
{
    dispText(" ==> Reliability tests <==");
    separator();
    dispText("Test caching", "Trying to Create, Release, Create and see if Cache gives the same object");
    bool cacheResult= dispResult("caching result", testCache());
    separator();
    dispText("Test RateLimitedFetch policy",
    "Trying to quickly create objects, then same scenario with pause in between");
    bool rateLimitedResult= dispResult("RateLimitedFetch policy result",fullTestRateLimitedFetchPolicy());
    separator();
    dispText("Test AmountLimitedCreation policy","Trying to create 2 objects with a limit of 1 creation max, you should see a destroyed object (eviction)");
    bool amountLimitedResult = dispResult("AmountLimitedCreation policy result", testAmountLimitedCreation());
    separator();
    dispText("Test eviction error", "An eviction should occur (Creation Policy), but all object are in use");
    bool evictionTest = dispResult("eviction error test result", testAllEvictionError());
    separator();
    dispText("Smart pointer", "The factory provides smart pointers, when pointers go out of scope, the object returns to Cache");
    bool spTest = dispResult("Smart Pointer test result", testSmartPointer());
    separator();
    
    if(cacheResult&&rateLimitedResult&&amountLimitedResult&&evictionTest&&spTest)
        dispText("All tests passed successfully");
    else
        dispText("One or more test have failed");
}

int main(int , char **)
{
    try{
        performanceTests();
        reliabilityTests();
    }catch(std::exception &e){
        cerr << e.what() << endl;
        cerr << "Error while performing tests" << endl;
    }
}

// TODO provide :
// +Efficiency tests
// +Overhead tests
// +Fiability tests
