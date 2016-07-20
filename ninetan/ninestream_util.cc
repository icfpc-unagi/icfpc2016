#include "ninetan/ninestream_util.h"

#include <signal.h>

#include "base/base.h"

namespace ninetan {
namespace {

void SendRequest(const string& data) {
  puts(data.c_str());
  fflush(stdout);
}

StreamUtil::StatusCode GetStatusCode(const string& status) {
  if (status == "OK") { return StreamUtil::OK; }
  if (status == "CANCELLED") { return StreamUtil::CANCELLED; }
  if (status == "UNKNOWN") { return StreamUtil::UNKNOWN; }
  if (status == "INVALID_ARGUMENT") { return StreamUtil::INVALID_ARGUMENT; }
  if (status == "DEADLINE_EXCEEDED") { return StreamUtil::DEADLINE_EXCEEDED; }
  if (status == "NOT_FOUND") { return StreamUtil::NOT_FOUND; }
  if (status == "ALREADY_EXISTS") { return StreamUtil::ALREADY_EXISTS; }
  if (status == "PERMISSION_DENIED") { return StreamUtil::PERMISSION_DENIED; }
  if (status == "RESOURCE_EXHAUSTED") { return StreamUtil::RESOURCE_EXHAUSTED; }
  if (status == "FAILED_PRECONDITION") {
    return StreamUtil::FAILED_PRECONDITION;
  }
  if (status == "ABORTED") { return StreamUtil::ABORTED; }
  if (status == "OUT_OF_RANGE") { return StreamUtil::OUT_OF_RANGE; }
  if (status == "UNIMPLEMENTED") { return StreamUtil::UNIMPLEMENTED; }
  if (status == "INTERNAL") { return StreamUtil::INTERNAL; }
  if (status == "UNAVAILABLE") { return StreamUtil::UNAVAILABLE; }
  if (status == "DATA_LOSS") { return StreamUtil::DATA_LOSS; }
  return StreamUtil::UNKNOWN;
}

template<class T>
T ReceiveResponse(const std::function<void(const string&, T*)>& converter) {
  string result = "";
  char buf[1024];
  while (fgets(buf, sizeof(buf) - 1, stdin) != nullptr) {
    result += buf;
    if (!result.empty() && result.back() == '\n') break;
  }
  while (!result.empty() &&
         (result.back() == '\n' || result.back() == '\r')) {
    result.erase(result.end() - 1, result.end());
  }

  vector<string> args =
      strings::Split(result, strings::delimiter::Limit(" ", 1));
  T response;
  response.code = GetStatusCode(args[0]);
  if (args.size() <= 1) { return response; }
  if (response.code == StreamUtil::OK) {
    converter(args[1], &response);
  } else {
    response.error = args[1];
  }
  return response;
}

StreamUtil::GenericResponse ReceiveGenericResponse() {
  return ReceiveResponse<StreamUtil::GenericResponse>(
      [](const string& data, StreamUtil::GenericResponse* response) {
        response->data = data;
      });
}

}  // namespace

constexpr char StreamUtil::kCommunicatorStreams[];
constexpr char StreamUtil::kWorkerStreams[];

StreamUtil::RunResponse StreamUtil::Run(
    int num_replicas, const string& command) {
  SendRequest(StrCat("run ", num_replicas, " ", command));
  return ReceiveResponse<RunResponse>(
      [](const string& data, RunResponse* response) {
        if (data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Exec(const string& command) {
  SendRequest(StrCat("exec ", command));
  return ReceiveGenericResponse();
}

StreamUtil::ReadResponse StreamUtil::Read(
    const string& stream_id, int timeout) {
  SendRequest(
      "read " + stream_id + (timeout == -1 ? "" : StrCat(" ", timeout)));
  return ReceiveResponse<ReadResponse>(
      [](const string& data, ReadResponse* response) {
        vector<string> args =
            strings::Split(data, strings::delimiter::Limit(" ", 1));
        if (args.size() != 2) {
          response->code = StreamUtil::INTERNAL;
          return;
        }
        response->stream_id = args[0];
        response->data = args[1];
      });
}

StreamUtil::WriteResponse StreamUtil::Write(
    const string& stream_id, const string& data) {
  SendRequest("write " + stream_id + " " + data);
  return ReceiveResponse<WriteResponse>(
      [](const string& data, WriteResponse* response) {
        if (data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Kill(
    const string& stream_id, int timeout) {
  SendRequest(
      "kill " + stream_id + (timeout == -1 ? "" : StrCat(" ", timeout)));
  return ReceiveGenericResponse();
}

StreamUtil::ListResponse StreamUtil::List(const string& stream_id) {
  SendRequest("list " + stream_id);
  return ReceiveResponse<ListResponse>(
      [](const string& data, ListResponse* response) {
        LOG(INFO) << "LIST";
        if (data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Exit(int exit_code) {
  signal(SIGTERM, SIG_IGN);
  SendRequest(StrCat("exit ", exit_code));
  return ReceiveGenericResponse();
}

}  // namespace ninetan
