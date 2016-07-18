#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <deque>
#include <memory>

#include "base/base.h"
#include "base/timer.h"

namespace ninetan {
namespace {

// Copied from:
// https://github.com/google/or-tools/blob/master/src/base/map_util.h
//
// Perform a lookup in a map or hash_map whose values are pointers.
// If the key is present a const pointer to the associated value is returned,
// otherwise a NULL pointer is returned.
// This function does not distinguish between a missing key and a key mapped
// to a NULL value.
template <class Collection>
const typename Collection::value_type::second_type FindPtrOrNull(
    const Collection& collection,
    const typename Collection::value_type::first_type& key) {
  typename Collection::const_iterator it = collection.find(key);
  if (it == collection.end()) {
    return 0;
  }
  return it->second;
}

// Perform a lookup in a map or hash_map.
// If the key is present a const pointer to the associated value is returned,
// otherwise a NULL pointer is returned.
template <class Collection>
const typename Collection::value_type::second_type* FindOrNull(
    const Collection& collection,
    const typename Collection::value_type::first_type& key) {
  typename Collection::const_iterator it = collection.find(key);
  if (it == collection.end()) {
    return 0;
  }
  return &it->second;
}

} // namespace

class Stream {
 public:
  enum Direction {
    READ = 0,
    WRITE = 1,
  };

  ~Stream() {
    if (pid_ > 0) {
      kill(pid_, SIGKILL);
    }
  }

  // NOTE: Terminate does intentionally not use const because this affects
  // the stream's process.
  void Terminate() {
    if (pid_ <= 0) { return; }
    kill(pid_, SIGTERM);
  }

  // Waits for the process's termination by deadline_in_micros.  Otherwise,
  // this function forcefully terminates the process.
  void Kill(int64 deadline_in_micros) {
    if (pid_ <= 0) { return; }
    do {
      int status;
      waitpid(pid_, &status, WNOHANG);
      if (WIFEXITED(status) || WIFSIGNALED(status)) { return; }
    } while (WallTimer::GetTimeInMicroSeconds() < deadline_in_micros);
    kill(pid_, SIGKILL);
  }

  static std::unique_ptr<Stream> NewInteractiveStream() {
    std::unique_ptr<Stream> stream(new Stream(""));
    stream->pid_ = 0;
    stream->stdin_ = 1;
    stream->stdout_ = 0;
    SetNonBlocking(0);
    SetNonBlocking(1);
    return stream;
  }

  static std::unique_ptr<Stream> NewStream(const string& command) {
    std::unique_ptr<Stream> stream(new Stream(command));
    stream->Run();
    return stream;
  }

  bool IsEof() {
    return stdout_ < 0 && stdout_buffer_.size() == 0;
  }

  string ReadLine() {
    while (!IsEof()) {
      for (int i = 0; i < stdout_buffer_.size(); i++) {
        if (stdout_buffer_[i] == '\n') {
          string result(stdout_buffer_.begin(), stdout_buffer_.begin() + i + 1);
          stdout_buffer_.erase(stdout_buffer_.begin(),
                               stdout_buffer_.begin() + i + 1);
          return result;
        }
      }
      char buf[1024];
      int size = read(stdout_, buf, sizeof(buf));
      if (size < 0) {
        LOG(INFO) << "Read error: " << strerror(errno);
        return "";
      } else if (size == 0) {
        LOG(INFO) << "Reached EOF.";
        stdout_ = -1;
        break;
      }
      for (int i = 0; i < size; i++) {
        stdout_buffer_.push_back(buf[i]);
      }
    }
    // Flush remainings if the pipe is closed.
    if (stdout_ < 0) {
      string result(stdout_buffer_.begin(), stdout_buffer_.end());
      stdout_buffer_.clear();
      return result;
    }
    return "";
  }

