/** @file DefaultAlloc.cpp  Tests compiler compliance when using the new and
 delete operators.

 @author Rich Sposato (original author)

 @par Purpose of Program
 This program tests how well C++ compilers comply with the ISO C++ Standard in
 implementing new, new [], delete, and delete [] operators.  The program should
 identify each known way in which a C++ compiler can fail to comply.  It
 provides versions of each new, new[], delete, and delete [] operators that
 classes typically overload.  These functions have the same signatures as
 those in the Loki project, and the same as those enumerated in section 18 of
 the ISO C++ Standard.

 @par Running the Program
 If the programs fails to compile, or crashes when executed, the compiler does
 not comply with the ISO C++ Standard.  As the program runs, it should:
 - identify your compiler,
 - execute each version of the new, new[], delete, and delete [] operators,
 - determine if compiler provides correct behavior when exceptions are thrown,
 - determine if correct operators were called for each appropriate situation,
 - and determine if the correct parameters were provided to each function.

 @par License
 This file is released under the MIT License - like the rest of Loki.  You
 may use it and modify it as you wish - except that the entire set of
 documentation commentary at the top of this file shall remain intact.

 @par Connection to Loki
 This file is provided as part of the Loki project - but does not depend upon
 any other files in Loki.  It was made for Loki by Rich Sposato to determine
 which compilers can safely use Loki's Small-Object Allocator.  During his
 work on Loki, Rich noticed that some compilers did not provide the correct
 values to the allocation / deallocation operators, or worse, called the wrong
 operator.  Rather than make several variations of allocator functions to work
 around areas of non-compliance, Rich decided to write a program that exposes
 which compilers do not comply with the ISO C++ Standard.  It is recommended
 that you execute this program before incorporating Loki's Small-Object
 Allocator from your source code.  Hence, this program is a "crash test dummy"
 for how well your compiler can handle Loki.
 */

// $Id: DefaultAlloc.cpp 761 2006-10-17 20:48:18Z syntheticpp $


// ----------------------------------------------------------------------------

#include <iostream>
#include <new>


// ----------------------------------------------------------------------------

using namespace std;


/** @par Array Overhead
 Most compilers require a few extra bytes when allocating arrays via the new []
 operators.  The overhead is needed to keep track of how many objects are in
 the array and how many bytes were allocated.  The overhead often exists as 4
 extra bytes at the start of the array.
 */
static const unsigned int ArrayOverhead = 4;
static const unsigned int ArrayCount = 1000;

static bool s_verbose = false;


// ----------------------------------------------------------------------------

class EmptyThing { };

class BadBee : public std::exception
{
public:
    BadBee( void ) : m_message( "Unknown exception." ) {}
    BadBee( const char * m ) : m_message( m ) {}
    virtual ~BadBee( void ) throw() {}
    virtual const char* what() const throw() { return m_message; }
private:
    const char * m_message;
};

class Bee
{
public:

    /// Enumerates all the new and delete operators.
    enum OperatorName
    {
        Unknown,
        NewScalarThrow,
        NewScalarNoThrow,
        NewScalarPlacement,
        DeleteScalarWithSize,
        DeleteScalarNoThrow,
        DeleteScalarPlacement,
        NewArrayThrow,
        NewArrayNoThrow,
        NewArrayPlacement,
        DeleteArrayWithSize,
        DeleteArrayNoThrow,
        DeleteArrayPlacement,
    };

