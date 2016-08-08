licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "prefilter",
    srcs = ["prefilter.cc", "common.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)

cc_binary(
    name = "postfilter1",
    srcs = ["postfilter1.cc", "common.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)

cc_binary(
    name = "postfilter2",
    srcs = ["postfilter2.cc", "common.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)

cc_binary(
    name = "score",
    srcs = ["score.cc", "common.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)

cc_binary(
    name = "perimeter_detector",
    srcs = ["perimeter_detector.cc", "common.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)