  void Write(const string& data) {
    if (stdin_ < 0) { return; }
    if (!data.empty()) {
      stdin_buffer_.insert(stdin_buffer_.end(), data.begin(), data.end());
    }

    // Write a shorter one of either the first 128 bytes or everything of
    // the current buffer.
    char chunk[min(stdin_buffer_.size(), 128UL)];
    for (int i = 0; i < sizeof(chunk) / sizeof(chunk[0]); i++) {
      chunk[i] = stdin_buffer_[i];
    }

    int size = write(stdin_, chunk, sizeof(chunk));
    if (size < 0) {
      if (errno != EAGAIN) {
        LOG(INFO) << "Stream is broken: " << strerror(errno);
        stdin_ = -1;
      }
      return;
    }
    stdin_buffer_.erase(stdin_buffer_.begin(), stdin_buffer_.begin() + size);
  }

  void WriteLine(const string& data) {
    Write(data);
    Write("\n");
  }

  // stream_id specifies the stream to poll.  If -1 is given, this polls all
  // slave streams.
  static int Poll(const vector<std::unique_ptr<Stream>>& streams,
                  int stream_id, int timeout = -1) {
    LOG(INFO) << "Polling " << stream_id << " with " << timeout;
    CHECK_GE(timeout, -1);
    while (true) {
      vector<struct pollfd> fds;
      int num_streams = 0;
      for (int stream_index = 0; stream_index < streams.size();
           stream_index++) {
        const Stream* stream = streams[stream_index].get();
        if (stream == nullptr) continue;
        if (stream->stdin_ >= 0 &&
            stream->stdin_buffer_.size() > 0) {
          fds.emplace_back();
          auto& pollfd = fds.back();
          pollfd.fd = stream->stdin_;
          LOG(INFO) << "Polling stdin: " << pollfd.fd;
          pollfd.events = POLLOUT;
          pollfd.revents = pollfd.events;
        }
        if (stream->stdout_ >= 0 &&
            (stream_index == stream_id ||
             (stream_id == -1 && stream_index > 0))) {
          fds.emplace_back();
          auto& pollfd = fds.back();
          pollfd.fd = stream->stdout_;
          LOG(INFO) << "Polling stdout: " << pollfd.fd;
          pollfd.events = POLLPRI | POLLIN;
          num_streams++;
        }
      }
      map<int, const struct pollfd*> descriptor_to_pollfd;
      for (const struct pollfd& pollfd : fds) {
        descriptor_to_pollfd[pollfd.fd] = &pollfd;
      }
      if (num_streams == 0) {
        return -1;
      }

      int result = poll(fds.data(), fds.size(), timeout);
      if (result < 0) {
        LOG(INFO) << "Poll error: " << strerror(errno);
        return -1;
      }
      if (result == 0) {
        return -1;
      }
      // TODO(imos): Add error handling for poll.
      for (int stream_index = 0; stream_index < streams.size();
           stream_index++) {
        Stream* stream = streams[stream_index].get();
        if (stream == nullptr) continue;

        auto* pollfd = FindPtrOrNull(descriptor_to_pollfd, stream->stdin_);
        if (pollfd != nullptr && pollfd->revents != 0) {
          stream->Write("");
        }

        pollfd = FindPtrOrNull(descriptor_to_pollfd, stream->stdout_);
        if (pollfd != nullptr && pollfd->revents != 0) {
          return stream_index;
        }
      }
    }
  }

 private:
  Stream(const string& command) : command_(command) {}

  void Run() {
    int pipe_c2p[2];
    int pipe_p2c[2];
    CHECK_EQ(0, pipe(pipe_c2p));
    CHECK_EQ(0, pipe(pipe_p2c));
    pid_ = fork();
    if (pid_ < 0) {
      LOG(FATAL) << "Failed to fork.";
    }
    if (pid_ == 0) {
      close(pipe_p2c[WRITE]);
      close(pipe_c2p[READ]);
      dup2(pipe_p2c[READ], 0);
      dup2(pipe_c2p[WRITE], 1);
      close(pipe_p2c[READ]);
      close(pipe_c2p[WRITE]);
      execlp("/bin/bash", "bash", "-c", command_.c_str());
      LOG(FATAL) << "Failed to run command: " << command_;
    }
    close(pipe_p2c[READ]);
    close(pipe_c2p[WRITE]);
    stdin_ = pipe_p2c[WRITE];
    stdout_ = pipe_c2p[READ];
    SetNonBlocking(stdin_);
    SetNonBlocking(stdout_);
  }

