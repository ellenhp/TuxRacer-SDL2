load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "sdl_mixer",
    srcs = glob(
        [
            "*.c",
            "*.h",
            "native_midi/*.c",
            "native_midi/*.h",
        ],
        exclude = ["play*"],
    ),
    hdrs = glob(["include/*.h"]),
    copts = [
        "-Ideps/sdl/include",
        "-Ideps/sdl_mixer",
        "-DMODPLUG_MUSIC",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//deps/sdl",
    ],
    alwayslink = True,
)

cc_library(
    name = "libmodplug",
    srcs = [
        "external/libmodplug-0.8.9.0/src/fastmix.cpp",
        "external/libmodplug-0.8.9.0/src/load_669.cpp",
        "external/libmodplug-0.8.9.0/src/load_abc.cpp",
        "external/libmodplug-0.8.9.0/src/load_amf.cpp",
        "external/libmodplug-0.8.9.0/src/load_ams.cpp",
        "external/libmodplug-0.8.9.0/src/load_dbm.cpp",
        "external/libmodplug-0.8.9.0/src/load_dmf.cpp",
        "external/libmodplug-0.8.9.0/src/load_dsm.cpp",
        "external/libmodplug-0.8.9.0/src/load_far.cpp",
        "external/libmodplug-0.8.9.0/src/load_it.cpp",
        "external/libmodplug-0.8.9.0/src/load_j2b.cpp",
        "external/libmodplug-0.8.9.0/src/load_mdl.cpp",
        "external/libmodplug-0.8.9.0/src/load_med.cpp",
        "external/libmodplug-0.8.9.0/src/load_mid.cpp",
        "external/libmodplug-0.8.9.0/src/load_mod.cpp",
        "external/libmodplug-0.8.9.0/src/load_mt2.cpp",
        "external/libmodplug-0.8.9.0/src/load_mtm.cpp",
        "external/libmodplug-0.8.9.0/src/load_okt.cpp",
        "external/libmodplug-0.8.9.0/src/load_pat.cpp",
        "external/libmodplug-0.8.9.0/src/load_psm.cpp",
        "external/libmodplug-0.8.9.0/src/load_ptm.cpp",
        "external/libmodplug-0.8.9.0/src/load_s3m.cpp",
        "external/libmodplug-0.8.9.0/src/load_stm.cpp",
        "external/libmodplug-0.8.9.0/src/load_ult.cpp",
        "external/libmodplug-0.8.9.0/src/load_umx.cpp",
        "external/libmodplug-0.8.9.0/src/load_wav.cpp",
        "external/libmodplug-0.8.9.0/src/load_xm.cpp",
        "external/libmodplug-0.8.9.0/src/mmcmp.cpp",
        "external/libmodplug-0.8.9.0/src/modplug.cpp",
        "external/libmodplug-0.8.9.0/src/snd_dsp.cpp",
        "external/libmodplug-0.8.9.0/src/snd_flt.cpp",
        "external/libmodplug-0.8.9.0/src/snd_fx.cpp",
        "external/libmodplug-0.8.9.0/src/sndfile.cpp",
        "external/libmodplug-0.8.9.0/src/sndmix.cpp",
    ] + glob(
        [
            "external/libmodplug-0.8.9.0/src/*.h",
            "external/libmodplug-0.8.9.0/src/libmodplug/*.h",
        ],
    ),
    hdrs = glob(["external/libmodplug-0.8.9.0/src/libmodplug/*.h"]),
    copts = [
        "-Ideps/sdl_mixer/external/libmodplug-0.8.9.0/src",
        "-Ideps/sdl_mixer/external/libmodplug-0.8.9.0/src/libmodplug",
        "-DHAVE_SETENV",
        "-DHAVE_SINF",
        "-DHAVE_STDINT_H",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//deps/sdl",
    ],
    alwayslink = True,
)