    /// Provides textual name for all new and delete operators.
    static const char * Name( OperatorName type )
    {
        switch ( type )
        {
            case NewScalarThrow:
                return "scalar throwing new operator";
            case NewScalarNoThrow:
                return "scalar nothrow new operator";
            case NewScalarPlacement:
                return "scalar placement new operator";
            case DeleteScalarWithSize:
                return "scalar delete operator with size";
            case DeleteScalarNoThrow:
                return "scalar nothrow delete operator";
            case DeleteScalarPlacement:
                return "scalar placement delete operator";
            case NewArrayThrow:
                return "array throwing new operator";
            case NewArrayNoThrow:
                return "array nothrow new operator";
            case NewArrayPlacement:
                return "array placement new operator";
            case DeleteArrayWithSize:
                return "array delete operator with size";
            case DeleteArrayNoThrow:
                return "array nothrow delete operator";
            case DeleteArrayPlacement:
                return "array placement delete operator";
        }
        return "Unknown";
    }

#ifdef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    static void * operator new ( std::size_t size )
#else
    static void * operator new ( std::size_t size ) throw ( std::bad_alloc )
#endif
    {
        CheckCallType( NewScalarThrow, "new" );
        void * place = ::operator new( size );
        if ( s_verbose )
            cout << "static void * operator new ( std::size_t size ) throw ( std::bad_alloc )" << endl
                 << "\tsize = " << size << endl;
        cout << endl;
        return place;
    }

    static void * operator new ( std::size_t size, const std::nothrow_t & nt ) throw ()
    {
        s_triedNoThrowNew = true;
        CheckCallType( NewScalarNoThrow, "new" );
        void * place = ::operator new( size, nt );
        if ( s_verbose )
            cout << "static void * operator new ( std::size_t size," << endl
                 << "\tconst std::nothrow_t & nt ) throw ()" << endl
                 << "\tsize = " << size << endl;
        cout << endl;
        return place;
    }

    static void * operator new ( std::size_t size, void * place )
    {
        void * place2 = ::operator new( size, place );
        CheckCallType( NewScalarPlacement, "new" );
        if ( s_verbose )
            cout << "static void * operator new ( std::size_t size, void * place )" << endl
                 << "\tsize = " << size << endl
                 << "\tplace = " << place << endl
                 << "\treturn = " << place2 << endl;
        cout << endl;
        return place2;
    }

    /** @note This version of delete [] is commented out since the C++ Standard
     requires compilers to use the version of delete [] with the size parameter
     if that one is available and this one is not.
     */
//    static void operator delete ( void * place ) throw ()
//    {
//        CheckCallType( DeleteScalarNoSize, "delete" );
//        cout << "scalar delete operator" << endl;
//        if ( s_verbose )
//            cout << "static void operator delete ( void * place ) throw ()" << endl
//                 << "\tplace = " << place << endl;
//        cout << endl;
//        ::operator delete( place );
//    }

    static void operator delete ( void * place, std::size_t size ) throw ()
    {
        CheckCallType( DeleteScalarWithSize, "delete" );
        if ( s_verbose )
            cout << "static void operator delete ( void * place, std::size_t size ) throw ()" << endl
                 << "\tplace = " << place << endl
                 << "\tsize = " << size << endl;
        cout << endl;
        ::operator delete( place );
    }

    static void operator delete ( void * place, const std::nothrow_t & nt ) throw()
    {
        CheckCallType( DeleteScalarNoThrow, "delete" );
        if ( s_verbose )
            cout << "static void operator delete ( void * place," << endl
                 << "\tconst std::nothrow_t & nt ) throw()" << endl
                 << "\tplace = " << place << endl;
        cout << endl;
        ::operator delete( place, nt );
    }

    static void operator delete ( void * place1, void * place2 )
    {
        CheckCallType( DeleteScalarPlacement, "delete" );
        if ( s_verbose )
            cout << "static void operator delete ( void * place1, void * place2 )" << endl
                 << "\tplace1 = " << place1 << endl
                 << "\tplace2 = " << place2 << endl;
        cout << endl;
        ::operator delete( place1, place2 );
    }


#ifdef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    static void * operator new [] ( std::size_t size )
#else
    static void * operator new [] ( std::size_t size ) throw ( std::bad_alloc )
#endif
    {
        s_newArraySize = size;
        CheckCallType( NewArrayThrow, "new []" );
        void * place = ::operator new [] ( size );
        if ( s_verbose )
           cout << "static void * operator new [] ( std::size_t size ) throw ( std::bad_alloc )" << endl
                 << "\tsize = " << size << endl;
        cout << endl;
        return place;
    }

