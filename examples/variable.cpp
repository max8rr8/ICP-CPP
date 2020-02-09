#include "../lib/tcp.hpp"
#include "../lib/variable.hpp"

void server() {
  boost::asio::io_context ioc{1};

  TcpICPServer serv(ioc, "0.0.0.0", 3232);

  LocalVariable<int> var(serv, "/var", 0);
    

  std::cerr<<"Started"<<std::endl;
  ioc.run();
}

void client() {
  boost::asio::io_context ioc{1};

  TcpICPConnection conn(ioc, "localhost", 3232);

  static RemoteVariable<int> var(conn, "/var");

  var.onChange([] (int val) {
    cout << "Changed to: " << val << "\n";
  });

  var.get([] (int val) {
    cout << "Value is: " << val << "\n";

    var.set(10, [] () {
      cout << "We changed it\n";

      var.get([] (int val) {
        cout << "And now value is: " << val << "\n";
      });
    });

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