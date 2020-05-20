load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "sdl",
    srcs = glob(
        [
            "sdl/*.c",
            "sdl/**/*.c",
            "sdl/**/**/*.c",
            "include/*.h",
            "sdl/*.h",
            "sdl/**/*.h",
            "sdl/**/**/*.h",
        ],
        exclude = [
            "sdl/windows/*",
            "sdl/core/windows/*",
            "sdl/main/**/*",
            "sdl/hidapi/**/*",
            "sdl/thread/generic/*.c",
            "sdl/haptic/windows/*",
            "sdl/screen/*",
            "sdl/video/qnx/*",
        ],
    ) + glob([
        "sdl/hidapi/android/*.c*",
    ]) + [
        "sdl/hidapi/hidapi/hidapi.h",
    ],
    hdrs = glob([
        "include/*.h",
    ]),
    copts = [
        "-Ideps/sdl/sdl",
        "-Iexternal/androidndk",
        "-Ideps/sdl/include",
        "-DGL_GLEXT_PROTOTYPES",
    ],
    linkopts = [
        "-ldl",
        "-lGLESv1_CM",
        "-lGLESv2",
        "-lOpenSLES",
        "-llog",
        "-landroid",
    ],
    visibility = ["//visibility:public"],
    deps = ["@androidndk//:cpufeatures"],
    alwayslink = True,
    # [
    # $(wildcard $(LOCAL_PATH)/src/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/audio/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/audio/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/audio/dummy/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/audio/openslES/*.c) \
    # $(LOCAL_PATH)/src/atomic/SDL_atomic.c.arm \
    # $(LOCAL_PATH)/src/atomic/SDL_spinlock.c.arm \
    # $(wildcard $(LOCAL_PATH)/src/core/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/cpuinfo/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/dynapi/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/events/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/file/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/haptic/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/haptic/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/joystick/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/joystick/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/joystick/hidapi/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/loadso/dlopen/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/power/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/power/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/filesystem/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/sensor/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/sensor/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/render/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/render/*/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/stdlib/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/thread/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/thread/pthread/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/timer/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/timer/unix/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/video/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/video/android/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/video/yuv2rgb/*.c) \
    # $(wildcard $(LOCAL_PATH)/src/test/*.c))
)
