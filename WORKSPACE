http_archive(
    name = "gbase",
    url = "https://github.com/imos/gbase/releases/download/v0.8.2/gbase.zip",
    sha256 = "c555b1853fac21a0fceb72e3a7658b9e9be232fca4fbadff4129bb01dc9659cf",
)

bind(
    name = "base",
    actual = "@gbase//base"
)

bind(
    name = "testing",
    actual = "@gbase//base:testing"
)

bind(
    name = "testing_main",
    actual = "@gbase//base:testing_main"
)

git_repository(
    name = "protobuf",
    tag = "v3.0.0-beta-3.3",
    remote = "https://github.com/google/protobuf.git",
)

bind(
    name = "protoc",
    actual = "@protobuf//:protoc",
)

bind(
    name = "protolib",
    actual = "@protobuf//:protobuf",
)
