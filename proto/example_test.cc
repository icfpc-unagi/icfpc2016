#include "base/base.h"
#include "base/testing.h"
#include "proto/example.pb.h"
#include "proto/proto.h"

TEST(ExampleTest, Test) {
  proto::Example example;
  proto::TextFormat::ParseFromString("id: 12345", &example);
  EXPECT_EQ(12345, example.id());
}

TEST(ExampleTest, ToJson) {
  proto::Example example;
  proto::TextFormat::ParseFromString("id: 12345", &example);
  EXPECT_EQ("{\n \"id\": 12345\n}\n", proto::ToJson(example));
}

TEST(ExampleTest, ToJson_WithDefaultValue) {
  proto::Example example;
  EXPECT_EQ("{}\n", proto::ToJson(example));
  example.set_id(0);
  EXPECT_EQ("{\n \"id\": 0\n}\n", proto::ToJson(example));
}

TEST(ExampleTest, FromJson) {
  proto::Example example;
  ASSERT_TRUE(proto::FromJson(R"({ "id": 12345 })", &example));
  EXPECT_EQ(12345, example.id());
}
