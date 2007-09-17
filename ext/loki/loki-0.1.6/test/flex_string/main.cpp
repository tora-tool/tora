////////////////////////////////////////////////////////////////////////////////
// flex_string
// Copyright (c) 2001 by Andrei Alexandrescu
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author makes no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: main.cpp 760 2006-10-17 20:36:13Z syntheticpp $


#ifdef _MSC_VER

#pragma warning (disable : 4786)    // Get rid of browser information too long names
#ifdef _DEBUG
#pragma warning (disable : 4786)
#endif

#endif

#include <stdio.h>
#include <list>

#ifndef __MWERKS__
//#define NO_ALLOCATOR_REBIND
//#define NO_ITERATOR_TRAITS
#endif

#include <loki/flex/flex_string.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
using namespace std;  

typedef flex_string<
    char,
    std::char_traits<char>,
    std::allocator<char>,
    AllocatorStringStorage<char, std::allocator<char> >
> my_string;

template class flex_string<
    char,
    std::char_traits<char>,
    std::allocator<char>,
    AllocatorStringStorage<char, std::allocator<char> >
>;

template <class Integral1, class Integral2>
Integral2 random(Integral1 low, Integral2 up)
{
    Integral2 low2(low);
    assert(up >= low2);
    if (low2 == up) return low;
    Integral2 x = Integral2(low2 + (rand() * (up - low2)) / RAND_MAX);
    assert(x >= low2 && x <= up);
    return x;
}

template <class String>
String RandomString(const String* /* model */, unsigned int maxSize)
{
    String result(random(0, maxSize), '\0');
    size_t i = 0;
    for (; i != result.size(); ++i)
    {
        result[i] = random('a', 'z');
    }
    return result;
}

template <class String, class Integral>
void Num2String(String& str, Integral )
{
    str.resize(10, '\0');
//    ultoa((unsigned long)n, &str[0], 10);
    sprintf(&str[0], "%ul", 10);
    str.resize(strlen(str.c_str()));
}

std::list<char> RandomList(unsigned int maxSize)
{
    std::list<char> lst(random(0u, maxSize));
    std::list<char>::iterator i = lst.begin(); 
    for (; i != lst.end(); ++i)
    {
        *i = random('a', 'z');
    }
    return lst;
}

int currentTest = 0;

