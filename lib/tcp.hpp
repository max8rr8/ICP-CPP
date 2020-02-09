#include "./server.hpp"
#include "./client.hpp"
#pragma once 
#include <string>
#include <regex>
#include <iostream>
#include <functional>
#include "./server/tcp.hpp"

class TcpICPServer : public ICPServer
{
private:
public:
  TcpICPServer(boost::asio::io_context &ioc)
  {
    TcpICPServer(ioc, 32);
  }

  TcpICPServer(boost::asio::io_context &ioc, int port)
  {
    TcpICPServer(ioc, "0.0.0.0", port);
  }

  TcpICPServer(boost::asio::io_context &ioc, string hostname, int port)
  {
    __tcp_server::tcp(hostname, port, ioc, [this](auto msg, auto write) {
      try
      {
        this->handle(json::parse(msg), [write](json rep) {
          write(
            (string)rep.dump() + "\n"
          );
        });
      }
      catch (json::exception &e)
      {
      }
    });
  }
};

class TcpICPConnection : public ICPConnection
{
private:

public:
  std::function<void(std::string t)> wr;

  TcpICPConnection(){
  
  }

  TcpICPConnection(boost::asio::io_context &ioc)
  {
    TcpICPConnection(ioc, 32);
  }

  TcpICPConnection(boost::asio::io_context &ioc, int port)
  {
    TcpICPConnection(ioc, (string)"192.168.0.1", port);
  }

  TcpICPConnection(boost::asio::io_context &ioc, string hostname, int port)
  {
    
    wr = __tcp_server::client(hostname, port, ioc, [&](std::string rep) {
      try
      {
        ICPConnection::handle(json::parse(rep));
      }
      catch (json::exception &e)
      {
      }
    });
  }

   void __write(std::string m) override {
    wr(m);
  }
};