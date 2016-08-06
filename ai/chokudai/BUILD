licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("//rules:archive.bzl", "archive")

sh_binary(
    name = "chokudai",
    srcs = ["chokudai.sh"],
    data = [
        "//iwiwi:postfilter1",
        "//iwiwi:postfilter2",
        "//chokudai:ICFPC2016.exe",
    ],
)

archive(binary = "chokudai")
