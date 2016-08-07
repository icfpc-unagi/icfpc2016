licenses(["notice"])

package(default_visibility = ["//visibility:public"])

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

sh_binary(
    name = "wata_strong",
    srcs = ["wata_strong.sh"],
    data = [
        "//iwiwi:prefilter",
        "//iwiwi:postfilter1",
        "//iwiwi:postfilter2",
        "//wata:wata_deploy.jar",
    ],
)

archive(binary = "wata_strong")

sh_binary(
    name = "wata_conflict",
    srcs = ["wata_conflict.sh"],
    data = [
        "//iwiwi:prefilter",
        "//iwiwi:postfilter1",
        "//iwiwi:postfilter2",
        "//wata:wata_deploy.jar",
    ],
)

archive(binary = "wata_conflict")