    static void * operator new [] ( std::size_t size, const std::nothrow_t & nt ) throw ()
    {
        s_triedNoThrowNew = true;
        s_newArraySize = size;
        CheckCallType( NewArrayNoThrow, "new []" );
        void * place = ::operator new [] ( size, nt );
        if ( s_verbose )
            cout << "static void * operator new [] ( std::size_t size," << endl
                 << "\tconst std::nothrow_t & nt ) throw ()" << endl
                 << "\tsize = " << size << endl;
        cout << endl;
        return place;
    }

    static void * operator new [] ( std::size_t size, void * place )
    {
        CheckCallType( NewArrayPlacement, "new []" );
        void * place2 = ::operator new [] ( size, place );
        if ( s_verbose )
            cout << "static void * operator new [] ( std::size_t size, void * place )" << endl
                 << "\tsize = " << size << endl
                 << "\tplace = " << place << endl
                 << "\treturn = " << place2 << endl;
        cout << endl;
        return place2;
    }

    /** @note This version of delete [] is commented out since the C++ Standard
     requires compilers to use the version of delete [] with the size parameter
     if that one is available and this one is not.
     */
//    static void operator delete [] ( void * place ) throw ()
//    {
//        CheckCallType( DeleteArrayNoSize, "delete []" );
//        if ( s_verbose )
//            cout << "static void operator delete [] ( void * place ) throw ()" << endl
//                 << "\tplace = " << place << endl;
//        cout << endl;
//        ::operator delete [] ( place );
//    }

    static void operator delete [] ( void * place, std::size_t size ) throw ()
    {
        s_deleteArraySize = size;
        CheckCallType( DeleteArrayWithSize, "delete []" );
        if ( s_verbose )
            cout << "static void operator delete [] ( void * place, std::size_t size ) throw ()" << endl
                 << "\tplace = " << place << endl
                 << "\tsize = " << size << endl;
        if ( s_newArraySize != size )
        {
            cout << "\tERROR: The size parameter should be: " << s_newArraySize << '!' << endl;
            s_errorCount++;
        }
        cout << endl;
        ::operator delete [] ( place );
    }

    static void operator delete [] ( void * place, const std::nothrow_t & nt ) throw()
    {
        CheckCallType( DeleteArrayNoThrow, "delete []" );
        if ( s_verbose )
            cout << "static void operator delete [] ( void * place," << endl
                 << "\tconst std::nothrow_t & nt ) throw()" << endl
                 << "\tplace = " << place << endl;
        cout << endl;
        ::operator delete [] ( place, nt );
    }

    static void operator delete [] ( void * place1, void * place2 )
    {
        CheckCallType( DeleteArrayPlacement, "delete []" );
        if ( s_verbose )
            cout << "static void operator delete [] ( void * place1, void * place2 )" << endl
                 << "\tplace1 = " << place1 << endl
                 << "\tplace2 = " << place2 << endl;
        cout << endl;
        ::operator delete [] ( place1, place2 );
    }

    static void OutputArraySizeInfo( void );

    inline static void Clear( void )
    {
        s_newArraySize = 0;
        s_deleteArraySize = 0;
    }

    inline static void SetConstructorToThrow( bool b ) { s_throw = b; }

    inline static bool WillConstructorThrow( void ) { return s_throw; }

    Bee( void ) : m_legs( 6 )
    {
        if ( s_throw )
        {
            s_expectedCall = ( NewArrayNoThrow == s_expectedCall ) ?
                DeleteArrayNoThrow : DeleteScalarNoThrow;
            throw BadBee( "Bee died before it hatched." );
        }
    }

    ~Bee( void ) {}

    inline static unsigned int GetErrorCount( void ) { return s_errorCount; }
    inline static void IncrementErrorCount( void ) { ++s_errorCount; }

