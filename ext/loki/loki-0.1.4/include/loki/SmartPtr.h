////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_SMARTPTR_INC_
#define LOKI_SMARTPTR_INC_

// $Header: /cvsroot/loki-lib/loki/include/loki/SmartPtr.h,v 1.24 2006/03/08 17:07:11 syntheticpp Exp $

///  \defgroup  SmartPointerGroup Smart pointers
///  Policy based implementation of a smart pointer
///  \defgroup  SmartPointerOwnershipGroup Ownership policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerStorageGroup Storage policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerConversionGroup Conversion policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerCheckingGroup Checking policies
///  \ingroup   SmartPointerGroup

#include "LokiExport.h"
#include "SmallObj.h"
#include "TypeManip.h"
#include "static_check.h"
#include "RefToValue.h"
#include "ConstPolicy.h"

#include <functional>
#include <stdexcept>
#include <cassert>

namespace Loki
{

////////////////////////////////////////////////////////////////////////////////
///  \class DefaultSPStorage
///
///  \ingroup  SmartPointerStorageGroup 
///  Implementation of the StoragePolicy used by SmartPtr
////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class DefaultSPStorage
    {
    public:
        typedef T* StoredType;    // the type of the pointee_ object
        typedef T* PointerType;   // type returned by operator->
        typedef T& ReferenceType; // type returned by operator*

        DefaultSPStorage() : pointee_(Default()) 
        {}

        // The storage policy doesn't initialize the stored pointer 
        //     which will be initialized by the OwnershipPolicy's Clone fn
        DefaultSPStorage(const DefaultSPStorage&) : pointee_(0)
        {}

        template <class U>
        DefaultSPStorage(const DefaultSPStorage<U>&) : pointee_(0)
        {}
        
        DefaultSPStorage(const StoredType& p) : pointee_(p) {}
        
        PointerType operator->() const { return pointee_; }
        
        ReferenceType operator*() const { return *pointee_; }
        
        void Swap(DefaultSPStorage& rhs)
        { std::swap(pointee_, rhs.pointee_); }
    
        // Accessors
        friend inline PointerType GetImpl(const DefaultSPStorage& sp)
        { return sp.pointee_; }
        
        friend inline const StoredType& GetImplRef(const DefaultSPStorage& sp)
        { return sp.pointee_; }

        friend inline StoredType& GetImplRef(DefaultSPStorage& sp)
        { return sp.pointee_; }

    protected:
        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
        { delete pointee_; }
        
        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }
    
    private:
        // Data
        StoredType pointee_;
    };

    
////////////////////////////////////////////////////////////////////////////////
///  \class ArrayStorage
///
///  \ingroup  SmartPointerStorageGroup 
///  Implementation of the ArrayStorage used by SmartPtr
////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class ArrayStorage
    {
    public:
        typedef T* StoredType;    // the type of the pointee_ object
        typedef T* PointerType;   // type returned by operator->
        typedef T& ReferenceType; // type returned by operator*

        ArrayStorage() : pointee_(Default()) 
        {}

        // The storage policy doesn't initialize the stored pointer 
        //     which will be initialized by the OwnershipPolicy's Clone fn
        ArrayStorage(const ArrayStorage&) : pointee_(0)
        {}

        template <class U>
        ArrayStorage(const ArrayStorage<U>&) : pointee_(0)
        {}
        
        ArrayStorage(const StoredType& p) : pointee_(p) {}
        
        PointerType operator->() const { return pointee_; }
        
        ReferenceType operator*() const { return *pointee_; }
        
        void Swap(ArrayStorage& rhs)
        { std::swap(pointee_, rhs.pointee_); }
    
        // Accessors
        friend inline PointerType GetImpl(const ArrayStorage& sp)
        { return sp.pointee_; }
        
        friend inline const StoredType& GetImplRef(const ArrayStorage& sp)
        { return sp.pointee_; }

        friend inline StoredType& GetImplRef(ArrayStorage& sp)
        { return sp.pointee_; }

    protected:
        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
        { delete [] pointee_; }
        
        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }
    
    private:
        // Data
        StoredType pointee_;
    };


