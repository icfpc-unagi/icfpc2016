licenses(["notice"])

package(default_visibility = ["//visibility:private"])

cc_library(
    name = "polygon",
    hdrs = ["polygon.h"],
    deps = [
        "//external:base",
        "//external:boost",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
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

cc_binary(
    name = "validate",
    srcs = ["validate.cc"],
    deps = [
        "//external:base",
        "//external:boost",
        ":libproblem",
        ":libsolution",
        ":polygon",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)

cc_binary(
    name = "make_problem",
    srcs = ["make_problem.cc"],
    deps = [
        "//external:base",
        "//external:boost",
        ":libproblem",
        ":libsolution",
        ":polygon",
    ],
    linkopts = ["-lgmp"],
    linkstatic = 1,
)
