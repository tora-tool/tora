#ifndef __TSLOG_CRITICAL_SECTION__
#define __TSLOG_CRITICAL_SECTION__

#include <sstream>
//
// thread safe logger
//
// This code is based on article by John Torjo
// published on:
// http://articles.techrepublic.com.com/5100-10878_11-5072104.html#
//

// comment this line and uncomment the following one,
// in order to use BOOST
//#define USE_WIN32_THREAD_MANAGER
//#define USE_BOOST_THREAD_MANAGER
//#define USE_QT_THREAD_MANAGER

////////////////////////////////////////////////////////////////////
// thread managers

#ifdef USE_WIN32_THREAD_MANAGER
#define NOMINMAX
#include <windows.h>

// the object to be started - on a given thread
struct win32_thread_obj
{
    virtual ~win32_thread_obj() {}
    virtual void operator()() = 0;
    DWORD dwThreadID;
    HANDLE hHandle;
    const char* name;

    void set_name(const char*n)
    {
        name = n;
    }
};


struct win32_thread_manager
{
        typedef win32_thread_obj thread_obj_base;

        static void sleep( int nMillisecs)
        {
            Sleep( nMillisecs);
        }

        static inline long long getTimeOfDay()
        {
        	LARGE_INTEGER now;
        	QueryPerformanceCounter(&now);
        	return now.QuadPart;
        }

        static inline long long timeDeltaHook(long long delta)
        {
            static LARGE_INTEGER s_frequency;
            static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);

            delta *= 1000000;               // times microseconds
            delta /= s_frequency.QuadPart;  // divide by performance ticks
            return delta;
        }

        static void create_thread( win32_thread_obj & obj)
        {
            obj.hHandle = CreateThread( NULL, 0, win32_thread_manager::ThreadProc, &obj, 0, &obj.dwThreadID);
        }
        static bool join_thread( thread_obj_base &obj)
        {
            WaitForSingleObject(obj.hHandle, INFINITE);
            return true;
        }

        static std::string tid(void)
        {
            ::std::stringstream s;
            s << GetCurrentThreadId();
            return s.str();
        }


// critical section for Win32

        class critical_section
        {
                critical_section & operator = ( const critical_section & Not_Implemented);
                critical_section( const critical_section & From);
            public:
                critical_section()
                {
                    InitializeCriticalSection( GetCsPtr() );
                }
                ~critical_section()
                {
                    DeleteCriticalSection( GetCsPtr() );
                }
                void Lock()
                {
                    EnterCriticalSection( GetCsPtr());
                }
                void Unlock()
                {
                    LeaveCriticalSection( GetCsPtr());
                }
                operator LPCRITICAL_SECTION() const
                {
                    return GetCsPtr();
                }
            private:
                LPCRITICAL_SECTION GetCsPtr() const
                {
                    return &m_cs;
                }
            private:
                // the critical section itself
                mutable CRITICAL_SECTION m_cs;
        };


        // automatic locking/unlocking of a resource
        class auto_lock_unlock
        {
                auto_lock_unlock operator=( auto_lock_unlock & Not_Implemented);
                auto_lock_unlock( const auto_lock_unlock & Not_Implemented);
            public:
                auto_lock_unlock( critical_section & cs) : m_cs( cs)
                {
                    m_cs.Lock();
                }
                ~auto_lock_unlock()
                {
                    m_cs.Unlock();
                }
            private:
                critical_section & m_cs;
        };

    private:
        static DWORD WINAPI ThreadProc( LPVOID lpData)
        {
            win32_thread_obj * pThread = ( win32_thread_obj *)lpData;

            // set self thread name
            // See MSDN: How to: Set a Thread Name in Native Code
            const DWORD MS_VC_EXCEPTION=0x406D1388;
#if !defined(QT_NO_DEBUG) && defined(Q_CC_MSVC) && !defined(Q_OS_WINCE)

#if !defined(_WIN64)
#  define ULONG_PTR DWORD
#endif

#pragma pack(push,8)
            typedef struct tagTHREADNAME_INFO
            {
                DWORD dwType; // Must be 0x1000.
                LPCSTR szName; // Pointer to name (in user addr space).
                DWORD dwThreadID; // Thread ID (-1=caller thread).
                DWORD dwFlags; // Reserved for future use, must be zero.
            } THREADNAME_INFO;
#pragma pack(pop)

            {
                THREADNAME_INFO info;
                info.dwType = 0x1000;
                info.szName = pThread->name;      // threadName;
                info.dwThreadID = -1;    // only -1(self) works for me// dwThreadID; // threadId;
                info.dwFlags = 0;

                __try
                {
                    RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
                }
                __except (EXCEPTION_CONTINUE_EXECUTION)
                {
                }
            }
#endif // !QT_NO_DEBUG && Q_CC_MSVC && !Q_OS_WINCE

            ( *pThread)();
            return 0;
        }
};

#endif // ifdef USE_WIN32_THREAD_MANAGER


#ifdef USE_BOOST_THREAD_MANAGER

#include "boost/thread/mutex.hpp"
#include "boost/thread/thread.hpp"
#include "boost/function.hpp"
#include "boost/thread/xtime.hpp"

class boost_thread_manager
{
    public:
        struct thread_obj_base
        {
            virtual void operator()() = 0;
            ~thread_obj_base()
            {
                if (_me)
                {
                    delete _me;
                    _me = NULL;
                }
            }
            void set_name(const char*)
            {
                // TODO get get thread native id and then use platform dependent way
                // boost does not support this
            }
            boost::thread *_me;
        };

