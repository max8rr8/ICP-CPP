#include "../lib/tcp.hpp"
#include "../lib/topic.hpp"

// #include <boost/chrono/chrono.hpp>

void server() {
  boost::asio::io_context ioc{1};

  TcpICPServer serv(ioc, "0.0.0.0", 3232);

  static LocalTopic<int> topic(serv, "/topic");
    

  std::cerr<<"Started"<<std::endl;

  topic.subscribe([] (int rep) {
    std::cerr << "Random: " << rep;
  });


  static boost::posix_time::seconds interval(5);  // 1 second
  static boost::asio::deadline_timer timer(ioc, interval);

  static std::function<void (const boost::system::error_code& e)> tick;
  
  tick = [] (const boost::system::error_code& e) {
    topic.publish(rand());

    timer.expires_at(timer.expires_at() + interval);
    timer.async_wait(tick);
  };

  timer.async_wait(tick);
  ioc.run();
}

void client() {
  boost::asio::io_context ioc{1};

  TcpICPConnection conn(ioc, "localhost", 3232);

  static RemoteTopic<int> topic(conn, "/topic");

  topic.subscribe([] (int rep) {
    std::cerr << "Random: " << rep;
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