  string DebugString() const {
    return StrCat("stream(pid=", pid_, ", stdin=", stdin_, ", stdout=", stdout_,
                  ", command=", command_, ")");
  }

  static Stream* Poll(const map<int, Stream*>& descriptor_to_stream) {
    vector<struct pollfd> fds;
    for (const auto& descriptor_and_stream : descriptor_to_stream) {
      fds.emplace_back();
      auto& pollfd = fds.back();
      pollfd.fd = descriptor_and_stream.first;
      pollfd.events = POLLIN;
      pollfd.revents = 0;
    };
    int descriptor = poll(fds.data(), fds.size(), -1);
    auto it = descriptor_to_stream.find(descriptor);
    if (it == descriptor_to_stream.end()) {
      return nullptr;
    }
    return it->second;
  }

  static void SetNonBlocking(int fd) {
    CHECK_GE(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK), 0);
  }

  string command_;
  int pid_;
  int stdin_;
  int stdout_;
  std::deque<char> stdin_buffer_;
  std::deque<char> stdout_buffer_;

  DISALLOW_COPY_AND_ASSIGN(Stream);
};

class StreamController {
 public:
  StreamController() {}

  void Init(const string& command) {
    signal(SIGPIPE, SIG_IGN);

    if (command.empty()) {
      streams_.push_back(Stream::NewInteractiveStream());
    } else {
      streams_.push_back(Stream::NewStream(command));
    }

    CHECK_GE(streams_.size(), 1) << "No master stream is found.";
    Stream* master = streams_[0].get();
    CHECK(master != nullptr);
    while (!master->IsEof() && exit_code_ < 0) {
      LOG(INFO) << "Waiting for master.";
      string line = master->ReadLine();
      while (line.empty() && !master->IsEof()) {
        Stream::Poll(streams_, 0 /* stream_id */);
        line = master->ReadLine();
      }
      if (line.back() == '\n') { line.erase(line.end() - 1, line.end()); }
      LOG(INFO) << "Master command: " << line;
      vector<string> args =
          strings::Split(line, strings::delimiter::Limit(" ", 1));
      auto* command = FindOrNull(commands_, args.front());
      if (command == nullptr) {
        LOG(WARNING) << "Invalid command: " << line;
        continue;
      }
      LOG(INFO) << "Calling " << args.front() << "...";
      string result = command->method(this, args.size() >= 2 ? args[1] : "");
      LOG(INFO) << "Result: " << result;
      master->WriteLine(result);
    }
    for (auto& stream : streams_) {
      stream->Terminate();
    }
    int64_t deadline_in_micros = WallTimer::GetTimeInMicroSeconds() + 1000000;
    for (auto& stream : streams_) {
      stream->Kill(deadline_in_micros);
    }
  }

  bool ParseCommand(const string& method, string* command) {
    if (TryStripPrefixString(*command, method + " ", command)) {
      return true;
    }
    if (*command == method) {
      command->clear();
      return true;
    }
    return false;
  }

  string Exit(const string& command) {
    if (command.empty()) {
      exit_code_ = 0;
      return "OK";
    }
    if (safe_strto32(command, &exit_code_) && exit_code_ >= 0) {
      return "OK";
    }
    exit_code_ = -1;
    return "INVALID_ARGUMENTS exit code must be an integer, but " + command;
  }

  string Run(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    if (args.size() != 2) {
      return StrCat(
          "INVALID_ARGUMENTS run should have two arguments, but ",
          args.size(), ".");
    }
    int32 replicas = 0;
    if (!safe_strto32(args[0], &replicas)) {
      return StrCat(
          "INVALID_ARGUMENTS # of replicas should be int32, but '",
          args[0], "'.");
    }
    string result = "OK";
    for (int i = 0; i < replicas; i++) {
      streams_.push_back(Stream::NewStream(args[1]));
      result += StringPrintf(" %lu", streams_.size() - 1);
    }
    return result;
  }

