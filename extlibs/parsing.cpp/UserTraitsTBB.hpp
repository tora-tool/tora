#define TBB_PREVIEW_MEMORY_POOL 1
#include "tbb/memory_pool.h"

namespace Antlr3Impl {
  
	extern char buf[];
	extern tbb::fixed_pool interim_pool;
	
	class TBBAllocPolicy
	{
	public:
		//limitation of c++. unable to write a typedef 
		template <class TYPE>
		class AllocatorType : public tbb::memory_pool_allocator<TYPE>	
		{
			typedef tbb::memory_pool_allocator<TYPE> super;
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

			ANTLR_INLINE void deallocate( pointer, size_type ) {}		
		
			AllocatorType() throw() : super(interim_pool) {}
			AllocatorType( const AllocatorType& alloc ) throw() : super(interim_pool) {}
			template<typename U> AllocatorType(const AllocatorType<U>& alloc) throw() : super(interim_pool) {}
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
			ANTLR_INLINE void push( const TYPE& elem ) {  this->push_back(elem); 	}
			ANTLR_INLINE void pop()  { this->pop_back(); }
			ANTLR_INLINE TYPE& peek() { return this->back(); }
			ANTLR_INLINE TYPE& top() { return this->back(); }
			ANTLR_INLINE const TYPE& peek() const { return this->back(); }
			ANTLR_INLINE const TYPE& top() const { return this->back(); }
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

		ANTLR_INLINE static void* operator new (std::size_t bytes) 
		{ 
			void* p = alloc(bytes);
			return p;
		}
		ANTLR_INLINE static void* operator new (std::size_t , void* p) { return p; }
		ANTLR_INLINE static void* operator new[]( std::size_t bytes)
		{
			void* p = alloc(bytes); 
			return p;
		}
		ANTLR_INLINE static void operator delete(void* p)
		{
			// TBBAllocPolicy::free(p);
		}
		ANTLR_INLINE static void operator delete(void* , void* ) {} //placement delete
		
		ANTLR_INLINE static void operator delete[](void* p)
		{
			// TBBAllocPolicy::free(p);
		}
		
		ANTLR_INLINE static void* alloc( std::size_t bytes )
		{
			void* p = interim_pool.malloc(bytes); 
			if( p== NULL )
				throw std::bad_alloc();
			return p;
		}

		ANTLR_INLINE static void* alloc0( std::size_t bytes )
		{
			void* p = TBBAllocPolicy::alloc(bytes); 
			memset(p, 0, bytes );
			return p;
		}
		
		ANTLR_INLINE static void  free( void* p )
		{
			// return interim_pool.free(p);
		}
	
		ANTLR_INLINE static void* realloc(void *ptr, size_t size)
		{
			return interim_pool.realloc( ptr, size );
		}
	}; // class TBBAllocPolicy
	
	template<class ImplTraits>
	class TBBTraits: public antlr3::CustomTraitsBase<ImplTraits>
	{
	public:
		typedef std::basic_string<char, std::char_traits<char>, TBBAllocPolicy::AllocatorType<char> > StringType;
		typedef std::basic_stringstream<char, std::char_traits<char>, TBBAllocPolicy::AllocatorType<char> > StringStreamType;
		typedef TBBAllocPolicy AllocPolicyType;	  
		ANTLR_INLINE static void displayRecognitionError( const StringType& str ) {  printf("%s", str.c_str() ); }
	};
  
	typedef antlr3::Traits<PLSQLLexer, PLSQLParser, TBBTraits> PLSQLTraits;
};
