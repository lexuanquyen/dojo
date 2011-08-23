//
//  ApplePlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//


#ifndef __ApplePlatform_h
#define __ApplePlatform_h

#include "dojo_common_header.h"

#include "Platform.h"
#include "FrameworkTests.h"

#ifdef __OBJC__
	#import <Foundation/NSAutoreleasePool.h>
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSAutoreleasePool;
	class NSString;
#endif
	
    class ApplePlatform : public Platform
    {
    public:
		
        ApplePlatform( const Table& config );
		
		virtual ~ApplePlatform();
		
		virtual void initialise()=0;        
		virtual void step( float dt );
		
		virtual String getCompleteFilePath( const String& name, const String& type, const String& path );
		virtual void getFilePathsForType( const String& type, const String& path, std::vector<String>& out );
		virtual uint loadFileContent( char*& bufptr, const String& path );
		
		virtual String getAppDataPath();
				
		virtual void loadPNGContent( void*& imageData, const String& path, uint& width, uint& height );		
		virtual uint loadAudioFileContent( ALuint& buffer, const String& path );
				
		virtual void load( Table* dest, const String& absPath = String::EMPTY );
		virtual void save( Table* table, const String& absPath = String::EMPTY );
		
		virtual void openWebPage( const String& site );
        
    protected:
		
		FrameworkTests testSuite;
		
		NSString* _getFullPath( const String& path );
		
		NSString* _getDestinationFilePath( Table* t, const String& absPath = String::EMPTY );
		
		void _createApplicationDirectory();
		
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
       	NSAutoreleasePool* pool;
    };
}

#endif