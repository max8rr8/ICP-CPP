#define BOOST_COROUTINES_NO_DEPRECATION_WARNING
#pragma once 
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using tcp = boost::asio::ip::tcp;
typedef std::function<void(std::string, std::function<void(std::string)>)> handler;

namespace __tcp_server
{
// #define ICP_SERVER_DEBUG

#ifdef ICP_SERVER_DEBUG
void fail(boost::system::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}
#else
void fail(boost::system::error_code ec, char const *what)
{
}
#endif

void do_session(tcp::socket &socket, boost::asio::yield_context yield, handler handl)
{
    boost::system::error_code ec;

    tcp::socket sock = std::move(socket);

    for (;;)
    {
        std::string data(100u, '\0');
        sock.async_read_some(boost::asio::buffer(data), yield[ec]);
        if (ec)
            return fail(ec, "read");

        handl(data, [&sock, &yield, &ec](std::string str) {
            sock.async_send(boost::asio::buffer(str.c_str(), str.size()), [] (boost::system::error_code ec,
                                                                             std::size_t bytes_transferred) {

            
        });
        });
    }
}

void do_listen(boost::asio::io_context &ioc, tcp::endpoint endpoint, boost::asio::yield_context yield, handler handl)
{
    boost::system::error_code ec;

    tcp::acceptor acceptor(ioc);
    acceptor.open(endpoint.protocol(), ec);
    acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor.bind(endpoint, ec);
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

    for (;;)
    {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if (ec)
            fail(ec, "accept");
        else
            boost::asio::spawn(
                acceptor.get_executor().context(),
                std::bind(
                    &do_session,
                    std::move(socket),
                    std::placeholders::_1,
                    handl));
    }
}

void tcp(std::string hostname, unsigned short port, boost::asio::io_context &ioc, handler handl)
{

    auto const address = boost::asio::ip::make_address(hostname);

    boost::asio::spawn(ioc,
                       std::bind(
                           &do_listen,
                           std::ref(ioc),
                           tcp::endpoint{address, port},
                           std::placeholders::_1, handl));
}

std::function<void(std::string)> client(std::string host, unsigned short port, boost::asio::io_context &ioc, std::function<void(string)> handl)
{
    static tcp::socket s(ioc);
    boost::system::error_code ec;
    tcp::resolver resolver(ioc);
    boost::asio::connect(s, resolver.resolve(host, std::to_string(port)));
    if (ec)
        std::cerr << ec;

    static std::function<void()> do_read;
    static std::string buf;
    do_read = [handl ]() {
        
        
        static std::string data(100u, '\0');
        s.async_read_some(boost::asio::buffer(data), [handl](boost::system::error_code ec,
                                                                             std::size_t bytes_transferred) {
            
            for(int i = 0; i<bytes_transferred; i++){
                if(data[i] == '\n'){
                    handl(buf);
                    buf = "";
                }else if(data[i]!='\0') buf+=data[i];
            }
            fail(ec, "read");
            if(!ec)do_read();
        });
    };

    do_read();

    return [] (std::string str) {
        s.async_send(boost::asio::buffer(str.c_str(), str.size()), [] (boost::system::error_code ec,
                                                                             std::size_t bytes_transferred) {

            
        });
    };
}

} // namespace __tcp_server
