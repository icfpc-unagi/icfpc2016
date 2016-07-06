#include <stdio.h>

#include "base/base.h"
#include "base/testing.h"

// NOTE: StringPrintf is implemented under base/, but this is placed here
// because this is often used with strings.
TEST(BaseTest, StringPrintf) {
  EXPECT_EQ("01234", StringPrintf("%05d", 1234));
}

TEST(StringsTest, StrCat) {
  EXPECT_EQ("FooBar", StrCat("Foo", "Bar"));
  EXPECT_EQ("Foo12345Bar", StrCat("Foo", 12345, "Bar"));
}

TEST(StringsTest, Split) {
  vector<string> result;
  
  result = strings::Split("a,bb,ccc", ",");
  EXPECT_THAT(result, testing::ElementsAre("a", "bb", "ccc"));

  result = strings::Split("a,bb,ccc", strings::delimiter::Limit(",", 1));
  EXPECT_THAT(result, testing::ElementsAre("a", "bb,ccc"));

  result = strings::Split("a,bb,,ccc", ",");
  EXPECT_THAT(result, testing::ElementsAre("a", "bb", "", "ccc"));

  result = strings::Split("a,bb,,ccc", ",", strings::SkipEmpty());
  EXPECT_THAT(result, testing::ElementsAre("a", "bb", "ccc"));
}

TEST(StringsTest, Join) {
  EXPECT_EQ("a,bb,ccc",
            strings::Join(vector<string>({"a", "bb", "ccc"}), ","));
}

TEST(StringsTest, JoinElements) {
  EXPECT_EQ("1,10,100",
            strings::JoinElements(vector<int>({1, 10, 100}), ","));
}

TEST(StringsTest, StringReplace) {
  EXPECT_EQ("aaaxbbccc", StringReplace("aaabbbccc", "b", "x", false));
  EXPECT_EQ("aaaxxxccc", StringReplace("aaabbbccc", "b", "x", true));
}

TEST(StringsTest, HasPrefixString) {
  EXPECT_TRUE(HasPrefixString("foobar", "foo"));
  EXPECT_FALSE(HasPrefixString("foobar", "bar"));
}

TEST(StringsTest, StripPrefixString) {
  EXPECT_EQ("bar", StripPrefixString("foobar", "foo"));
}

TEST(StringsTest, Substitute) {
  EXPECT_EQ("this is an apple",
            strings::Substitute("$1 is $0", "an apple", "this"));
}
