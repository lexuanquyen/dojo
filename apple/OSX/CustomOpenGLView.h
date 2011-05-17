//
//  StepCallback.h
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSEvent.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AppKit/NSOpenGLView.h>

namespace Dojo
{
	class Platform;
	class TouchSource;
}

@interface CustomOpenGLView : NSOpenGLView<NSWindowDelegate> {
@private
	Dojo::Platform* platform;
	CFAbsoluteTime time;
    
	Dojo::TouchSource* input;
}

- (void)setPlatform:(Dojo::Platform*)targetPlatform;

//window callbacks
- (void)windowWillClose:(NSNotification *)notification;

//timer callback
- (void) stepCallback:(NSTimer *)timer;

- (void)mouseDown: (NSEvent *)theEvent;

- (void)mouseDragged: (NSEvent *)theEvent;

- (void)mouseUp: (NSEvent *)theEvent;

- (void)mouseMoved: (NSEvent *)theEvent;

- (void)rightMouseDown: (NSEvent *)theEvent;

- (void)rightMouseDragged: (NSEvent *)theEvent;

- (void)rightMouseUp: (NSEvent *)theEvent;

- (void)otherMouseDown: (NSEvent *)theEvent;

- (void)otherMouseDragged: (NSEvent *)theEvent;

- (void)otherMouseUp: (NSEvent *)theEvent;

@end
