#include "stdafx.h"

#include "Font.h"
#include "Platform.h"
#include "Table.h"
#include "FontSystem.h"
#include "Timer.h"

using namespace Dojo;

void Font::_blit( Dojo::byte* dest, FT_Bitmap* bitmap, uint x, uint y, uint destside )
{
	DEBUG_ASSERT( dest );
	DEBUG_ASSERT( bitmap );

	uint rowy, idx;
	if( bitmap->pixel_mode != FT_PIXEL_MODE_MONO )
	{
		for( int i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( int j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				dest[ idx + 3 ] = bitmap->buffer[ j + i * bitmap->pitch ];  //the font is really the alpha
			}
		}
	}
	else
	{
		Dojo::byte b;
		for( int i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( int j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				b = bitmap->buffer[ j/8 + i * bitmap->pitch ];
				b &= 1 << (7-(j%8));

				dest[ idx + 3 ] = (b > 0) ? 0xff : 0;  //the font is really the alpha
			}
		}
	}
}

void Font::_blitborder( Dojo::byte* dest, FT_Bitmap* bitmap, uint x, uint y, uint destside, const Color& col )
{
	DEBUG_ASSERT( dest );
	DEBUG_ASSERT( bitmap );

	uint rowy;
	Dojo::byte* ptr;

	for( int i = 0; i < bitmap->rows; ++i )
	{
		rowy = ( i + y ) * destside;

		for( int j = 0; j < bitmap->width; ++j )
		{
			ptr = dest + 4*( (j+x) + rowy );

			float a = (float)bitmap->buffer[ j + i * bitmap->pitch ];
			a /= 255.f;
			float inva = 1.f - a;

			ptr[0] = (Dojo::byte)(ptr[0] * inva + a * col.r * 255.f);
			ptr[1] = (Dojo::byte)(ptr[1] * inva + a * col.g * 255.f);
			ptr[2] = (Dojo::byte)(ptr[2] * inva + a * col.b * 255.f);
			ptr[3] = (Dojo::byte)(ptr[3] * inva + a * a * 255.f);  //the font is really the alpha
		}
	}
}

void Font::Character::init( Page* p, unichar c, int x, int y, int sx, int sy, FT_Glyph_Metrics* metrics )
{
	DEBUG_ASSERT( p );
	DEBUG_ASSERT( metrics );

	float fsx = (float)sx;
	float fsy = (float)sy;
	float fw = (float)p->getFont()->getFontWidth();
	float fh = (float)p->getFont()->getFontHeight();
	Font* f = p->getFont();

	page = p;
	character = c;
	gliphIdx = f->getCharIndex( this );

	pixelWidth = (uint)( (float)(metrics->width ) * FONT_PPI );
	float pixelHeight = (float)metrics->height * FONT_PPI;

	uvWidth = (float)pixelWidth / fsx;
	uvHeight = pixelHeight / fsy;
	uvPos.x = (float)x / fsy;
	uvPos.y = (float)y / fsy;
	widthRatio = (float)pixelWidth / fw;
	heightRatio = pixelHeight / fh;

	bearingU = ((float)metrics->horiBearingX * FONT_PPI ) / fsy;
	bearingV = ((float)(metrics->height - metrics->horiBearingY) * FONT_PPI ) / fh;

	advance = ((float)metrics->horiAdvance * FONT_PPI ) / fw;
}


