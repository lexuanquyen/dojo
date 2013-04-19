#include "stdafx.h"

#include "SoundBuffer.h"

#include "SoundManager.h"
#include "MemoryInputStream.h"
#include "FileStream.h"
#include "Utils.h"
#include "Platform.h"

using namespace Dojo;

#define OGG_ENDIAN 0

//store the callbacks
ov_callbacks SoundBuffer::VORBIS_CALLBACKS = 
{ 
	SoundBuffer::_vorbisRead, 
	SoundBuffer::_vorbisSeek, 
	SoundBuffer::_vorbisClose, 
	SoundBuffer::_vorbisTell 
};

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const String& path ) :
Resource( creator, path ),
size(0),
mDuration( 0 )
{
	DEBUG_ASSERT( creator, "SoundBuffer needs a creator object" );
}

SoundBuffer::~SoundBuffer()
{

}

bool SoundBuffer::onLoad()
{
	DEBUG_ASSERT( isLoaded() == false, "The SoundBuffer is already loaded" );
	
	String ext = Utils::getFileExtension( filePath );
	
	DEBUG_ASSERT( ext == String( "ogg" ), "Sound file extension is not ogg" );
	
	_loadOggFromFile();
			
	return CHECK_AL_ERROR;
}


void SoundBuffer::onUnload(bool soft)
{
	DEBUG_ASSERT( isLoaded(), "SoundBuffer is not loaded" );

	//just push the event to all its chunks
	for( auto chunk : mChunks )
		chunk->onUnload( soft );
}

////-------------------------------------////-------------------------------------////------------------------------------
//	STATIC VORBIS CALLBACK METHODS

size_t SoundBuffer::_vorbisRead( void* out, size_t size, size_t count, void* userdata )
{
	Stream* source = (Stream*)userdata;

	return source->read( (byte*)out, size * count );
}

int SoundBuffer::_vorbisSeek( void *userdata, ogg_int64_t offset, int whence )
{
	Stream* source = (Stream*)userdata;

	return source->seek( (long)offset, whence );
}

int SoundBuffer::_vorbisClose( void *userdata )
{
	Stream* source = (Stream*)userdata;

	source->close();
	return 0;
}

long SoundBuffer::_vorbisTell( void *userdata )
{
	return ((Stream*)userdata)->getCurrentPosition();
}

bool SoundBuffer::Chunk::onLoad()
{
	DEBUG_ASSERT( !isLoaded(), "The Chunk is already loaded" );

	alGenBuffers( 1, &alBuffer ); //gen the buffer if it didn't exist

	CHECK_AL_ERROR;

	//reopen the source
	Stream* source = pParent->mSource;
	if( !source->isOpen() )
		source->open();

	DEBUG_ASSERT( source->isReadable(), "The data source for the Ogg stream could not be open, or isn't readable" );

	CHECK_AL_ERROR;

	char* uncompressedData = (char*)malloc( mUncompressedSize );

	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int totalRead = 0;

	int error = ov_open_callbacks( source, &file, NULL, 0, VORBIS_CALLBACKS );

	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );

	info = ov_info( &file, -1 );

	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	int bitrate = info->rate;// * info->channels;

	//read all vorbis packets in the same buffer
	long read = 0;

	//seek to the start of the file segment
	error = ov_raw_seek( &file, mStartPosition );
	DEBUG_ASSERT( error == 0, "Cannot seek into file" );

	bool corrupt = false;
	do
	{
		int section = -1;
		read = ov_read( &file, uncompressedData + totalRead, mUncompressedSize - totalRead, 0, wordSize, 1, &section );

		if( read == OV_HOLE || read == OV_EBADLINK || read == OV_EINVAL )
			corrupt = true;

		else if( read == 0 )
			break;

		else
			totalRead += read;

		DEBUG_ASSERT( totalRead <= mUncompressedSize, "Total read bytes overflow the buffer" ); //this should always be true

	} while( !corrupt && totalRead < mUncompressedSize );

	ov_clear( &file );

	DEBUG_ASSERT( !corrupt, "an ogg vorbis stream was corrupt and could not be read" );
	DEBUG_ASSERT( totalRead > 0, "no data was read from the stream" );

	alBufferData( alBuffer, format, uncompressedData, totalRead, bitrate );

	loaded = CHECK_AL_ERROR;

	free( uncompressedData );

	return loaded;
}

void SoundBuffer::Chunk::loadAsync()
{
	DEBUG_ASSERT( !isLoaded(), "The Chunk is already loaded" );

	//async load
	Platform::getSingleton()->getBackgroundQueue().queueTask( [ & ]()
	{
		onLoad();
	} );
}

void SoundBuffer::Chunk::onUnload( bool soft /* = false */ )
{
	DEBUG_ASSERT( isLoaded(), "Tried to unload an unloaded Chunk" );

	//either unload if forced, or if the parent is reloadable (loaded from file or persistent source)
	if( !soft || pParent->isReloadable() )
	{
		alDeleteBuffers( 1, &alBuffer );
		alBuffer = 0;

		loaded = false;

		CHECK_AL_ERROR;
	}
}

bool SoundBuffer::_loadOgg( Stream* source )
{
	DEBUG_ASSERT( source, "the data source cannot be null" );

	if( !source->open() )
	{
		DEBUG_ASSERT( mSource->isReadable(), "The data source for the Ogg stream could not be open, or isn't readable" );

		return false;
	}

	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	ogg_int64_t uncompressedSize;
	
	int error = ov_open_callbacks( source, &file, NULL, 0, VORBIS_CALLBACKS );
	
	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	int bitrate = info->rate;// * info->channels;
	ogg_int64_t totalPCM = ov_pcm_total( &file, -1 );
	int section = -1;

	uncompressedSize = totalPCM * wordSize * info->channels;

	//find the number of chunks that we want
	int chunkN = 1;
	for( ; uncompressedSize / chunkN > Chunk::MAX_SIZE; chunkN++ );
	ogg_int64_t chunkPCM = totalPCM / chunkN;

	mDuration = (float)totalPCM / (float)info->rate;

	int streamPosition = 0;
	ogg_int64_t fileStart = 0, fileEnd = -1;
	ogg_int64_t pcmStart = 0, pcmEnd = -1;

	//load all the needed chunks
	while(1)
	{
		pcmEnd = min( totalPCM, pcmStart + chunkPCM );
		ov_pcm_seek_page( &file, pcmEnd );

		pcmEnd = ov_pcm_tell( &file );
		fileEnd = ov_raw_tell( &file );

		if( fileStart == fileEnd ) //check if EOF
			break;

		if( pcmEnd == pcmStart ) //buffer is too small, let's assume that there is just one page
			pcmEnd = totalPCM; 

		ogg_int64_t byteSize = (pcmEnd-pcmStart) * wordSize * info->channels;
		mChunks.add( new Chunk( this, (long)fileStart, (long)byteSize ) );

		pcmStart = pcmEnd;
		fileStart = fileEnd;
	}

	ov_clear( &file );

	if( !isStreaming() )
		mChunks[0]->get(); //get() it to avoid that it is unloaded by the sources, and load synchronously

	return true;
}

bool SoundBuffer::_loadOggFromFile()
{
	mSource = Platform::getSingleton()->getFile( filePath );
	
	_loadOgg( mSource );

	return isLoaded();
}
