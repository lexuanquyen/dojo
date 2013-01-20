#ifndef FontSystem_h__
#define FontSystem_h__

#include "dojo_common_header.h"

#include "Utils.h"
#include "Platform.h"

namespace Dojo
{
	class FontSystem
	{
	public:

		typedef std::unordered_map< String, FT_Face > FaceMap;

		FontSystem()
		{
			//launch FreeType
			int err = FT_Init_FreeType( &freeType );

			DEBUG_ASSERT( err == 0 );
		}

		virtual ~FontSystem()
		{
			FT_Done_FreeType( freeType );
		}

		FT_Face getFace( const String& fileName )
		{
			FaceMap::iterator where = faceMap.find( fileName );

			if( where == faceMap.end() )
				return _createFaceForFile( fileName );
			else
				return where->second;
		}

		FT_Stroker getStroker( float width )
		{
			FT_Stroker s;
			FT_Stroker_New( freeType, &s );

			FT_Stroker_Set( s, 
				(FT_Fixed)(width * 64.f), 
				FT_STROKER_LINECAP_ROUND, 
				FT_STROKER_LINEJOIN_ROUND, 
				0 ); 

			return s;
		}

	protected:

		FaceMap faceMap;

		FT_Library freeType;

		FT_Face _createFaceForFile( const String& fileName )
		{
			char* buf;
			FT_Long size = Platform::getSingleton()->loadFileContent( buf, fileName );

			//create new face from memory - loading from memory is needed for zip loading
			FT_Face face;
			int err = FT_New_Memory_Face( freeType, (FT_Byte*)buf, size, 0, &face );
			faceMap[ fileName ] = face;

			DEBUG_ASSERT( err == 0 );

			free( buf );
			return face;
		}

	private:
	};
}
#endif // FontSystem_h__