template <class String>
String Test(String, unsigned int count, bool avoidAliasing)
{
    typedef typename String::size_type size_type;
    const size_type maxString = 1000;

    String test;
    while (count--)
    {
        test = RandomString(&test, maxString);

        static unsigned int functionSelector = 0;
        ++functionSelector;
        currentTest = functionSelector % 90; 
        //std::cout << currentTest <<"\n";
        switch (currentTest)
        {
        case 0:
            // test default constructor 21.3.1
            return String();
            break;
        case 1:
            // test copy constructor 21.3.1
            {
                const size_type pos = random(0, test.size());
                String s(test, pos, random(0, static_cast<typename String::size_type>(test.size() - pos)));
                test = s;
            }
            break;
        case 2:
            // test constructor 21.3.1
            { 
                const size_type
                    pos = random(0, test.size()),
                    n = random(0, test.size() - pos);
                String s(test.c_str() + pos, n);
                test = s;
            }
            break;
        case 3:
            // test constructor 21.3.1
            { 
                const size_type pos = random(0, test.size());
                String s(test.c_str() + pos);
                test = s;
            }
            break;
        case 4:
            // test assignment 21.3.1
            {
                String s(random(0, 1000), '\0');
                size_type i = 0;
                for (; i != s.size(); ++i)
                {
                    s[i] = random('a', 'z');
                }
                test = s;
            }
            break;
        case 5:
            // test assignment 21.3.1
            {
                String s(random(0, 1000), '\0');
                size_type i = 0;
                for (; i != s.size(); ++i)
                {
                    s[i] = random('a', 'z');
                }
                test = s.c_str();
            }
            break;
        case 6:
            // test aliased assignment 21.3.1
            {
                const size_t pos = random(0, test.size());
                if (avoidAliasing)
                {
                    test = String(test.c_str() + pos);
                }
                else
                {
                    test = test.c_str() + pos;
                }
            }
            break;
        case 7:
            // test assignment 21.3.1
            test = random('a', 'z');
            break;
        case 8:
            // exercise iterators 21.3.2
            test.begin();
            test.end();
            test.rbegin();
            test.rend();
            break;
        case 9:
            // exercise capacity 21.3.3
            test.size();
            test.length();
            test.max_size();
            test.capacity();
            break;
        case 10:
            // test resize
            test.resize(random(0, test.size()), random('a', 'z'));
            break;
        case 11:
            // test resize with 1 arg
            test.resize(random(0, test.size()));
            break;
        case 12:
            // test reserve
            test.reserve(random(0, 1000));
            break;
        case 13:
            // test clear
            test.clear(); // skip because std::string doesn't support it
            break;
        case 14:
            // exercise empty
            {
                const char* kEmptyString = "empty";
                const char* kNonEmptyString = "not empty";
                if (test.empty()) test = "empty";
                else test = "not empty";
                // the above assignments don't work yet; use iterator assign
                if (test.empty()) test = String(kEmptyString, kEmptyString + strlen(kEmptyString));
                else test = String(kNonEmptyString, kNonEmptyString + strlen(kNonEmptyString));
            }
            break;
        case 15:
            // exercise element access 21.3.4
            if(!test.empty())
            {
                test[random(0, test.size() - 1)];
                test.at(random(0, test.size() - 1));
            }
            break;
        case 16:
            // 21.3.5 modifiers
            test += RandomString(&test, maxString);
            break;
        case 17:
            // aliasing modifiers
            test += test;
            break;
        case 18:
            // 21.3.5 modifiers
            test += RandomString(&test, maxString).c_str();
            break;
        case 19:
            // aliasing modifiers
            if (avoidAliasing)
            {
                test += String(test.c_str() + random(0, test.size()));
            }
            else
            {
                test += test.c_str() + random(0, test.size());
            }
            break;
        case 20:
            // 21.3.5 modifiers
            test += random('a', 'z');
            break;
        case 21:
            // 21.3.5 modifiers
            test.append(RandomString(&test, maxString));
            break;
        case 22:
            // 21.3.5 modifiers
            {
                String s(RandomString(&test, maxString));
                test.append(s, random(0, s.size()), random(0, maxString));
            }
            break;
        case 23:
            // 21.3.5 modifiers
            {
                String s = RandomString(&test, maxString);
                test.append(s.c_str(), random(0, s.size()));
            }
            break;
        case 24:
            // 21.3.5 modifiers
            test.append(RandomString(&test, maxString).c_str());
            break;
        case 25:
            // 21.3.5 modifiers
            test.append(random(0, maxString), random('a', 'z'));
            break;
        case 26:
            {
                std::list<char> lst(RandomList(maxString));
                test.append(lst.begin(), lst.end());
            }
            break;
        case 27:
            // 21.3.5 modifiers
            // skip push_back, Dinkumware doesn't support it
            test.push_back(random('a', 'z'));
            break;
        case 28:
            // 21.3.5 modifiers
            test.assign(RandomString(&test, maxString));
            break;
        case 29:
            // 21.3.5 modifiers
            {
                String str = RandomString(&test, maxString);
                test.assign(str, random(0, str.size()), random(0, maxString));
            }
            break;
        case 30:
            // 21.3.5 modifiers
            {
                String str = RandomString(&test, maxString);
                test.assign(str.c_str(), random(0, str.size()));
            }
            break;
        case 31:
            // 21.3.5 modifiers
            test.assign(RandomString(&test, maxString).c_str());
            break;
        case 32:
            // 21.3.5 modifiers
            test.assign(random(0, maxString), random('a', 'z'));
            break;
        case 33:
            // 21.3.5 modifiers
            {
                // skip, Dinkumware doesn't support it
                std::list<char> lst(RandomList(maxString));
                test.assign(lst.begin(), lst.end());
            }
            break;
        case 34:
            // 21.3.5 modifiers
            test.insert(random(0, test.size()), RandomString(&test, maxString));
            break;
        case 35:
            // 21.3.5 modifiers
            {
                String str = RandomString(&test, maxString);
                test.insert(random(0, test.size()), 
                    str, random(0, str.size()), 
                    random(0, maxString));
            }
            break;
        case 36:
            // 21.3.5 modifiers
            {
                String str = RandomString(&test, maxString);
                test.insert(random(0, test.size()), 
                    str.c_str(), random(0, str.size()));
            }
            break;
        case 37:
            // 21.3.5 modifiers
            test.insert(random(0, test.size()), 
                RandomString(&test, maxString).c_str());
            break;
        case 38:
            // 21.3.5 modifiers
            test.insert(random(0, test.size()), 
                random(0, maxString), random('a', 'z'));
            break;
        case 39:
            // 21.3.5 modifiers
            test.insert(test.begin() + random(0, test.size()), 
                random('a', 'z'));
            break;
        case 40:
            // 21.3.5 modifiers
            {
                std::list<char> lst(RandomList(maxString));
                test.insert(test.begin() + random(0, test.size()), 
                      lst.begin(), lst.end());
            }
            break;
        case 41: 
            // 21.3.5 modifiers
            test.erase(random(0, test.size()), random(0, maxString));
            break;
        case 42:
            // 21.3.5 modifiers
            if(!test.empty())
                test.erase(test.begin() + random(0, test.size()));
            break;
        case 43:
            // 21.3.5 modifiers
            {
                const typename String::iterator i = test.begin() + random(0, test.size());
                test.erase(i, i + random(0, size_t(test.end() - i)));
            }
            break;
        case 44:
            // 21.3.5 modifiers
            {
                const typename String::size_type pos = random(0, test.size());
                if (avoidAliasing)
                {
                    test.replace(pos, random(0, test.size() - pos), 
                        String(test));
                }
                else
                {
                    test.replace(pos, random(0, test.size() - pos), test);
                }
            }
            break;
        case 45:
            // 21.3.5 modifiers
            {
                const typename String::size_type pos = random(0, test.size());
                test.replace(pos, pos + random(0, test.size() - pos), 
                    RandomString(&test, maxString));
            }
            break;
        case 46:
            // 21.3.5 modifiers
            {
                const size_type 
                    pos1 = random(0, test.size()),
                    pos2 = random(0, test.size());
                if (avoidAliasing)
                {
                    test.replace(pos1, pos1 + random(0, test.size() - pos1), 
                        String(test), 
                        pos2, pos2 + random(0, test.size() - pos2));
                }
                else
                {
                    test.replace(pos1, pos1 + random(0, test.size() - pos1), 
                        test, pos2, pos2 + random(0, test.size() - pos2));
                }
            }
            break;
        case 47:
            // 21.3.5 modifiers
            {
                const size_type pos1 = random(0, test.size());
                String str = RandomString(&test, maxString);
                const size_type pos2 = random(0, str.size());
                test.replace(pos1, pos1 + random(0, test.size() - pos1), 
                    str, pos2, pos2 + random(0, str.size() - pos2));
            }
            break;
        case 48:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                if (avoidAliasing)
                {
                    test.replace(pos, random(0, test.size() - pos), 
                        String(test).c_str(), test.size());
                }
                else
                {
                    test.replace(pos, random(0, test.size() - pos), 
                        test.c_str(), test.size());
                }
            }
            break;
        case 49:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                String str = RandomString(&test, maxString);
                test.replace(pos, pos + random(0, test.size() - pos), 
                    str.c_str(), str.size());
            }
            break;
        case 50:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                String str = RandomString(&test, maxString);
                test.replace(pos, pos + random(0, test.size() - pos), 
                    str.c_str());
            }
            break;
        case 51:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                test.replace(pos, random(0, test.size() - pos), 
                    random(0, maxString), random('a', 'z'));
            }
            break;
        case 52:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                if (avoidAliasing)
                {
                    test.replace(
                        test.begin() + pos, 
                        test.begin() + pos + random(0, test.size() - pos), 
                        String(test));
                }
                else
                {
                    test.replace(
                        test.begin() + pos, 
                        test.begin() + pos + random(0, test.size() - pos), 
                        test);
                }
            }
            break;
        case 53:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                if (avoidAliasing)
                {
                    test.replace(
                        test.begin() + pos, 
                        test.begin() + pos + random(0, test.size() - pos), 
                        String(test).c_str(), 
                        test.size() - random(0, test.size()));
                }
                else
                {
                    test.replace(
                        test.begin() + pos, 
                        test.begin() + pos + random(0, test.size() - pos), 
                        test.c_str(), 
                        test.size() - random(0, test.size()));
                }
            }
            break;
        case 54:
            // 21.3.5 modifiers
            {
                const size_type 
                    pos = random(0, test.size()),
                    n = random(0, test.size() - pos);
                typename String::iterator b = test.begin();
                const String str = RandomString(&test, maxString);
                const typename String::value_type* s = str.c_str();
                test.replace(
                    b + pos, 
                    b + pos + n, 
                    s);
            }
            break;
        case 55:
            // 21.3.5 modifiers
            {
                const size_type pos = random(0, test.size());
                test.replace(
                    test.begin() + pos, 
                    test.begin() + pos + random(0, test.size() - pos), 
                    random(0, maxString), random('a', 'z'));
            }
            break;
        case 56:
            // 21.3.5 modifiers
            {
                std::vector<typename String::value_type> 
                    vec(random(0, maxString));
                test.copy(
                    &vec[0], 
                    vec.size(), 
                    random(0, test.size()));
            }
            break;
        case 57:
            // 21.3.5 modifiers
            RandomString(&test, maxString).swap(test);
            break;
        case 58:
            // 21.3.6 string operations
            // exercise c_str() and data()
            assert(test.c_str() == test.data());
            // exercise get_allocator()
            assert(test.get_allocator() == 
                RandomString(&test, maxString).get_allocator());
            break;
        case 59:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.find(str, random(0, test.size())));
            }
            break;
        case 60:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.find(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 61:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.find(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 62:
            // 21.3.6 string operations
            Num2String(test, test.find(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 63:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.rfind(str, random(0, test.size())));
            }
            break;
        case 64:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.rfind(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 65:
            // 21.3.6 string operations
            {
                String str = test.substr(
                    random(0, test.size()), 
                    random(0, test.size()));
                Num2String(test, test.rfind(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 66:
            // 21.3.6 string operations
            Num2String(test, test.rfind(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 67:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_of(str, 
                    random(0, test.size())));
            }
            break;
        case 68:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_of(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 69:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_of(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 70:
            // 21.3.6 string operations
            Num2String(test, test.find_first_of(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 71:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_of(str, 
                    random(0, test.size())));
            }
            break;
        case 72:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_of(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 73:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_of(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 74:
            // 21.3.6 string operations
            Num2String(test, test.find_last_of(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 75:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_not_of(str, 
                    random(0, test.size())));
            }
            break;
        case 76:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_not_of(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 77:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_first_not_of(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 78:
            // 21.3.6 string operations
            Num2String(test, test.find_first_not_of(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 79:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_not_of(str, 
                    random(0, test.size())));
            }
            break;
        case 80:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_not_of(str.c_str(), 
                    random(0, test.size()),
                    random(0, str.size())));
            }
            break;
        case 81:
            // 21.3.6 string operations
            {
                String str = RandomString(&test, maxString);
                Num2String(test, test.find_last_not_of(str.c_str(), 
                    random(0, test.size())));
            }
            break;
        case 82:
            // 21.3.6 string operations
            Num2String(test, test.find_last_not_of(
                random('a', 'z'), 
                random(0, test.size())));
            break;
        case 83:
            // 21.3.6 string operations
            test = test.substr(random(0, test.size()), random(0, test.size()));
            break;
        case 84:
            {
                int tristate = test.compare(RandomString(&test, maxString));
                if (tristate > 0) tristate = 1;
                else if (tristate < 0) tristate = 2;
                Num2String(test, tristate);
            }
            break;
        case 85:
            {
                int tristate = test.compare(
                    random(0, test.size()), 
                    random(0, test.size()), 
                    RandomString(&test, maxString));
                if (tristate > 0) tristate = 1;
                else if (tristate < 0) tristate = 2;
                Num2String(test, tristate);
            }
            break;
        case 86:
            {
                String str = RandomString(&test, maxString);
                int tristate = test.compare(
                    random(0, test.size()), 
                    random(0, test.size()), 
                    str,
                    random(0, str.size()), 
                    random(0, str.size()));
                if (tristate > 0) tristate = 1;
                else if (tristate < 0) tristate = 2;
                Num2String(test, tristate);
            }
            break;
        case 87:
            {
                int tristate = test.compare(
                    RandomString(&test, maxString).c_str());
                if (tristate > 0) tristate = 1;
                else if (tristate < 0) tristate = 2;
                Num2String(test, tristate);
            }
            break;
        case 88:
            {
                String str = RandomString(&test, maxString);
                int tristate = test.compare(
                    random(0, test.size()), 
                    random(0, test.size()), 
                    str.c_str(),
                    random(0, str.size()));
                if (tristate > 0) tristate = 1;
                else if (tristate < 0) tristate = 2;
                Num2String(test, tristate);
            }
            break;
        case 89:
            test = RandomString(&test, maxString) + 
                RandomString(&test, maxString);
            break;
        case 90:
            test = RandomString(&test, maxString).c_str() + 
                RandomString(&test, maxString);
            break;
        case 91:
            test = typename String::value_type(random('a', 'z')) + 
                RandomString(&test, maxString);
            break;
        case 92:
            test = RandomString(&test, maxString) + 
                RandomString(&test, maxString).c_str();
            break;
        case 93:
            test = RandomString(&test, maxString) + 
                RandomString(&test, maxString).c_str();
            break;
        case 94:
            test = RandomString(&test, maxString) + 
                typename String::value_type(random('a', 'z'));
            break;
        default:
            assert(((functionSelector + 1) % 96) == 0);
            break;
        }
    }
    return test;
}

template<class T>
void checkResults(const std::string& reference, const T& tested)
{
    if( (tested.size() != reference.size())||
        (std::string(tested.data(), tested.size()) != reference) )
    {
        std::cout << "\nTest " << currentTest << " failed: \n";
        std::cout << "reference.size() = " << reference.size() << "\n";
        std::cout << "tested.size()    = " << tested.size()    << "\n";
        std::cout << "reference data   = " <<  reference << "\n";
        std::cout << "tested    data   = " <<  tested    << "\n";
    }

    //assert(tested.size() == reference.size());
    //assert(std::string(tested.data(), tested.size()) == reference);
}

void Compare()
{
    unsigned int count = 0;
    srand(10);
    for (;;)
    {
        cout << ++count << '\r';
        
        unsigned long t = rand(); //time(0);

        srand(t);
        const std::string reference = Test(std::string(), 1, true);

        {
            srand(t);
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                SimpleStringStorage<char, std::allocator<char> >
            > my_string;
            const my_string tested = Test(my_string(), 1, false);
            checkResults(reference, tested);
        }

        {
            srand(t);
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                AllocatorStringStorage<char, std::allocator<char> >
            > my_string;
            const my_string tested = Test(my_string(), 1, false);
            checkResults(reference, tested);
        }

        {
            srand(t);
            typedef flex_string<
                char,
                std::char_traits<char>,
                mallocator<char>,
                AllocatorStringStorage<char, mallocator<char> >
            > my_string;
            const my_string tested = Test(my_string(), 1, false);
            checkResults(reference, tested);
        }

        {
            srand(t);
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                VectorStringStorage<char, std::allocator<char> >
            > my_string;
            const my_string tested = Test(my_string(), 1, false);
            checkResults(reference, tested);
        }
        {
            srand(t);
            typedef VectorStringStorage<char, std::allocator<char> >
                Storage;
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                SmallStringOpt<Storage, 23>
            > my_string;
            static my_string sample;
            const my_string tested(Test(sample, 1, false));
            checkResults(reference, tested);
        }
        {
            srand(t);
            typedef SimpleStringStorage<char, std::allocator<char> >
                Storage;
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                CowStringOpt<Storage>
            > my_string;
            static my_string sample;
            const my_string tested(Test(sample, 1, false));
            checkResults(reference, tested);
        }
        {
            srand(t);
            typedef AllocatorStringStorage<char, std::allocator<char> >
                Storage;
            typedef flex_string<
                char,
                std::char_traits<char>,
                std::allocator<char>,
                CowStringOpt<Storage>
            > my_string;
            static my_string sample;
            const my_string tested(Test(sample, 1, false));
            checkResults(reference, tested);
        }
/*
        {    // SimpleStringStorage with UTF16 Encoding
            srand(t);
            typedef SimpleStringStorage<unicode::UTF16Char>
                Storage;
            typedef flex_string<
                unicode::UTF16Char,
                std::char_traits<unicode::UTF16Char>,
                std::allocator<unicode::UTF16Char>,
                UTF16Encoding<Storage>
            > my_string;
            static my_string sample;
            const my_string tested(Test(sample, 1, false));
            assert(tested.size() == reference.size());
            //assert(std::string(tested.data(), tested.size()) == reference);
       }
       */
    }
}
/*
#include <string>
#include <limits>
#include <cassert>
//#include "flex_string.h"

int main()
{
   typedef wchar_t char_type;
   typedef std::basic_string<char_type> std_string_t;

   typedef flex_string<
    char_type,
    std::char_traits<char_type>,
    std::allocator<char_type>,
    SimpleStringStorage<char_type, std::allocator<char_type> > > flex_string_t;

   flex_string_t s1(L"A flex string");
   std_string_t s2(L"A std string");

   assert(std::numeric_limits<char_type>::is_specialized);
 
    assert(std::numeric_limits<std_string_t::iterator::value_type>::is_specialized); 
    s1.replace<std_string_t::iterator>(s1.begin(), s1.begin(), 
    s2.begin(),s2.end());
    return 0;
}
*/
int main()
{
    srand(unsigned(time(0)));
    Compare();
    return 0;
}
