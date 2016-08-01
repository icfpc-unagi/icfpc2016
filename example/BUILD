licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("//proto:proto.bzl", "cc_proto_library")
load("//rules:haskell.bzl", "haskell_binary")

cc_proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
)

cc_test(
    name = "proto_test",
    srcs = ["proto_test.cc"],
    deps = [
        ":example_proto",
        "//external:base",
        "//external:protolib",
        "//external:testing_main",
    ],
)

cc_binary(
    name = "glog_main",
    srcs = ["glog_main.cc"],
    deps = [
        "//external:base",
    ],
    linkstatic = 1,
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

haskell_binary(
    name = "hello_haskell",
    srcs = ["hello_haskell.hs"],
)
