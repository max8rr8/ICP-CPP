#include <string>
#include <functional>
#include <vector>
#include <regex>
#include <iostream>
#include "./json.hpp"
#pragma once 
using namespace std;
using json = nlohmann::json;

typedef function<void(json msg)> ICPCallback;
typedef function<void(json msg)> ICPWrite;

class ICPConnection {
public:
  std::map<int, ICPCallback> handlers;

  ICPConnection() {
    handlers = {};
  }

  void send(json msg, ICPCallback callback){

    int id = rand();
    msg["id"] = id;
    handlers[id] = callback;
    this->__write(msg.dump() + "\n");
  }

  virtual void __write(string msg){};

  void handle(json rep) {
    int id =  rep["id"];
    if(handlers.find(id) != handlers.end()){
      handlers[id](rep);
      handlers.erase(id);
	  }
  }
};
