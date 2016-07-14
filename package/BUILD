licenses(["notice"])

package(default_visibility = ["//visibility:private"])

sh_binary(
    name = "data",
    srcs = ["empty.sh"],
    data = ["//example:glog_main"],
)

genrule(
    name = "empty_sh",
    srcs = [],
    outs = ["empty.sh"],
    cmd = "touch \"$@\"; chmod +x \"$@\"",
)

sh_binary(
    name = "upload",
    srcs = ["upload.sh"],
    data = [":data"],
)