    static OperatorName s_expectedCall;

private:
    static std::size_t s_newArraySize;
    static std::size_t s_deleteArraySize;
    static unsigned int s_errorCount;
    static bool s_triedNoThrowNew;
    static bool s_throw;

    static void CheckCallType( OperatorName actual, const char * name )
    {
        cout << Name( actual ) << endl;
        if ( s_expectedCall != actual )
        {
            s_errorCount++;
            cout << "\tERROR!  The wrong " << name << " operator was called!" << endl
                 << "\tExpected " << Name( s_expectedCall ) << " instead!" << endl;
        }
    }

    unsigned int m_legs;
};

// ----------------------------------------------------------------------------

Bee::OperatorName Bee::s_expectedCall = Bee::Unknown;
std::size_t Bee::s_newArraySize = 0;
std::size_t Bee::s_deleteArraySize = 0;
unsigned int Bee::s_errorCount = 0;
bool Bee::s_triedNoThrowNew = false;
bool Bee::s_throw = false;

// ----------------------------------------------------------------------------

void Bee::OutputArraySizeInfo( void )
{
    cout << endl;
    if ( ( 0 == s_newArraySize ) || ( 0 == s_deleteArraySize ) )
        cout << "I do not have sufficient information to know if your compiler complies" << endl
             << "with Section 12.5/5 of the C++ Standard.  You should not use Loki to" << endl
             << "allocate arrays.  Please do not #define LOKI_SMALL_OBJECT_USE_NEW_ARRAY." << endl;
    else if ( s_newArraySize == s_deleteArraySize )
        cout << "Your compiler provides correct value for delete [] operator.  You may use" << endl
             << "Loki to allocate arrays via #define LOKI_SMALL_OBJECT_USE_NEW_ARRAY.  If" << endl
             << "you do, please run the SmallObjBench tests to see if Loki is faster or" << endl
             << "slower at allocating arrays than the default allocator." << endl
             << endl;
    else
        cout << "Your compiler does NOT provide correct size value for delete [] operator." << endl
             << "It should provide the same size value to delete [] as it did to new []." << endl
             << "Please ask your compiler vendor to comply with Section 12.5/5 of the C++" << endl
             << "Standard.  You should NOT use Loki to allocate arrays.  Please do *not*" << endl
             << "#define LOKI_SMALL_OBJECT_USE_NEW_ARRAY." << endl
             << endl;
    cout << endl;
}

// ----------------------------------------------------------------------------

const char * GetCompilerName( void )
{
    /** @note This list of predefined compiler version macros comes from
      http://predef.sourceforge.net/precomp.html  Feel free to update this
      function to add specific version numbers or add other compilers.
     */
#if (__INTEL_COMPILER)
    return "Intel";

#elif (__ICC)
    return "Intel";

#elif (__KCC)
    return "Intel's KAI C++";

#elif (__MINGW32__)
    return "MinGW";

#elif (__COMO__)
    return "Comeau C++";

#elif (__DECC)
    return "Compaq C/C++";

#elif (VAXC)
    return "Compaq C/C++";

#elif (__VAXC)
    return "Compaq C/C++";

#elif (_CRAYC)
    return "Cray C/C++";

#elif (__CYGWIN__)
    return "Cygwin";

#elif (__DCC__)
    return "Diab C/C++";

#elif (__DMC__)
    return "Digital Mars";

#elif (__SC__)
    return "Digital Mars";

#elif (__ZTC__)
    return "Digital Mars";

#elif (__EDG__)
    return "EDG C++ Front End";

#elif (__GNUC__)
    return "Gnu C/C++";

#elif (__HP_cc)
    return "HP ANSI C/aC++";

#elif (__HP_aCC)
    return "HP ANSI C/aC++";

#elif (__xlC__)
    return "IBM XL C/C++";

#elif (__IBMC__)
    return "IBM XL C/C++";

#elif (__IBMCPP__)
    return "IBM XL C/C++";

#elif (LCC)
    return "LCC";

#elif (__HIGHC__)
    return "MetaWare High C/C++";

#elif (sgi)
    return "MIPSpro";

#elif (__sgi)
    return "MIPSpro";

#elif (__MRC__)
    return "MPW C++";

#elif (MPW_C)
    return "MPW C++";

#elif (MPW_CPLUS)
    return "MPW C++";

#elif (__CC_NORCROFT)
    return "Norcroft C";

#elif (__POCC__)
    return "Pelles C";

#elif (SASC)
    return "SAS/C";

#elif (__SASC)
    return "SAS/C";

#elif (__SASC__)
    return "SAS/C";

#elif (_SCO_DS)
    return "SCO";

#elif (__SUNPRO_C)
    return "Sun Workshop C/C++";

#elif (__SUNPRO_CC)
    return "Sun Workshop C/C++";

#elif (__TenDRA__)
    return "TenDRA C/C++";

#elif (__TINYC__)
    return "Tiny C";

#elif (__USLC__)
    return "USL C";

#elif (__WATCOMC__)
    return "Watcom C++";

#elif (__MWERKS__)
    return "MetroWerks CodeWarrior";

#elif (__TURBOC__)
    return "Borland Turbo C";

#elif (__BORLANDC__)
    return "Borland C++";

#elif (_MSC_VER >= 1400)
    return "Microsoft 8.0 or higher";

#elif (_MSC_VER >= 1300)
    return "Microsoft 7";

#elif (_MSC_VER >= 1200)
    return "Microsoft 6";

#elif (_MSC_VER)
    return "Microsoft, but a version lower than 6";

#else
    return "an Unknown type";
#endif
}

