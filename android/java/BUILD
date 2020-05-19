load("@rules_android//android:rules.bzl", "android_binary", "android_library")

android_library(
    name = "lib",
    srcs = glob(["org/libsdl/app/*.java"]),
    custom_package = "me.ellenhp.tuxracer",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**/*"]),
)

android_binary(
    name = "app",
    manifest = "AndroidManifest.xml",
    deps = [
        ":lib",
        "//native",
    ],
)
