#include "./server.hpp"
#include "./client.hpp"
#pragma once 
#include <string>
#include <regex>
#include <iostream>
#include "./server/ws.hpp"

class WsICPServer: public ICPServer {
  private:



  public: 
  WsICPServer (boost::asio::io_context& ioc) {
    WsICPServer(ioc, 32);
  }

  WsICPServer (boost::asio::io_context& ioc, int port) {
    WsICPServer(ioc, "0.0.0.0", port);
  }

  WsICPServer (boost::asio::io_context& ioc, string hostname, int port) {
    __ws_server::ws(hostname, port, ioc, [this] (auto msg, auto rep) {
      try {
      this->handle(json::parse(msg), [&rep] (json msg) {
        rep((string)msg.dump());
      });
      

    }catch(json::exception& e){
     }

    }      );
  }

};