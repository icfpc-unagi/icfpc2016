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

http_archive(
    name = "protobuf",
    url = "https://github.com/google/protobuf/archive/v3.0.0.zip",
    strip_prefix = "protobuf-3.0.0",
    sha256 = "881f7af19166ce729d877d1bc65700d937e55fcc49b062623c0cdbc5aad3e0c4",
)

bind(
    name = "protoc",
    actual = "@protobuf//:protoc",
)

bind(
    name = "protolib",
    actual = "@protobuf//:protobuf",
)

http_archive(
    name = "boost_archive",
    url = "https://storage.googleapis.com/archive-imoz-jp/Repository/boost/boost-20160806-0056.zip",
    sha256 = "06b8e7018142e5a99757d5dd4288116beae4b06723251593b4a9edb17c4085f8",
)

bind(
    name = "boost",
    actual = "@boost_archive//:boost",
)
