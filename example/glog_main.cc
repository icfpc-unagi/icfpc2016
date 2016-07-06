#include <stdio.h>

#include "base/base.h"

DEFINE_string(message, "", "Message.");

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  LOG(INFO) << FLAGS_message;
  puts(FLAGS_message.c_str());
  return 0;
}
