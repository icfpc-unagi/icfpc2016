#include "base/base.h"
#include "ninetan/ninestream_util.h"

using ninetan::StreamUtil;

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  LOG(INFO) << "Start ninestream_example.";
  vector<string> worker_streams =
      GetResponseOrDie(StreamUtil::List(
          StreamUtil::kWorkerStreams)).stream_ids;
  CHECK_GT(worker_streams.size(), 0);
  while (true) {
    auto status = StreamUtil::Read(StreamUtil::kCommunicatorStreams);
    if (status.code != StreamUtil::OK) break;
    GetResponseOrDie(StreamUtil::Write(
        StreamUtil::kWorkerStreams,
        status.data));
  }
  while (true) {
    auto status = StreamUtil::Read(StreamUtil::kWorkerStreams, 1000);
    if (status.code != StreamUtil::OK) break;
    StreamUtil::Write(
        StreamUtil::kCommunicatorStreams,
        StrCat("Stream ", status.stream_id, " outputs ", status.data));
  }
  StreamUtil::Exit();
  return 0;
}
