#include "src/google/protobuf/text_format.h"
#include "proto/json_options.pb.h"

namespace proto {

using google::protobuf::Message;
using google::protobuf::TextFormat;

string ToJson(const Message& message, const JsonOptions& options);

string ToJson(const Message& message) {
  return ToJson(message, JsonOptions());
}

bool FromJson(const string& json, Message* message);

}  // namespace proto
