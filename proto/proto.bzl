load(":proto_native.bzl", "cc_proto_library_native")

def cc_proto_library(
    default_runtime="//external:protolib",
    protoc="//external:protoc",
    **kargs):
  cc_proto_library_native(
      default_runtime=default_runtime,
      protoc=protoc,
      **kargs)
