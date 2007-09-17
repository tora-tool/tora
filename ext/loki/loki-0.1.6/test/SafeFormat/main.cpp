////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 by Andrei Alexandrescu
// Permission to use, copy, modify, distribute, and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author makes no representations about the suitability of this software
//     for any purpose. It is provided "as is" without express or implied
//     warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/SafeFormat.h>
#include <iostream>
#include <cassert>
#include <utility>

#include "../SmallObj/timer.h"

#if defined(_MSC_VER)
#if _MSC_VER >= 1400
#define sprintf     sprintf_s
#define _snprintf _snprintf_s
#else
#define sprintf     sprintf
#define _snprintf _snprintf
#endif
#endif

using namespace std;
using namespace Loki;

template <class Integral1, class Integral2>
Integral2 RandomInt(Integral1 low, Integral2 up)
{
    // From ``Accelerated C++'', page 135:
    // random integer in the range [0, n)
    // We adjust to generate in the range [0, n]
    const Integral2
    low2 = low,
           n = up - low;

    assert(n > 0);

    const size_t bucket_size = RAND_MAX / n;
    assert(bucket_size > 0);

    Integral2 r;
    do
        r = Integral2(rand() / bucket_size);
    while (r > n);

    r = r + low2;
    assert(r >= low2 && r <= up);
    return r;
}

string RandomString(unsigned int maxSize)
{
    string result(RandomInt(0, maxSize), '\0');
    unsigned int i = 0;
    for (; i != result.size(); ++i)
    {
        result[i] = RandomInt('a', 'z');
    }
    return result;
}

template <class T>
void TestCase(const string& fmt, T value)
{
    char buf[4096];
    std::string s;
    const int i1 = SPrintf(s, fmt.c_str())(value);

#ifdef _MSC_VER
    const int i2 = _snprintf(buf, sizeof(buf), fmt.c_str(), value);
#else
    const int i2 = snprintf(buf, sizeof(buf), fmt.c_str(), value);
#endif

    if (i1 != i2 || s != buf)
    {
        cout << endl
        << "Reference: " << i2 << "; Actual: " << i1 << ",  Difference = " << i2-i1 << endl
        << "V: [" << value << "]" << endl
        << "F: [" << fmt << "]" << endl
        << "R: [" << buf << "]" << endl
        << "A: [" << s.c_str() << "]" << endl;
        assert(false);
    }
}

template <class T, class U>
void TestCase2(const string& fmt, T value, U value2)
{
    char buf[4096];
    std::string s;
    const int i1 = SPrintf(s, fmt.c_str())(value)(value2);
    const int i2 = snprintf(buf, sizeof(buf), fmt.c_str(), value, value2);
    assert(i1 == i2);
    assert(s == buf);
}

int main(int argc, char** argv)
{        

    if (argc == 2)
    {
        // test speed

        Timer t;

        int loop = atoi(argv[1]);

        if(loop < 100)
            loop = 100;

        t.start();
        for (int i=loop; i > 0; --i)
            printf("Hey, %u frobnicators and %u twiddlicators\n",i, i);
        t.stop();
        t.t100 = t.t();
        int t_printf = t.t();


        t.start();
        for (int i=loop; i > 0; --i)
            Printf("Hey, %u frobnicators and %u twiddlicators\n")(i)(i);
        t.stop();
        int t_Printf = t.t();


        t.start();
        for (int i=loop; i > 0; --i)
            cout << "Hey, " << i << " frobnicators and " << i <<" twiddlicators\n";
        t.stop();
        int t_cout = t.t();


        Printf("\n\nElapsed time for %i outputs\n\n")(loop);
        t.print(t_printf,"printf   : ");
        t.print(t_Printf,"Printf   : ");
        t.print(t_cout,  "std::cout: ");
    }
    else
    {
        //srand(time(0));
        srand(0);
        printf("\nNumber of tests:\n");
        for (unsigned i = 0; ; ++i)
        {
            printf("%u\r", i);

            // Generate a random string for the head
            string lead = RandomString(100);
            // This string will hold a random format specification
            string formatSpec(lead + "|%");
            // Generate a random set of flags
            static const string flags("-+0 #");
            size_t maxFlags = RandomInt(0u, flags.length() - 1);
            for (size_t i = 0; i != maxFlags; ++i)
            {
                formatSpec += flags[RandomInt(0u, flags.length() - 1)];
            }
            // Generate an optional random width
            if (RandomInt(0, 1))
            {
                const unsigned int width = RandomInt(0, 100);
                char buf[4];
                sprintf(buf, "%u", width);
                formatSpec += buf;
            }
            // Generate an optional random precision
            if (RandomInt(0, 1))
            {
                const unsigned int prec = RandomInt(0, 100);
                char buf[4];
                sprintf(buf, "%u", prec);
                formatSpec += '.';
                formatSpec += buf;
            }

            // Generate a random type character
            static const string type("cdeEfgGinopsuxX");

            const char typeSpec = type[RandomInt(0u, type.size() - 1)];
            // Generate an optional type prefix
            static const string prefix("hl");
            if (typeSpec != 's' && RandomInt(0, 1))
            {
                formatSpec += prefix[RandomInt(0u, prefix.size() - 1)];
            }
            formatSpec += typeSpec;
            formatSpec += '|';
            formatSpec += RandomString(100);

            switch (typeSpec)
            {
            case 'c':
                TestCase(formatSpec, RandomInt(1, 127));
                break;
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                //TestCase(formatSpec, RandomInt(-10000, 10000));
                // don't test negative values on 64bit systems, because 
                // snprintf does not support 64 Bit values
                TestCase(formatSpec, RandomInt( -10000 * (sizeof(size_t)>4 ? 0 : 1) , 10000));   
                break;
            case 'e':
            case 'E':
            case 'f':
            case 'g':
            case 'G':
                TestCase(formatSpec,
                         RandomInt(-10000, 10000) / double(RandomInt(1, 100)));
                break;
            case 'n':
                break;
            case 'p':
                {
                    void * p = malloc(RandomInt(1, 1000));
                    TestCase(formatSpec, p);
                    free(p);
                }
                break;
            case 's':
                TestCase(formatSpec, RandomString(100).c_str());
                break;
            default:
                assert(false);
                break;
            }
        }
    }
}

void test_dword()
{
    typedef signed int Int;
    typedef unsigned int UInt;
    typedef signed long Long;
    typedef unsigned long ULong;
	Int i(0);
    UInt ui(0);
    Long l(0);
    ULong ul(0);
	Printf("%d")(i);
    Printf("%d")(ui);
    Printf("%d")(l);
    Printf("%d")(ul);
}


