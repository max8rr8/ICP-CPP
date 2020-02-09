#include "../lib/client.hpp"
#include <iostream>
#include "../lib/json.hpp"

using namespace std;
using json = nlohmann::json;

class StaticICPConnection: public ICPConnection {
  public: 

  void __write(string msg){
    cout<<msg<<endl;
  }
};

int main() {
  StaticICPConnection conn = StaticICPConnection();

  conn.send({
    {"type", "test"},
    {"endpoint", "/test"},
    {"id", 12},
    {"echo", "test"}
  }, [] (json msg) {
    cout << "Replied with echo: " << msg["echo"] << endl;
  });

  return 0;
}