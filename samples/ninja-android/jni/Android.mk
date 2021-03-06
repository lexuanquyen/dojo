
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE := game
LOCAL_ARM_MODE := arm

DOJO_PATH := $(LOCAL_PATH)/../../..

############################################################################
############################################################################
#DOJO DIP
#paths
DIP_INCLUDE_PATH_DIP := $(DOJO_PATH)/dependencies-android/include
DIP_INCLUDE_PATH_AND := $(DOJO_PATH)/include/dojo/android
DIP_INCLUDE_PATH_DOJ := $(DOJO_PATH)/include/dojo/
DIP_INCLUDE_PATH := $(DOJO_PATH)/include
DIP_LIB_PATH := $(DOJO_PATH)/dependencies-android/lib
#libs
LOCAL_LDLIBS:=  $(DOJO_PATH)/Android/obj/local/armeabi/libdojo-android.a\
				$(DIP_LIB_PATH)/libopenal.a\
				$(DIP_LIB_PATH)/libzzip.a\
                $(DIP_LIB_PATH)/libvorbis.a\
                $(DIP_LIB_PATH)/libfreetype2.a\
                $(DIP_LIB_PATH)/libzzip.a\
                $(DIP_LIB_PATH)/libjpeg.a\
                $(DIP_LIB_PATH)/libPocoFoundation.a\
                -L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/ \
                -llog -lz -landroid -lGLESv1_CM -landroid -lEGL  -s\
                -lgnustl_static -lsupc++
 
#openGL ES 2
#LOCAL_LDLIBS += -lGLESv2
#LOCAL_CFLAGS += -DDEF_SET_OPENGL_ES2

LOCAL_C_INCLUDES:=$(DIP_INCLUDE_PATH)
LOCAL_C_INCLUDES+=$(DIP_INCLUDE_PATH_DOJ)
LOCAL_C_INCLUDES+=$(DIP_INCLUDE_PATH_AND)
LOCAL_C_INCLUDES+=$(DIP_INCLUDE_PATH_DIP)

#openGL extra propriety
LOCAL_CFLAGS += -D__STDC_LIMIT_MACROS -DGL_GLEXT_PROTOTYPES
#LOCAL_CFLAGS += -g -ggdb //debug
#LOCAL_CFLAGS += -ffast-math -O3
LOCAL_CPPFLAGS   += -std=gnu++11 -frtti -fexceptions
############################################################################
############################################################################
		

#GAME MAKE
#c/cpp files
GAME_PATH_ANDROID := $(LOCAL_PATH)
GAME_PATH_ANDROID_INCLUDE := $(LOCAL_PATH)

#c/cpp game files
GAME_CPP:=$(wildcard $(GAME_PATH_ANDROID)/*.cpp)
GAME_C:=$(wildcard $(GAME_PATH_ANDROID)/*.c)
#h files
GAME_H+=$(wildcard $(GAME_PATH_ANDROID_INCLUDE)/*.h)

#add cpp file
LOCAL_SRC_FILES+=$(GAME_CPP:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(GAME_C:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(GAME_H:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES+=$(GAME_H:$(LOCAL_PATH)/%=%)


include $(BUILD_SHARED_LIBRARY)



