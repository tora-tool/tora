// 
// thread safe logger
//
// This code is based on article by John Torjo
// published on:
// http://articles.techrepublic.com.com/5100-10878_11-5072104.html#
//

#ifndef __TS_DECORATOR__
#define __TS_DECORATOR__

#include <iostream>
#include <sstream>
#include <ostream>

/********************************************************************************
 *                                                                              *
 * Small excerpt from LOKI                                                      *
 * Typelist                                                                             *
 ********************************************************************************/
#define TSLOG_TYPELIST_1(a)           Typelist< a, NullType>
#define TSLOG_TYPELIST_2(a, b)        Typelist< a, TSLOG_TYPELIST_1(b)>
#define TSLOG_TYPELIST_3(a, b, c)     Typelist< a, TSLOG_TYPELIST_2(b,c)>
#define TSLOG_TYPELIST_4(a, b, c, d)  Typelist< a, TSLOG_TYPELIST_3(b,c,d)>
#define TSLOG_TYPELIST_5(a, b, c, d, e)           Typelist< a, TSLOG_TYPELIST_4(b,c,d,e)>
#define TSLOG_TYPELIST_6(a, b, c, d, e, f)        Typelist< a, TSLOG_TYPELIST_5(b,c,d,e,f)>
#define TSLOG_TYPELIST_7(a, b, c, d, e, f, g)     Typelist< a, TSLOG_TYPELIST_6(b,c,d,e,f,g)>
#define TSLOG_TYPELIST_8(a, b, c, d, e, f, g, h)  Typelist< a, TSLOG_TYPELIST_7(b,c,d,e,f,g,h)>

class NullType {};

template<class T, class U>
struct Typelist
{
	typedef T Head;
	typedef U Tail;
};

////////////////////////////////////////////////////////////////////////////////
// class template IsSameType
// Return true iff two given types are the same
// Invocation: SameType<T, U>::value
// where:
// T and U are types
// Result evaluates to true iff U == T (types equal)
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename U>
struct IsSameType
{
	enum { value = false };
};

template <typename T>
struct IsSameType<T,T>
{
        enum { value = true };
};

////////////////////////////////////////////////////////////////////////////////
// class template IndexOf
// Finds the index of a type in a typelist
// Invocation (TList is a typelist and T is a type):
// IndexOf<TList, T>::value
// returns the position of T in TList, or NullType if T is not found in TList
////////////////////////////////////////////////////////////////////////////////

template <class TList, class T>
struct IndexOf;

template <class T>
struct IndexOf<NullType, T>
{
	enum { value = -1 };
};
        
template <class T, class Tail>
        struct IndexOf<Typelist<T, Tail>, T>
{
	enum { value = 0 };
};
        
template <class Head, class Tail, class T>
        struct IndexOf<Typelist<Head, Tail>, T>
{
private:
	enum { temp = IndexOf<Tail, T>::value };
public:
	enum { value = (temp == -1 ? -1 : 1 + temp) };
};

template< int i> struct int_to_type
{
	int_to_type()
	{
	}
};

/********************************************************************************
 *                                                                              *
 * Log header decorator inrerface                                               *
 *                                                                              *
 ********************************************************************************/
class decoratorInterface
{
public:
	static inline void decorate(std::ostream &s)
	{};
	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

class null_decorator : public decoratorInterface
{};

class hereDecorator;

template<class X> // Typelist
class Tdecorator: public decoratorInterface
{
public:
	enum { needs_here = IndexOf<X, hereDecorator>::value >=0 };
	static inline void decorate(std::ostream &s)
	{
		typedef typename X::Head Head;
		typedef typename X::Tail Tail;
		Head::decorate(s);
		Tdecorator<Tail>::decorate(s);
	}

	static inline void decorate(std::ostream &s, const std::string & here)
	{
		typedef typename X::Head Head;
		typedef typename X::Tail Tail;
		if( IsSameType<Head, hereDecorator>::value) // do use SuperSubclass instead of IsSameType
		{
			Head::decorate(s, here);
		} else {
			Head::decorate(s);
		}
		if( IndexOf<Tail, hereDecorator>::value >= 0 )
		{
			Tdecorator<Tail>::decorate(s, here);
		} else {
			Tdecorator<Tail>::decorate(s);
		}		
	}
};

template<>
class Tdecorator<NullType>: public decoratorInterface
{
public:
	static inline void decorate(std::ostream &s)
	{};
	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

/********************************************************************************
 *
 * Some usefull decorators
 *
 ********************************************************************************/

template<unsigned cnt>
class dashDecorator : public decoratorInterface
{
public:
	static inline void decorate(std::ostream &s)
	{
#ifndef min
#define min(a,b) ((a < b) ? (a) : (b))
#endif	  
		static char c[] = "--------------------------------------------------------------------------------";
		s << (c + sizeof(c) - 1 - min(sizeof(c)-1, cnt));
#undef min		
	};

	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

template<const char *c>
class wordDecorator : public decoratorInterface
{
	public:
	static inline void decorate(std::ostream &s)
	{
		s << c;
	};

	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

template<const char c>
class charDecorator : public decoratorInterface
{
	public:
	static inline void decorate(std::ostream &s)
	{
		s << c;
	};

	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

// The only decorator that uses second argument - __LINE__
class hereDecorator : public decoratorInterface
{
public:
	static inline void decorate(std::ostream &s)
	{
		s << "Internal error: " << __HERE__;
	}
	static inline void decorate(std::ostream &s, const std::string & here)
	{
		s << here;
	}
};

template<class thread_manager>
class tidDecorator : public decoratorInterface
{
public:
	static inline void decorate(std::ostream &s)
	{
		const std::string &tid = thread_manager::tid();
#ifdef __linux__	
		unsigned int color;
		{
			const unsigned int fnv_prime = 31;
			unsigned int hash      = 0;
			unsigned int i         = 0;
			color = 31;
			
			for(i = 0; i < tid.length(); i++)
			{
				hash *= fnv_prime;
				hash ^= tid.at(i) - '0';
			}
			hash &= 7;
			color += hash;
		}
		/* End Of FNV Hash Function */
#endif		
		
		s 
#ifdef __linux__
			<< "\x1B" "[" << color << ";1m"
#endif
			<< tid
#ifdef __linux__
			<< "\x1B" "[0m" 
#endif
			;
	}
	static inline void decorate(std::ostream &s, const std::string & here)
	{};
};

#endif
