/** \file
 * Definition of the ANTLR3 common tree adaptor.
 */

#ifndef	_ANTLR3_COMMON_TREE_ADAPTOR_HPP
#define	_ANTLR3_COMMON_TREE_ADAPTOR_HPP

// [The "BSD licence"]
// Copyright (c) 2005-2009 Gokulakannan Somasundaram, ElectronDB

//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include    "antlr3defs.hpp"

ANTLR_BEGIN_NAMESPACE()

template<class ImplTraits>
class CommonTreeAdaptor : public ImplTraits::AllocPolicyType
{
public:
	typedef typename ImplTraits::StringType StringType;
	typedef typename ImplTraits::TreeType TreeType;
	typedef	TreeType TokenType;
	typedef typename ImplTraits::CommonTokenType CommonTokenType;
	typedef typename ImplTraits::DebugEventListenerType DebuggerType;

public:
	//The parameter is there only to provide uniform constructor interface
	CommonTreeAdaptor(DebuggerType* dbg = NULL);
    TreeType*	  nilNode();
	TreeType*	  dupTree( TreeType* tree);
    TreeType*	  dupTreeTT( TreeType* t, TreeType* tree);

    void	addChild( TreeType* t, TreeType* child);
    void	addChildToken( TreeType* t, CommonTokenType* child);
    void	setParent( TreeType* child, TreeType* parent);
    TreeType*		getParent( TreeType* child);

	TreeType*		errorNode( CommonTokenType* tnstream, CommonTokenType* startToken, CommonTokenType* stopToken);
	bool	isNilNode( TreeType* t);

    TreeType*	    becomeRoot( TreeType* newRoot, TreeType* oldRoot);
    TreeType*	   	rulePostProcessing( TreeType* root);

    TreeType*	becomeRootToken(CommonTokenType* newRoot, TreeType* oldRoot);

    TreeType*	 	create( CommonTokenType* payload);
    TreeType* 		createTypeToken( ANTLR_UINT32 tokenType, CommonTokenType* fromToken);
    TreeType*	   	createTypeTokenText	( ANTLR_UINT32 tokenType, CommonTokenType* fromToken, const ANTLR_UINT8* text);
    TreeType*	    createTypeText		( ANTLR_UINT32 tokenType, const ANTLR_UINT8* text);

    TreeType*	    dupNode( TreeType* treeNode);
    ANTLR_UINT32			getType( TreeType* t);
    StringType			getText( TreeType* t);
        
    TreeType*	    getChild( TreeType* t, ANTLR_UINT32 i);
    void	setChild( TreeType* t, ANTLR_UINT32 i, TreeType* child);
    void	deleteChild( TreeType* t, ANTLR_UINT32 i);
    void	setChildIndex( TreeType* t, ANTLR_INT32 i);
    ANTLR_INT32	getChildIndex( TreeType* t);

    ANTLR_UINT32	getChildCount( TreeType*);
	ANTLR_UINT64	getUniqueID( TreeType*);

    CommonTokenType*    createToken( ANTLR_UINT32 tokenType, const ANTLR_UINT8* text);
    CommonTokenType*    createTokenFromToken( CommonTokenType* fromToken);
    CommonTokenType*    getToken( TreeType* t);

    void setTokenBoundaries( TreeType* t, CommonTokenType* startToken, CommonTokenType* stopToken);
    ANTLR_MARKER	getTokenStartIndex( TreeType* t);
    ANTLR_MARKER	getTokenStopIndex( TreeType* t);

	/// Produce a DOT (see graphviz freeware suite) from a base tree
	///
	StringType			makeDot( TreeType* theTree);

	/// Replace from start to stop child index of parent with t, which might
	/// be a list.  Number of children may be different
	/// after this call.
	///
	/// If parent is null, don't do anything; must be at root of overall tree.
	/// Can't replace whatever points to the parent externally.  Do nothing.
	///
	void replaceChildren( TreeType* parent, ANTLR_INT32 startChildIndex,
								  ANTLR_INT32 stopChildIndex, TreeType* t);

    ~CommonTreeAdaptor();

protected:
	void defineDotNodes(TreeType* t, const StringType& dotSpec);
	void defineDotEdges(TreeType* t, const StringType& dotSpec);
};

//If someone can override the CommonTreeAdaptor at the compile time, that will be 
//inherited here. Still you can choose to override the DebugTreeAdaptor, if you wish to
//change the DebugTreeAdaptor
template<class ImplTraits>
class DebugTreeAdaptor : public ImplTraits::CommonTreeAdaptorType
{
public:
	//DebugEventListener implements functionality through virtual functions
	//the template parameter is required for pointing back at the adaptor
	typedef typename ImplTraits::DebugEventListener DebuggerType;
	typedef typename ImplTraits::TreeType TreeType;
	typedef typename ImplTraits::CommonTokenType CommonTokenType;

private:
	/// If set to something other than NULL, then this structure is
	/// points to an instance of the debugger interface. In general, the
	/// debugger is only referenced internally in recovery/error operations
	/// so that it does not cause overhead by having to check this pointer
	/// in every function/method
	///
	DebuggerType*		m_debugger;

public:
	DebugTreeAdaptor( DebuggerType* debugger );
	void setDebugEventListener( DebuggerType* debugger);
	TreeType*	  nilNode();
	void	addChild(TreeType* t, TreeType* child);
	void	addChildToken(TreeType* t, CommonTokenType* child);
	TreeType* becomeRoot( TreeType* newRootTree, TreeType* oldRootTree );
	TreeType* becomeRootToken(TreeType* newRoot, TreeType* oldRoot);
	TreeType* createTypeToken(ANTLR_UINT32 tokenType, CommonTokenType* fromToken);
	TreeType* createTypeTokenText(ANTLR_UINT32 tokenType, CommonTokenType* fromToken, ANTLR_UINT8* text);	
	TreeType* createTypeText( ANTLR_UINT32 tokenType, ANTLR_UINT8* text);
	TreeType* dupTree( TreeType* tree);

	/// Sends the required debugging events for duplicating a tree
	/// to the debugger.
	///
	void simulateTreeConstruction(TreeType* tree);
};


ANTLR_END_NAMESPACE()

#include "antlr3commontreeadaptor.inl"

#endif
