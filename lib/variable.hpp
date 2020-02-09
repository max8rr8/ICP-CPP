
#include "./client.hpp"
#include <algorithm>
#pragma once

typedef std::function<void(json msg)> ContinueCallback;
typedef std::function<void(json msg, ICPWrite write, ContinueCallback next)> BeforeCallback;
typedef std::function<void(json msg, ICPWrite write)> AfterCallback;

#ifndef ICP_NO_LOCAL 
template <typename T>

#include "./server.hpp"

class LocalVariable
{
protected:
  std::string endpoint;
  T defaultValue;
  std::map<string, T> values;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;
  std::map<string, std::vector<std::function<void(T value)>>> subscribers;

public:
  LocalVariable(ICPServer &server, std::string endpointP, T defaultValueP) : endpoint{endpointP},
                                                                             defaultValue{defaultValueP}
  {
    values[endpoint] = defaultValue;
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
            if (msg["type"] == "get")
            {
              write(json{{"type", "ok"},
                         {"value", this->get(msg["endpoint"])}});
            }
            else if (msg["type"] == "set")
            {
              this->set(msg["endpoint"], msg["value"]);
              write(json{
                  {"type", "ok"},
              });
            }
            else if (msg["type"] == "subscribe")
            {
              this->onChange(
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

  T get()
  {
    return get(endpoint);
  }

  T get(std::string endpoint)
  {
    if (values.find(endpoint) != values.end())
      return values[endpoint];
    return defaultValue;
  }

  void set(T val)
  {
    set(endpoint, val);
  }

  void set(std::string endpoint, T val)
  {
    if (subscribers.find(endpoint) == subscribers.end())
      subscribers[endpoint] = {};
    values[endpoint] = val;
    for (auto sub : subscribers[endpoint])
      sub(val);

    subscribers[endpoint] = {};
  }

  void onChange(std::function<void(T val)> callback)
  {
    onChange(endpoint, callback);
  }

  void onChange(std::string endpoint, std::function<void(T val)> callback)
  {
    if (subscribers.find(endpoint) == subscribers.end())
      subscribers[endpoint] = {};
    subscribers[endpoint].push_back(callback);
  }
};

#endif

template <typename T>
class RemoteVariable
{
protected:
  ICPConnection *connection;
  std::string endpoint;
  BeforeCallback beforeCallback;
  AfterCallback afterCallback;

public:
  RemoteVariable(ICPConnection *connectionP, std::string endpointP):
    connection{connectionP},
    endpoint{endpointP}
  {
    beforeCallback = [] (json msg, ICPWrite write, std::function<void (json)> next) { next(msg); };
    afterCallback = [] (json msg, ICPWrite write) { write(msg); };
  }

public:

  void get(std::function<void (T val)> rep)
  {
    connection->send({
      {"endpoint", endpoint},
      {"type", "get"}
    }, [rep] (json m) {
      rep(m["value"]);
    });
  }

  void set(T val, std::function<void ()> rep)
  {
    connection->send({
      {"endpoint", endpoint},
      {"type", "set"},
      {"value", val}
    }, [rep] (json m) {
      rep();
    });
  }

  void onChange(std::function<void (T val)> rep)
  {
    connection->send({
      {"endpoint", endpoint},
      {"type", "subscribe"}
    }, [rep] (json m) {
      rep(m["value"]);
    });
  }
};