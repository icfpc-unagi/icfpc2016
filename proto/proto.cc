#include <memory>

#include "base/base.h"
#include "proto/proto.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/util/type_resolver.h"
#include "google/protobuf/util/type_resolver_util.h"

using google::protobuf::DescriptorPool;
using google::protobuf::util::TypeResolver;
using google::protobuf::util::NewTypeResolverForDescriptorPool;

namespace proto {
namespace {

const char kTypeUrlPrefix[] = "type.googleapis.com";

string GetTypeUrl(const Message& message) {
  return string(kTypeUrlPrefix) + "/" + message.GetDescriptor()->full_name();
}

TypeResolver* GetTypeResolver() {
  static std::unique_ptr<TypeResolver> resolver(
      NewTypeResolverForDescriptorPool(
          kTypeUrlPrefix, DescriptorPool::generated_pool()));
  return resolver.get();
}

}  // namespace

string ToJson(const Message& message, const JsonOptions& options) {
  string result;
  google::protobuf::util::JsonOptions protobuf_options;
  protobuf_options.add_whitespace = !options.compact();
  protobuf_options.always_print_primitive_fields = false;
  BinaryToJsonString(GetTypeResolver(), GetTypeUrl(message),
                     message.SerializeAsString(), &result, protobuf_options);
  return result;
}

string ToJson(const Message& message) {
  return ToJson(message, JsonOptions());
}

bool FromJson(const string& json, Message* message) {
  string binary;
  const auto status = JsonToBinaryString(
          GetTypeResolver(), GetTypeUrl(*message), json, &binary);
  if (!status.ok()) {
    LOG(ERROR) << status;
    return false;
  }
  return message->ParseFromString(binary);
}

}  // namespace proto
