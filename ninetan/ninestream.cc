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

DEFINE_string(master, "", "Command for a master.");
DEFINE_string(worker, "", "Command for a worker.");
DEFINE_bool(communicate, false, "Enable STDIO communication.");
DEFINE_int32(replicas, 1, "# of worker replicas.");
DEFINE_bool(debug, false, "Output master I/O.");

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

void StripNewLine(string* line) {
  while (!line->empty() && (line->back() == '\n' || line->back() == '\r')) {
    line->erase(line->end() - 1, line->end());
  }
}

} // namespace

class Stream {
 public:
  enum PipeDirection {
    READ = 0,
    WRITE = 1,
  };

  enum StreamType {
    UNKNOWN_TYPE = 0,
    ALL_TYPE = 1,
    MASTER = 2,
    WORKER = 3,
    COMMUNICATOR = 4,
    INVALID_TYPE = 5,
  };

  ~Stream() {
    if (pid_ > 0) {
      kill(pid_, SIGKILL);
    }
  }

  int pid() const { return pid_; }
  int stream_id() const { return stream_id_; }
  StreamType stream_type() const { return stream_type_; }

  bool MatchStreamType(StreamType stream_predicate) const {
    if (stream_predicate == ALL_TYPE) { return true; }
    return stream_predicate == stream_type();
  }

  // NOTE: Terminate does intentionally not use const because this affects
  // the stream's process.
  void Terminate() {
    if (pid_ <= 0) { return; }
    LOG(INFO) << "Stream[" << stream_id() << "]: Terminating...";
    kill(pid_, SIGTERM);
  }