// ----------------------------------------------------------------------------

void OutputCompilerType( void )
{
    cout << "Your compiler is " << GetCompilerName() << '.' << endl << endl;

    const bool zeroSize = ( 0 == sizeof(EmptyThing) );
    if ( zeroSize )
    {
        cout << "Your compiler allows datatypes to be zero bytes, which is contrary to" << endl;
        cout << "the ISO C++ Standard." << endl;
    }
    else
    {
        cout << "Your compiler correctly sets the sizeof empty data types to "
             << sizeof(EmptyThing) << " byte(s) instead of zero bytes." << endl;
    }
    cout << "( sizeof(EmptyThing) == " << sizeof(EmptyThing) << " )" << endl << endl;
}

// ----------------------------------------------------------------------------

void RunTests( void )
{

    char localQueen[ sizeof(Bee) ];
    char localHive[ sizeof(Bee) * ArrayCount + ArrayOverhead ];
    void * here = localQueen;

    // test throwing new
    if ( s_verbose )
        cout << "Bee * queen = new Bee;" << endl;
    Bee::s_expectedCall = Bee::NewScalarThrow;
    Bee * queen = new Bee;
    if ( s_verbose )
        cout << "delete queen;" << endl;
    // test normal delete
    Bee::s_expectedCall = Bee::DeleteScalarWithSize;
    delete queen;

    // test throwing new []
    if ( s_verbose )
        cout << "Bee * hive = new Bee [ ArrayCount ];" << endl;
    Bee::s_expectedCall = Bee::NewArrayThrow;
    Bee * hive = new Bee [ ArrayCount ];
    if ( s_verbose )
        cout << "delete [] hive;" << endl;
    // test normal delete []
    Bee::s_expectedCall = Bee::DeleteArrayWithSize;
    delete [] hive;
    Bee::OutputArraySizeInfo();

    // test nothrow new
    if ( s_verbose )
        cout << "Bee * queen = new nothrow Bee;" << endl;
    Bee::s_expectedCall = Bee::NewScalarNoThrow;
    queen = new (nothrow) Bee;
    if ( s_verbose )
        cout << "delete queen;" << endl;
    // test normal delete
    Bee::s_expectedCall = Bee::DeleteScalarWithSize;
    delete queen;

    // test nothrow new []
    if ( s_verbose )
        cout << "Bee * hive = new (nothrow) Bee [ 1000 ];" << endl;
    Bee::s_expectedCall = Bee::NewArrayNoThrow;
    hive = new (nothrow) Bee [ ArrayCount ];
    if ( s_verbose )
        cout << "delete [] hive;" << endl;
    // test normal delete []
    Bee::s_expectedCall = Bee::DeleteArrayWithSize;
    delete [] hive;

    Bee::Clear();
    Bee::SetConstructorToThrow( true );
    std::nothrow_t nothrow;

    try
    {
        // test nothrow new when constructor throws.
        if ( s_verbose )
            cout << "Bee * queen = new nothrow Bee;" << endl;
        Bee::s_expectedCall = Bee::NewScalarNoThrow;
        queen = new (nothrow) Bee;
        if ( s_verbose )
            cout << "delete queen;" << endl;
        // test normal delete
        delete queen;
    }
    catch ( const std::exception & ex0 )
    {
        // nothrow delete should be called before catch block starts
        if ( s_verbose )
            cout << "catch ( const std::exception & ex0 )" << endl
                 << "\t" << ex0.what() << endl;
    }

    try
    {
        // test nothrow new [] when constructor throws.
        if ( s_verbose )
            cout << "Bee * hive = new (nothrow) Bee [ 1000 ];" << endl;
        Bee::s_expectedCall = Bee::NewArrayNoThrow;
        hive = new (nothrow) Bee [ ArrayCount ];
        if ( s_verbose )
            cout << "delete queen;" << endl;
        // test normal delete
        delete [] hive;
    }
    catch ( const std::exception & ex1 )
    {
        // nothrow delete should be called before catch block starts
        if ( s_verbose )
            cout << "catch ( const std::exception & ex0 )" << endl
                 << "\t" << ex1.what() << endl;
    }
    Bee::SetConstructorToThrow( false );

    // test placement new
    if ( s_verbose )
        cout << "Bee * queen = new (here) Bee;" << endl;
    Bee::s_expectedCall = Bee::NewScalarPlacement;
    queen = new (here) Bee;
    if ( s_verbose )
        cout << "delete (here, queen);" << endl;
    // test placement delete;
    Bee::s_expectedCall = Bee::DeleteScalarPlacement;
    delete (here, queen);

    // test placement new []
    here = localHive;
    if ( s_verbose )
        cout << "Bee * hive = new (here) Bee [ ArrayCount ];" << endl;
    Bee::s_expectedCall = Bee::NewArrayPlacement;
    hive = new (here) Bee [ ArrayCount ];
    if ( s_verbose )
        cout << "delete [] (here, hive);" << endl;
    // test placement delete [];
    Bee::s_expectedCall = Bee::DeleteArrayPlacement;
    delete [] (here, hive);

}

