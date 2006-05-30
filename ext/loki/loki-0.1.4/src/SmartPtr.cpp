////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// Copyright (c) 2006 Richard Sposato
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above  copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Header: /cvsroot/loki-lib/loki/src/SmartPtr.cpp,v 1.3 2006/03/01 02:08:10 rich_sposato Exp $

#include <loki/SmartPtr.h>

#include <cassert>


// ----------------------------------------------------------------------------

namespace Loki
{

namespace Private
{

// ----------------------------------------------------------------------------

RefLinkedBase::RefLinkedBase(const RefLinkedBase& rhs) 
{
    prev_ = &rhs;
    next_ = rhs.next_;
    prev_->next_ = this;
    next_->prev_ = this;
}

// ----------------------------------------------------------------------------

bool RefLinkedBase::Release()
{
    if ( NULL == next_ )
    {
        assert( NULL == prev_ );
        // Return false so it does not try to destroy shared object
        // more than once.
        return false;
    }
    else if (next_ == this)
    {   
        assert(prev_ == this);
        // Set these to NULL to prevent re-entrancy.
        prev_ = NULL;
        next_ = NULL;
        return true;
    }
    assert( this != prev_ );
    assert( NULL != prev_ );
    prev_->next_ = next_;
    next_->prev_ = prev_;
    return false;
}

// ----------------------------------------------------------------------------

void RefLinkedBase::Swap(RefLinkedBase& rhs)
{
    if (next_ == this)
    {
        assert(prev_ == this);
        if (rhs.next_ == &rhs)
        {
            assert(rhs.prev_ == &rhs);
            // both lists are empty, nothing 2 do
            return;
        }
        prev_ = rhs.prev_;
        next_ = rhs.next_;
        prev_->next_ = next_->prev_ = this;
        rhs.next_ = rhs.prev_ = &rhs;
        return;
    }
    if (rhs.next_ == &rhs)
    {
        rhs.Swap(*this);
        return;
    }
    if (next_ == &rhs ) // rhs is next neighbour
    {
        if ( prev_ == &rhs )
            return;  // cycle of 2 pointers - no need to swap.
        std::swap(prev_, next_);
        std::swap(rhs.prev_, rhs.next_);
        std::swap(rhs.prev_, next_);
        std::swap(rhs.prev_->next_,next_->prev_);
    }
    else if ( prev_ == &rhs ) // rhs is prev neighbor
    {
        if ( next_ == &rhs )
            return;  // cycle of 2 pointers - no need to swap.
        std::swap( prev_, next_ );
        std::swap( rhs.next_, rhs.prev_ );
        std::swap( rhs.next_, prev_ );
        std::swap( rhs.next_->prev_, prev_->next_ );
    }
    else // not neighhbors
    {
        std::swap(prev_, rhs.prev_);
        std::swap(next_, rhs.next_);
        std::swap(prev_->next_, rhs.prev_->next_);
        std::swap(next_->prev_, rhs.next_->prev_);
    }

    assert( next_ == this ? prev_ == this : prev_ != this);
}

// ----------------------------------------------------------------------------

} // end namespace Private

} // end namespace Loki

// ----------------------------------------------------------------------------

// $Log: SmartPtr.cpp,v $
// Revision 1.3  2006/03/01 02:08:10  rich_sposato
// Fixed bug 1440694 by adding check if rhs is previous neighbor.
//
// Revision 1.2  2006/02/25 13:07:15  syntheticpp
// gcc does not like ; when closing a namespace
//
// Revision 1.1  2006/02/25 01:52:17  rich_sposato
// Moved a monolithic base class from header file to new source file.
//
