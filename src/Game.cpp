#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "dojomath.h"

using namespace Dojo;

Game::Game( const String& gamename, uint w, uint h, Render::RenderOrientation r, float nativedt, float maximumdt ) :
name( gamename ),
nativeWidth( w ),
nativeHeight( h ),
nativeOrientation( r ),
mNativeFrameLength( nativedt ),
mMaxFrameLength( maximumdt )
{	
	DEBUG_ASSERT( name.size() );
	DEBUG_ASSERT( w > 0 );
	DEBUG_ASSERT( h > 0 );
	DEBUG_ASSERT( r <= Render::RO_LANDSCAPE_RIGHT );
	DEBUG_ASSERT( mNativeFrameLength > 0 );
	DEBUG_ASSERT( mMaxFrameLength >= mNativeFrameLength );

	Math::seedRandom();
	
	focusListeners.add( this );
}
	
Game::~Game()
{

}