  string Write(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    string error;
    int stream_id = GetStreamId(args[0], &error);
    if (stream_id < 0) {
      return StrCat("NOT_FOUND ", error);
    }
    streams_[stream_id]->WriteLine(args[1]);
    return "OK";
  }

  string Read(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    string error;
    int stream_id = GetStreamId(args[0], &error);
    if (stream_id < 0) {
      return StrCat("NOT_FOUND ", error);
    }
    int32 timeout_in_millis = -1;
    if (args.size() >= 2) {
      if (!safe_strto32(args[1], &timeout_in_millis) ||
          timeout_in_millis <= 0) {
        return StrCat("INVALID_ARGUMENTS timeout must be a positive integer, ",
                      "but ", args[1]);
      }
    }
    return ReadInternal(stream_id, timeout_in_millis);
  }

  string ReadAll(const string& command) {
    int32 timeout_in_millis = -1;
    if (!command.empty()) {
      if (!safe_strto32(command, &timeout_in_millis) ||
          timeout_in_millis <= 0) {
        return StrCat("INVALID_ARGUMENT timeout must be a positive integer, ",
                      "but ", command);
      }
    }
    return ReadInternal(-1, timeout_in_millis);
  }

  string ReadInternal(int stream_predicate, int timeout_in_millis) {
    bool no_deadline = timeout_in_millis < 0;
    int64 deadline_in_micros =
        WallTimer::GetTimeInMicroSeconds() + timeout_in_millis * 1000LL;
    string result;
    while (result.empty()) {
      timeout_in_millis = (deadline_in_micros + 999 -
                           WallTimer::GetTimeInMicroSeconds()) / 1000;
      int stream_id = Stream::Poll(
          streams_, stream_predicate,
          no_deadline ? -1 : max(timeout_in_millis, 0));
      if (stream_id < 0) {
        return "DEADLINE_EXCEEDED No ready stream.";
      }
      result = streams_[stream_id]->ReadLine();
      if (!no_deadline && result.empty() && timeout_in_millis < 0) {
        return "DEADLINE_EXCEEDED Result is delayed.";
      }
    }
    if (result.empty()) {
      return "UNAVAILABLE";
    }
    if (result.back() == '\n') {
      result.erase(result.end() - 1, result.end());
    }
    return "OK " + result;
  }

 private:
  struct Command {
    string usage;
    string description;
    std::function<string(StreamController*, const string&)> method;
  };

  int GetStreamId(const string& stream_key, string* error) {
    int stream_id;
    if (!safe_strto32(stream_key, &stream_id)) {
      if (error != nullptr) {
        *error = "Invalid stream ID: " + stream_key;
      }
      return -1;
    }
    if (stream_id < 0 || streams_.size() <= stream_id) {
      if (error != nullptr) {
        *error = StrCat("Out of stream ID range: ", stream_id);
      }
      return -1;
    }
    return stream_id;
  }

  map<string, Command> commands_ = {
      {"run",
       Command{"<# of replicas> <command> <args>...",
               "Spawns child processes, and returns stream IDs",
               &StreamController::Run}},
      {"write",
       Command{"<stream ID> <message>",
               "Writes a message to the stream.",
               &StreamController::Write}},
      {"read",
       Command{"<stream ID> (<timeout>)",
               "Read a line from the stream.",
               &StreamController::Read}},
      {"readall",
       Command{"(<timeout>)",
               "Read a line from some stream.",
               &StreamController::ReadAll}},
      {"exit",
       Command{"(<exit code>)",
               "Exits with the exit code.  Exits with 0 if not explicitly "
               "given.",
               &StreamController::Exit}},
  };
  vector<std::unique_ptr<Stream>> streams_;
  int exit_code_ = -1;
};

}  // namespace ninetan

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  ninetan::StreamController stream_master;
  stream_master.Init("");
  return 0;
}
