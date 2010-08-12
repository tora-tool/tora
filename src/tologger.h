#ifndef __TORA_LOGGER__
#define __TORA_LOGGER__


#ifdef DEBUG

#define USE_QT_THREAD_MANAGER 
#include "ts_log/critical_section.h"
#include "ts_log/message_handler_log.h"
#include "ts_log/thread_safe_log.h"
#include "ts_log/ts_log_utils.h"
#include "ts_log/decorator.h"
#include <iostream>
#include <QString>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/stream.hpp>
using namespace boost;
using namespace boost::iostreams;

#define TLOG(lognumber, decorator, where) get_log(lognumber).ts<decorator>( where)

typedef Tdecorator<
	TSLOG_TYPELIST_6(dashDecorator<4>,
			 tidDecorator<qt_thread_manager>,
			 charDecorator<' '>,
			 hereDecorator, 
			 dashDecorator<4>,
			 charDecorator<'\n'>
		)> toDecorator;


struct s_null_sink : public boost::iostreams::stream_buffer<boost::iostreams::null_sink>
{
	s_null_sink()
	{
		open(boost::iostreams::null_sink() );
	}
};

template< int idxLog>
inline thread_safe_log templ_get_log_ownthread( int_to_type< idxLog> *i = NULL )
{
	/* static std::ofstream out( get_out_name< idxLog>( false).c_str() ); */
#ifdef DEBUG
	static internal_thread_safe_log_ownthread log( std::cout );
#else	
	static s_null_sink nsink;
	static std::ostream out(&nsink);
	static internal_thread_safe_log_ownthread log( out );
#endif	
	return thread_safe_log( log);
}

// tonoblockquery<3> - debugging(disabled)
template<>
inline thread_safe_log templ_get_log_ownthread(int_to_type<3>*)
{
	static s_null_sink nsink;
	static std::ostream out(&nsink);
	static internal_thread_safe_log_ownthread log( out );
	return thread_safe_log( log);
}

// tonoblockquery<4> - data read(disabled)
template<>
inline thread_safe_log templ_get_log_ownthread(int_to_type<4>*)
{
	static s_null_sink nsink;
	static std::ostream out(&nsink);
	static internal_thread_safe_log_ownthread log( out );
	return thread_safe_log( log);
}

/*
thread_safe_log templ_get_log_ownthread<1>( int_to_type< 1> * = NULL )
{
	static std::ofstream out( get_out_name< idxLog>( false).c_str() );
	static internal_thread_safe_log_ownthread log( out);
	return thread_safe_log( log);
}
*/

inline thread_safe_log get_log( int idxLog)
{
	switch( idxLog)
	{
	case 0: return templ_get_log_ownthread< 0>(); // tooracleconnection log
	case 1: return templ_get_log_ownthread< 1>(); // exception log
	case 2: return templ_get_log_ownthread< 2>(); // qDebug log
	case 3: return templ_get_log_ownthread< 3>(NULL); // tonoblockquery log
	case 4: return templ_get_log_ownthread< 4>(NULL); // data read log
	default: 
		assert( false);
		return templ_get_log_ownthread< 0>();
	}
}

#else // no DEBUGing at all

#include "ts_log/ts_log_utils.h"
#include "ts_log/decorator.h"
#include <QString>

class NullLogger;

template<typename C>
inline NullLogger& operator<<( NullLogger& stream, const C &str);
//template<typename C>
//inline NullLogger& operator<<( NullLogger& stream, const C str);

class NullLogger : public NullType
{
public:
	inline NullLogger&  operator<<(std::ostream& (*f)(std::ostream&))
	{
		return *this;
	}
	template<class C> friend NullLogger& operator<<(NullLogger& stream, const C &str);
//	template<class C> friend NullLogger& operator<<(NullLogger& stream, const C str); 
};

template<typename C>
inline NullLogger& operator<<( NullLogger& stream, const C &str)
{
	return stream;
};

//template<typename C>
//inline NullLogger& operator<<( NullLogger& stream, const C str)
//{
//	return stream;
//};

inline NullLogger& get_null_log()
{
	static NullLogger null;
	return null;
}

#define TLOG(lognumber, decorator, where) get_null_log()
  
#endif


// TODO add some comment on this MSVC
inline std::ostream& operator<<( std::ostream & stream, const QString & str)
{ // TODO review this
	QByteArray b( str.toAscii());
	const char *c = b.constData();
	stream << c;
	return stream;
}


#endif
