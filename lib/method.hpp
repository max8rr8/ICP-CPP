#include "./server.hpp"
#include "./client.hpp"
#include <algorithm>
#pragma once
typedef std::function<void(json msg)> ContinueCallback;
typedef std::function<void(json msg, ICPWrite write, ContinueCallback next)> BeforeCallback;
typedef std::function<void(json msg, ICPWrite write)> AfterCallback;

template <typename TI, typename TO>
class LocalMethod
{
protected:
  std::string endpoint;
  std::function<void(TI inp, std::function<void(TO out)> res)> handler;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;

public:
  LocalMethod(ICPServer *server, std::string endpointP,
              std::function<void(TI inp, std::function<void(TO out)> res)> handlerP) : endpoint{endpointP},
                                                                                        handler{handlerP}
  {
    beforeCallback = [](auto msg, auto write, auto next) {
      next(msg);
    };
    afterCallback = [](auto msg, auto write) {
      write(msg);
    };

    server->subscribe(
        endpoint, [this](auto dMsg, auto dWrite) {
          auto write = [this, dWrite](json msg) {
            this->afterCallback(msg, dWrite);
          };

          this->beforeCallback(dMsg, write, [write, this](json msg) {
            if (msg["type"] == "call")
            {
              this->call(msg["params"], [write](auto out) {
                write(json{{"type", "ok"},
                           {"returns", out}});
              });
            }
            else
            {
              write({{"type", "error"},
                     {"code", 1},
                     {"msg", "Incoreect type"}});
            }
          });
        });
  }

  void call(TI inp, std::function<void (TO out)> done)
  {
    handler(inp, done);
  }
};

template <typename TI, typename TO>
class RemoteMethod
{
protected:
  ICPConnection *connection;
  std::string endpoint;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;

public:
  RemoteMethod(ICPConnection *connectionP, std::string endpointP) : connection{connectionP},
                                                                      endpoint{endpointP}
  {
    beforeCallback = [](json msg, ICPWrite write, auto next) { next(msg); };
    afterCallback = [](json msg, ICPWrite write) { write(msg); };
  }

public:
  void call(TI inp, std::function<void(TO out)> rep)
  {
    connection->send({{"endpoint", endpoint},
                     {"type", "call"},
                     {"params", inp}},
                    [rep](json m) {
                      rep(m["returns"]);
                    });
  }
};