#include "../lib/tcp.hpp"

void server() {
  boost::asio::io_context ioc{1};
  TcpICPServer serv(ioc, "0.0.0.0", 32);

  serv.subscribe("/echo", [] (json msg, ICPWrite write) {
    std::cerr << "Echoing: " << msg["echo"];
    
    write({
      {"echoed", msg["echo"]}
    });
  });
    

  std::cerr<<"Started"<<std::endl;
  ioc.run();
}

void client() {
  boost::asio::io_context ioc{1};

  TcpICPConnection conn(ioc, "localhost", 32);

  conn.send(json{
    {"endpoint", "/echo"},
    {"echo", "Smth to echo"}
  }, [] (json rep) {
    std::cerr<<"Got echoed: " << rep["echoed"];
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