ANTLR_BEGIN_NAMESPACE()

template<class ImplTraits>
ANTLR_INLINE CommonTreeAdaptor<ImplTraits>::CommonTreeAdaptor(DebuggerType*)
{
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	  CommonTreeAdaptor<ImplTraits>::nilNode()
{
	return this->create(NULL);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	  CommonTreeAdaptor<ImplTraits>::dupTree( TreeType* tree)
{
	return this->dupTreeTT(tree, NULL);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	  CommonTreeAdaptor<ImplTraits>::dupTreeTT( TreeType* t, TreeType* parent)
{
	TreeType*	newTree;
	TreeType*	child;
	TreeType*	newSubTree;
	ANTLR_UINT32		n;
	ANTLR_UINT32		i;

	if	(t == NULL)
		return NULL;

	newTree = t->dupNode();

	// Ensure new subtree root has parent/child index set
	//
	this->setChildIndex( newTree, t->getChildIndex() );
	this->setParent(newTree, parent);
	n = this->getChildCount(t);

	for	(i=0; i < n; i++)
	{
		child = this->getChild(t, i);
		newSubTree = this->dupTreeTT(child, t);
		this->addChild(newTree, newSubTree);
	}
	return	newTree;
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::addChild( TreeType* t, TreeType* child)
{
	if	(t != NULL && child != NULL)
	{
		t->addChild(child);
	}
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::addChildToken( TreeType* t, CommonTokenType* child)
{
	if	(t != NULL && child != NULL)
	{
		this->addChild(t, this->create(child));
	}
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::setParent( TreeType* child, TreeType* parent)
{
	child->setParent(parent);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*		CommonTreeAdaptor<ImplTraits>::getParent( TreeType* child)
{
	return child->getParent();
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*		CommonTreeAdaptor<ImplTraits>::errorNode( CommonTokenType* tnstream, CommonTokenType* startToken, CommonTokenType* stopToken)
{
	// Use the supplied common tree node stream to get another tree from the factory
	// TODO: Look at creating the erronode as in Java, but this is complicated by the
	// need to track and free the memory allocated to it, so for now, we just
	// want something in the tree that isn't a NULL pointer.
	//
	return this->createTypeText( CommonTokenType::TOKEN_INVALID, "Tree Error Node");

}

template<class ImplTraits>
bool	CommonTreeAdaptor<ImplTraits>::isNilNode( TreeType* t)
{
	return t->isNilNode();
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	    CommonTreeAdaptor<ImplTraits>::becomeRoot( TreeType* newRootTree, TreeType* oldRootTree)
{
	TreeType* saveRoot;

	/* Protect against tree rewrites if we are in some sort of error
	 * state, but have tried to recover. In C we can end up with a null pointer
	 * for a tree that was not produced.
	 */
	if	(newRootTree == NULL)
	{
		return	oldRootTree;
	}

	/* root is just the new tree as is if there is no
	 * current root tree.
	 */
	if	(oldRootTree == NULL)
	{
		return	newRootTree;
	}

	/* Produce ^(nil real-node)
	 */
	if	(newRootTree->isNilNode())
	{
		if	(newRootTree->getChildCount() > 1)
		{
			/* TODO: Handle tree exceptions 
			 */
			fprintf(stderr, "More than one node as root! TODO: Create tree exception handling\n");
			return newRootTree;
		}

		/* The new root is the first child, keep track of the original newRoot
         * because if it was a Nil Node, then we can reuse it now.
		 */
        saveRoot    = newRootTree;
		newRootTree = newRootTree->getChild(0);

        // Reclaim the old nilNode()
        //
        saveRoot->reuse();
	}

	/* Add old root into new root. addChild takes care of the case where oldRoot
	 * is a flat list (nill rooted tree). All children of oldroot are added to
	 * new root.
	 */
	newRootTree->addChild(oldRootTree);

    // If the oldroot tree was a nil node, then we know at this point
    // it has become orphaned by the rewrite logic, so we tell it to do
    // whatever it needs to do to be reused.
    //
    if  (oldRootTree->isNilNode())
    {
        // We have taken an old Root Tree and appended all its children to the new
        // root. In addition though it was a nil node, which means the generated code
        // will not reuse it again, so we will reclaim it here. First we want to zero out
        // any pointers it was carrying around. We are just the baseTree handler so we
        // don't know necessarilly know how to do this for the real node, we just ask the tree itself
        // to do it.
        //
        oldRootTree->reuse();
    }
	/* Always returns new root structure
	 */
	return	newRootTree;
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	CommonTreeAdaptor<ImplTraits>::becomeRootToken(CommonTokenType* newRoot, TreeType* oldRoot)
{
	return	this->becomeRoot(this->create(newRoot), oldRoot);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	 	CommonTreeAdaptor<ImplTraits>::create( CommonTokenType* payload)
{
	return new TreeType(payload);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*  CommonTreeAdaptor<ImplTraits>::createTypeToken( ANTLR_UINT32 tokenType, 
																						  CommonTokenType* fromToken)
{
	/* Create the new token
	 */
	fromToken = this->createTokenFromToken(fromToken);

	/* Set the type of the new token to that supplied
	 */
	fromToken->setType(tokenType);

	/* Return a new node based upon this token
	 */
	return	this->create(fromToken);

}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	CommonTreeAdaptor<ImplTraits>::createTypeTokenText( ANTLR_UINT32 tokenType, CommonTokenType* fromToken, const ANTLR_UINT8* text)
{
	/* Create the new token
	 */
	fromToken = this->createTokenFromToken(fromToken);

	/* Set the type of the new token to that supplied
	 */
	fromToken->setType(tokenType);

	/* Set the text of the token accordingly
	 */
	fromToken->setText(text);

	/* Return a new node based upon this token
	 */
	return	this->create(fromToken);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	    CommonTreeAdaptor<ImplTraits>::createTypeText( ANTLR_UINT32 tokenType, const ANTLR_UINT8* text)
{
	CommonTokenType*	fromToken;

	/* Create the new token
	 */
	fromToken = this->createToken(tokenType, text);

	/* Return a new node based upon this token
	 */
	return	this->create(fromToken);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	CommonTreeAdaptor<ImplTraits>::dupNode( TreeType* treeNode)
{
	return  (treeNode == NULL) ? NULL : treeNode->dupNode();
}

template<class ImplTraits>
ANTLR_UINT32	CommonTreeAdaptor<ImplTraits>::getType( TreeType* t)
{
	return t->getType();
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::StringType	CommonTreeAdaptor<ImplTraits>::getText( TreeType* t)
{
	return t->getText();
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType*	    CommonTreeAdaptor<ImplTraits>::getChild( TreeType* t, ANTLR_UINT32 i)
{
	return t->getChild(i);
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::setChild( TreeType* t, ANTLR_UINT32 i, TreeType* child)
{
	t->setChild(i, child);
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::deleteChild( TreeType* t, ANTLR_UINT32 i)
{
	t->deleteChild(i);
}

template<class ImplTraits>
void	CommonTreeAdaptor<ImplTraits>::setChildIndex( TreeType* t, ANTLR_INT32 i)
{
	t->setChildIndex(i);
}

template<class ImplTraits>
ANTLR_INT32	CommonTreeAdaptor<ImplTraits>::getChildIndex( TreeType * t)
{
	return t->getChildIndex();
}

template<class ImplTraits>
ANTLR_UINT32	CommonTreeAdaptor<ImplTraits>::getChildCount( TreeType* t)
{
	return t->getChildCount();
}

template<class ImplTraits>
ANTLR_UINT64	CommonTreeAdaptor<ImplTraits>::getUniqueID( TreeType* node )
{
	return	reinterpret_cast<ANTLR_UINT64>(node);
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::CommonTokenType*    
	     CommonTreeAdaptor<ImplTraits>::createToken( ANTLR_UINT32 tokenType, const ANTLR_UINT8* text)
{
	CommonTokenType*    newToken = new CommonTokenType;

    if	(newToken != NULL)
    {	
		newToken->set_tokText( (const char*) text );
		newToken->setType(tokenType);
    }
    return  newToken;

}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::CommonTokenType*    
	CommonTreeAdaptor<ImplTraits>::createTokenFromToken( CommonTokenType* fromToken)
{
	CommonTokenType*    newToken;

    newToken	= new CommonTokenType;
    
    if	(newToken != NULL)
    {
		// Create the text using our own string factory to avoid complicating
		// commontoken.
		//
		StringType	text = fromToken->getText();
		newToken->set_tokText( text );
		newToken->setLine( fromToken->getLine() );
		newToken->setTokenIndex( fromToken->getTokenIndex() );
		newToken->setCharPositionInLine( fromToken->getCharPositionInLine() );
		newToken->setChannel( fromToken->getChannel() );
		newToken->setType( fromToken->getType() );
    }

    return  newToken;
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::CommonTokenType*  
		 CommonTreeAdaptor<ImplTraits>::getToken( TreeType* t)
{
	return t->getToken();
}

template<class ImplTraits>
void CommonTreeAdaptor<ImplTraits>::setTokenBoundaries( TreeType* t, CommonTokenType* startToken, CommonTokenType* stopToken)
{
	ANTLR_MARKER   start;
	ANTLR_MARKER   stop;

	TreeType*	    ct;

	if	(t == NULL)
	{
		return;
	}

	if	( startToken != NULL)
	{
		start = startToken->getTokenIndex();
	}
	else
	{
		start = 0;
	}

	if	( stopToken != NULL)
	{
		stop = stopToken->getTokenIndex();
	}
	else
	{
		stop = 0;
	}

	ct	= t;

	ct->set_startIndex(start);
	ct->set_stopIndex(stop);
}

template<class ImplTraits>
ANTLR_MARKER	CommonTreeAdaptor<ImplTraits>::getTokenStartIndex( TreeType* t)
{
	return t->get_tokenStartIndex();
}

template<class ImplTraits>
ANTLR_MARKER	CommonTreeAdaptor<ImplTraits>::getTokenStopIndex( TreeType* t)
{
	return t->get_tokenStopIndex();
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::StringType	 CommonTreeAdaptor<ImplTraits>::makeDot( TreeType* theTree)
{
	// The string we are building up
	//
	StringType		dotSpec;
	char            buff[64];
	StringType      text;
	
	dotSpec = "digraph {\n\n"
			"\tordering=out;\n"
			"\tranksep=.4;\n"
			"\tbgcolor=\"lightgrey\";  node [shape=box, fixedsize=false, fontsize=12, fontname=\"Helvetica-bold\", fontcolor=\"blue\"\n"
			"\twidth=.25, height=.25, color=\"black\", fillcolor=\"white\", style=\"filled, solid, bold\"];\n\n"
			"\tedge [arrowsize=.5, color=\"black\", style=\"bold\"]\n\n";

    if	(theTree == NULL)
	{
		// No tree, so create a blank spec
		//
		dotSpec->append("n0[label=\"EMPTY TREE\"]\n");
		return dotSpec;
	}

    sprintf(buff, "\tn%p[label=\"", theTree);
	dotSpec.append(buff);
    text = this->getText(theTree);
    for (std::size_t j = 0; j < text.size(); j++)
    {
            switch(text[j])
            {
                case '"':
                    dotSpec.append("\\\"");
                    break;

                case '\n':
                    dotSpec.append("\\n");
                    break;

                case '\r':
                    dotSpec.append("\\r");
                    break;

                default:
                    dotSpec += text[j];
                    break;
            }
    }
	dotSpec->append("\"]\n");

	// First produce the node defintions
	//
	this->defineDotNodes(theTree, dotSpec);
	dotSpec.append("\n");
	this->defineDotEdges(theTree, dotSpec);
	
	// Terminate the spec
	//
	dotSpec.append("\n}");

	// Result
	//
	return dotSpec;
}

template<class ImplTraits>
void CommonTreeAdaptor<ImplTraits>::replaceChildren( TreeType* parent, ANTLR_INT32 startChildIndex, ANTLR_INT32 stopChildIndex, TreeType* t)
{
	if	(parent != NULL)
		parent->replaceChildren(startChildIndex, stopChildIndex, t);
}

template<class ImplTraits>
CommonTreeAdaptor<ImplTraits>::~CommonTreeAdaptor()
{
}

template<class ImplTraits>
void CommonTreeAdaptor<ImplTraits>::defineDotNodes(TreeType* t, const StringType& dotSpec)
{
	// How many nodes are we talking about?
	//
	int	nCount;
	int i;
    TreeType* child;
	char	buff[64];
	StringType	text;
	int		j;

	// Count the nodes
	//
	nCount = this->getChildCount(t);

	if	(nCount == 0)
	{
		// This will already have been included as a child of another node
		// so there is nothing to add.
		//
		return;
	}

	// For each child of the current tree, define a node using the
	// memory address of the node to name it
	//
	for	(i = 0; i<nCount; i++)
	{

		// Pick up a pointer for the child
		//
		child = this->getChild(t, i);

		// Name the node
		//
		sprintf(buff, "\tn%p[label=\"", child);
		dotSpec->append(buff);
		text = this->getText(child);
		for (j = 0; j < text.size(); j++)
		{
            switch(text[j])
            {
                case '"':
                    dotSpec.append("\\\"");
                    break;

                case '\n':
                    dotSpec.append("\\n");
                    break;

                case '\r':
                    dotSpec.append("\\r");
                    break;

                default:
                    dotSpec += text[j];
                    break;
            }
		}
		dotSpec.append("\"]\n");

		// And now define the children of this child (if any)
		//
		this->defineDotNodes(child, dotSpec);
	}
	
	// Done
	//
	return;
}

template<class ImplTraits>
void CommonTreeAdaptor<ImplTraits>::defineDotEdges(TreeType* t, const StringType& dotSpec)
{
	// How many nodes are we talking about?
	//
	int	nCount;
	if	(t == NULL)
	{
		// No tree, so do nothing
		//
		return;
	}

	// Count the nodes
	//
	nCount = this->getChildCount(t);

	if	(nCount == 0)
	{
		// This will already have been included as a child of another node
		// so there is nothing to add.
		//
		return;
	}

	// For each child, define an edge from this parent, then process
	// and children of this child in the same way
	//
	for	(int i=0; i<nCount; i++)
	{
		TreeType* child;
		char	buff[128];
        StringType text;

		// Next child
		//
		child	= this->getChild(t, i);

		// Create the edge relation
		//
		sprintf(buff, "\t\tn%p -> n%p\t\t// ",  t, child);
        
		dotSpec.append(buff);

		// Document the relationship
		//
        text = this->getText(t);
		for (std::size_t j = 0; j < text.size(); j++)
        {
                switch(text[j])
                {
                    case '"':
                        dotSpec.append("\\\"");
                        break;

                    case '\n':
                        dotSpec.append("\\n");
                        break;

                    case '\r':
                        dotSpec.append("\\r");
                        break;

                    default:
                        dotSpec += text[j];
                        break;
                }
        }

        dotSpec.append(" -> ");

        text = this->getText(child);
        for (std::size_t j = 0; j < text.size(); j++)
        {
                switch(text[j])
                {
                    case '"':
                        dotSpec.append("\\\"");
                        break;

                    case '\n':
                        dotSpec.append("\\n");
                        break;

                    case '\r':
                        dotSpec.append("\\r");
                        break;

                    default:
                        dotSpec += text[j];
                        break;
                }
        }
		dotSpec.append("\n");
        
		// Define edges for this child
		//
		this->defineDotEdges(child, dotSpec);
	}

	// Done
	//
	return;
}

template<class ImplTraits>
typename CommonTreeAdaptor<ImplTraits>::TreeType* CommonTreeAdaptor<ImplTraits>::rulePostProcessing( TreeType* root)
{
	TreeType* saveRoot;

    // Keep track of the root we are given. If it is a nilNode, then we
    // can reuse it rather than orphaning it!
    //
    saveRoot = root;

	if (root != NULL && root->isNilNode())
	{
		if	(root->getChildCount() == 0)
		{
			root = NULL;
		}
		else if	(root->getChildCount() == 1)
		{
			root = root->getChild(0);
			root->setParent(NULL);
			root->setChildIndex(-1);

            // The root we were given was a nil node, wiht one child, which means it has
            // been abandoned and would be lost in the node factory. However
            // nodes can be flagged as resuable to prevent this terrible waste
            //
            saveRoot->reuse();
		}
	}
	return root;
}

template<class ImplTraits>
DebugTreeAdaptor<ImplTraits>::DebugTreeAdaptor( DebuggerType* debugger )
{
	m_debugger = debugger;
}

template<class ImplTraits>
void DebugTreeAdaptor<ImplTraits>::setDebugEventListener( DebuggerType* debugger)
{
	m_debugger = debugger;
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType*	  DebugTreeAdaptor<ImplTraits>::nilNode()
{
	TreeType*	t = this->create(NULL);
	m_debugger->createNode(t);
	return	t;
}

template<class ImplTraits>
void	DebugTreeAdaptor<ImplTraits>::addChild(TreeType* t, TreeType* child)
{
	if	(t != NULL && child != NULL)
	{
		t->addChild(child);
		m_debugger->addChild(t, child);
	}
}

template<class ImplTraits>
void	DebugTreeAdaptor<ImplTraits>::addChildToken(TreeType* t, CommonTokenType* child)
{
	TreeType*	tc;
	if	(t != NULL && child != NULL)
	{
		tc = this->create(child);
		this->addChild(t, tc);
		m_debugger->addChild(t, tc);
	}
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::becomeRoot( TreeType* newRootTree, TreeType* oldRootTree )
{
	TreeType* t;
	t = this->becomeRoot(newRootTree, oldRootTree);
	m_debugger->becomeRoot(newRootTree, oldRootTree);
	return t;
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::becomeRootToken(TreeType* newRoot, TreeType* oldRoot)
{
	TreeType*	t;
	t =	this->becomeRoot(this->create(newRoot), oldRoot);
	m_debugger->becomeRoot(t, oldRoot);
	return t;
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::createTypeToken(ANTLR_UINT32 tokenType, CommonTokenType* fromToken)
{
	TreeType* t;
	t = this->createTypeToken(tokenType, fromToken);
	m_debugger->createNode(t);
	return t;
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::createTypeTokenText(ANTLR_UINT32 tokenType, CommonTokenType* fromToken, ANTLR_UINT8* text)
{
	TreeType* t;
	t = this->createTypeTokenText(tokenType, fromToken, text);
	m_debugger->createNode(t);
	return t;
}
	
template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::createTypeText( ANTLR_UINT32 tokenType, ANTLR_UINT8* text)
{
	TreeType* t;
	t = this->createTypeText(tokenType, text);
	m_debugger->createNode(t);
	return t;
}

template<class ImplTraits>
typename DebugTreeAdaptor<ImplTraits>::TreeType* DebugTreeAdaptor<ImplTraits>::dupTree( TreeType* tree)
{
	TreeType* t;

	// Call the normal dup tree mechanism first
	//
	t = this->dupTreeTT(tree, NULL);

	// In order to tell the debugger what we have just done, we now
	// simulate the tree building mechanism. THis will fire
	// lots of debugging events to the client and look like we
	// duped the tree..
	//
	this->simulateTreeConstruction( t);

	return t;
}

template<class ImplTraits>
void DebugTreeAdaptor<ImplTraits>::simulateTreeConstruction(TreeType* tree)
{
	ANTLR_UINT32		n;
	ANTLR_UINT32		i;
	TreeType*	child;

	// Send the create node event
	//
	m_debugger->createNode(tree);

	n = this->getChildCount(tree);
	for	(i = 0; i < n; i++)
	{
		child = this->getChild(tree, i);
		this->simulateTreeConstruction(child);
		m_debugger->addChild(tree, child);
	}
}


ANTLR_END_NAMESPACE()
