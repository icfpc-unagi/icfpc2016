licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("@protobuf//:protobuf.bzl", "cc_proto_library")
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
    data = [
        ":ninestream",
        "//bin",
    ],
)
