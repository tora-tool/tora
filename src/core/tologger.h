#ifndef __TORA_LOGGER__
#define __TORA_LOGGER__


#if defined(DEBUG) || defined(TORA_EXPERIMENTAL)

#ifdef WIN32
#define USE_WIN32_THREAD_MANAGER
#else
#define USE_QT_THREAD_MANAGER
#endif

#include "ts_log/critical_section.h"
#include "ts_log/message_handler_log.h"
#include "ts_log/thread_safe_log.h"
#include "ts_log/ts_log_utils.h"
#include "ts_log/decorator.h"
#include "ts_log/toostream.h"

#include <iostream>
#include <QtCore/QString>

#ifdef __clang__
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#ifdef __clang__
#pragma GCC diagnostic pop
#endif

using namespace boost;
using namespace boost::iostreams;

////////////////////////////////////////////////////////////////////////////////
// EXAMPLE USAGE
//  - each log channel is identified by constant number (1-9)
//  - decorator is a template class which prints log messages header(see beloow)
//  - for where use preprocessor macro __HERE__
//
// Usage:
// TLOG(1, toDecorator, __HERE__) << "The value for a is:" << a << std::endl;
// TLOG(5, toNoDecorator, __HERE__) << "The value for a is:" << a << std::endl;
////////////////////////////////////////////////////////////////////////////////
#define TLOG(lognumber, decorator, where) get_log(lognumber).ts<decorator>(where)

#define DISABLE_LOG(lognumber)                                                 \
    template<>                                                                 \
    inline thread_safe_log templ_get_log_ownthread(int_to_type<lognumber>*)    \
    {                                                                          \
        static s_null_sink nsink;                                              \
        static std::ostream out(&nsink);                                       \
        static internal_thread_safe_log_ownthread log(out);                    \
        return thread_safe_log(log);                                           \
    };
/** Wrapper class for Boost's null_sink ostream
*/
struct s_null_sink : public boost::iostreams::stream_buffer<boost::iostreams::null_sink>
{
    s_null_sink()
    {
        open(boost::iostreams::null_sink() );
    }
};

#define DOCKLET_LOG(lognumber)                                                \
    template<>                                                                \
    inline thread_safe_log templ_get_log_ownthread(int_to_type<lognumber>*)   \
    {                                                                         \
        static toOStream out;                                                 \
        static internal_thread_safe_log_ownthread log(out);                   \
        return thread_safe_log(log);                                          \
    };


////////////////////////////////////////////////////////////////////////////////
// DECORATORS toDecorator, toNoDecorator
////////////////////////////////////////////////////////////////////////////////
// toDecorator header:
// "----<tid> file.cpp:5 ----\n"
typedef Tdecorator <
TSLOG_TYPELIST_8(dashDecorator<4>,
                 timeTotalDecorator<DEFAULT_THREAD_MANAGER>,
                 charDecorator < ':' > ,
                 tidDecorator<DEFAULT_THREAD_MANAGER>,
                 charDecorator< ' ' > ,
                 hereDecorator,
                 charDecorator< ' ' >,
                 charDecorator< '\n' >
                ) > toDecorator;

typedef tidDecorator<DEFAULT_THREAD_MANAGER,0> tidDecoratorNC;
typedef Tdecorator <
TSLOG_TYPELIST_6(dashDecorator<4>,
                 tidDecoratorNC,
                 charDecorator < ' ' > ,
                 hereDecorator,
                 dashDecorator<4>,
                 charDecorator < '\n' >
                ) > toDecoratorNC;

// toNoDecorator header:
// " "
typedef Tdecorator <
TSLOG_TYPELIST_1(noDecorator
                ) > toNoDecorator;

// timed decorator, display delta time since last time TLOG was called (should be thread afinite)
typedef Tdecorator <
TSLOG_TYPELIST_2(timeStartDecorator<DEFAULT_THREAD_MANAGER>,
                 //dashDecorator<4>,
				 //hereDecorator,
				 charDecorator < ' ' >
                ) > toTimeStart;

typedef Tdecorator <
TSLOG_TYPELIST_2(timeDeltaDecorator<DEFAULT_THREAD_MANAGER>,
                 //dashDecorator<4>,
				 //hereDecorator,
				 charDecorator < ' ' >
                ) > toTimeDelta;

