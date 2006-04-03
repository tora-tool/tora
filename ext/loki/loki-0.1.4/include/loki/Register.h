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
#ifndef LOKI_REGISTER_INC_
#define LOKI_REGISTER_INC_

// $Header: /cvsroot/loki-lib/loki/include/loki/Register.h,v 1.3 2006/03/08 18:33:38 syntheticpp Exp $ 

#include "TypeManip.h"
#include "HierarchyGenerators.h"

///  \defgroup RegisterGroup Register 

namespace Loki
{

    ////////////////////////////////////////////////////////////////////////////////
    //
    //  Helper classes/functions for RegisterByCreateSet
    //
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    ///  \ingroup RegisterGroup
    ///  Must be specialized be the user
    ////////////////////////////////////////////////////////////////////////////////
	template<class t> bool RegisterFunction();

    ////////////////////////////////////////////////////////////////////////////////
    ///  \ingroup RegisterGroup
    ///  Must be specialized be the user
    ////////////////////////////////////////////////////////////////////////////////
	template<class t> bool UnRegisterFunction();

	namespace Private
    {
    	template<class T> 
    	struct RegisterOnCreate
        {
        	RegisterOnCreate()  { RegisterFunction<T>(); }
        };

    	template<class T> 
    	struct UnRegisterOnDelete
        {
            ~UnRegisterOnDelete() { UnRegisterFunction<T>(); }
        };    

    	template<class T>
    	struct RegisterOnCreateElement
        {
        	RegisterOnCreate<T> registerObj;
        };

    	template<class T>
    	struct UnRegisterOnDeleteElement
        {
        	UnRegisterOnDelete<T> unregisterObj;
        };
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///  \class RegisterOnCreateSet
    ///
    ///  \ingroup RegisterGroup
    ///  Implements a generic register class which registers classes of a typelist
    ///
    ///  \par Usage
    ///  see test/Register
    ////////////////////////////////////////////////////////////////////////////////

	template<typename ElementList>
	struct RegisterOnCreateSet 
        : GenScatterHierarchy<ElementList, Private::RegisterOnCreateElement>
    {};

    ////////////////////////////////////////////////////////////////////////////////
    ///  \class UnRegisterOnDeleteSet
    ///
    ///  \ingroup RegisterGroup
    ///  Implements a generic register class which unregisters classes of a typelist
    ///
    ///  \par Usage
    ///  see test/Register
    ////////////////////////////////////////////////////////////////////////////////
	template<typename ElementList>
	struct UnRegisterOnDeleteSet 
        : GenScatterHierarchy<ElementList, Private::UnRegisterOnDeleteElement>
    {};


    ////////////////////////////////////////////////////////////////////////////////
    ///  \def  LOKI_CHECK_CLASS_IN_LIST( CLASS , LIST )
    ///
    ///  \ingroup RegisterGroup
    ///  Check if CLASS is in the typelist LIST.
    ///
    ///  \par Usage
    ///  see test/Register
    ////////////////////////////////////////////////////////////////////////////////

#define LOKI_CHECK_CLASS_IN_LIST( CLASS , LIST )                                \
                                                                                \
	struct Loki_##CLASS##LIST_OK{typedef int class_##CLASS##_is_not_in_##LIST;};\
	typedef Loki::Select<Loki::TL::IndexOf<LIST, CLASS>::value == -1,            \
	CLASS,Loki_##CLASS##LIST_OK >::Result IsInList##CLASS##LIST;                \
	typedef IsInList##CLASS##LIST::class_##CLASS##_is_not_in_##LIST	            \
                                                	isInListTest##CLASS##LIST;

} // namespace Loki


#endif

