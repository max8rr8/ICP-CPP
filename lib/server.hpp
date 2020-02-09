#include <string>
#include <functional>
#include <vector>
#include <regex>
#include <iostream>
#include "./json.hpp"
#pragma once 
using namespace std;
using json = nlohmann::json;

typedef function<void(json msg)> ICPWrite;
typedef function<void(json msg, ICPWrite write)> ICPEndpointCallback;

struct ICPEndpoint {
  string endpoint;
  ICPEndpointCallback callback;
  std::regex endpointRegex;
};

class ICPServer {
  vector<ICPEndpoint> endpoints;
  std::vector<json> documentation;

public:

  void subscribe(string endpointPath, ICPEndpointCallback callback){
    ICPEndpoint endpoint;
    endpoint.endpoint = endpointPath;
    endpoint.callback = callback;

    endpointPath = std::regex_replace(endpointPath, std::regex("\\*"), ".*");
    endpointPath = std::regex_replace(endpointPath, std::regex("\\/:[a-zA-Z0-9]+"), "/[^/]*");
    endpointPath = std::regex_replace(endpointPath, std::regex(  "/"), "/");

    endpoint.endpointRegex =  std::regex("^" +endpointPath+ "$"); 

    endpoints.push_back(endpoint);
  }
  

  void documentate(json doc){
    documentation.push_back(doc);
  }

  string getDocumentation(){
    json j = documentation;
	  return j.dump();
  }

  void handle(json msg, ICPWrite write){
    bool found = false;
    if(!msg.contains("id"))msg["id"] = 0;

    if(msg.contains("endpoint")){
    for(int i = 0; (i < endpoints.size()) && !found; i++){
	    if(std::regex_match((string)msg["endpoint"], endpoints[i].endpointRegex)){
        found=true;
        endpoints[i].callback(msg, [write, msg] (json rep) {
          rep["id"] = msg["id"];
          write(rep);
        });
	    }
    }}else{
      json reply = json::parse("{\"type\": \"error\", \"msg\":\"Endpoint not specified\", \"code\": -1}");
	    reply["id"] = msg["id"];
	    write(reply);

      return;
    }

    if(!found){
	    json reply = json::parse("{\"type\": \"error\", \"msg\":\"Endpoint '" + (string)msg["endpoint"] + "' not found\", \"code\": -1}");
	    reply["id"] = msg["id"];
	    write(reply);
    }
  }
};