load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "glm",
    hdrs = glob([
        "glm/**/*.hpp",
        "glm/**/*.h",
        "glm/*.hpp",
        "glm/*.h",
        "glm/**/*.inl",
    ]),
    visibility = [
        "//native:__subpackages__",
    ],
)