////////////////////////////////////////////////////////////////////////////////
///  \class RefCounted
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Provides a classic external reference counting implementation
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class RefCounted
    {
    public:
        RefCounted() 
            : pCount_(static_cast<uintptr_t*>(
                SmallObject<>::operator new(sizeof(uintptr_t))))
        {
            assert(pCount_!=0);
            *pCount_ = 1;
        }
        
        RefCounted(const RefCounted& rhs) 
        : pCount_(rhs.pCount_)
        {}
        
        // MWCW lacks template friends, hence the following kludge
        template <typename P1>
        RefCounted(const RefCounted<P1>& rhs) 
        : pCount_(reinterpret_cast<const RefCounted&>(rhs).pCount_)
        {}
        
        P Clone(const P& val)
        {
            ++*pCount_;
            return val;
        }
        
        bool Release(const P&)
        {
            if (!--*pCount_)
            {
                SmallObject<>::operator delete(pCount_, sizeof(uintptr_t));
                pCount_ = NULL;
                return true;
            }
            return false;
        }
        
        void Swap(RefCounted& rhs)
        { std::swap(pCount_, rhs.pCount_); }
    
        enum { destructiveCopy = false };

    private:
        // Data
        uintptr_t* pCount_;
    };
    
////////////////////////////////////////////////////////////////////////////////
///  \struct RefCountedMT
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements external reference counting for multithreaded programs
///  Policy Usage: RefCountedMTAdj<ThreadingModel>::RefCountedMT
////////////////////////////////////////////////////////////////////////////////
    
    template <template <class, class> class ThreadingModel,
              class MX = LOKI_DEFAULT_MUTEX >
    struct RefCountedMTAdj
    {
        template <class P>
        class RefCountedMT : public ThreadingModel< RefCountedMT<P>, MX >
        {
            typedef ThreadingModel< RefCountedMT<P>, MX > base_type;
            typedef typename base_type::IntType       CountType;
            typedef volatile CountType               *CountPtrType;

        public:
            RefCountedMT() 
            {
                pCount_ = static_cast<CountPtrType>(
                    SmallObject<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::operator new(
                        sizeof(*pCount_)));
                assert(pCount_);
                //*pCount_ = 1;
                ThreadingModel<RefCountedMT, MX>::AtomicAssign(*pCount_, 1);
            }

            RefCountedMT(const RefCountedMT& rhs) 
            : pCount_(rhs.pCount_)
            {}

            //MWCW lacks template friends, hence the following kludge
            template <typename P1>
            RefCountedMT(const RefCountedMT<P1>& rhs) 
            : pCount_(reinterpret_cast<const RefCountedMT<P>&>(rhs).pCount_)
            {}

            P Clone(const P& val)
            {
                ThreadingModel<RefCountedMT, MX>::AtomicIncrement(*pCount_);
                return val;
            }

            bool Release(const P&)
            {
                if (!ThreadingModel<RefCountedMT, MX>::AtomicDecrement(*pCount_))
                {
                    SmallObject<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::operator delete(
                        const_cast<CountType *>(pCount_), 
                        sizeof(*pCount_));
                    return true;
                }
                return false;
            }

            void Swap(RefCountedMT& rhs)
            { std::swap(pCount_, rhs.pCount_); }

            enum { destructiveCopy = false };

        private:
            // Data
            CountPtrType pCount_;
        };
    };

