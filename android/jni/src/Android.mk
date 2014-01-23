LOCAL_PATH := $(TUXRACER_HOME)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_C_INCLUDES += $(SDL2_HOME)/include
LOCAL_C_INCLUDES += $(SDL2_MIXER_HOME)
LOCAL_C_INCLUDES += $(GAMEMENU_HOME)/src
LOCAL_C_INCLUDES += $(TCL_HOME)/generic
LOCAL_C_INCLUDES += $(GLM_HOME)

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL2_HOME)/src/main/android/SDL_android_main.c
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/src/*.c)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/src/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/src/android/*.c)
LOCAL_SRC_FILES += $(wildcard $(GAMEMENU_HOME)/src/*.c)

LOCAL_CFLAGS += -DSDL_PREFIX=com_moonlite_tuxracer

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_mixer
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += scoreloopcore
LOCAL_SHARED_LIBRARIES += tcl

LOCAL_LDLIBS := -llog -ldl -lc -lm -lGLESv2 -lEGL

include $(BUILD_SHARED_LIBRARY)
