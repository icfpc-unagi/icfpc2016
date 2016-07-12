http_archive(
    name = "gbase",
    url = "https://s3-ap-northeast-1.amazonaws.com/ninecluster/binary/gbase.zip",
    sha256 = "97fd444fa869b73a679946e4808c7af83b56031c771104875a7827bea2090cdf",
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
