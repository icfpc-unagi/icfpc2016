#include "base/base.h"
#include "base/testing.h"
#include "example/example.pb.h"
#include "src/google/protobuf/text_format.h"

using google::protobuf::TextFormat;

TEST(GTest, Test) {
  Example example;
  TextFormat::ParseFromString("id: 12345", &example);
  EXPECT_EQ(12345, example.id());
}
