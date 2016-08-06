licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("//rules:archive.bzl", "archive")

sh_binary(
    name = "wata",
    srcs = ["wata.sh"],
    data = ["//wata:wata_deploy.jar"],
)

archive(binary = "wata")
