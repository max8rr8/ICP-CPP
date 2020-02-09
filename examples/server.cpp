#include "../lib/server.hpp"
#include <iostream>
#include "../lib/json.hpp"

using namespace std;
using json = nlohmann::json;

class StaticICPServer: public ICPServer {
  public: 

  void runTest() {
  this->handle({
    {"type", "test"},
    {"endpoint", "/test"},
    {"id", 12},
    {"echo", "test"}
  }, [](json msg){
    std::cout << "Got reply from endpoint: " << msg.dump() << endl;
  });
  }
};

int main() {
  StaticICPServer server = StaticICPServer();


  server.subscribe("/test", [] (json msg, ICPWrite write) {
    cout << "Endpoint called with echo: " << msg["echo"] << endl;
    write({
      {"type", "ok"},
      {"echo", msg["echo"]}
    });
  });

  server.documentate({
    
    {"endpoint", "/path/to/endpoint"},
    {"type", "typeOfEndpoint"},
    {"specificationOfType", "http://url.to/specification/ofTypeOfEndpoint"},
    {"description", "Here is documentation of endpoint"}
    
  });

  server.runTest();

  std::cout << "Documenation: " << server.getDocumentation() << endl;

  return 0;
}