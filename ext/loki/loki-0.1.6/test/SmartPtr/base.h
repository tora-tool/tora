////////////////////////////////////////////////////////////////////////////////
// Test program for The Loki Library
// Copyright (c) 2006 Richard Sposato
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The authors make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id: base.h 761 2006-10-17 20:48:18Z syntheticpp $

#include <assert.h>


// ----------------------------------------------------------------------------

class BaseClass
{
public:
    BaseClass( void )
    {
        s_constructions++;
    }

    virtual ~BaseClass( void )
    {
        s_destructions++;
    }

    // These 2 functions are so we can pretend we have a COM object.
    void AddRef( void ) {}
    void Release( void ) {}

    // This function is used only for the DeepCopy policy.
    virtual BaseClass * Clone( void ) const
    {
        return new BaseClass();
    }

    void DoThat( void ) const {}

    static inline bool AllDestroyed( void )
    {
        return ( s_constructions == s_destructions );
    }

    static inline bool ExtraConstructions( void )
    {
        return ( s_constructions > s_destructions );
    }

    static inline bool ExtraDestructions( void )
    {
        return ( s_constructions < s_destructions );
    }

    static inline unsigned int GetCtorCount( void )
    {
        return s_constructions;
    }

    static inline unsigned int GetDtorCount( void )
    {
        return s_destructions;
    }

private:
    /// Not implemented.
    BaseClass( const BaseClass & );
    /// Not implemented.
    BaseClass & operator = ( const BaseClass & );

    static unsigned int s_constructions;
    static unsigned int s_destructions;
};


// ----------------------------------------------------------------------------

class PublicSubClass : public BaseClass
{
public:
    // This function is used only for the DeepCopy policy.
    virtual BaseClass * Clone( void ) const
    {
        return new PublicSubClass;
    }
};

// ----------------------------------------------------------------------------

class PrivateSubClass : private BaseClass
{
public:
    // This function is used only for the DeepCopy policy.
    virtual BaseClass * Clone( void ) const
    {
        return new PrivateSubClass;
    }
};


// ----------------------------------------------------------------------------

/** @class MimicCOM Acts like a COM object by having an intrusive ref count.
 */
class MimicCOM
{
public:

    static MimicCOM * QueryInterface( void )
    {
        MimicCOM * p = new MimicCOM;
        p->AddRef();
        return p;
    }

    virtual ~MimicCOM( void )
    {
        s_destructions++;
    }

    void AddRef( void )
    {
        m_count++;
        m_AddRefCount++;
    }

    void Release( void )
    {
        m_ReleaseCount++;
        assert( 0 < m_count );
        m_count--;
        if ( 0 == m_count )
        {
            /** @note I consider "delete this;" to be very unsafe!  I'm only
             using it here for the purpose of testing.
             */
            delete this;
        }
    }

    void DoThat( void ) {}

    static inline bool AllDestroyed( void )
    {
        return ( s_constructions == s_destructions );
    }

    static inline bool ExtraConstructions( void )
    {
        return ( s_constructions > s_destructions );
    }

    static inline bool ExtraDestructions( void )
    {
        return ( s_constructions < s_destructions );
    }

    static inline unsigned int GetCtorCount( void )
    {
        return s_constructions;
    }

    static inline unsigned int GetDtorCount( void )
    {
        return s_destructions;
    }

private:
    /// Not implemented.
    MimicCOM( const MimicCOM & );
    /// Not implemented.
    MimicCOM & operator = ( const MimicCOM & );

    MimicCOM( void )
        : m_count( 0 )
        , m_AddRefCount( 0 )
        , m_ReleaseCount( 0 )
    {
        s_constructions++;
    }

    static unsigned int s_constructions;
    static unsigned int s_destructions;

    unsigned int m_count;
    unsigned int m_AddRefCount;
    unsigned int m_ReleaseCount;
};

// ----------------------------------------------------------------------------

