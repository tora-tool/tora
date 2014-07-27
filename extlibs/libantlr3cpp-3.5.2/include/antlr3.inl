ANTLR_BEGIN_NAMESPACE()

//static 
ANTLR_INLINE void GenericStream::displayRecognitionError( const StringType& str )
{
	fprintf(stderr, str.c_str() );
}

ANTLR_END_NAMESPACE()