////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////
#ifndef LOKI_PIMPL_H
#define LOKI_PIMPL_H

// $Header: /cvsroot/loki-lib/loki/include/loki/Pimpl.h,v 1.19 2006/03/08 17:07:11 syntheticpp Exp $

///  \defgroup PimplGroup Pimpl 

#ifndef LOKI_INHERITED_PIMPL_NAME
#define LOKI_INHERITED_PIMPL_NAME d
#endif

#ifndef LOKI_INHERITED_RIMPL_NAME
#define LOKI_INHERITED_RIMPL_NAME d
#endif

namespace Loki
{

    //////////////////////////////////////////
    ///  \class ConstPropPtr
    ///
    ///  \ingroup PimplGroup
    ///   Simple const propagating smart pointer
    ///   Is the default smart pointer of Pimpl.
    //////////////////////////////////////////

	template<class T>
	struct ConstPropPtr
    {
    	explicit ConstPropPtr(T* p) : ptr_(p) {}
        ~ConstPropPtr() { delete  ptr_; ptr_ = 0; }
    	T* operator->()    { return  ptr_; }
    	T& operator*()    { return *ptr_; }
    	const T* operator->() const	{ return  ptr_; }
    	const T& operator*()  const	{ return *ptr_; }
    
	private:
    	ConstPropPtr();
    	ConstPropPtr(const ConstPropPtr&);
    	ConstPropPtr& operator=(const ConstPropPtr&);
    	T* ptr_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    ///  \class Pimpl
    ///
    ///  \ingroup PimplGroup
    ///
    ///  Implements the Pimpl idiom. It's a wrapper for a smart pointer which
    ///  automatically creates and deletes the implementation object and adds
    ///  const propagation to the smart pointer.
    ///  
    ///  \par Usage
    ///  see test/Pimpl
    ////////////////////////////////////////////////////////////////////////////////

	template
    <    
    	class T, 
    	typename Pointer = ConstPropPtr<T>
    >
    class Pimpl 
    {
    public:

    	typedef T Impl;

    	Pimpl() : ptr_(new T)
        {}

        ~Pimpl()
        {
            // Don't compile with incomplete type
            //
            // If compilation brakes here make sure
            // the compiler does not auto-generate the 
            // destructor of the class hosting the pimpl:
            // - implement the destructor of the class 
            // - don't inline the destructor
        	typedef char T_must_be_defined[sizeof(T) ? 1 : -1 ];
        }


    	T* operator->()
        {
            return ptr_.operator->();
        }

        T& operator*()
        {
            return ptr_.operator*();
        }

        const T* operator->() const
        {
            return ptr_.operator->();
        }

        const T& operator*() const
        {
            return ptr_.operator*();
        }

    	Pointer& wrapped()
        {
        	return ptr_;
        }

    	const Pointer& wrapped() const
        {
        	return ptr_;
        }


    private:
    	Pimpl(const Pimpl&);
        Pimpl& operator=(const Pimpl&);

    	Pointer ptr_;
    };


	template<class T, typename Pointer = ConstPropPtr<T> >
    struct PimplOwner 
    {    
        Pimpl<T,Pointer> LOKI_INHERITED_PIMPL_NAME;
    };


    //////////////////////////////////////////
    /// \class  ImplT
    ///
    ///  \ingroup PimplGroup
    /// Convenience template for the 
    /// implementations which PimplT points to.
    //////////////////////////////////////////

    template<class T>
    struct ImplT;


    //////////////////////////////////////////
    /// \class  PImplT
    ///
    ///  \ingroup PimplGroup
    /// Convenience template which uses ImplT
    /// as implementation structure
    //////////////////////////////////////////


    template<class T, template<class> class Ptr = ConstPropPtr>
    struct PimplT
    {
    	typedef T Impl;

        // declare pimpl
        typedef Pimpl<ImplT<T>, Ptr<ImplT<T> > > Type;

        // inherit pimpl
        typedef PimplOwner<ImplT<T>, Ptr<ImplT<T> > > Owner;
    };


	template<class T, class UsedPimpl = typename PimplT<T>::Type >
    struct RimplT
    {
    	typedef typename UsedPimpl::Impl & Type;

        class Owner
        {
        	UsedPimpl pimpl;

    	public:
        	Owner() : LOKI_INHERITED_RIMPL_NAME(*pimpl)
            {}

        	Type LOKI_INHERITED_RIMPL_NAME;
        };

    };
  
}

#endif

// $Log: Pimpl.h,v $
// Revision 1.19  2006/03/08 17:07:11  syntheticpp
// replace tabs with 4 spaces in all files
//
// Revision 1.18  2006/03/08 16:39:27  syntheticpp
// add documenation
//
// Revision 1.17  2006/03/02 09:55:37  syntheticpp
// don't compile with incomplete types
//
// Revision 1.16  2006/03/01 15:20:19  syntheticpp
// add documenation how to avoid the -deletion of pointer to incomplete type- error
//
// Revision 1.15  2006/01/28 20:12:56  syntheticpp
// replace implementation with a auto-create and propagating-const wrapper for smart pointers which auto delete the holded pointer on destruction
//
// Revision 1.14  2006/01/26 14:28:59  syntheticpp
// remove wrong 'typename'
//
// Revision 1.13  2006/01/23 17:22:49  syntheticpp
// add support of deep constness, only supported by (future) Loki::SmartPtr, not supported by boost::shared_ptr and plain pointer. Maybe deep constness forces a redesign of Pimpl. Is there a way to support deep constness by a rimpl?
//
// Revision 1.12  2006/01/19 18:16:39  syntheticpp
// disable usage with auto_ptr: don't compile with std::auto_ptr
//
// Revision 1.11  2006/01/18 19:03:06  syntheticpp
// make rimpl type really a reference
//
// Revision 1.10  2006/01/18 16:49:06  syntheticpp
// move definition of the destructor to extra file, because msvc tries to use the incomplete type; not inlining does not help; maybe this is a compiler bug.
//
// Revision 1.9  2006/01/17 12:03:36  syntheticpp
// add comment about auto_ptr
//
// Revision 1.8  2006/01/17 11:07:34  syntheticpp
// AutoPtrHolderChecked inherits from AutoPtrHolder
//
// Revision 1.7  2006/01/16 19:48:23  syntheticpp
// add error policy
//
// Revision 1.6  2006/01/16 19:05:09  rich_sposato
// Added cvs keywords.
//
