licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("//bin:imosh_test.bzl", "imosh_test")

cc_binary(
    name = "ninestream",
    srcs = ["ninestream.cc"],
    deps = [
        "//external:base",
    ],
    linkstatic = 1,
)

imosh_test(
    name = "ninestream_test",
    srcs = ["ninestream_test.sh"],
    timeout = "short",
    data = [
        ":ninestream",
        "//bin",
    ],
)

cc_library(
    name = "ninestream_util",
    srcs = ["ninestream_util.cc"],
    hdrs = ["ninestream_util.h"],
    deps = ["//external:base"],
)

cc_binary(
    name = "ninestream_example",
    srcs = ["ninestream_example.cc"],
    deps = [
        ":ninestream_util",
        "//external:base",
    ],
    linkstatic = 1,
)

imosh_test(
    name = "ninestream_example_test",
    srcs = ["ninestream_example_test.sh"],
    timeout = "short",
    data = [
        ":ninestream",
        ":ninestream_example",
    ],
)
