ANTLR_BEGIN_NAMESPACE()

template<class ImplTraits, class SuperType>
RewriteRuleElementStream<ImplTraits, SuperType>::RewriteRuleElementStream(TreeAdaptorType* adaptor, 
													RecognizerType* rec, ANTLR_UINT8* description)
{
	this->init(adaptor, rec, description);
}

template<class ImplTraits, class SuperType>
RewriteRuleElementStream<ImplTraits, SuperType>::RewriteRuleElementStream(TreeAdaptorType* adaptor, 
								RecognizerType* rec, ANTLR_UINT8* description, TokenType* oneElement)
{
	this->init(adaptor, rec, description);
	if( oneElement != NULL )
		this->add( oneElement );
}

template<class ImplTraits, class SuperType>
RewriteRuleElementStream<ImplTraits, SuperType>::RewriteRuleElementStream(TreeAdaptorType* adaptor, 
						RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements)
						:m_elements(elements)
{
	this->init(adaptor, rec, description);
}

template<class ImplTraits, class SuperType>
void RewriteRuleElementStream<ImplTraits, SuperType>::init(TreeAdaptorType* adaptor, 
								RecognizerType* rec, ANTLR_UINT8* description)
{
	m_rec = rec;
	m_adaptor = adaptor;
	m_cursor  = 0;
	m_dirty	  = false;
	m_singleElement = NULL;
}

template<class ImplTraits>
RewriteRuleTokenStream<ImplTraits>::RewriteRuleTokenStream(TreeAdaptorType* adaptor, 
							    RecognizerType* rec, ANTLR_UINT8* description)
                                                              :BaseType(adaptor, rec, description)
{
}

template<class ImplTraits>
RewriteRuleTokenStream<ImplTraits>::RewriteRuleTokenStream(TreeAdaptorType* adaptor, RecognizerType* rec, 
							    ANTLR_UINT8* description, TokenType* oneElement)
							:BaseType(adaptor, rec, description, oneElement)
{
}

template<class ImplTraits>
RewriteRuleTokenStream<ImplTraits>::RewriteRuleTokenStream(TreeAdaptorType* adaptor, 
						RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements)
						:BaseType(adaptor, rec, description, elements)
{
}

template<class ImplTraits>
RewriteRuleSubtreeStream<ImplTraits>::RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, 
								RecognizerType* rec, ANTLR_UINT8* description)
						 :BaseType(adaptor, rec, description)
{
}

template<class ImplTraits>
RewriteRuleSubtreeStream<ImplTraits>::RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, RecognizerType* rec, 
								ANTLR_UINT8* description, TokenType* oneElement)
							:BaseType(adaptor, rec, description, oneElement)
{
}

template<class ImplTraits>
RewriteRuleSubtreeStream<ImplTraits>::RewriteRuleSubtreeStream(TreeAdaptorType* adaptor, 
						RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements)
						:BaseType(adaptor, rec, description, elements)
{
}

template<class ImplTraits>
RewriteRuleNodeStream<ImplTraits>::RewriteRuleNodeStream(TreeAdaptorType* adaptor, 
							 RecognizerType* rec, ANTLR_UINT8* description)
						:BaseType(adaptor, rec, description)
{
}

template<class ImplTraits>
RewriteRuleNodeStream<ImplTraits>::RewriteRuleNodeStream(TreeAdaptorType* adaptor, RecognizerType* rec, 
							ANTLR_UINT8* description, TokenType* oneElement)
						:BaseType(adaptor, rec, description, oneElement)
{
}

template<class ImplTraits>
RewriteRuleNodeStream<ImplTraits>::RewriteRuleNodeStream(TreeAdaptorType* adaptor, 
						RecognizerType* rec, ANTLR_UINT8* description, const ElementsType& elements)
						:BaseType(adaptor, rec, description, elements)
{
}

template<class ImplTraits, class SuperType>
void	RewriteRuleElementStream<ImplTraits, SuperType>::reset()
{
	m_dirty = true;
	m_cursor = 0;
}

template<class ImplTraits, class SuperType>
void	RewriteRuleElementStream<ImplTraits, SuperType>::add(TokenType* el)
{
	if ( el== NULL ) 
		return;

	if ( !m_elements.empty() ) 
	{ 
		// if in list, just add
		m_elements.push_back(el);
		return;
	}
		
	if ( m_singleElement == NULL ) 
	{ 
		// no elements yet, track w/o list
		m_singleElement = el;
		return;
	}

	// adding 2nd element, move to list
	m_elements.push_back(m_singleElement);
	m_singleElement = NULL;
	m_elements.push_back(el);
}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::TokenType*  
RewriteRuleElementStream<ImplTraits, SuperType>::_next()
{
	ANTLR_UINT32		n;
	TreeType*	t;

	n = this->size();

	if (n == 0)
	{
		// This means that the stream is empty
		//
		return NULL;	// Caller must cope with this
	}

	// Traversed all the available elements already?
	//
	if ( m_cursor >= n)
	{
		if (n == 1)
		{
			// Special case when size is single element, it will just dup a lot
			//
			return this->toTree(m_singleElement);
		}

		// Out of elements and the size is not 1, so we cannot assume
		// that we just duplicate the entry n times (such as ID ent+ -> ^(ID ent)+)
		// This means we ran out of elements earlier than was expected.
		//
		return NULL;	// Caller must cope with this
	}

	// Elements available either for duping or just available
	//
	if ( m_singleElement != NULL)
	{
		m_cursor++;   // Cursor advances even for single element as this tells us to dup()
		return this->toTree(m_singleElement);
	}

	// More than just a single element so we extract it from the 
	// vector.
	//
	t = this->toTree( m_elements.at(m_cursor));
	m_cursor++;
	return t;
}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::TreeType*  
RewriteRuleElementStream<ImplTraits, SuperType>::nextTree()
{
	ANTLR_UINT32		n;
	TreeType*  el;

	n = this->size();

	if ( m_dirty || ( (m_cursor >=n) && (n==1)) ) 
	{
		// if out of elements and size is 1, dup
		//
		el = this->_next();
		return this->dup(el);
	}

	// test size above then fetch
	//
	el = this->_next();
	return el;
}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::TokenType*	
RewriteRuleElementStream<ImplTraits, SuperType>::nextToken()
{
	return this->_next();
}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::TokenType*		
RewriteRuleElementStream<ImplTraits, SuperType>::next()
{
	ANTLR_UINT32   s;
	s = this->size();
	if ( (m_cursor >= s) && (s == 1) )
	{
		TreeType* el;
		el = this->_next();
		return	this->dup(el);
	}
	return this->_next();
}

