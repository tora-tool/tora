#ifndef	ANTLR3REWRITESTREAM_HPP
#define	ANTLR3REWRITESTREAM_HPP

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

/// A generic list of elements tracked in an alternative to be used in
/// a -> rewrite rule.  
///
/// In the C implementation, all tree oriented streams return a pointer to 
/// the same type: pANTLR3_BASE_TREE. Anything that has subclassed from this
/// still passes this type, within which there is a super pointer, which points
/// to it's own data and methods. Hence we do not need to implement this as
/// the equivalent of an abstract class, but just fill in the appropriate interface
/// as usual with this model.
///
/// Once you start next()ing, do not try to add more elements.  It will
/// break the cursor tracking I believe.
///
/// 
/// \see #pANTLR3_REWRITE_RULE_NODE_STREAM
/// \see #pANTLR3_REWRITE_RULE_ELEMENT_STREAM
/// \see #pANTLR3_REWRITE_RULE_SUBTREE_STREAM
///
/// TODO: add mechanism to detect/puke on modification after reading from stream
///
ANTLR_BEGIN_NAMESPACE()

template<class ImplTraits, class SuperType>
class RewriteRuleElementStream  : public ImplTraits::AllocPolicyType
{
public:
	typedef typename ImplTraits::TreeType TreeType;
	typedef typename ImplTraits::AllocPolicyType AllocPolicyType;
	typedef typename ImplTraits::TreeAdaptorType TreeAdaptorType;

	typedef typename ImplTraits::template RecognizerType< typename SuperType::StreamType > RecognizerType;
	typedef typename ImplTraits::StringType StringType;
	typedef typename SuperType::TokenType TokenType; 
	typedef typename AllocPolicyType::template VectorType< TokenType* > ElementsType;

protected:
	/// Track single elements w/o creating a list.  Upon 2nd add, alloc list 
    ///
    TokenType*			m_singleElement;

    /// The list of tokens or subtrees we are tracking 
    ///
    ElementsType		m_elements;

    /// The element or stream description; usually has name of the token or
    /// rule reference that this list tracks.  Can include rulename too, but
    /// the exception would track that info.
    ///
    StringType			m_elementDescription;

	/// Pointer to the tree adaptor in use for this stream
	///
    TreeAdaptorType*	m_adaptor;

	// Pointer to the recognizer shared state to which this stream belongs
	//
	RecognizerType*			m_rec;

	/// Cursor 0..n-1.  If singleElement!=NULL, cursor is 0 until you next(),
    /// which bumps it to 1 meaning no more elements.
    ///
    ANTLR_UINT32		m_cursor;

	/// Once a node / subtree has been used in a stream, it must be dup'ed
	/// from then on.  Streams are reset after sub rules so that the streams
	/// can be reused in future sub rules.  So, reset must set a dirty bit.
	/// If dirty, then next() always returns a dup.
	///
	bool				m_dirty;

public:
	RewriteRuleElementStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description);
	RewriteRuleElementStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, TokenType* oneElement);
	RewriteRuleElementStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements);

	~RewriteRuleElementStream();
    //   Methods 

    /// Reset the condition of this stream so that it appears we have
    ///  not consumed any of its elements.  Elements themselves are untouched.
    ///
    void	reset(); 

    /// Add a new pANTLR3_BASE_TREE to this stream
    ///
    void	add(TokenType* el);

    /// Return the next element in the stream.  If out of elements, throw
    /// an exception unless size()==1.  If size is 1, then return elements[0].
    ///
	TokenType*	next();
    TreeType*	nextTree();
    TokenType*	nextToken();
    TokenType*	_next();

	/// When constructing trees, sometimes we need to dup a token or AST
    ///	subtree.  Dup'ing a token means just creating another AST node
    /// around it.  For trees, you must call the adaptor.dupTree().
    ///
	TokenType* dup( TokenType* el );

    /// Ensure stream emits trees; tokens must be converted to AST nodes.
    /// AST nodes can be passed through unmolested.
    ///
    TreeType*	toTree(TreeType* el);

    /// Returns true if there is a next element available
    ///
    bool	hasNext();

    /// Treat next element as a single node even if it's a subtree.
    /// This is used instead of next() when the result has to be a
    /// tree root node.  Also prevents us from duplicating recently-added
    /// children; e.g., ^(type ID)+ adds ID to type and then 2nd iteration
    /// must dup the type node, but ID has been added.
    ///
    /// Referencing to a rule result twice is ok; dup entire tree as
    /// we can't be adding trees; e.g., expr expr. 
    ///
    TreeType*	nextNode();

    /// Number of elements available in the stream
    ///
    ANTLR_UINT32	size();

    /// Returns the description string if there is one available (check for NULL).
    ///
    StringType getDescription();

