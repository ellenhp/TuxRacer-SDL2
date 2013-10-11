LOCAL_PATH := $(TUXRACER_HOME)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_C_INCLUDES += $(SDL2_HOME)/include
LOCAL_C_INCLUDES += $(SDL2_MIXER_HOME)
LOCAL_C_INCLUDES += $(GAMEMENU_HOME)/src
LOCAL_C_INCLUDES += $(TCL_HOME)/generic

# Add your application source files here...
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/src/*.c)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/src/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/src/android/*.c)
LOCAL_SRC_FILES += $(wildcard $(GAMEMENU_HOME)/src/*.c)

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_mixer
LOCAL_SHARED_LIBRARIES += tcl

LOCAL_LDLIBS := -llog -ldl -lc -lm -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)