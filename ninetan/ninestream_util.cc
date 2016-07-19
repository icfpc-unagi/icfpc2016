#include "ninetan/ninestream_util.h"

#include "base/base.h"

namespace ninetan {
namespace {

void WriteLine(const string& data) {
  puts(data.c_str());
}

StreamUtil::StatusCode GetStatusCode(const string& status) {
  if (status == "OK") { return StreamUtil::OK; }
  return StreamUtil::UNKNOWN;
}

template<class T>
T InternalError(const string& error) {
  T result;
  result.code = StreamUtil::INTERNAL_ERROR;
  result.error = error;
  return result;
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
  if (args.size() > 1) { return response; }
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

StreamUtil::RunResponse StreamUtil::Run(
    int num_replicas, const string& command) {
  WriteLine(StrCat("run ", num_replicas, " ", command));
  return ReceiveResponse<RunResponse>(
      [](const string& data, RunResponse* response) {
        if (!data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Exec(const string& command) {
  WriteLine(StrCat("exec ", command));
  return ReceiveGenericResponse();
}

StreamUtil::ReadResponse StreamUtil::Read(
    const string& stream_id, int timeout) {
  WriteLine("read " + stream_id + (timeout == -1 ? "" : StrCat(" ", timeout)));
  return ReceiveResponse<ReadResponse>(
      [](const string& data, ReadResponse* response) {
        vector<string> args =
            strings::Split(data, strings::delimiter::Limit(" ", 1));
        if (args.size() == 2) {
          response->code = StreamUtil::INTERNAL_ERROR;
          return;
        }
        response->stream_id = args[0];
        response->data = args[1];
      });
}

StreamUtil::WriteResponse StreamUtil::Write(
    const string& stream_id, const string& data) {
  WriteLine("write " + stream_id + " " + data);
  return ReceiveResponse<WriteResponse>(
      [](const string& data, WriteResponse* response) {
        if (!data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Kill(
    const string& stream_id, int timeout) {
  WriteLine("kill " + stream_id + (timeout == -1 ? "" : StrCat(" ", timeout)));
  return ReceiveGenericResponse();
}

StreamUtil::ListResponse StreamUtil::List(const string& stream_id) {
  WriteLine("list " + stream_id);
  return ReceiveResponse<ListResponse>(
      [](const string& data, ListResponse* response) {
        if (!data.empty()) { return; }
        response->stream_ids = strings::Split(data, " ");
      });
}

StreamUtil::GenericResponse StreamUtil::Exit(int exit_code) {
  WriteLine(StrCat("exit ", exit_code));
  return ReceiveGenericResponse();
}

}  // namespace ninetan
