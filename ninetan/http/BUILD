licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("//proto:proto.bzl", "cc_proto_library")
load("//bin:imosh_test.bzl", "imosh_test")

cc_proto_library(
    name = "server_proto",
    srcs = ["server.proto"],
)

cc_library(
    name = "server",
    srcs = ["server.cc"],
    hdrs = ["server.h"],
    deps = [
        ":server_proto",
        "//external:base",
    ],
)

cc_binary(
    name = "server_example",
    srcs = ["server_example.cc"],
    deps = [
        ":server_proto",
        ":server",
        "//external:base",
    ],
    linkstatic = 1,
)

imosh_test(
    name = "server_example_test",
    srcs = ["server_example_test.sh"],
    timeout = "short",
    data = [
        ":server_example",
    ],
)
