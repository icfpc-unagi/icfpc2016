http_archive(
    name = "gbase",
    url = "https://github.com/imos/gbase/releases/download/v0.8.3/gbase.zip",
    sha256 = "bd1925d2b25b947f2e151fc440a760c3d557f79b6f7ca4cd003f8d4fdbead699",
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
