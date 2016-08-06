licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("//rules:archive.bzl", "archive")

sh_binary(
    name = "postfilter",
    srcs = ["postfilter.sh"],
    data = [
        "//iwiwi:postfilter1",
        "//iwiwi:postfilter2",
    ],
)

archive(binary = "postfilter")
