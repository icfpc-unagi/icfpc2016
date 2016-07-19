#include "base/base.h"

namespace ninetan {

class StreamUtil {
 public:
  static constexpr char kWorkerStreams[] = "worker";

  enum StatusCode {
    UNKNOWN = 0,
    OK = 1,
    INTERNAL_ERROR = 2,
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

}  // namespace ninetan
