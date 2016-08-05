licenses(["notice"])

package(default_visibility = ["//visibility:private"])

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
