#include "../lib/tcp.hpp"
#include "../lib/method.hpp"

void server() {
  boost::asio::io_context ioc{1};

  TcpICPServer serv(ioc, "0.0.0.0", 3232);

  LocalMethod<int, int> method(serv, "/method", [] (auto msg, auto done) {
    done(msg * 2);
  });
    

  std::cerr<<"Started"<<std::endl;
  ioc.run();
}

void client() {
  boost::asio::io_context ioc{1};

  TcpICPConnection conn(ioc, "localhost", 3232);

  static RemoteMethod<int, int> method(conn, "/method");

  method.call(2, [] (int rep) {
    std::cerr << "2 * 2 = " << rep;
  });

  std::cerr<<"Started"<<std::endl;
  ioc.run();
}

int main(int argc, char* argv[]) {
  if(argc == 2 && argv[1][0] == 's'){
    std::cout << "Starting server" << std::endl;
    server();
  } else if(argc == 2 && argv[1][0] == 'c'){
    std::cout << "Starting client" << std::endl;
    client();
  } else if(argc == 2) {
    std::cout << "Unknown task: " << argv[1];
  } else {
    std::cout << "Plese specify task: server or client";
  }
  
}