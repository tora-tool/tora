////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code is from the article:
//     "Generic<Programming>: volatile — Multithreaded Programmer’s Best Friend
//     Volatile-Correctness or How to Have Your Compiler Detect Race Conditions
//     for You" by Alexandrescu, Andrei.
//     Published in the February 2001 issue of the C/C++ Users Journal.
//     http://www.cuj.com/documents/s=7998/cujcexp1902alexandr/
// Prepared for Loki library by Richard Sposato
////////////////////////////////////////////////////////////////////////////////

// $Header: /cvsroot/loki-lib/loki/include/loki/LockingPtr.h,v 1.11 2006/03/08 18:22:42 syntheticpp Exp $


#ifndef LOKI_LOCKING_PTR_INC_
#define LOKI_LOCKING_PTR_INC_

#include <loki/ConstPolicy.h>

namespace Loki
{
    /** @class LockingPtr
     Locks a volatile object and casts away volatility so that the object
     can be safely used in a single-threaded region of code.
     Original version of LockingPtr had only one template - for the shared
     object, but not the mutex type.  This version allows users to specify a
     the mutex type as a LockingPolicy class.  The only requirements for a
     LockingPolicy class are to provide Lock and Unlock methods.
     */
    template < typename SharedObject, typename LockingPolicy = LOKI_DEFAULT_MUTEX, 
               template<class> class ConstPolicy = LOKI_DEFAULT_CONSTNESS >
    class LockingPtr
    {
    public:

        typedef typename ConstPolicy<SharedObject>::Type ConstOrNotType;

        /** Constructor locks mutex associated with an object.
         @param object Reference to object.
         @param mutex Mutex used to control thread access to object.
         */
        LockingPtr( volatile ConstOrNotType & object, LockingPolicy & mutex )
           : pObject_( const_cast< SharedObject * >( &object ) ),
            pMutex_( &mutex )
        {
            mutex.Lock();
        }

        /// Destructor unlocks the mutex.
        ~LockingPtr()
        {
            pMutex_->Unlock();
        }

        /// Star-operator dereferences pointer.
        ConstOrNotType & operator * ()
        {
            return *pObject_;
        }

        /// Point-operator returns pointer to object.
        ConstOrNotType * operator -> ()
        {
            return pObject_;
        }

    private:

        /// Default constructor is not implemented.
        LockingPtr();

        /// Copy-constructor is not implemented.
        LockingPtr( const LockingPtr & );

        /// Copy-assignment-operator is not implemented.
        LockingPtr & operator = ( const LockingPtr & );

        /// Pointer to the shared object.
        ConstOrNotType * pObject_;

        /// Pointer to the mutex.
        LockingPolicy * pMutex_;

    }; // end class LockingPtr

} // namespace Loki

#endif  // end file guardian



// $Log: LockingPtr.h,v $
// Revision 1.11  2006/03/08 18:22:42  syntheticpp
// doxygen fixes
//
// Revision 1.10  2006/02/20 21:45:40  rich_sposato
// Removed struct Locking - no longer needed.
//
// Revision 1.9  2006/02/19 22:04:28  rich_sposato
// Moved Const-policy structs from SmartPtr.h to ConstPolicy.h.
//
// Revision 1.8  2006/01/30 20:33:01  syntheticpp
// use policies from SmartPtr.h, clean up
//
// Revision 1.7  2006/01/22 13:37:33  syntheticpp
// use macro LOKI_DEFAULT_MUTEX for Mutex default value, defined in Threads.h
//
// Revision 1.6  2006/01/21 14:09:09  syntheticpp
// replace LockPtr/ConstLockPtr implementation with a template policy based one
//
// Revision 1.5  2006/01/21 01:02:12  rich_sposato
// Added Mutex class to Loki.  Made it the default policy class for locking.
//
// Revision 1.4  2006/01/19 19:34:19  rich_sposato
// Added ConstLockingPtr class.
//
// Revision 1.3  2006/01/16 18:34:37  rich_sposato
// Changed return type from LockingPtr to SharedObject.
//
// Revision 1.2  2006/01/14 00:20:10  syntheticpp
// remove c&p error
//
// Revision 1.1  2005/11/19 22:00:23  rich_sposato
// Adding LockingPtr class to Loki project.
//
