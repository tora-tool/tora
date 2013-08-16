ANTLR_BEGIN_NAMESPACE()

template<class ImplTraits>
CommonTree<ImplTraits>::CommonTree()
{
	m_savedIndex = 0;
	m_startIndex = 0;
	m_stopIndex  = 0;
	m_token		 = NULL;
	m_parent     = NULL;
	m_childIndex = 0;
}

template<class ImplTraits>
CommonTree<ImplTraits>::CommonTree( const CommonTree& ctree )
	:m_children( ctree.m_children)
{
	m_savedIndex = ctree.m_savedIndex;
	m_startIndex = ctree.m_startIndex;
	m_stopIndex  = ctree.m_stopIndex;
	m_token		 = ctree.m_token;
	m_parent     = ctree.m_parent;
	m_childIndex = ctree.m_childIndex;
}

template<class ImplTraits>
CommonTree<ImplTraits>::CommonTree( CommonTokenType* token )
{
	m_savedIndex = 0;
	m_startIndex = 0;
	m_stopIndex  = 0;
	m_token		 = token;
	m_parent     = NULL;
	m_childIndex = 0;
}

template<class ImplTraits>
CommonTree<ImplTraits>::CommonTree( CommonTree* tree )
{
	m_savedIndex = 0;
	m_startIndex = 0;
	m_stopIndex  = 0;
	m_token		 = tree->get_token();
	m_parent     = NULL;
	m_childIndex = 0;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TokenType*   CommonTree<ImplTraits>::get_token() const
{
	return m_token;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::ChildrenType& CommonTree<ImplTraits>::get_children()
{
	return m_children;
}

template<class ImplTraits>
const typename CommonTree<ImplTraits>::ChildrenType& CommonTree<ImplTraits>::get_children() const
{
	return m_children;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::ChildrenType* CommonTree<ImplTraits>::get_children_p()
{
	return &m_children;
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::addChild(TreeType* child)
{
	ANTLR_UINT32   n;
	ANTLR_UINT32   i;

	if	(child == NULL)
		return;

	ChildrenType& child_children = child->get_children();
	ChildrenType& tree_children  = this->get_children();

	if	(child->isNilNode() == true)
	{
		if ( !child_children.empty() && child_children == tree_children )
		{
			// TODO: Change to exception rather than ANTLR3_FPRINTF?
			//
			fprintf(stderr, "ANTLR3: An attempt was made to add a child list to itself!\n");
			return;
		}

        // Add all of the children's children to this list
        //
        if ( !child_children.empty() )
        {
            if (tree_children.empty())
            {
                // We are build ing the tree structure here, so we need not
                // worry about duplication of pointers as the tree node
                // factory will only clean up each node once. So we just
                // copy in the child's children pointer as the child is
                // a nil node (has not root itself).
                //
                tree_children.swap( child_children );
                this->freshenPACIndexesAll();               
            }
            else
            {
                // Need to copy the children
                //
                n = child_children.size();

                for (i = 0; i < n; i++)
                {
                    TreeType* entry;
                    entry = child_children[i];

                    // ANTLR3 lists can be sparse, unlike Array Lists
                    //
                    if (entry != NULL)
                    {
                        tree_children.push_back(entry);
                    }
                }
            }
		}
	}
	else
	{
		// Tree we are adding is not a Nil and might have children to copy
		//
		if  (tree_children.empty())
		{
			// No children in the tree we are adding to, so create a new list on
			// the fly to hold them.
			//
			this->createChildrenList();
		}
		tree_children.push_back( child );
	}
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::addChildren(const ChildListType& kids)
{
	for( typename ChildListType::const_iterator iter = kids.begin();
		 iter != kids.end(); ++iter )
	{
		this->addChild( *iter );
	}
}

//dummy one, as vector is always there
template<class ImplTraits>
void    CommonTree<ImplTraits>::createChildrenList()
{
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TreeType*	CommonTree<ImplTraits>::deleteChild(ANTLR_UINT32 i)
{
	if( m_children.empty() )
		return	NULL;

	return  m_children.erase( m_children.begin() + i);
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::replaceChildren(ANTLR_INT32 startChildIndex, ANTLR_INT32 stopChildIndex, TreeType* newTree)
{
	ANTLR_INT32	replacingHowMany;		// How many nodes will go away
	ANTLR_INT32	replacingWithHowMany;	// How many nodes will replace them
	ANTLR_INT32	numNewChildren;			// Tracking variable
	ANTLR_INT32	delta;					// Difference in new vs existing count

	ANTLR_INT32	i;
	ANTLR_INT32	j;

	if	( m_children.empty() )
	{
		fprintf(stderr, "replaceChildren call: Indexes are invalid; no children in list for %s", this->getText().c_str() );
		return;
	}

	// Either use the existing list of children in the supplied nil node, or build a vector of the
	// tree we were given if it is not a nil node, then we treat both situations exactly the same
	//
	ChildrenType newChildren_temp;
	ChildrenType*	newChildren;			// Iterator for whatever we are going to add in

	if	(newTree->isNilNode())
	{
		newChildren = newTree->get_children_p();
	}
	else
	{
		newChildren = &newChildren_temp;
		newChildren->push_back(newTree);
	}

	// Initialize
	//
	replacingHowMany		= stopChildIndex - startChildIndex + 1;
	replacingWithHowMany	= newChildren->size();
	delta					= replacingHowMany - replacingWithHowMany;
	numNewChildren			= newChildren->size();

	// If it is the same number of nodes, then do a direct replacement
	//
	if	(delta == 0)
	{
		TreeType*	child;

		// Same number of nodes
		//
		j	= 0;
		for	(i = startChildIndex; i <= stopChildIndex; i++)
		{
			child = newChildren->at(j);
			ChildrenType& parent_children = this->get_children();
			parent_children[i] = child;
			child->setParent(this);
			child->setChildIndex(i);
		}
	}
	else if (delta > 0)
	{
		ANTLR_UINT32	indexToDelete;

		// Less nodes than there were before
		// reuse what we have then delete the rest
		//
		ChildrenType& parent_children = this->get_children();
		for	(j = 0; j < numNewChildren; j++)
		{
			parent_children[ startChildIndex + j ] = newChildren->at(j);
		}

		// We just delete the same index position until done
		//
		indexToDelete = startChildIndex + numNewChildren;

		for	(j = indexToDelete; j <= stopChildIndex; j++)
		{
			parent_children.erase( parent_children.begin() + indexToDelete);
		}

		this->freshenPACIndexes(startChildIndex);
	}
	else
	{
		ChildrenType& parent_children = this->get_children();
		ANTLR_UINT32 numToInsert;

		// More nodes than there were before
		// Use what we can, then start adding
		//
		for	(j = 0; j < replacingHowMany; j++)
		{
			parent_children[ startChildIndex + j ] = newChildren->at(j);
		}

		numToInsert = replacingWithHowMany - replacingHowMany;

		for	(j = replacingHowMany; j < replacingWithHowMany; j++)
		{
			parent_children.push_back( newChildren->at(j) );
		}

		this->freshenPACIndexes(startChildIndex);
	}
}

template<class ImplTraits>
CommonTree<ImplTraits>*	CommonTree<ImplTraits>::dupNode() const
{
	// The node we are duplicating is in fact the common tree (that's why we are here)
    // so we use the super pointer to duplicate.
    //
    TreeType*   clone = new TreeType();

	// The pointer we return is the base implementation of course
    //
	clone->set_token( m_token );
	return  clone;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TreeType*	CommonTree<ImplTraits>::dupTree()
{
	TreeType*	newTree;
	ANTLR_UINT32	i;
	ANTLR_UINT32	s;

	newTree = this->dupNode();

	if	( !m_children.empty() )
	{
		s	    = m_children.size();

		for	(i = 0; i < s; i++)
		{
			TreeType*    t;
			TreeType*    newNode;

			t   = m_children[i];

			if  (t!= NULL)
			{
				newNode	    = t->dupTree();
				newTree->addChild(newNode);
			}
		}
	}

	return newTree;
}

template<class ImplTraits>
ANTLR_UINT32	CommonTree<ImplTraits>::getCharPositionInLine()
{
	CommonTokenType*    token;
	token   = m_token;

	if	(token == NULL || (token->getCharPositionInLine() == -1) )
	{
		if  (this->getChildCount() > 0)
		{
			TreeType*	child;

			child   = this->getChild(0);

			return child->getCharPositionInLine();
		}
		return 0;
	}
	return  token->getCharPositionInLine();
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TreeType*	CommonTree<ImplTraits>::getChild(ANTLR_UINT32 i)
{
	if	(  m_children.empty()
		|| i >= m_children.size() )
	{
		return NULL;
	}
	return  m_children[i];

}

template<class ImplTraits>
void    CommonTree<ImplTraits>::set_childIndex( ANTLR_INT32 i)
{
	m_childIndex = i;
}

template<class ImplTraits>
ANTLR_INT32	CommonTree<ImplTraits>::get_childIndex() const
{
	return m_childIndex;
}

template<class ImplTraits>
ANTLR_UINT32	CommonTree<ImplTraits>::getChildCount() const
{
	return static_cast<ANTLR_UINT32>( m_children.size() );
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TreeType* CommonTree<ImplTraits>::get_parent() const
{
	return m_parent;
}

template<class ImplTraits>
void     CommonTree<ImplTraits>::set_parent( TreeType* parent)
{
	m_parent = parent;
}

template<class ImplTraits>
ANTLR_UINT32	CommonTree<ImplTraits>::getType()
{
	if	(this == NULL)
	{
		return	0;
	}
	else
	{
		return	m_token->getType();
	}
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::TreeType*	CommonTree<ImplTraits>::getFirstChildWithType(ANTLR_UINT32 type)
{
	ANTLR_UINT32   i;
	std::size_t   cs;

	TreeType*	t;
	if	( !m_children.empty() )
	{
		cs	= m_children.size();
		for	(i = 0; i < cs; i++)
		{
			t = m_children[i];
			if  (t->getType() == type)
			{
				return  t;
			}
		}
	}
	return  NULL;
}

template<class ImplTraits>
ANTLR_UINT32	CommonTree<ImplTraits>::getLine()
{
	TreeType*	    cTree = this;
	CommonTokenType* token;
	token   = cTree->get_token();

	if	(token == NULL || token->getLine() == 0)
	{
		if  ( this->getChildCount() > 0)
		{
			TreeType*	child;
			child   = this->getChild(0);
			return child->getLine();
		}
		return 0;
	}
	return  token->getLine();
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::StringType	CommonTree<ImplTraits>::getText()
{
	return this->toString();
}

template<class ImplTraits>
bool	CommonTree<ImplTraits>::isNilNode()
{
	// This is a Nil tree if it has no payload (Token in our case)
	//
	if(m_token == NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::setChild(ANTLR_UINT32 i, TreeType* child)
{
	if( m_children.size() >= i )
		m_children.resize(i+1);
	m_children[i] = child;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::StringType	CommonTree<ImplTraits>::toStringTree()
{
	StringType  string;
	ANTLR_UINT32   i;
	ANTLR_UINT32   n;
	TreeType*   t;

	if( m_children.empty() )
	{
		return	this->toString();
	}

	/* Need a new string with nothing at all in it.
	*/
	if	(this->isNilNode() == false)
	{
		string.append("(");
		string.append(this->toString());
		string.append(" ");
	}
	if	( m_children != NULL)
	{
		n = m_children.size();

		for	(i = 0; i < n; i++)
		{   
			t   = m_children[i];

			if  (i > 0)
			{
				string.append(" ");
			}
			string.append(t->toStringTree());
		}
	}
	if	(this->isNilNode() == false)
	{
		string.append(")");
	}

	return  string;
}

template<class ImplTraits>
typename CommonTree<ImplTraits>::StringType	CommonTree<ImplTraits>::toString()
{
	if  (this->isNilNode() )
	{
		StringType  nilNode;

		nilNode	= "nil";

		return nilNode;
	}

	return	m_token->getText();
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::freshenPACIndexesAll()
{
	this->freshenPACIndexes(0);
}

template<class ImplTraits>
void	CommonTree<ImplTraits>::freshenPACIndexes(ANTLR_UINT32 offset)
{
	ANTLR_UINT32	count;
	ANTLR_UINT32	c;

	count	= this->getChildCount();		// How many children do we have 

	// Loop from the supplied index and set the indexes and parent
	//
	for	(c = offset; c < count; c++)
	{
		TreeType*	child;

		child = this->getChild(c);

		child->setChildIndex(c);
		child->setParent(this);
	}
}

template<class ImplTraits>
void    CommonTree<ImplTraits>::reuse()
{
	delete this; //memory re-use should be taken by the library user
}

template<class ImplTraits>
CommonTree<ImplTraits>::~CommonTree()
{
}

ANTLR_END_NAMESPACE()
