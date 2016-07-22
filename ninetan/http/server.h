#include <functional>

#include "ninetan/http/server.pb.h"

namespace ninetan {
namespace http {

void StartServer(const std::function<void(const Request&, Response*)>& handler,
                 int port = 0);

} // namespace server
} // namespace ninetan
