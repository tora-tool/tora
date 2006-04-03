////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Richard Sposato
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_CONST_POLICY_INC_
#define LOKI_CONST_POLICY_INC_

// $Header: /cvsroot/loki-lib/loki/include/loki/ConstPolicy.h,v 1.1 2006/02/19 22:04:28 rich_sposato Exp $


namespace Loki
{

////////////////////////////////////////////////////////////////////////////////
/// @note These policy classes are used in LockingPtr and SmartPtr to define
///  how const is propagated from the pointee.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
///  \class DontPropagateConst
///
///  \ingroup ConstGroup
///  Don't propagate constness of pointed or referred object.
////////////////////////////////////////////////////////////////////////////////

    template< class T >
    struct DontPropagateConst
    {
        typedef T Type;
    };

////////////////////////////////////////////////////////////////////////////////
///  \class PropagateConst
///
///  \ingroup ConstGroup
///  Propagate constness of pointed or referred object.
////////////////////////////////////////////////////////////////////////////////

    template< class T >
    struct PropagateConst
    {
        typedef const T Type;
    };

// default will not break existing code
#ifndef LOKI_DEFAULT_CONSTNESS
#define LOKI_DEFAULT_CONSTNESS DontPropagateConst
#endif

} // end namespace Loki

#endif // end file guardian

// $Log: ConstPolicy.h,v $
// Revision 1.1  2006/02/19 22:04:28  rich_sposato
// Moved Const-policy structs from SmartPtr.h to ConstPolicy.h.
//
