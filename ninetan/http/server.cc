#include "ninetan/http/server.h"

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <functional>
#include <thread>
#include <string>
#include <algorithm>

#include "base/base.h"

namespace ninetan {
namespace http {

#define CHECK_SYS(...) CHECK((__VA_ARGS__) >= 0) << strerror(errno)

std::unique_ptr<Request> ReadRequest(int fd) {
  string head;
  string data;
  bool done = false;
  for (; !done;) {
    char buf[1024];
    int len = read(fd, buf, sizeof(buf));
    if (len == 0) break;
    if (len < 0) {
      LOG(WARNING) << "Failed to read: " << strerror(errno);
      return nullptr;
    }
    for (int i = 0; i < len; i++) {
      if (buf[i] == '\r') continue;
      if (buf[i] == '\n' && head.size() > 0 && head.back() == '\n') {
        data.append(&buf[i + 1], len - i - 1);
        done = true;
        break;
      }
      head.append(1, buf[i]);
    }
  }

  LOG(INFO) << "Parsing headers...";
  std::unique_ptr<Request> request(new Request);
  vector<string> headers = strings::Split(head, "\n", strings::SkipEmpty());
  if (headers.size() == 0) {
    LOG(WARNING) << "No header.";
    return nullptr;
  }
  vector<string> first =
      strings::Split(headers[0], strings::delimiter::Limit(" ", 2));
  if (first.size() != 3) {
    LOG(WARNING) << "First header should be 3 parts, but " << first.size();
    return nullptr;
  }
  request->set_method(first[0]);
  request->set_path(first[1]);
  request->set_version(first[2]);

  headers.erase(headers.begin());
  int content_length = 0;
  for (const string& header : headers) {
    vector<string> parts =
        strings::Split(header, strings::delimiter::Limit(":", 1));
    if (parts.size() != 2) {
      LOG(WARNING) << "Invalid header: " << header;
      return nullptr;
    }
    StripWhiteSpace(&parts[0]);
    StripWhiteSpace(&parts[1]);
    Request::Header* key_value = request->add_header();
    std::transform(parts[0].begin(), parts[0].end(),
                   parts[0].begin(), ::tolower);
    key_value->set_key(parts[0]);
    key_value->set_value(parts[1]);
    if (parts[0] == "content-length") {
      content_length = atoi32(parts[1]);
    }
  }

  while (data.size() < content_length) {
    char buf[1024];
    int len = read(fd, buf, sizeof(buf));
    if (len < 0) {
      LOG(WARNING) << "Failed to read: " << strerror(errno);
      return nullptr;
    }
    data.append(buf, len);
  }
  data.erase(data.begin() + content_length, data.end());
  if (!data.empty()) {
    request->set_data(data);
  }

  return request;
}

void WriteResponse(int fd, const Response& response) {
  string data;
  data += StrCat("HTTP/1.1 ", (int)response.code(),
                 " ", response.status(), "\r\n");
  data += "Content-Type: " + response.content_type() + "\r\n";
  data += StrCat("Content-Length: ", response.data().size(), "\r\n");
  data += "Connection: close\r\n";
  data += "\r\n";
  data += response.data();
  int length = write(fd, data.c_str(), data.size());
  if (length < 0) {
    LOG(WARNING) << "Failed to return a response.";
    return;
  }
}

void HandleRequest(
    int fd, const std::function<void(const Request&, Response*)>& handler) {
  std::unique_ptr<Request> request = ReadRequest(fd);
  Response response;
  if (request != nullptr) {
    handler(*request, &response);
  } else {
    response.set_code(Response::INTERNAL_SERVER_ERROR);
    response.set_status("Internal Server Error");
  }
  WriteResponse(fd, response);
  close(fd);
}

void StartServer(const std::function<void(const Request&, Response*)>& handler,
                 int port) {
  int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  CHECK_SYS(listen_fd);

  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);
  int so_reuseaddr = 1;
  CHECK_SYS(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
                       sizeof(so_reuseaddr)));
  CHECK_SYS(::bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)));
  CHECK_SYS(listen(listen_fd, 10));

  LOG(INFO) << "Listening :" << ntohs(server_addr.sin_port) << "...";
  for (;;) {
    int fd = accept(listen_fd, nullptr, nullptr);
    LOG(INFO) << "Accepted " << fd << ".";
    CHECK_SYS(fd);
    std::thread t([=](){ HandleRequest(fd, handler); });
    t.detach();
  }
}

} // namespace server
} // namespace ninetan