////////////////////////////////////////////////////////////////////////////////
///  \class COMRefCounted
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Adapts COM intrusive reference counting to OwnershipPolicy-specific syntax
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class COMRefCounted
    {
    public:
        COMRefCounted()
        {}
        
        template <class U>
        COMRefCounted(const COMRefCounted<U>&)
        {}
        
        static P Clone(const P& val)
        {
            if(val!=0)
               val->AddRef();
            return val;
        }
        
        static bool Release(const P& val)
        { 
            if(val!=0) 
                val->Release(); 
            return false; 
        }
        
        enum { destructiveCopy = false };
        
        static void Swap(COMRefCounted&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct DeepCopy
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements deep copy semantics, assumes existence of a Clone() member 
///  function of the pointee type
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct DeepCopy
    {
        DeepCopy()
        {}
        
        template <class P1>
        DeepCopy(const DeepCopy<P1>&)
        {}
        
        static P Clone(const P& val)
        { return val->Clone(); }
        
        static bool Release(const P&)
        { return true; }
        
        static void Swap(DeepCopy&)
        {}
        
        enum { destructiveCopy = false };
    };
    
////////////////////////////////////////////////////////////////////////////////
///  \class RefLinked
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements reference linking
////////////////////////////////////////////////////////////////////////////////

    namespace Private
    {
        class LOKI_EXPORT RefLinkedBase
        {
        public:
            RefLinkedBase() 
            { prev_ = next_ = this; }

            RefLinkedBase(const RefLinkedBase& rhs);

            bool Release();

            void Swap(RefLinkedBase& rhs);

            enum { destructiveCopy = false };

        private:
            mutable const RefLinkedBase* prev_;
            mutable const RefLinkedBase* next_;
        };
    }
    
    template <class P>
    class RefLinked : public Private::RefLinkedBase
    {
    public:
        RefLinked()
        {}
        
        template <class P1>
        RefLinked(const RefLinked<P1>& rhs) 
        : Private::RefLinkedBase(rhs)
        {}

        static P Clone(const P& val)
        { return val; }

        bool Release(const P&)
        { return Private::RefLinkedBase::Release(); }
    };
    
////////////////////////////////////////////////////////////////////////////////
///  \class DestructiveCopy
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements destructive copy semantics (a la std::auto_ptr)
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class DestructiveCopy
    {
    public:
        DestructiveCopy()
        {}
        
        template <class P1>
        DestructiveCopy(const DestructiveCopy<P1>&)
        {}
        
        template <class P1>
        static P Clone(P1& val)
        {
            P result(val);
            val = P1();
            return result;
        }
        
        static bool Release(const P&)
        { return true; }
        
        static void Swap(DestructiveCopy&)
        {}
        
        enum { destructiveCopy = true };
    };
    
////////////////////////////////////////////////////////////////////////////////
///  \class NoCopy
///
///  \ingroup  SmartPointerOwnershipGroup 
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements a policy that doesn't allow copying objects
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class NoCopy
    {
    public:
        NoCopy()
        {}
        
        template <class P1>
        NoCopy(const NoCopy<P1>&)
        {}
        
        static P Clone(const P&)
        {
            // Make it depended on template parameter
            static const bool DependedFalse = sizeof(P*) == 0;

            LOKI_STATIC_CHECK(DependedFalse, This_Policy_Disallows_Value_Copying);
        }
        
        static bool Release(const P&)
        { return true; }
        
        static void Swap(NoCopy&)
        {}
        
        enum { destructiveCopy = false };
    };
    
////////////////////////////////////////////////////////////////////////////////
///  \struct AllowConversion
///  
///  \ingroup  SmartPointerConversionGroup 
///  Implementation of the ConversionPolicy used by SmartPtr
///  Allows implicit conversion from SmartPtr to the pointee type
////////////////////////////////////////////////////////////////////////////////

    struct AllowConversion
    {
        enum { allow = true };

        void Swap(AllowConversion&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct DisallowConversion
///
///  \ingroup  SmartPointerConversionGroup 
///  Implementation of the ConversionPolicy used by SmartPtr
///  Does not allow implicit conversion from SmartPtr to the pointee type
///  You can initialize a DisallowConversion with an AllowConversion
////////////////////////////////////////////////////////////////////////////////

    struct DisallowConversion
    {
        DisallowConversion()
        {}
        
        DisallowConversion(const AllowConversion&)
        {}
        
        enum { allow = false };

        void Swap(DisallowConversion&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct NoCheck
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Well, it's clear what it does :o)
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct NoCheck
    {
        NoCheck()
        {}
        
        template <class P1>
        NoCheck(const NoCheck<P1>&)
        {}
        
        static void OnDefault(const P&)
        {}

        static void OnInit(const P&)
        {}

        static void OnDereference(const P&)
        {}

        static void Swap(NoCheck&)
        {}
    };


////////////////////////////////////////////////////////////////////////////////
///  \struct AssertCheck
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct AssertCheck
    {
        AssertCheck()
        {}
        
        template <class P1>
        AssertCheck(const AssertCheck<P1>&)
        {}
        
        template <class P1>
        AssertCheck(const NoCheck<P1>&)
        {}
        
        static void OnDefault(const P&)
        {}

        static void OnInit(const P&)
        {}

        static void OnDereference(P val)
        { assert(val); (void)val; }

        static void Swap(AssertCheck&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct AssertCheckStrict
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer against zero upon initialization and before dereference
///  You can initialize an AssertCheckStrict with an AssertCheck 
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct AssertCheckStrict
    {
        AssertCheckStrict()
        {}
        
        template <class U>
        AssertCheckStrict(const AssertCheckStrict<U>&)
        {}
        
        template <class U>
        AssertCheckStrict(const AssertCheck<U>&)
        {}
        
        template <class P1>
        AssertCheckStrict(const NoCheck<P1>&)
        {}
        
        static void OnDefault(P val)
        { assert(val); }
        
        static void OnInit(P val)
        { assert(val); }
        
        static void OnDereference(P val)
        { assert(val); }
        
        static void Swap(AssertCheckStrict&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct NullPointerException
///
///  \ingroup SmartPointerGroup 
///  Used by some implementations of the CheckingPolicy used by SmartPtr
////////////////////////////////////////////////////////////////////////////////

    struct NullPointerException : public std::runtime_error
    {
        NullPointerException() : std::runtime_error("")
        { }
        const char* what() const throw()
        { return "Null Pointer Exception"; }
    };
        
////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNullStatic
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer upon initialization and before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNullStatic
    {
        RejectNullStatic()
        {}
        
        template <class P1>
        RejectNullStatic(const RejectNullStatic<P1>&)
        {}
        
        template <class P1>
        RejectNullStatic(const NoCheck<P1>&)
        {}
        
        template <class P1>
        RejectNullStatic(const AssertCheck<P1>&)
        {}
        
        template <class P1>
        RejectNullStatic(const AssertCheckStrict<P1>&)
        {}
        
        static void OnDefault(const P&)
        {
            // Make it depended on template parameter
            static const bool DependedFalse = sizeof(P*) == 0;

            LOKI_STATIC_CHECK(DependedFalse, ERROR_This_Policy_Does_Not_Allow_Default_Initialization);
        }
        
        static void OnInit(const P& val)
        { if (!val) throw NullPointerException(); }
        
        static void OnDereference(const P& val)
        { if (!val) throw NullPointerException(); }
        
        static void Swap(RejectNullStatic&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNull
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNull
    {
        RejectNull()
        {}
        
        template <class P1>
        RejectNull(const RejectNull<P1>&)
        {}
        
        static void OnInit(P)
        {}

        static void OnDefault(P)
        {}
        
        void OnDereference(P val)
        { if (!val) throw NullPointerException(); }
        
        void OnDereference(P val) const
        { if (!val) throw NullPointerException(); }

        void Swap(RejectNull&)
        {}        
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNullStrict
///
///  \ingroup  SmartPointerCheckingGroup 
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer upon initialization and before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNullStrict
    {
        RejectNullStrict()
        {}
        
        template <class P1>
        RejectNullStrict(const RejectNullStrict<P1>&)
        {}
        
        template <class P1>
        RejectNullStrict(const RejectNull<P1>&)
        {}
        
        static void OnInit(P val)
        { if (!val) throw NullPointerException(); }

        void OnDereference(P val)
        { OnInit(val); }

        void OnDereference(P val) const
        { OnInit(val); }

        void Swap(RejectNullStrict&)
        {}        
    };


////////////////////////////////////////////////////////////////////////////////
// class template SmartPtr (declaration)
// The reason for all the fuss above
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy = RefCounted,
        class ConversionPolicy = DisallowConversion,
        template <class> class CheckingPolicy = AssertCheck,
        template <class> class StoragePolicy = DefaultSPStorage,
        template<class> class ConstnessPolicy = LOKI_DEFAULT_CONSTNESS 
     >
     class SmartPtr;

////////////////////////////////////////////////////////////////////////////////
// class template SmartPtrDef (definition)
// this class added to unify the usage of SmartPtr 
// instead of writing SmartPtr<T,OP,CP,KP,SP> write SmartPtrDef<T,OP,CP,KP,SP>::type
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy = RefCounted,
        class ConversionPolicy = DisallowConversion,
        template <class> class CheckingPolicy = AssertCheck,
        template <class> class StoragePolicy = DefaultSPStorage,
    	template<class> class ConstnessPolicy = LOKI_DEFAULT_CONSTNESS 
    >
    struct SmartPtrDef
    {
        typedef SmartPtr
        <
            T,
            OwnershipPolicy,
            ConversionPolicy,
            CheckingPolicy,
            StoragePolicy,
        	ConstnessPolicy
        >
        type;
    };

////////////////////////////////////////////////////////////////////////////////
///  \class SmartPtr
///
///  \ingroup SmartPointerGroup 
///
///  \param OwnershipPolicy  default =  RefCounted,
///  \param ConversionPolicy default = DisallowConversion,
///  \param CheckingPolicy default = AssertCheck,
///  \param StoragePolicy default = DefaultSPStorage
///  \param ConstnessPolicy default = LOKI_DEFAULT_CONSTNESS
///
///  \par IMPORTANT NOTE
///  Due to threading issues, the OwnershipPolicy has been changed as follows:
///
///     - Release() returns a boolean saying if that was the last release
///        so the pointer can be deleted by the StoragePolicy
///     - IsUnique() was removed
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy,
        class ConversionPolicy,
        template <class> class CheckingPolicy,
        template <class> class StoragePolicy,
        template <class> class ConstnessPolicy
    >
    class SmartPtr
        : public StoragePolicy<T>
        , public OwnershipPolicy<typename StoragePolicy<T>::PointerType>
        , public CheckingPolicy<typename StoragePolicy<T>::StoredType>
        , public ConversionPolicy
    {
        typedef StoragePolicy<T> SP;
        typedef OwnershipPolicy<typename StoragePolicy<T>::PointerType> OP;
        typedef CheckingPolicy<typename StoragePolicy<T>::StoredType> KP;
        typedef ConversionPolicy CP;
        
    public:
        typedef typename ConstnessPolicy<T>::Type* ConstPointerType;
        typedef typename ConstnessPolicy<T>::Type& ConstReferenceType;

        typedef typename SP::PointerType PointerType;
        typedef typename SP::StoredType StoredType;
        typedef typename SP::ReferenceType ReferenceType;
        
        typedef typename Select<OP::destructiveCopy,SmartPtr, const SmartPtr>::Result
                CopyArg;
    
    private:
        struct NeverMatched;
       
#ifdef LOKI_SMARTPTR_CONVERSION_CONSTRUCTOR_POLICY
        typedef typename Select< CP::allow, const StoredType&, NeverMatched>::Result ImplicitArg;
        typedef typename Select<!CP::allow, const StoredType&, NeverMatched>::Result ExplicitArg;
#else
        typedef const StoredType& ImplicitArg;
        typedef typename Select<false, const StoredType&, NeverMatched>::Result ExplicitArg;
#endif

    public:

        SmartPtr()
        { KP::OnDefault(GetImpl(*this)); }
        
        explicit
        SmartPtr(ExplicitArg p) : SP(p)
        { KP::OnInit(GetImpl(*this)); }

        SmartPtr(ImplicitArg p) : SP(p)
        { KP::OnInit(GetImpl(*this)); }

        SmartPtr(CopyArg& rhs)
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        { GetImplRef(*this) = OP::Clone(GetImplRef(rhs)); }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        { GetImplRef(*this) = OP::Clone(GetImplRef(rhs)); }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr(SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        { GetImplRef(*this) = OP::Clone(GetImplRef(rhs)); }

        SmartPtr(RefToValue<SmartPtr> rhs)
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        {}
        
        operator RefToValue<SmartPtr>()
        { return RefToValue<SmartPtr>(*this); }

        SmartPtr& operator=(CopyArg& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr& operator=(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }
        
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr& operator=(SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }
        
        void Swap(SmartPtr& rhs)
        {
            OP::Swap(rhs);
            CP::Swap(rhs);
            KP::Swap(rhs);
            SP::Swap(rhs);
        }
        
        ~SmartPtr()
        {
            if (OP::Release(GetImpl(*static_cast<SP*>(this))))
            {
                SP::Destroy();
            }
        }
        
        friend inline void Release(SmartPtr& sp, typename SP::StoredType& p)
        {
            p = GetImplRef(sp);
            GetImplRef(sp) = SP::Default();
        }
        
        friend inline void Reset(SmartPtr& sp, typename SP::StoredType p)
        { SmartPtr(p).Swap(sp); }

        PointerType operator->()
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator->();
        }

        ConstPointerType operator->() const
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator->();
        }

        ReferenceType operator*()
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator*();
        }
        
        ConstReferenceType operator*() const
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator*();
        }
        
        bool operator!() const // Enables "if (!sp) ..."
        { return GetImpl(*this) == 0; }
        

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator==(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return GetImpl(*this) == GetImpl(rhs); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator!=(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return !(*this == rhs); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator<(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return GetImpl(*this) < GetImpl(rhs); }

    private:
        // Helper for enabling 'if (sp)'
        struct Tester
        {
            Tester(int) {}
            void dummy() {}
        };
        
        typedef void (Tester::*unspecified_boolean_type_)();

        typedef typename Select<CP::allow, Tester, unspecified_boolean_type_>::Result
            unspecified_boolean_type;

    public:
        // enable 'if (sp)'
        operator unspecified_boolean_type() const
        {
            return !*this ? 0 : &Tester::dummy;
        }

    private:
        // Helper for disallowing automatic conversion
        struct Insipid
        {
            Insipid(PointerType) {}
        };
        
        typedef typename Select<CP::allow, PointerType, Insipid>::Result
            AutomaticConversionResult;
    
    public:        
        operator AutomaticConversionResult() const
        { return GetImpl(*this); }
    };

////////////////////////////////////////////////////////////////////////////////
// free comparison operators for class template SmartPtr
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///  operator== for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP1,
        typename U
    >
    inline bool operator==(const SmartPtr<T, OP, CP, KP, SP, CNP1 >& lhs,
        U* rhs)
    { return GetImpl(lhs) == rhs; }
    
////////////////////////////////////////////////////////////////////////////////
///  operator== for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP1,
        typename U
    >
    inline bool operator==(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP1 >& rhs)
    { return rhs == lhs; }

////////////////////////////////////////////////////////////////////////////////
///  operator!= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator!=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return !(lhs == rhs); }
    
////////////////////////////////////////////////////////////////////////////////
///  operator!= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator!=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return rhs != lhs; }

////////////////////////////////////////////////////////////////////////////////
///  operator< for lhs = SmartPtr, rhs = raw pointer -- NOT DEFINED
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs);
        
////////////////////////////////////////////////////////////////////////////////
///  operator< for lhs = raw pointer, rhs = SmartPtr -- NOT DEFINED
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs);
        
////////////////////////////////////////////////////////////////////////////////
//  operator> for lhs = SmartPtr, rhs = raw pointer -- NOT DEFINED
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return rhs < lhs; }
        
////////////////////////////////////////////////////////////////////////////////
///  operator> for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return rhs < lhs; }
  
////////////////////////////////////////////////////////////////////////////////
///  operator<= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return !(rhs < lhs); }
        
////////////////////////////////////////////////////////////////////////////////
///  operator<= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return !(rhs < lhs); }

////////////////////////////////////////////////////////////////////////////////
///  operator>= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return !(lhs < rhs); }
        
////////////////////////////////////////////////////////////////////////////////
///  operator>= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return !(lhs < rhs); }

} // namespace Loki

////////////////////////////////////////////////////////////////////////////////
///  specialization of std::less for SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

namespace std
{
    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP
    >
    struct less< Loki::SmartPtr<T, OP, CP, KP, SP, CNP > >
        : public binary_function<Loki::SmartPtr<T, OP, CP, KP, SP, CNP >,
            Loki::SmartPtr<T, OP, CP, KP, SP, CNP >, bool>
    {
        bool operator()(const Loki::SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
            const Loki::SmartPtr<T, OP, CP, KP, SP, CNP >& rhs) const
        { return less<T*>()(GetImpl(lhs), GetImpl(rhs)); }
    };
}

////////////////////////////////////////////////////////////////////////////////
// Change log:
// June 20,     2001: ported by Nick Thurn to gcc 2.95.3. Kudos, Nick!!!
// December 09, 2001: Included <cassert>
// February 2,  2003: fixed dependent names - credit due to Rani Sharoni
// August   21, 2003: Added support for policy based explicit/implicit constructor.
//  The new code will be effective if a macro named 
//  *** LOKI_SMARTPTR_CONVERSION_CONSTRUCTOR_POLICY ***
//  is defined (due to backward computability concerns).
//  In case that the macro is defined and the conversion policy allow flag is off 
//  (e.g. DisallowConversion) then the conversion from the "pointer" to the 
//  SmartPtr must be explicit.
// October 21, 2005: fix void Swap(RefLinkedBase& rhs). Peter Kümmel
// November 3, 2005: doxygen like documentation. Peter Kümmel
////////////////////////////////////////////////////////////////////////////////

#endif // SMARTPTR_INC_

// $Log: SmartPtr.h,v $
// Revision 1.24  2006/03/08 17:07:11  syntheticpp
// replace tabs with 4 spaces in all files
//
// Revision 1.23  2006/02/28 16:55:56  syntheticpp
// undo disabling checking, remove warnings, many thanks to Sam Miller
//
// Revision 1.22  2006/02/28 12:59:59  syntheticpp
// fix wrong RejectNull implementation, thanks to Sam Miller
//
// Revision 1.21  2006/02/27 19:59:20  syntheticpp
// add support of loki.dll
//
// Revision 1.20  2006/02/25 13:48:54  syntheticpp
// add constness policy to doc
//
// Revision 1.19  2006/02/25 13:01:40  syntheticpp
// add const member function OnDereference to non static RejectNull policies
//
// Revision 1.18  2006/02/25 01:52:17  rich_sposato
// Moved a monolithic base class from header file to new source file.
//
// Revision 1.17  2006/02/19 22:04:28  rich_sposato
// Moved Const-policy structs from SmartPtr.h to ConstPolicy.h.
//
// Revision 1.16  2006/02/14 11:54:46  syntheticpp
// rename SmartPtr-ByRef and ScopeGuard-ByRefHolder into RefToValue and move it to loki/RefToValue.h
//
// Revision 1.15  2006/02/08 18:12:29  rich_sposato
// Fixed bug 1425890.  Last SmartPtr in linked chain NULLs its prev & next
// pointers to prevent infinite recursion.  Added asserts.
//
// Revision 1.14  2006/01/30 20:07:38  syntheticpp
// replace tabss
//
// Revision 1.13  2006/01/30 20:01:37  syntheticpp
// add ArrayStorage and propagating constness policies
//
// Revision 1.12  2006/01/27 08:58:17  syntheticpp
// replace unsigned int with the platform independent uintptr_t to make it more 64bit portable, and work around for mac gcc 4.0.0 bug in assert
//
// Revision 1.11  2006/01/22 13:31:12  syntheticpp
// add additional template parameter for the changed threading classes
//
// Revision 1.10  2006/01/18 17:21:31  lfittl
// - Compile library with -Weffc++ and -pedantic (gcc)
// - Fix most issues raised by using -Weffc++ (initialization lists)
//
// Revision 1.9  2006/01/16 19:05:09  rich_sposato
// Added cvs keywords.
//
