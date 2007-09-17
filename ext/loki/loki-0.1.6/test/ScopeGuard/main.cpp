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

// $Id: main.cpp 800 2006-12-22 01:08:21Z rich_sposato $


#include <loki/ScopeGuard.h>

#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

void HasNone( void )
{
}

void HasOne( int p1 )
{
    (void)p1;
}

void HasTwo( int p1, int p2 )
{
    (void)p1;
    (void)p2;
}

void HasThree( int p1, int p2, int p3 )
{
    (void)p1;
    (void)p2;
    (void)p3;
}

void HasFour( int p1, int p2, int p3, int p4 )
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
}

void HasFive( int p1, int p2, int p3, int p4, int p5 )
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
    (void)p5;
}

void Decrement( unsigned int & x ) 
{ 
    --x; 
}

struct UserDatabase
{
    void AddFriend(const std::string&, const std::string&)
    {
        throw 55;
    }
};

class User
{
public:
    User(UserDatabase* db) : fCount(0), pDB_(db)
    {}

    std::string GetName() const;

    void AddFriend(User& newFriend);
    void AddFriendGuarded(User& newFriend);
    void AddFriendGuardedMacros(User& newFriend);

    size_t countFriends() const;

	void DoSomething() const;
   
    unsigned int fCount;

    void HasNone( void ) const;
    void HasOne( int p1 ) const;
    void HasTwo( int p1, int p2 ) const;
    void HasThree( int p1, int p2, int p3 ) const;
    void HasFour( int p1, int p2, int p3, int p4 ) const;
    void HasFive( int p1, int p2, int p3, int p4, int p5 ) const;

private:
	void CheckIfValid( const char * function, unsigned int line ) const;

    typedef std::vector<User*> UserCont;
    UserCont friends_;
    UserDatabase* pDB_;
};

void User::HasNone( void ) const
{
}

void User::HasOne( int p1 ) const
{
    (void)p1;
}

void User::HasTwo( int p1, int p2 ) const
{
    (void)p1;
    (void)p2;
}

void User::HasThree( int p1, int p2, int p3 ) const
{
    (void)p1;
    (void)p2;
    (void)p3;
}

void User::HasFour( int p1, int p2, int p3, int p4 ) const
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
}

void User::HasFive( int p1, int p2, int p3, int p4, int p5 ) const
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
    (void)p5;
}

void User::DoSomething() const
{
}

void User::CheckIfValid( const char * function, unsigned int line ) const
{
	assert( friends_.size() == fCount );
	(void)function;
	(void)line;
}

std::string User::GetName() const
{
    return "A name";
}

size_t User::countFriends() const
{
    return friends_.size();
}

void User::AddFriend(User& newFriend)
{
	::Loki::ScopeGuard invariantGuard = ::Loki::MakeObjGuard( *this,
		&User::CheckIfValid, __FUNCTION__, __LINE__ );
	(void)invariantGuard;
    friends_.push_back(&newFriend);
    fCount++;
    pDB_->AddFriend(GetName(), newFriend.GetName());
}

void User::AddFriendGuarded(User& newFriend)
{
	::Loki::ScopeGuard invariantGuard = ::Loki::MakeObjGuard( *this,
		&User::CheckIfValid, __FUNCTION__, __LINE__ );
	(void)invariantGuard;
	::Loki::ScopeGuard guard1 = ::Loki::MakeObjGuard( *this, &User::DoSomething );
    (void)guard1;

    friends_.push_back(&newFriend);
    Loki::ScopeGuard guard = Loki::MakeObjGuard(friends_, &UserCont::pop_back);
    
    fCount++;
    Loki::ScopeGuard guardRef = Loki::MakeGuard(Decrement, Loki::ByRef(fCount));

    pDB_->AddFriend(GetName(), newFriend.GetName());
    guard.Dismiss();
    guardRef.Dismiss();
}

void User::AddFriendGuardedMacros(User&)
{
	::Loki::ScopeGuard invariantGuard = ::Loki::MakeObjGuard( *this,
		&User::CheckIfValid, __FUNCTION__, __LINE__ );
	(void)invariantGuard;
    LOKI_ON_BLOCK_EXIT_OBJ(friends_, &UserCont::pop_back); (void) LOKI_ANONYMOUS_VARIABLE(scopeGuard);
    LOKI_ON_BLOCK_EXIT(Decrement, Loki::ByRef(fCount)); (void) LOKI_ANONYMOUS_VARIABLE(scopeGuard);
}

void DoStandaloneFunctionTests()
{
    ::Loki::ScopeGuard guard0 = ::Loki::MakeGuard( &HasNone );
    ::Loki::ScopeGuard guard1 = ::Loki::MakeGuard( &HasOne, 1 );
    ::Loki::ScopeGuard guard2 = ::Loki::MakeGuard( &HasTwo, 1, 2 );
    ::Loki::ScopeGuard guard3 = ::Loki::MakeGuard( &HasThree, 1, 2, 3 );
    ::Loki::ScopeGuard guard4 = ::Loki::MakeGuard( &HasFour, 1, 2, 3, 4 );
    ::Loki::ScopeGuard guard5 = ::Loki::MakeGuard( &HasFive, 1, 2, 3, 4, 5 );
    (void)guard0;
    (void)guard1;
    (void)guard2;
    (void)guard3;
    (void)guard4;
    (void)guard5;
}

void DoMemberFunctionTests( User & user )
{
    ::Loki::ScopeGuard guard0 = ::Loki::MakeObjGuard( user, &User::HasNone );
    ::Loki::ScopeGuard guard1 = ::Loki::MakeObjGuard( user, &User::HasOne,   1 );
    ::Loki::ScopeGuard guard2 = ::Loki::MakeObjGuard( user, &User::HasTwo,   1, 2 );
    ::Loki::ScopeGuard guard3 = ::Loki::MakeObjGuard( user, &User::HasThree, 1, 2, 3 );
    (void)guard0;
    (void)guard1;
    (void)guard2;
    (void)guard3;
}

int main()
{
    UserDatabase db;

    User u1(&db);
    User u2(&db);

    try{ u1.AddFriend(u2); }
    catch (...){}
    std::cout << "u1 countFriends: " << u1.countFriends() << "\n";
    std::cout << "u1 fCount      : " << u1.fCount << "\n";

    try{ u2.AddFriendGuarded(u1); }
    catch (...){}
    std::cout << "u2 countFriends: " << u2.countFriends() << "\n";
    std::cout << "u2 fCount      : " << u2.fCount << "\n";

    DoStandaloneFunctionTests();
    DoMemberFunctionTests( u1 );

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

}
