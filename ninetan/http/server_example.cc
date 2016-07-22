#include "base/base.h"
#include "ninetan/http/server.h"
#include "ninetan/http/server.pb.h"

DEFINE_int32(port, 8080, "Port number.");

using ninetan::http::Request;
using ninetan::http::Response;

void Handler(const Request& request, Response* response) {
  response->set_content_type("text/plain");
  response->set_data(request.DebugString());
}

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  ninetan::http::StartServer(Handler, FLAGS_port);
  return 0;
}
