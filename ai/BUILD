licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("//rules:archive.bzl", "archive")

sh_binary(
    name = "wata",
    srcs = ["wata.sh"],
    data = [
        "//iwiwi:prefilter",
        "//iwiwi:postfilter1",
        "//iwiwi:postfilter2",
        "//wata:wata_deploy.jar",
    ],
)

archive(binary = "wata")
