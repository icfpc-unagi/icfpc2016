load("@protobuf//:protobuf.bzl", "cc_proto_library")

def cc_proto_library_native(**kargs):
  cc_proto_library(**kargs)