// ----------------------------------------------------------------------------

int main( unsigned int argc, const char * const argv[] )
{

    bool caught = false;
    for ( unsigned int aa = 1; aa < argc; ++aa )
    {
        if ( ::strcmp( argv[aa], "-v" ) == 0 )
            s_verbose = true;
    }

    OutputCompilerType();
    try
    {
        RunTests();
    }
    catch ( const std::bad_alloc & ex1 )
    {
        caught = true;
        if ( s_verbose )
            cout << "catch ( const std::bad_alloc & ex1 )" << endl
                 << "\t" << ex1.what() << endl;
    }
    catch ( const std::exception & ex2 )
    {
        caught = true;
        if ( s_verbose )
            cout << "catch ( const std::exception & ex2 )" << endl
                 << "\t" << ex2.what() << endl;
    }
    catch ( ... )
    {
        caught = true;
        if ( s_verbose )
            cout << "catch ( ... )" << endl;
    }

    if ( caught )
    {
        Bee::IncrementErrorCount();
        cout << "Tests of default new and delete operators aborted because of exception!"
             << endl;
    }

    cout << "Done running tests of default allocators." << endl;
    cout << "Total errors detected: " << Bee::GetErrorCount() << endl;

    return 0;
}

// ----------------------------------------------------------------------------
