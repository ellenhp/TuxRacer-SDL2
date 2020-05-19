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
    ],
    visibility = ["//android:__subpackages__"],
    deps = ["//deps/tcl"],
)