	static inline long long getTimeOfDay()
	{
	    long            ns; // nanoseconds
	    time_t          s;  // Seconds
	    struct timespec monotime;
	    clock_gettime(CLOCK_MONOTONIC, &monotime);
	    s  = spec.tv_sec;
	    return = s * 1.0e9 + ns;
	}

        static inline long long timeDeltaHook(long long delta)
        {
            return delta / 1.0e3; // get microseconds from nanoseconds
        }
	
    private:
        struct function_wrapper
        {
                function_wrapper( thread_obj_base & base) : m_base( base) {}
                void operator()() const
                {
                    m_base();
                }
            private:
                mutable thread_obj_base & m_base;
        };
    public:
        static void sleep( int nMillisecs)
        {
            boost::xtime xt;
            boost::xtime_get(&xt, boost::TIME_UTC);
            // Sleep for n Millisecs
            xt.nsec += 1000000 * nMillisecs + 100000 /* just in case*/;
            boost::thread::sleep( xt);
        }

        static void create_thread( thread_obj_base & obj)
        {
            boost::function0< void> f = function_wrapper( obj);
            // creates the thread
            // boost::thread t( f);
            obj._me = new boost::thread ( f);
        }

        static bool join_thread( thread_obj_base & obj)
        {
            obj._me->join();
            delete obj._me;
            obj._me = NULL;
            return true;
        }

        static std::string tid(void)
        {
            ::std::stringstream s;
            s << ::boost::this_thread::get_id();
            return s.str();
        }

        typedef boost::mutex critical_section;
        typedef boost::mutex::scoped_lock auto_lock_unlock;
};

#endif // #ifdef USE_BOOST_THREAD_MANAGER


#ifdef USE_QT_THREAD_MANAGER

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QDateTime>
#if defined(__linux__)
#include <sys/prctl.h>
#endif

class qt_thread_manager
{
    public:
        struct thread_obj_base : public QThread
        {
                virtual void operator()() = 0;

                void set_name(const char* n)
                {
                    QThread::setObjectName(QString::fromLatin1(n));
                }

            private:
                virtual void run()
                {
                    // This was copied from QT trunk - as of 4.8.2 it is not used yet
                    QByteArray objectName = QThread::objectName().toLatin1();
                    if (objectName.isEmpty())
                        objectName = QThread::metaObject()->className();
#if defined(__linux__)
                    prctl(PR_SET_NAME, (unsigned long)objectName.constData(), 0, 0, 0);
#elif defined(Q_OS_MAC)
                    pthread_setname_np(objectName.constData());
#endif
                    (*this)();
                }
                bool join()
                {
                    while (isRunning())
                    {
                        if (!QThread::wait(50))
                            QThread::yieldCurrentThread();
                    }
                    return isFinished();
                }
                friend class qt_thread_manager;
        };

    public:
        static void sleep( int nMillisecs)
        {
            thread_obj_base::msleep( nMillisecs);
        }

	static inline long long getTimeOfDay()
        {
	    // TODO use QElapsedTimer
	    return QDateTime::currentMSecsSinceEpoch() * 1.0e3; // get microseconds from miliseconds
        }

        static inline long long timeDeltaHook(long long delta)
        {
	    return delta;
        }

        static void create_thread( thread_obj_base & obj)
        {
            obj.start();
        }

        static bool join_thread( thread_obj_base & obj)
        {
            return obj.join();
        }

        static std::string tid(void)
        {
// Visual Studio 2013
#if (defined _MSC_VER) && (_MSC_VER <= 1800)
	    __declspec(thread) static ::std::string stid;
#else
	    thread_local static std::string stid;
#endif
	    if (stid.empty())
	    {
	      //char buffer[24];
	      //int len = snprintf(buffer, sizeof(buffer), "%#lx", pthread_self());
	      ::std::stringstream s;
	      //s << QThread::currentThreadId();
	      s << qPrintable(QThread::currentThread()->objectName());
	      stid = s.str();
	    }
            return stid;
        }

        class critical_section : public QMutex
        {
                critical_section(const critical_section &);
            public:
                critical_section(void): QMutex(QMutex::NonRecursive) {};
        };

        // automatic locking/unlocking of a resource
        class auto_lock_unlock
        {
                auto_lock_unlock operator=( auto_lock_unlock & Not_Implemented);
                auto_lock_unlock( const auto_lock_unlock & Not_Implemented);
            public:
                auto_lock_unlock( critical_section & cs) : m_cs( cs)
                {
                    m_cs.lock();
                }
                ~auto_lock_unlock()
                {
                    m_cs.unlock();
                }
            private:
                critical_section & m_cs;
        };
};

#endif // #ifdef USE_QT_THREAD_MANAGER


// if you want a custom default manager,
// create your custom thread_manager class, like
// the ones shown in win32_thread_manager or
// boost_thread_manager, and
// #define DEFAULT_THREAD_MANAGER <your_custom_class>
//

#ifdef OCI_THREAD_MANAGER
#endif // OCI_THREAD_MANAGER

#if defined( DEFAULT_THREAD_MANAGER)
// custom thread manager
#elif defined( USE_WIN32_THREAD_MANAGER)
#define DEFAULT_THREAD_MANAGER win32_thread_manager
#elif defined( USE_BOOST_THREAD_MANAGER)
#define DEFAULT_THREAD_MANAGER boost_thread_manager
#elif defined( USE_QT_THREAD_MANAGER)
#define DEFAULT_THREAD_MANAGER qt_thread_manager
#else
#error "No thread manager. #define either of USE_WIN32_THREAD_MANAGER, USE_BOOST_THREAD_MANAGER, QT_THREAD_MANAGER"
#endif

// END OF thread managers
////////////////////////////////////////////////////////////////////

#endif
