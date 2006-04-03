////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2005 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_ORDEREDSTATIC_H_
#define LOKI_ORDEREDSTATIC_H_

// $Header: /cvsroot/loki-lib/loki/include/loki/OrderedStatic.h,v 1.10 2006/02/27 19:59:20 syntheticpp Exp $


#include <vector>
#include <iostream>

#include "LokiExport.h"
#include "Singleton.h"
#include "Typelist.h"
#include "Sequence.h"

// usage: see test/OrderedStatic

namespace Loki
{
    namespace Private
    {
        ////////////////////////////////////////////////////////////////////////////////
        // polymorph base class for OrderedStatic template,
        // necessary because of the creator
        ////////////////////////////////////////////////////////////////////////////////
        class LOKI_EXPORT OrderedStaticCreatorFunc
        {
        public:
            virtual void createObject() = 0;
        
        protected:
            OrderedStaticCreatorFunc();
            virtual ~OrderedStaticCreatorFunc();
        
        private:
            OrderedStaticCreatorFunc(const OrderedStaticCreatorFunc&);
        };

        ////////////////////////////////////////////////////////////////////////////////
        // template base clase for OrderedStatic template, 
        // common for all specializations
        ////////////////////////////////////////////////////////////////////////////////
        template<class T>
        class OrderedStaticBase : public OrderedStaticCreatorFunc
        {
        public:
            T& operator*()
            {
                return *val_;
            }

            T* operator->()
            {
                return val_;
            }

        protected:

            OrderedStaticBase(unsigned int longevity) :  val_(0), longevity_(longevity)
            {
            }
            
            virtual ~OrderedStaticBase()
            {
            }
            
            void SetLongevity(T* ptr)
            {
                val_=ptr;
                Loki::SetLongevity(val_,longevity_);
            }

        private:
            OrderedStaticBase();
            OrderedStaticBase(const OrderedStaticBase&);
            OrderedStaticBase& operator=(const OrderedStaticBase&);
            T* val_;
            unsigned int longevity_;
            
        };

        ////////////////////////////////////////////////////////////////////////////////
        // OrderedStaticManagerClass implements details 
        // OrderedStaticManager is then defined as a Singleton
        ////////////////////////////////////////////////////////////////////////////////
        class LOKI_EXPORT OrderedStaticManagerClass
        {
        public:
            OrderedStaticManagerClass();
            virtual ~OrderedStaticManagerClass();

            typedef void (OrderedStaticCreatorFunc::*Creator)();

            void createObjects();
            void registerObject(unsigned int longevity,OrderedStaticCreatorFunc*,Creator);

        private:
            OrderedStaticManagerClass(const OrderedStaticManagerClass&);
            OrderedStaticManagerClass& operator=(const OrderedStaticManagerClass&);
            
            struct Data
            {
                Data(unsigned int,OrderedStaticCreatorFunc*, Creator);
                unsigned int longevity;
                OrderedStaticCreatorFunc* object;
                Creator creator;
            };

            std::vector<Data> staticObjects_;
            unsigned int max_longevity_;
            unsigned int min_longevity_;
        };

    }// namespace Private

    ////////////////////////////////////////////////////////////////////////////////
    // OrderedStaticManager is only a Singleton typedef
    ////////////////////////////////////////////////////////////////////////////////

    typedef Loki::SingletonHolder
    <
        Loki::Private::OrderedStaticManagerClass, 
        Loki::CreateUsingNew,
        Loki::NoDestroy,
        Loki::SingleThreaded
    >
    OrderedStaticManager;

    ////////////////////////////////////////////////////////////////////////////////
    // template OrderedStatic template: 
    // L        : longevity
    // T        : object type
    // TList    : creator parameters
    ////////////////////////////////////////////////////////////////////////////////

    template<unsigned int L, class T, class TList = Loki::NullType>
    class OrderedStatic;


    ////////////////////////////////////////////////////////////////////////////////
    // OrderedStatic specializations
    ////////////////////////////////////////////////////////////////////////////////

    template<unsigned int L, class T>
    class OrderedStatic<L, T, Loki::NullType> : public Private::OrderedStaticBase<T>
    {
    public:    
        OrderedStatic() : Private::OrderedStaticBase<T>(L)
        {
            OrderedStaticManager::Instance().registerObject
                                (L,this,&Private::OrderedStaticCreatorFunc::createObject);
        }

        void createObject()
        {
            Private::OrderedStaticBase<T>::SetLongevity(new T);
        }

    private:
        OrderedStatic(const OrderedStatic&);
        OrderedStatic& operator=(const OrderedStatic&);
    };

    template<unsigned int L, class T, typename P1>
    class OrderedStatic<L, T, Loki::Seq<P1> > : public Private::OrderedStaticBase<T>
    {
    public:
        OrderedStatic(P1 p) : Private::OrderedStaticBase<T>(L), para_(p)
        {
            OrderedStaticManager::Instance().registerObject
                                (L,this,&Private::OrderedStaticCreatorFunc::createObject);
        }
        
        void createObject()
        {
            Private::OrderedStaticBase<T>::SetLongevity(new T(para_));
        }

    private:
        OrderedStatic();
        OrderedStatic(const OrderedStatic&);
        OrderedStatic& operator=(const OrderedStatic&);
        P1 para_;
    };

    template<unsigned int L, class T, typename P1>
    class OrderedStatic<L, T,  P1(*)() > : public Private::OrderedStaticBase<T>
    {
    public:

        typedef P1(*Func)();

        OrderedStatic(Func p) : Private::OrderedStaticBase<T>(L), para_(p)
        {
            OrderedStaticManager::Instance().registerObject
                                (L,this,&Private::OrderedStaticCreatorFunc::createObject);
        }

        void createObject()
        {
            Private::OrderedStaticBase<T>::SetLongevity(new T(para_()));
        }

    private:
        OrderedStatic();
        OrderedStatic(const OrderedStatic&);
        OrderedStatic& operator=(const OrderedStatic&);
        Func para_;
    };

}// namespace Loki


#endif

// $Log: OrderedStatic.h,v $
// Revision 1.10  2006/02/27 19:59:20  syntheticpp
// add support of loki.dll
//
// Revision 1.9  2006/01/16 19:05:09  rich_sposato
// Added cvs keywords.
//