typedef Tdecorator <
TSLOG_TYPELIST_2(timeTotalDecorator<DEFAULT_THREAD_MANAGER>,
                 //dashDecorator<4>,
				 //hereDecorator,
				 charDecorator < ' ' >
                ) > toTimeTotal;

////////////////////////////////////////////////////////////////////////////////
// default templatetized log channel, by default it prints onto STDOUT
//
// macros DISABLE_LOG, DOCKLET_LOG generate templates specialization
// these classes use other outputs
////////////////////////////////////////////////////////////////////////////////
template< int idxLog>
inline thread_safe_log templ_get_log_ownthread( int_to_type< idxLog> *i = NULL )
{
    static internal_thread_safe_log_ownthread log( std::cout );
    /* TODO it can crash here
       The main thread(1) already exited the funcion main. Runs __run_exit_handlers and calls ~thread_safe_log_writer_ownthread.
       It is waiting till the writter thread stops, m_bShouldBeDestructed == true.
       The static instance "log" is already destructed. vptr => onto base class basic_internal_thread_safe_log_base.
       Some other thread still uses logging and enters this line.
       basic_thread_safe_log( internal_type & tsLog)
       : m_tsLog( tsLog)
       {
       // get underlying stream state
       =>              tsLog.copy_state_to( ts() ); // pure virtual call - SEGFAULT
       useColor = is_color_terminal(tsLog);
       }
     */
    return thread_safe_log(log);
}

DISABLE_LOG(0)   // generic debug <0>
DISABLE_LOG(1)   // exceptions debug<1>
DISABLE_LOG(2)
DISABLE_LOG(3)   // debugger(disabled)
DOCKLET_LOG(4)   // data read(disabled)
DISABLE_LOG(5)   // connection provider finder
DISABLE_LOG(6)   //
DOCKLET_LOG(7)   // toEventQuery
DISABLE_LOG(8)   // syntax analyzer, QScintilla
DISABLE_LOG(9)   // report focus

////////////////////////////////////////////////////////////////////////////////
// EXAMPLE USAGE
////////////////////////////////////////////////////////////////////////////////
//
//    /* use this if you want output into the file*/
//    thread_safe_log templ_get_log_ownthread<1>( int_to_type< 1> * = NULL )
//    {
//          static std::ofstream out( get_out_name< idxLog>( false).c_str() );
//          static internal_thread_safe_log_ownthread log( out);
//          return thread_safe_log( log);
//    }
//
////////////////////////////////////////////////////////////////////////////////

inline thread_safe_log get_log(int idxLog)
{
    switch (idxLog)
    {
        case 0:
            return templ_get_log_ownthread<0>(NULL); // tooracleconnection log
        case 1:
            return templ_get_log_ownthread<1>(NULL); // exception log
        case 2:
            return templ_get_log_ownthread<2>(NULL); // qDebug log
        case 3:
            return templ_get_log_ownthread<3>(NULL); // tonoblockquery log
        case 4:
            return templ_get_log_ownthread<4>(NULL); // data read log
        case 5:
            return templ_get_log_ownthread<5>(NULL); // logger docket log
        case 6:
            return templ_get_log_ownthread<6>(NULL); // not used yet
        case 7:
            return templ_get_log_ownthread<7>(NULL); // toEventQuery
        case 8:
            return templ_get_log_ownthread<8>(NULL); // browser, other tools
        case 9:
            return templ_get_log_ownthread<9>(NULL); // focus switching
        default:
            assert(false);
            return templ_get_log_ownthread<0>(NULL);
    }
}

#else // no DEBUGing at all

#include "ts_log/ts_log_utils.h"
#include "ts_log/decorator.h"
#include <QtCore/QString>

#define TLOG(lognumber, decorator, where) get_null_log()

class NullLogger;

template<typename C>
inline NullLogger& operator<<(NullLogger& stream, const C &str);

class NullLogger : public NullType
{
    public:
        inline NullLogger&  operator<<(std::ostream & (*f)(std::ostream&))
        {
            return *this;
        }
        template<class C> friend NullLogger& operator<<(NullLogger& stream, const C &str);
};

template<typename C>
inline NullLogger& operator<<(NullLogger& stream, const C &str)
{
    return stream;
};

inline NullLogger& get_null_log()
{
    static NullLogger null;
    return null;
}

#endif // DEBUGing

inline std::ostream& operator<<(std::ostream & stream, const QString & str)
{
    stream << qPrintable(str);
    return stream;
}


#endif