Font::Page::Page( Font* f, int idx ) :
index( idx ),
firstCharIdx( index * FONT_CHARS_PER_PAGE ),
font( f )
{
	DEBUG_ASSERT( font );

	//create the texture
	int sx = font->mCellWidth * FONT_PAGE_SIDE;
	int sy = font->mCellHeight * FONT_PAGE_SIDE;

	int sxp2 = Math::nextPowerOfTwo( sx );
	int syp2 = Math::nextPowerOfTwo( sy );

	byte* buf = (byte*)malloc( sxp2 * syp2 * 4 );
	
	unsigned int * ptr = (unsigned int*)buf;
	//set alpha to 0 and colours to white
	for( int i = sxp2*syp2-1; i >= 0; --i )
		*ptr++ = 0x00ffffff;

	//render into buffer
	FT_Render_Mode renderMode = ( font->isAntialiased() ) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
	FT_GlyphSlot slot = font->face->glyph;
	FT_Bitmap* bitmap;
	uint gliphidx;
	int err;

	font->_prepareFace();

	unichar c = firstCharIdx;
	Font::Character* character = chars;
	uint x, y;

	Timer timer;

	for( uint i = 0; i < FONT_PAGE_SIDE; ++i )
	{
		for( uint j = 0; j < FONT_PAGE_SIDE; ++j, ++c, ++character )
		{
			gliphidx = FT_Get_Char_Index( font->face, c );
			err = FT_Load_Glyph( font->face, gliphidx, FT_LOAD_DEFAULT );

			x = j * font->mCellWidth;
			y = i * font->mCellHeight;

			//load character data
			character->init( 
				this, 
				c, 
				x, y,
				sxp2, syp2, 
				&(slot->metrics) );

			FT_Render_Glyph( slot, renderMode );

			bitmap = &(slot->bitmap);

			//blit the bitmap if it was rendered
			if( bitmap->buffer )
				_blit( buf, bitmap, x + font->glowRadius, y + font->glowRadius, sxp2 );
		}
	}
	
	//glow?
	unsigned int glowCol = font->glowColor.toRGBA();
	byte* glowColChannel = (byte*)&glowCol;
	
	for( int iteration = 0; iteration < font->glowRadius; ++iteration )
	{
		for( int i = 1; i < syp2-1; ++i )
		{
			for( int j = 1; j < sxp2-1; ++j )
			{
				byte* cur = buf + (j + i * sxp2) * 4;
				byte* up = buf + (j + (i+1)*sxp2) * 4;
				byte* down = buf + (j + (i-1)*sxp2) * 4;
				byte* left = buf + (j+1 + i*sxp2) * 4;
				byte* right = buf + (j-1 + i*sxp2) * 4;
				
				byte alpha = cur[3];
				byte blur = (byte)((float)(up[3] + down[3] + left[3] + right[3]) * 0.25f);

				if( blur > alpha )
				{					
					/*cur[0] = 255*(1.f-s) + s * glowColChannel[0];
					cur[1] = 255*(1.f-s) + s * glowColChannel[1];
					cur[2] = 255*(1.f-s) + s * glowColChannel[2];*/
					
					cur[0] = cur[1] = cur[2] = 10;
					
					cur[3] = blur;
				}
			}
		}
	}
	
	//drop the buffer in the texture
	texture = new Texture( NULL, String::EMPTY );
	texture->loadFromMemory( buf, sxp2, syp2, GL_RGBA, GL_RGBA );
	texture->disableTiling();

	free( buf );
}

/// --------------------------------------------------------------------------------

/// --------------------------------------------------------------------------------

Font::Font( const String& path )
{			
	//miss all the pages
	memset( pages, 0, sizeof( void* ) * FONT_MAX_PAGES );

	//load table
	Table t;
	Platform::getSingleton()->load( &t, path );
	
	fontName = t.getName();

	fontFile = Utils::getDirectory( path ) + '/' + t.getString( "truetype" );
	fontWidth = fontHeight = t.getInt( "size" );	

	antialias = t.getBool( "antialias" );
	kerning = t.getBool( "kerning" );
	spacing = t.getNumber( "spacing" );
	
	glowRadius = t.getInt( "glowRadius" );
	glowColor = t.getColor( "glowColor" );
	
	mCellWidth = fontWidth + glowRadius * 2;
	mCellHeight = fontHeight + glowRadius * 2;

	face = Platform::getSingleton()->getFontSystem()->getFace( fontFile );

	Table* preload = t.getTable( "preloadedPages" );
	for( int i = 0; i < preload->getAutoMembers(); ++i )
		getPage( preload->getInt( i ) );
}

Font::~Font()
{
	unload();
}

float Font::getKerning( Character* next, Character* prev )
{
	DEBUG_ASSERT( kerning );
	DEBUG_ASSERT( next );
	DEBUG_ASSERT( prev );

	FT_Vector vec;

	FT_Get_Kerning( 
		face,
		prev->gliphIdx,
		next->gliphIdx,
		FT_KERNING_DEFAULT,
		&vec );

	return ((float)vec.x * FONT_PPI) / (float)fontWidth;
}

void Font::_prepareFace()
{
	//set dimensions
	FT_Set_Pixel_Sizes(
		face,
		fontWidth,
		fontHeight );
}
