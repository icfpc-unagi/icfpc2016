http_archive(
    name = "gbase",
    url = "https://s3-ap-northeast-1.amazonaws.com/ninecluster/binary/gbase.zip",
    sha256 = "f399580449abd1112f4078e056732f7cd8a17b244b7e3bd0da85e40530500f59",
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
