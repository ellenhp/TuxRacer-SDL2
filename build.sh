#!/bin/bash

if [[ -z "${ANDROID_HOME}" ]]; then
    echo "Please set ANDROID_HOME."
    exit 1;
fi

if [[ -z "${ANDROID_NDK_HOME}" ]]; then
    echo "Please set ANDROID_NDK_HOME to point to 21.1.6352462"
    exit 1;
fi

bazel build --verbose_failures --subcommands --verbose_failures //android/java:app --compilation_mode=opt