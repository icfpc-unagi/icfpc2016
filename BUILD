licenses(["notice"])

package(default_visibility = ["//visibility:private"])

cc_library(
    name = "polygon",
    hdrs = ["polygon.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
)

cc_binary(
    name = "problem",
    srcs = ["problem.cc"],
    deps = [
        "//external:base",
        "//external:boost",
        ":polygon",
    ],
)
