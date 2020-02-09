#include "../lib/ws.hpp"

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc{1};
  WsICPServer(ioc, "0.0.0.0", 8080);
  ioc.run();
}