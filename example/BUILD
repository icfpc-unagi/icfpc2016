licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("@protobuf//:protobuf.bzl", "cc_proto_library")

cc_proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
    default_runtime = "@protobuf//:protobuf",
    protoc = "@protobuf//:protoc",
#    include = "external/protobuf/src",
)

cc_test(
    name = "proto_test",
    srcs = ["proto_test.cc"],
    deps = [
        ":example_proto",
        "@protobuf//:protobuf",
        "//external:base",
        "//external:testing_main",
    ],
)

cc_binary(
    name = "glog_main",
    srcs = ["glog_main.cc"],
    deps = [
        "//external:base",
    ],
)

cc_test(
    name = "gtest_test",
    srcs = ["gtest_test.cc"],
    deps = [
        "//external:testing_main",
    ],
)

cc_test(
    name = "strings_test",
    srcs = ["strings_test.cc"],
    deps = [
        "//external:testing_main",
    ],
)