protected:
	void init(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description);
};

/// This is an implementation of a token stream, which is basically an element
///  stream that deals with tokens only.
///
template<class ImplTraits>
class RewriteRuleTokenStream : public ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::ParserType> 
{
public:
	typedef typename ImplTraits::AllocPolicyType AllocPolicyType;
	typedef typename ImplTraits::TreeAdaptorType TreeAdaptorType;
	typedef typename ImplTraits::ParserType ComponentType;
	typedef typename ComponentType::StreamType StreamType;
	typedef typename ImplTraits::CommonTokenType TokenType;
	typedef typename ImplTraits::TreeType TreeType;
	typedef typename AllocPolicyType::template VectorType< TokenType* > ElementsType;
	typedef typename ImplTraits::template RecognizerType< StreamType > RecognizerType;
	typedef typename ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::ParserType> BaseType;

public:
	RewriteRuleTokenStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description);
	RewriteRuleTokenStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, TokenType* oneElement);
	RewriteRuleTokenStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements);
	TreeType*	nextNode();

private:
	TreeType*	nextNodeToken();
};

/// This is an implementation of a subtree stream which is a set of trees
///  modelled as an element stream.
///
template<class ImplTraits>
class RewriteRuleSubtreeStream : public ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::TreeParserType> 
{
public:
	typedef typename ImplTraits::AllocPolicyType AllocPolicyType;
	typedef typename ImplTraits::TreeAdaptorType TreeAdaptorType;
	typedef typename ImplTraits::TreeParserType ComponentType;
	typedef typename ComponentType::StreamType StreamType;
	typedef typename ImplTraits::TreeType TreeType;
	typedef TreeType TokenType;
	typedef typename ImplTraits::template RecognizerType< StreamType > RecognizerType;
	typedef typename AllocPolicyType::template VectorType< TokenType* > ElementsType;
	typedef typename ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::TreeParserType>  BaseType;

public:
	RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description);
	RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, TokenType* oneElement);
	RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements);

	TreeType* dup( TreeType* el );

private:
	TreeType* dupTree( TreeType* el );
};

/// This is an implementation of a node stream, which is basically an element
///  stream that deals with tree nodes only.
///
template<class ImplTraits>
class RewriteRuleNodeStream : public ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::TreeParserType> 
{
public:
	typedef typename ImplTraits::AllocPolicyType AllocPolicyType;
	typedef typename ImplTraits::TreeAdaptorType TreeAdaptorType;
	typedef typename ImplTraits::TreeParserType ComponentType;
	typedef typename ComponentType::StreamType StreamType;
	typedef typename ImplTraits::TreeType TreeType;
	typedef TreeType TokenType;	
	typedef typename ImplTraits::template RecognizerType< StreamType > RecognizerType;
	typedef typename AllocPolicyType::template VectorType< TokenType* > ElementsType;
	typedef typename ImplTraits::template RewriteRuleElementStreamType< typename ImplTraits::TreeParserType>  BaseType;

public:
	RewriteRuleNodeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description);
	RewriteRuleNodeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, TokenType* oneElement);
	RewriteRuleNodeStream(TreeAdaptorType* adaptor, RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements);

	TreeType*	toTree(TreeType* element);

private:
	TreeType*	toTreeNode(TreeType* element);
};

ANTLR_END_NAMESPACE()

#include "antlr3rewritestreams.inl"

#endif
