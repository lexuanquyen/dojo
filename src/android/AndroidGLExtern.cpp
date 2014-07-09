#include "AndroidGLExtern.h"
#include "DebugUtils.h"

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT=NULL;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT=NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT=NULL;

void ExternInitOpenGL(){
	
        DEBUG_MESSAGE("set ext ExternInitOpenGL");

	glGenVertexArraysOESEXT=(PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	DEBUG_ASSERT(glGenVertexArraysOESEXT)

	glBindVertexArrayOESEXT=(PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	DEBUG_ASSERT(glBindVertexArrayOESEXT)

	glDeleteVertexArraysOESEXT=(PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
	DEBUG_ASSERT(glDeleteVertexArraysOESEXT)


        DEBUG_MESSAGE("glGenVertexArraysOESEXT "<< (int)glGenVertexArraysOESEXT);
        DEBUG_MESSAGE("glBindVertexArrayOESEXT "<< (int)glBindVertexArrayOESEXT);
        DEBUG_MESSAGE("glDeleteVertexArraysOESEXT "<< (int)glDeleteVertexArraysOESEXT);

}
