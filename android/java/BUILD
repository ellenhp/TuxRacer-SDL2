load("@rules_android//android:rules.bzl", "android_binary")

android_binary(
    name = "app",
    srcs = glob([
        "org/libsdl/app/*.java",
        "me/ellenhp/tuxracer/*.java",
    ]),
    assets = glob(["assets/**/*"]),
    assets_dir = "assets",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**/*"]),
    deps = [
        "//deps/sdl",
        "//deps/sdl_image",
        "//deps/sdl_image:libjpeg",
        "//deps/sdl_image:libpng",
        "//deps/sdl_mixer",
        "//deps/sdl_mixer:libmodplug",
        "//deps/tcl",
        "//native",
    ],
)
