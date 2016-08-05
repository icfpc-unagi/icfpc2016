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

cc_library(
    name = "libproblem",
    hdrs = ["problem.h"],
    deps = [
        "//external:base",
        "//external:boost",
        ":polygon",
    ],
)

cc_library(
    name = "libsolution",
    srcs = ["solution.h"],
    deps = [
        "//external:base",
        "//external:boost",
        ":polygon",
    ],
)

cc_binary(
    name = "problem",
    srcs = ["problem.cc"],
    deps = [
        "//external:base",
        "//external:boost",
        ":libproblem",
        ":polygon",
    ],
)

cc_binary(
    name = "solution",
    srcs = ["solution.cc"],
    deps = [
        "//external:base",
        "//external:boost",
        ":libsolution",
        ":polygon",
    ],
)
