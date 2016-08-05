#include "base/testing.h"
#include "boost/multiprecision/cpp_int.hpp"

typedef boost::multiprecision::int128_t int128;
typedef boost::multiprecision::int256_t int256;
typedef boost::multiprecision::cpp_int bigint;

TEST(BigInt, Initialization) {
  EXPECT_EQ(1234, bigint(1234));
  EXPECT_EQ(1234, bigint("1234"));
  EXPECT_EQ(0x1234, bigint("0x1234"));
}

TEST(BigInt, Addition) {
  EXPECT_EQ(10100, bigint(100) + bigint(10000));
}

TEST(BigInt, ToString) {
  EXPECT_EQ("12345", bigint(12345).str());
}

TEST(BigInt, Cast) {
  int value = static_cast<int>(bigint(12345));
  EXPECT_EQ(12345, value);
}
