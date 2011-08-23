//
//  EAGLView.h
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

/*
#import <MessageUI/MFMailComposeViewController.h>
*/
 
#include <dojo/Render.h>
#include <dojo/SoundManager.h>
#include <dojo/InputSystem.h>
#include <dojo/Timer.h>

namespace Dojo {
	class Game;
	class IOSPlatform;
}

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
//#ifdef OPENFEINT_ENABLED
//@interface Application : UIView < UIAccelerometerDelegate, ApplicationSetupDelegate, OpenFeintDelegate >
//#elseif
@interface Application : UIView < UIAccelerometerDelegate > //, MFMailComposeViewControllerDelegate >
//#endif
{    
@protected	
	Dojo::IOSPlatform* platform;
	
	Dojo::Render* renderImpl;
	Dojo::InputSystem* touchSource;
	
	Dojo::Timer frameTimer;
	
@private
	float lastAccelerationX, lastAccelerationY, lastRoll;
	
	BOOL animating;
	BOOL displayLinkSupported;
	BOOL displayVisible;
	// Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	// CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	// The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
	// isn't available.
	id displayLink;
    NSTimer *animationTimer;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;

- (void) initialise;
- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

- (BOOL)canBecomeFirstResponder;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event;

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;
/*
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error;
*/
@end
