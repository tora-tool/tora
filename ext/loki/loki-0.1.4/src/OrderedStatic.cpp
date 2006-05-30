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

// $Header: /cvsroot/loki-lib/loki/src/OrderedStatic.cpp,v 1.6 2006/01/18 17:21:31 lfittl Exp $

#include <loki/OrderedStatic.h>
#include <limits>

namespace Loki
{
    namespace Private
    {

        OrderedStaticCreatorFunc::OrderedStaticCreatorFunc()
        {
        }
        
        OrderedStaticCreatorFunc::~OrderedStaticCreatorFunc()
        {
        }
    

        OrderedStaticManagerClass::OrderedStaticManagerClass() :
                    staticObjects_(),
                    max_longevity_(std::numeric_limits<unsigned int>::min()),
                    min_longevity_(std::numeric_limits<unsigned int>::max())
        {
        }

        OrderedStaticManagerClass::~OrderedStaticManagerClass()
        {
        }

        void OrderedStaticManagerClass::createObjects()
        {
            for(unsigned int longevity=max_longevity_; longevity>=min_longevity_; longevity--)
            {
                for(unsigned int i=0; i<staticObjects_.size(); i++)
                {
                    Data cur = staticObjects_.at(i);
                    if(cur.longevity==longevity)
                        ( (*cur.object).*cur.creator )();
                }
            }
        }

        void OrderedStaticManagerClass::registerObject(unsigned int l, OrderedStaticCreatorFunc* o,Creator f)
        {
            staticObjects_.push_back(Data(l,o,f));

            if(l>max_longevity_) max_longevity_=l;
            if(l<min_longevity_) min_longevity_=l;
        }

        OrderedStaticManagerClass::Data::Data(unsigned int l, OrderedStaticCreatorFunc* o, Creator f)
            : longevity(l), object(o), creator(f)
        {
        }

    }//namespace Private

}//namespace Loki

// $Log: OrderedStatic.cpp,v $
// Revision 1.6  2006/01/18 17:21:31  lfittl
// - Compile library with -Weffc++ and -pedantic (gcc)
// - Fix most issues raised by using -Weffc++ (initialization lists)
//
// Revision 1.5  2006/01/16 20:59:53  rich_sposato
// Added cvs keywords.
//
