
#ifndef __TORA_LOGGER__
#define __TORA_LOGGER__

#define USE_QT_THREAD_MANAGER 
#include "ts_log/critical_section.h"
#include "ts_log/message_handler_log.h"
#include "ts_log/thread_safe_log.h"
#include "ts_log/ts_log_utils.h"
#include "ts_log/decorator.h"

#include <iostream>

typedef Tdecorator<
	TSLOG_TYPELIST_6(dashDecorator<4>,
			 tidDecorator<qt_thread_manager>,
			 charDecorator<' '>,
			 hereDecorator, 
			 dashDecorator<4>,
			 charDecorator<'\n'>
		)> toDecorator;

template< int idxLog>
thread_safe_log templ_get_log_ownthread( int_to_type< idxLog> * = NULL )
{
	/* static std::ofstream out( get_out_name< idxLog>( false).c_str() ); */
	static internal_thread_safe_log_ownthread log( std::cout );
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
	default: assert( false);
	}
}

// TODO add some comment on this MSVC
inline std::ostream& operator<<( std::ostream & stream, const QString & str)
{ // TODO review this
	QByteArray b( str.toAscii());
	const char *c = b.constData();
	stream << c;
	return stream;
}

#endif
