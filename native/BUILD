load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "native",
    srcs = glob([
        "*.c",
        "*.cpp",
        "*.h",
    ]),
    copts = [
        "-Inative",
        "-DHAVE_SDL",
        "-D__ANDROID__",
        "-Ideps/sdl/include",
        "-Ideps/sdl_mixer",
        "-Ideps/glm",
        "-Ideps/glm/glm/detail",
        "-Ideps/sdl_image",
    ],
    linkopts = [
        "-llog",
        "-landroid",
        "-lOpenSLES",
        "-ldl",
        "-lc",
        "-lm",
        "-lGLESv2",
        "-lEGL",
    ],
    visibility = ["//android:__subpackages__"],
    deps = [
        "//deps/glm",
        "//deps/sdl",
        "//deps/sdl_image",
        "//deps/sdl_image:libpng",
        "//deps/sdl_mixer",
        "//deps/sdl_mixer:libmodplug",
        "//deps/tcl",
    ],
    alwayslink = True,
)
