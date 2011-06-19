#include "stdafx.h"

#include "dojo/FrameSet.h"

#include "dojo/ResourceGroup.h"
#include "dojo/Platform.h"

using namespace Dojo;

bool FrameSet::load()
{			
	if( isLoaded() )
		return true;
	
	for( uint i = 0; i < frames.size(); ++i )
	{
		if( frames.at(i)->load() )
	
			// count bytesize
			size += frames.at(i)->getByteSize();
	}		
		
	loaded = true;		
		
	return true;	
}


bool FrameSet::loadAtlas( Table* data, ResourceGroup* atlasTextureProvider )
{
	DEBUG_ASSERT( data );
	DEBUG_ASSERT( atlasTextureProvider );

	if( isLoaded() )
		return true;
	
	//get atlas texture
	FrameSet* atlasSet = atlasTextureProvider->getFrameSet( data->getString( "texture" ) );	

	DEBUG_ASSERT( atlasSet );

	Texture* atlas = atlasSet->getFrame(0);

	Table* tiles = data->getTable( "tiles" );

	uint x, y, sx, sy;
	for( uint i = 0; i < tiles->getAutoMembers(); ++i )
	{
		Table* tile = tiles->getTable( tiles->autoMember(i) );

		x = tile->getInt( tile->autoMember(0) );
		y = tile->getInt( tile->autoMember(1) );
		sx = tile->getInt( tile->autoMember(2) );
		sy = tile->getInt( tile->autoMember(3) );

		Texture* tiletex = new Texture( NULL, String::EMPTY );

		if( tiletex->loadFromAtlas( atlas, x,y, sx,sy ) )			
			addTexture( tiletex, true );
	}
		
	//loaded at least one?
	loaded = frames.size() > 0;
	
	return true;
}

