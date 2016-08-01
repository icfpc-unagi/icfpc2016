licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("//proto:proto.bzl", "cc_proto_library")

cc_proto_library(
    name = "json_options",
    srcs = ["json_options.proto"],
)

cc_library(
    name = "proto",
    srcs = ["proto.cc"],
    hdrs = ["proto.h"],
    deps = [
        ":json_options",
        "//external:base",
        "//external:protolib",
    ],
)

cc_proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
    testonly = 1,
)

cc_test(
    name = "example_test",
    srcs = ["example_test.cc"],
    deps = [
        ":example_proto",
        ":proto",
        "//external:base",
        "//external:testing_main",
        "//external:protolib",
    ],
)