  // Waits for the process's termination by deadline_in_micros.  Otherwise,
  // this function forcefully terminates the process.
  void Kill(int64 deadline_in_micros) {
    if (pid_ <= 0) { return; }
    do {
      int status;
      waitpid(pid_, &status, WNOHANG);
      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        Reset();
        return;
      }
    } while (WallTimer::GetTimeInMicroSeconds() < deadline_in_micros);
    LOG(INFO) << "Stream[" << stream_id() << "]: Killing...";
    kill(pid_, SIGKILL);
    Reset();
  }

  void Reset() {
    LOG(INFO) << "Stream[" << stream_id() << "]: Closed.";
    pid_ = -1;
    stdin_ = -1;
    stdout_ = -1;
    stdin_buffer_.clear();
    stdout_buffer_.clear();
  }

  static std::unique_ptr<Stream> NewInteractiveStream(
      StreamType stream_type, int stream_id) {
    std::unique_ptr<Stream> stream(new Stream(stream_type, stream_id, ""));
    stream->pid_ = 0;
    stream->stdin_ = 1;
    stream->stdout_ = 0;
    SetNonBlocking(0);
    SetNonBlocking(1);
    return stream;
  }

  static std::unique_ptr<Stream> NewStream(
      StreamType stream_type, int stream_id, const string& command) {
    std::unique_ptr<Stream> stream(new Stream(stream_type, stream_id, command));
    stream->Run();
    return stream;
  }

  bool IsRunning() const {
    if (pid_ < 0) { return false; }
    if (!IsEof()) { return true; }
    if (stream_type() == COMMUNICATOR && stdin_ >= 0) { return true; }
    return false;
  }

  bool IsEof() const {
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
        LOG(INFO) << "Stream[" << stream_id() << "]: Read error: "
                  << strerror(errno);
        return "";
      } else if (size == 0) {
        LOG(INFO) << "Stream[" << stream_id() << "]: STDOUT reached EOF."; 
        // Flush remainings.
        string result(stdout_buffer_.begin(), stdout_buffer_.end());
        stdout_ = -1;
        stdout_buffer_.clear();
        return result;
      }
      stdout_buffer_.insert(stdout_buffer_.end(), buf, buf + size);
    }
    return "";
  }

  bool Write(const string& data) {
    if (stdin_ < 0) { return false; }
    stdin_buffer_.insert(stdin_buffer_.end(), data.begin(), data.end());

    // Write a shorter one of either the first 128 bytes or everything of
    // the current buffer.
    vector<char> chunk(
        stdin_buffer_.begin(),
        stdin_buffer_.begin() + min(stdin_buffer_.size(), 128UL));
    int size = write(stdin_, chunk.data(), chunk.size());
    if (size < 0) {
      if (errno == EAGAIN) { return true; }
      LOG(INFO) << "Stream[" << stream_id() << "]: Stream is broken: "
                << strerror(errno);
      stdin_ = -1;
      return false;
    }
    stdin_buffer_.erase(stdin_buffer_.begin(), stdin_buffer_.begin() + size);
    return true;
  }

  bool WriteLine(const string& data) {
    return Write(data) && Write("\n");
  }

  static int Poll(const vector<std::unique_ptr<Stream>>& streams,
                  const vector<int>& stream_ids, int timeout = -1) {
    CHECK_GE(timeout, -1);
    while (true) {
      vector<struct pollfd> fds;
      string poll_message;
      for (const std::unique_ptr<Stream>& stream : streams) {
        if (stream->stdin_ < 0 || stream->stdin_buffer_.size() == 0) continue;
        fds.emplace_back();
        auto& pollfd = fds.back();
        pollfd.fd = stream->stdin_;
        pollfd.events = POLLOUT;
        poll_message += StrCat(" ", stream->stream_id(), "(in)");
      }
      int num_streams = 0;
      for (int stream_id : stream_ids) {
        const std::unique_ptr<Stream>& stream = streams[stream_id];
        if (stream->stdout_ < 0) continue;
        fds.emplace_back();
        auto& pollfd = fds.back();
        pollfd.fd = stream->stdout_;
        pollfd.events = POLLPRI | POLLIN;
        poll_message += StrCat(" ", stream->stream_id(), "(out)");
        num_streams++;
      }
      if (num_streams == 0) {
        LOG(INFO) << "No streams to poll.";
        return -1;
      }
      LOG(INFO) << "Polling" << poll_message << "...";

      int result = poll(fds.data(), fds.size(), timeout);
      // poll times out.
      if (result == 0) { return -1; }

      if (result < 0) {
        LOG(ERROR) << "Poll error: " << strerror(errno);
        return -1;
      }

      map<int, const struct pollfd*> descriptor_to_pollfd;
      for (const struct pollfd& pollfd : fds) {
        descriptor_to_pollfd[pollfd.fd] = &pollfd;
      }
      for (const std::unique_ptr<Stream>& stream : streams) {
        auto* pollfd = FindPtrOrNull(descriptor_to_pollfd, stream->stdin_);
        if (pollfd != nullptr && pollfd->revents != 0) {
          stream->Write("");
        }
      }
      for (const std::unique_ptr<Stream>& stream : streams) {
        auto* pollfd = FindPtrOrNull(descriptor_to_pollfd, stream->stdout_);
        if (pollfd != nullptr && pollfd->revents != 0) {
          return stream->stream_id();
        }
      }
    }
  }

 private:
  Stream(StreamType stream_type, int stream_id, const string& command)
      : stream_type_(stream_type), stream_id_(stream_id), command_(command) {}

  void Run() {
    int pipe_c2p[2];
    int pipe_p2c[2];
    CHECK_EQ(0, pipe(pipe_c2p));
    CHECK_EQ(0, pipe(pipe_p2c));
    pid_ = fork();
    if (pid_ < 0) {
      LOG(FATAL) << "Failed to fork: " << strerror(errno);
    }
    if (pid_ == 0) {
      close(pipe_p2c[WRITE]);
      close(pipe_c2p[READ]);
      dup2(pipe_p2c[READ], 0);
      dup2(pipe_c2p[WRITE], 1);
      close(pipe_p2c[READ]);
      close(pipe_c2p[WRITE]);
      setenv("NINESTREAM_STREAM_ID", StrCat(stream_id()).c_str(), 1);
      setenv("NINESTREAM_REPLICAS", StrCat(FLAGS_replicas).c_str(), 1);
      int result = execlp("bash", "bash", "-c", command_.c_str(), nullptr);
      if (result < 0) {
        LOG(ERROR) << "execlp error: " << strerror(errno);
      }
      LOG(FATAL) << "Failed to run command: " << command_;
    }
    close(pipe_p2c[READ]);
    close(pipe_c2p[WRITE]);
    stdin_ = pipe_p2c[WRITE];
    stdout_ = pipe_c2p[READ];
    SetNonBlocking(stdin_);
    SetNonBlocking(stdout_);
    SetCloseExec(stdin_);
    SetCloseExec(stdout_);
  }

  string DebugString() const {
    return StrCat("stream(pid=", pid_, ", stdin=", stdin_, ", stdout=", stdout_,
                  ", command=", command_, ")");
  }

  static void SetNonBlocking(int fd) {
    CHECK_GE(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK), 0);
  }

  static void SetCloseExec(int fd) {
    CHECK_GE(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | FD_CLOEXEC), 0);
  }

  StreamType stream_type_;
  int stream_id_;
  string command_;
  int pid_ = -1;
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
      streams_.push_back(Stream::NewInteractiveStream(
          Stream::MASTER, streams_.size()));
    } else {
      streams_.push_back(Stream::NewStream(
          Stream::MASTER, streams_.size(), command));
    }
    if (FLAGS_communicate) {
      CHECK(!command.empty())
          << "command must be given to communicate via standard I/O.";
      streams_.push_back(Stream::NewInteractiveStream(
          Stream::COMMUNICATOR, streams_.size()));
    }
    if (!FLAGS_worker.empty()) {
      for (int i = 0; i < FLAGS_replicas; i++) {
        streams_.push_back(Stream::NewStream(
            Stream::WORKER, streams_.size(), FLAGS_worker));
      }
    }

    CHECK_GE(streams_.size(), 1) << "No master stream is found.";
    while (streams_[0] != nullptr && !streams_[0]->IsEof() && exit_code_ < 0) {
      // Updates master because "exec" may replace it.
      Stream* master = streams_[0].get();
      CHECK(master != nullptr);
      LOG(INFO) << "Controller: Waiting for master...";
      string line;
      while (line.empty() && !master->IsEof()) {
        Stream::Poll(streams_, {0} /* stream_ids */);
        line = master->ReadLine();
      }
      StripNewLine(&line);
      LOG(INFO) << "Stream[0]: Command: " << line;
      if (FLAGS_debug) {
        puts(line.c_str());
      }
      vector<string> args =
          strings::Split(line, strings::delimiter::Limit(" ", 1));
      auto* command = FindOrNull(commands_, args.front());
      if (command == nullptr) {
        LOG(INFO) << "Controller: Invalid command: " << args.front();
        master->WriteLine("INVALID_ARGUMENT Invalid command: " + args.front());
        continue;
      }
      LOG(INFO) << "Controller: Calling " << args.front() << "...";
      string result = command->method(this, args.size() >= 2 ? args[1] : "");
      LOG(INFO) << "Controller: Result: " << result;
      if (FLAGS_debug) {
        puts(result.c_str());
      }
      // Check if the master process is not replaced because exec may have
      // terminated the master process.
      if (streams_[0].get() == master) {
        master->WriteLine(result);
      }
    }
    for (auto& stream : streams_) {
      stream->Terminate();
    }
    int64_t deadline_in_micros = WallTimer::GetTimeInMicroSeconds() + 1000000;
    for (auto& stream : streams_) {
      stream->Kill(deadline_in_micros);
    }
  }

  string Exec(const string& command) {
    CHECK_GE(streams_.size(), 1);
    streams_[0]->WriteLine("OK");
    streams_[0]->Terminate();
    streams_[0]->Kill(WallTimer::GetTimeInMicroSeconds() + 1000000);
    streams_[0] = Stream::NewStream(Stream::MASTER, 0, command);
    return "OK";
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
    return "INVALID_ARGUMENT exit code must be an integer, but " + command;
  }

  string Run(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    if (args.size() != 2) {
      return StrCat(
          "INVALID_ARGUMENT run should have two arguments, but ",
          args.size(), ".");
    }
    int32 replicas = 0;
    if (!safe_strto32(args[0], &replicas)) {
      return StrCat(
          "INVALID_ARGUMENT # of replicas should be int32, but '",
          args[0], "'.");
    }
    string result = "OK";
    for (int i = 0; i < replicas; i++) {
      streams_.push_back(Stream::NewStream(
          Stream::WORKER, streams_.size(), args[1]));
      result += StringPrintf(" %d", streams_.back()->stream_id());
    }
    return result;
  }

  string Write(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    string error;
    vector<int> stream_ids = GetStreamIds(args[0], &error);
    if (!error.empty()) {
      return error;
    }
    string result = "OK";
    for (int stream_id : stream_ids) {
      if (streams_[stream_id]->WriteLine(args[1])) {
        result += StrCat(" ", stream_id);
      }
    }
    return result;
  }

  string Read(const string& command) {
    vector<string> args =
        strings::Split(command, strings::delimiter::Limit(" ", 1));
    string error;
    vector<int> stream_ids = GetStreamIds(args[0], &error);
    if (!error.empty()) {
      return error;
    }
    int32 timeout_in_millis = -1;
    if (args.size() >= 2) {
      if (!safe_strto32(args[1], &timeout_in_millis) ||
          timeout_in_millis <= 0) {
        return StrCat("INVALID_ARGUMENT timeout must be a positive integer, ",
                      "but ", args[1]);
      }
    }

    bool has_deadline = timeout_in_millis >= 0;
    int64 deadline_in_micros =
        WallTimer::GetTimeInMicroSeconds() + timeout_in_millis * 1000LL;
    string result;
    int stream_id;
    while (result.empty() && (!has_deadline || timeout_in_millis >= 0)) {
      timeout_in_millis = (deadline_in_micros + 999 -
                           WallTimer::GetTimeInMicroSeconds()) / 1000;
      stream_id = Stream::Poll(streams_, stream_ids,
                               has_deadline ? max(timeout_in_millis, 1) : -1);
      if (stream_id < 0) {
        return "DEADLINE_EXCEEDED No ready stream.";
      }
      result = streams_[stream_id]->ReadLine();
    }
    if (result.empty()) {
      return "UNAVAILABLE";
    }
    StripNewLine(&result);
    return StrCat("OK ", stream_id, " ", result);
  }

  string Kill(const string& command) {
    string error;
    vector<int> stream_ids = GetStreamIds(command, &error);
    if (!error.empty()) {
      return error;
    }
    string result = "OK";
    for (int stream_id : stream_ids) {
      streams_[stream_id]->Kill(0);
    }
    return "OK";
  }

  string List(const string& command) {
    string error;
    // TODO(imos): Make the first argument required.
    vector<int> stream_ids =
        GetStreamIds(command.empty() ? "worker" : command, &error);
    if (!error.empty()) {
      return error;
    }
    string result = "OK";
    for (int stream_id : stream_ids) {
      result += StrCat(" ", stream_id);
    }
    return result;
  }

  string Help(const string& command) {
    string result = "OK";
    if (command.empty()) {
      for (const auto& key_and_command : commands_) {
        result += " " + key_and_command.first;
      }
      return result;
    }

    auto* definition = FindOrNull(commands_, command);
    if (definition == nullptr) {
      return "NOT_FOUND Command is not found: " + command;
    }
    return StrCat("OK ", command, " ", definition->usage, " ... ",
                  definition->description);
  }

  string Communicate(const string& command) {
    if (!command.empty()) {
      return "INVALID_ARGUMENT communicate takes no arguments.";
    }
    for (int stream_id = 0; stream_id < streams_.size(); stream_id++) {
      if (streams_[stream_id]->IsRunning() && streams_[stream_id]->pid() == 0) {
        return StrCat("RESOURCE_EXHAUSTED Stream ", stream_id,
                      " already uses STDIO.");
      }
    }
    streams_.push_back(Stream::NewInteractiveStream(
        Stream::COMMUNICATOR, streams_.size()));
    return StrCat("OK ", streams_.size() - 1);
  }

 private:
  const int kInvalidStream = -static_cast<int>(Stream::INVALID_TYPE);

  struct Command {
    string usage;
    string description;
    std::function<string(StreamController*, const string&)> method;
  };

  vector<int> GetStreamIds(const string& stream_predicate, string* error) {
    Stream::StreamType stream_type = GetStreamType(stream_predicate);
    if (stream_type != Stream::UNKNOWN_TYPE) {
      vector<int> stream_ids;
      for (int stream_id = 0; stream_id < streams_.size(); stream_id++) {
        if (streams_[stream_id]->IsRunning() &&
            streams_[stream_id]->MatchStreamType(stream_type)) {
          stream_ids.push_back(stream_id);
        }
      }
      return stream_ids;
    }

    int stream_id;
    if (!safe_strto32(stream_predicate, &stream_id)) {
      if (error != nullptr) {
        *error = "INVALID_ARGUMENT Invalid stream ID: " + stream_predicate;
      }
      return {};
    }
    if (stream_id < 0 || (int)streams_.size() <= stream_id) {
      if (error != nullptr) {
        *error = StrCat("NOT_FOUND Out of stream ID range: ", stream_id);
      }
      return {};
    }
    return {stream_id};
  }

  static Stream::StreamType GetStreamType(const string& value) {
    if (value == "all") { return Stream::ALL_TYPE; }
    if (value == "master") { return Stream::MASTER; }
    if (value == "worker" || value == "-1") { return Stream::WORKER; }
    if (value == "communicator") { return Stream::COMMUNICATOR; }
    return Stream::UNKNOWN_TYPE;
  }

  const map<string, Command> commands_ = {
      {"run",
       Command{"<# of replicas> <command...>",
               "Spawns child processes, and returns stream IDs.",
               &StreamController::Run}},
      {"exec",
       Command{"<command...>",
               "Spawns a master process.  Kills the previous master process "
               "if exists.  Always returns \"OK\".",
               &StreamController::Exec}},
      {"write",
       Command{"<stream ID> <message...>",
               "Writes a message to the stream.",
               &StreamController::Write}},
      {"read",
       Command{"<stream ID> (<timeout in ms>)",
               "Reads a line from the stream(s).  Returns DEADLINE_EXCEEDED if "
               "all of the stream(s) reach EOF.",
               &StreamController::Read}},
      {"kill",
       Command{"<stream ID> (<timeout in ms>)",
               "Kills the stream.",
               &StreamController::Kill}},
      {"list",
       Command{"(all|master|worker|communicator)",
               "List streams that do not explicitly reach EOF.",
               &StreamController::List}},
      {"communicate",
       Command{"", "Creates a stream to communicate via standard I/O.",
               &StreamController::Communicate}},
      {"exit",
       Command{"(<exit code>)",
               "Exits with the exit code.  Exits with 0 if not explicitly "
               "given.",
               &StreamController::Exit}},
      {"help", Command{"", "Shows this message.", &StreamController::Help}},
  };
  vector<std::unique_ptr<Stream>> streams_;
  int exit_code_ = -1;

  DISALLOW_COPY_AND_ASSIGN(StreamController);
};

}  // namespace ninetan

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  ninetan::StreamController stream_master;
  stream_master.Init(FLAGS_master);
  return 0;
}