template<class ImplTraits>
typename RewriteRuleSubtreeStream<ImplTraits>::TreeType*	
RewriteRuleSubtreeStream<ImplTraits>::dup(TreeType* element)
{
	return this->dupTree(element);
}

template<class ImplTraits>
typename RewriteRuleSubtreeStream<ImplTraits>::TreeType*	
RewriteRuleSubtreeStream<ImplTraits>::dupTree(TreeType* element)
{
	return BaseType::m_adaptor->dupNode(element);
}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::TreeType*	
RewriteRuleElementStream<ImplTraits, SuperType>::toTree( TreeType* element)
{
	return element;
}

template<class ImplTraits>
typename RewriteRuleNodeStream<ImplTraits>::TreeType*	
RewriteRuleNodeStream<ImplTraits>::toTree(TreeType* element)
{
	return this->toTreeNode(element);
}

template<class ImplTraits>
typename RewriteRuleNodeStream<ImplTraits>::TreeType*	
RewriteRuleNodeStream<ImplTraits>::toTreeNode(TreeType* element)
{
	return BaseType::m_adaptor->dupNode(element);
}

template<class ImplTraits, class SuperType>
bool RewriteRuleElementStream<ImplTraits, SuperType>::hasNext()
{
	if (	((m_singleElement != NULL) && (m_cursor < 1))
		||	 ( !m_elements.empty() && m_cursor < m_elements.size()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<class ImplTraits >
typename RewriteRuleTokenStream<ImplTraits>::TreeType*
RewriteRuleTokenStream<ImplTraits>::nextNode()
{
	return this->nextNodeToken();
}

template<class ImplTraits>
typename RewriteRuleTokenStream<ImplTraits>::TreeType*
RewriteRuleTokenStream<ImplTraits>::nextNodeToken()
{
	return BaseType::m_adaptor->create(this->_next());
}

/// Number of elements available in the stream
///
template<class ImplTraits, class SuperType>
ANTLR_UINT32	RewriteRuleElementStream<ImplTraits, SuperType>::size()
{
	ANTLR_UINT32   n = 0;

	/// Should be a count of one if singleElement is set. I copied this
	/// logic from the java implementation, which I suspect is just guarding
	/// against someone setting singleElement and forgetting to NULL it out
	///
	if ( m_singleElement != NULL)
	{
		n = 1;
	}
	else
	{
		if ( !m_elements.empty() )
		{
			return (ANTLR_UINT32)(m_elements.size());
		}
	}
	return n;

}

template<class ImplTraits, class SuperType>
typename RewriteRuleElementStream<ImplTraits, SuperType>::StringType
RewriteRuleElementStream<ImplTraits, SuperType>::getDescription()
{
	if ( m_elementDescription.empty() )
	{
		m_elementDescription = "<unknown source>";
	}
	return  m_elementDescription;
}

template<class ImplTraits, class SuperType>
RewriteRuleElementStream<ImplTraits, SuperType>::~RewriteRuleElementStream()
{
	TreeType* tree;

    // Before placing the stream back in the pool, we
	// need to clear any vector it has. This is so any
	// free pointers that are associated with the
	// entires are called. However, if this particular function is called
    // then we know that the entries in the stream are definately
    // tree nodes. Hence we check to see if any of them were nilNodes as
    // if they were, we can reuse them.
	//
	if	( !m_elements.empty() )
	{
        // We have some elements to traverse
        //
        ANTLR_UINT32 i;

        for (i = 1; i<= m_elements.size(); i++)
        {
            tree = m_elements.at(i-1);
            if  ( (tree != NULL) && tree->isNilNode() )
            {
                // Had to remove this for now, check is not comprehensive enough
                // tree->reuse(tree);
            }
        }
		m_elements.clear();
	}
	else
	{
        if  (m_singleElement != NULL)
        {
            tree = m_singleElement;
            if  (tree->isNilNode())
            {
                // Had to remove this for now, check is not comprehensive enough
              //   tree->reuse(tree);
            }
        }
        m_singleElement = NULL;
	}
}

ANTLR_END_NAMESPACE()
