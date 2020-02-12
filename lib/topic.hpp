#include "./client.hpp"
#include <algorithm>
#pragma once
typedef std::function<void(json msg)> ContinueCallback;
typedef std::function<void(json msg, ICPWrite write, ContinueCallback next)> BeforeCallback;
typedef std::function<void(json msg, ICPWrite write)> AfterCallback;



#ifndef ICP_NO_LOCAL 
#include "./server.hpp"

template <typename T>
class LocalTopic
{
protected:
  std::string endpoint;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;
  std::map<string, std::vector<std::function<void(T value)>>> subscribers;

public:
  LocalTopic(ICPServer *server, std::string endpointP) : endpoint{endpointP}
  {
    beforeCallback = [](auto msg, auto write, auto next) {
      next(msg);
    };
    afterCallback = [](auto msg, auto write) {
      write(msg);
    };

    server.subscribe(
        endpoint, [this](auto dMsg, auto dWrite) {
          auto write = [this, dWrite](json msg) {
            this->afterCallback(msg, dWrite);
          };

          this->beforeCallback(dMsg, write, [write, this](json msg) {
            if (msg["type"] == "subscribe")
            {

              this->subscribe(
                  msg["endpoint"], [write](T val) {
                    
                    write({{"type", "publish"},
                           {"value", val}});
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

  void publish(T val)
  {
    publish(endpoint, val);
  }

  void publish(std::string endpoint, T val)
  {
    if (subscribers.find(endpoint) == subscribers.end())
      subscribers[endpoint] = {};

    for (auto sub : subscribers[endpoint])
      sub(val);

    subscribers[endpoint] = {};
  }

  void subscribe(std::function<void(T val)> callback)
  {
    subscribe(endpoint, callback);
  }

  void subscribe(std::string endpoint, std::function<void(T val)> callback)
  {
    if (subscribers.find(endpoint) == subscribers.end())
      subscribers[endpoint] = {};
    subscribers[endpoint].push_back(callback);
  }
};
#endif

template <typename T>
class RemoteTopic
{
protected:
  ICPConnection* connection;
  std::string endpoint;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;

public:
  RemoteTopic(ICPConnection* connectionP, std::string endpointP):
    connection{connectionP},
    endpoint{endpointP}
  {
    beforeCallback = [] (json msg, ICPWrite write, std::function<void (json)> next) { next(msg); };
    afterCallback = [] (json msg, ICPWrite write) { write(msg); };
  }

  void subscribe(std::function<void (T val)> rep)
  {
    connection->send({
      {"endpoint", endpoint},
      {"type", "subscribe"}
    }, [rep] (json m) {
      rep(m["value"]);
    });
  }
};