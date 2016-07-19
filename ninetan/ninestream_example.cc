#include "base/base.h"
#include "ninetan/ninestream_util.h"

using ninetan::StreamUtil;

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  CHECK_EQ(StreamUtil::Run(3, "bash").code, StreamUtil::OK);
  return 0;
}
