
#include <boost/pool/pool_alloc.hpp>

namespace Antlr3BackendImpl {

	class BoostAllocPolicy
	{
	public:
		//limitation of c++. unable to write a typedef 
		template <class TYPE>
		class AllocatorType : public boost::fast_pool_allocator<TYPE>
		{
		public:
			typedef TYPE value_type;
			typedef value_type* pointer;
			typedef const value_type* const_pointer;
			typedef value_type& reference;
			typedef const value_type& const_reference;
			typedef size_t size_type;
			typedef ptrdiff_t difference_type;
			template<class U> struct rebind {
				typedef AllocatorType<U> other;
			};
		
			AllocatorType() throw() {}
			AllocatorType( const AllocatorType& alloc ) throw() {}
			template<typename U> AllocatorType(const AllocatorType<U>& alloc) throw(){}
		};

		template<class TYPE>
		class VectorType : public std::vector< TYPE, AllocatorType<TYPE> >
		{
		};
	
		template<class TYPE>
		class ListType : public std::deque< TYPE, AllocatorType<TYPE> >
		{
		};	

		template<class TYPE>
		class StackType : public std::deque< TYPE, AllocatorType<TYPE> >
		{
		public:
			void push( const TYPE& elem ) {  this->push_back(elem); 	}
			void pop()  { this->pop_back(); }
			TYPE& peek() { return this->back(); }
			TYPE& top() { return this->back(); }
			const TYPE& peek() const { return this->back(); }
			const TYPE& top() const { return this->back(); }
		};	


		template<class TYPE>
		class OrderedSetType : public std::set< TYPE, std::less<TYPE>, AllocatorType<TYPE> >
		{
		};

		template<class TYPE>
		class UnOrderedSetType : public std::set< TYPE, std::less<TYPE>, AllocatorType<TYPE> >
		{
		};

		template<class KeyType, class ValueType>
		class UnOrderedMapType : public std::map< KeyType, ValueType, std::less<KeyType>, 
							  AllocatorType<std::pair<KeyType, ValueType> > >
		{
		};

		template<class KeyType, class ValueType>
		class OrderedMapType : public std::map< KeyType, ValueType, std::less<KeyType>, 
							AllocatorType<std::pair<KeyType, ValueType> > >
		{
		};

		static void* operator new (std::size_t bytes) 
		{ 
			void* p = alloc(bytes);
			return p;
		}
		static void* operator new (std::size_t , void* p) { return p; }
		static void* operator new[]( std::size_t bytes)
		{
			void* p = alloc(bytes); 
			return p;
		}
		static void operator delete(void* p)
		{
			BoostAllocPolicy::free(p);
		}
		static void operator delete(void* , void* ) {} //placement delete

		static void operator delete[](void* p)
		{
			BoostAllocPolicy::free(p);
		}

		static void* alloc( std::size_t bytes )
		{
			void* p = malloc(bytes); 
			if( p== NULL )
				throw std::bad_alloc();
			return p;
		}

		static void* alloc0( std::size_t bytes )
		{
			void* p = BoostAllocPolicy::alloc(bytes); 
			memset(p, 0, bytes );
			return p;
		}

		static void  free( void* p )
		{
			return ::free(p);
		}
	
		static void* realloc(void *ptr, size_t size)
		{
			return ::realloc( ptr, size );
		}
	}; // class BoostAllocPolicy
  
	template<class ImplTraits>
	class BoostTraits: public antlr3::CustomTraitsBase<ImplTraits>
	{
	public:
		typedef std::basic_string<char, std::char_traits<char>, boost::fast_pool_allocator<char> > StringType;
		typedef std::basic_stringstream<char, std::char_traits<char>, boost::fast_pool_allocator<char> > StringStreamType;
		typedef BoostAllocPolicy AllocPolicyType;
		static void displayRecognitionError( const StringType& str ) {  printf("%s", str.c_str() ); }
		///static void displayRecognitionError( const std::string& str ) {  printf("%s", str.c_str() ); }	
	};
	
	typedef antlr3::Traits<PLSQLLexer, PLSQLParser, BoostTraits> PLSQLTraits;
};

