#include "base/base.h"

namespace ninetan {

class StreamUtil {
 public:
  static constexpr char kCommunicatorStreams[] = "communicator";
  static constexpr char kWorkerStreams[] = "worker";

  enum StatusCode {
    OK = 0,
    CANCELLED = 1,
    UNKNOWN = 2,
    INVALID_ARGUMENT = 3,
    DEADLINE_EXCEEDED = 4,
    NOT_FOUND = 5,
    ALREADY_EXISTS = 6,
    PERMISSION_DENIED = 7,
    RESOURCE_EXHAUSTED = 8,
    FAILED_PRECONDITION = 9,
    ABORTED = 10,
    OUT_OF_RANGE = 11,
    UNIMPLEMENTED = 12,
    INTERNAL = 13,
    UNAVAILABLE = 14,
    DATA_LOSS = 15,
  };

  struct GenericResponse {
    StatusCode code;
    string error;
    string data;
  };

  struct RunResponse {
    StatusCode code;
    string error;
    vector<string> stream_ids;
  };

  struct ReadResponse {
    StatusCode code;
    string error;
    string stream_id;
    string data;
  };

  struct WriteResponse {
    StatusCode code;
    string error;
    vector<string> stream_ids;
  };

  struct ListResponse {
    StatusCode code;
    string error;
    vector<string> stream_ids;
  };

  static RunResponse Run(int num_replicas, const string& command);
  static GenericResponse Exec(const string& command);
  static ReadResponse Read(const string& stream_id, int timeout = -1);
  static WriteResponse Write(const string& stream_id, const string& data);
  static GenericResponse Kill(const string& stream_id, int timeout = 1000);
  static ListResponse List(const string& stream_id);
  static GenericResponse Exit(int exit_code = 0);

 private:
  StreamUtil() {}

  DISALLOW_COPY_AND_ASSIGN(StreamUtil);
};

template<class T>
T GetResponseOrDie(T response) {
  CHECK_EQ(response.code, StreamUtil::OK) << "Error: " << response.error;
  return response;
}

}  // namespace ninetan
