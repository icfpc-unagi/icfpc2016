http_archive(
    name = "gbase",
    url = "https://s3-ap-northeast-1.amazonaws.com/ninecluster/binary/gbase.zip",
    sha256 = "c9b5cb7cec3b2ee96080e99861c1f79d1b3b8198ad0bd8eb88f8c7d0f3f0131f",
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
