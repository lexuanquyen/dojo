#ifndef String_h__
#define String_h__

#include "dojo_common_header.h"

#ifdef __OBJC__
	#import <Foundation/NSString.h>
#endif

#define STRING_MAX_FLOAT_DIGITS 6

namespace Dojo 
{
#ifndef PLATFORM_LINUX
	typedef unsigned int uint;
#endif
	typedef unsigned char byte;
	typedef wchar_t unichar;

	//define the unicode stuff
	typedef std::basic_stringstream< unichar > StringStream;

	typedef std::basic_string< unichar > _ustring;

	class String : public _ustring
	{
	public:
		
		static const String EMPTY;
		
		String() :
		_ustring()
		{

		}
		
		String( const String& s ) :
		_ustring( s )
		{
			
		}
		
		String( const _ustring& s ) :
		_ustring( s )
		{
			
		}
		
		String( char s ) :
		_ustring()
		{
			append( 1, (unichar)s );
		}
		
		String( const char * s ) :
		_ustring()
		{
			appendASCII( s );
		}
		
		//converts a string from UTF8
		String( const std::string& utf8 ) :
		_ustring()
		{
			appendUTF8( utf8 );
		}
		
		String( int i, unichar paddingChar = 0 ) :
		_ustring()
		{
			appendInt( i, paddingChar );
		}
		
		String( uint i, unichar paddingChar = 0 ) :
		_ustring()
		{
			appendInt( i, paddingChar );
		}
		
		String( float f ) :
		_ustring()
		{
			appendFloat( f );
		}
		
		///if found, replace the given substr with the given replacement - they can be of different lengths.
		inline void replaceToken( const String& substring, const String& replacement )
		{
			DEBUG_ASSERT( substring.size() );
			
			size_t start = find( substring );
			
			if( start != String::npos )
			{
				String postfix = substr( start + substring.size() );				
				resize( start );
				append( replacement );
				append( postfix );
			}
		}
		
		inline size_t byteSize()
		{
			return size() * sizeof( unichar );
		}
		
		///converts this string into ASCII. WARNING: fails silently on unsupported chars!!!
		inline std::string ASCII() const 
		{
			std::string res;
			
			unichar c;
			for( uint i = 0; i < size(); ++i )
			{
				c = at(i);
				if( c <= 0xff )
					res += (char)c;
			}
			
			return res;
		}
		
		inline std::string UTF8() const 
		{
			//HACK!!!!! make a real parser!!!
			return ASCII();
		}
		
		inline void appendASCII( const char* s )
		{
			DEBUG_ASSERT( s );

			for( int i = 0; s[i] != 0; ++i )
				append( 1, (unichar)s[i] );
		}
		
		inline void appendUTF8( const std::string& utf8 )
		{
			//TODO do it better
			appendASCII( utf8.c_str() );
		}
		
		void appendInt( int i, unichar paddingChar = 0 )
		{
			uint div = 1000000000;
			unichar c;
			
			for( ; div > 0; i %= div, div /= 10 )
			{
				c = '0' + (i / div);
				
				if( c != '0' )  break;
				else if( paddingChar )
					*this += paddingChar;
			}
			
			if( i == 0 )
				*this += '0';
			
			for( ; div > 0; i %= div, div /= 10 )
				*this += '0' + (i / div);
			
		}
		
		inline void appendFloat( float f )
		{
			//TODO this is really slow
			std::stringstream str;
			str << f;
			
			appendASCII( str.str().c_str() );
		}
		
		///appends raw data to this string. It has to be wchar_t bytes aligned!
		inline void appendRaw( void* data, uint sz )
		{
			DEBUG_ASSERT_MSG( sz % sizeof( unichar ) == 0, "Data is not aligned to string elements" );
			
			append( (unichar*)data, sz / sizeof( unichar ) );
		}
				
#ifdef __OBJC__
		inline NSString* toNSString() const 
		{                       
			return [ NSString stringWithCharacters: data() length: size() ];
		}
		
		String( NSString* nss ) :
		_ustring()
		{
			appendNSString( nss );
		}
		
		inline String& appendNSString( NSString* nss )
		{
			DEBUG_ASSERT( nss );
			
			uint sz = size();
			resize( sz + [nss length] );
			
			///copy the whole string verbatim
			[nss getCharacters: (unichar*)data() + sz range: NSMakeRange( 0, [nss length] )];
			
			return *this;
		}
#endif
		
	};
	
	inline String operator+ ( const String& lhs, const String& rhs)
	{
		return String( lhs ).append( rhs );
	}
	
	inline String operator+ (const char* lhs, const String& rhs)
	{
		return String( lhs ) + rhs;
	}
	
	inline String operator+ (char lhs, const String& rhs)
	{
		return String( lhs ) + rhs;
	}
	
	inline String operator+ (const String& lhs, const char* rhs)
	{
		return lhs + String( rhs );
	}
	
	inline String operator+ (const String& lhs, char rhs)
	{
		return lhs + String( rhs );
	}

}

namespace std
{
	///hash specialization for unordered_maps
	template<>
	struct hash<Dojo::String>
		: public unary_function<Dojo::String, size_t>
	{	// hash functor for basic_string
		typedef Dojo::String _Kty;

		size_t operator()(const _Kty& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			return (_Hash_seq((const unsigned char *)_Keyval.c_str(),
				_Keyval.size() * sizeof ( Dojo::unichar )));
		}
	};
}


#endif // String_h__
