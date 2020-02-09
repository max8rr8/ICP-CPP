#define BOOST_COROUTINES_NO_DEPRECATION_WARNING
#pragma once 
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
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
namespace websocket = boost::beast::websocket; 

typedef std::function<void(std::string, std::function<void (std::string)>)> handler;

namespace __ws_server {
#ifdef ICP_SERVER_DEBUG
void fail(boost::system::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
#else
void fail(boost::system::error_code ec, char const* what) {}
#endif

void do_session(tcp::socket& socket, boost::asio::yield_context yield, handler handl) {
    boost::system::error_code ec;

    websocket::stream<tcp::socket> ws{std::move(socket)};
    ws.async_accept(yield[ec]);
    if(ec)
        return fail(ec, "accept");

    for(;;)  {
        boost::beast::multi_buffer buffer;
        ws.async_read(buffer, yield[ec]);

        if(ec == websocket::error::closed)
            break;

        if(ec)
            return fail(ec, "read");

        ws.got_text();

        handl(boost::beast::buffers_to_string(buffer.data()), [&ws, &yield, &ec] (std::string str) {
	        ws.text(ws.got_text());
	        ws.async_write(boost::asio::buffer(str.c_str(), str.size()), yield[ec]);
        });
    }
}

void do_listen(boost::asio::io_context& ioc, tcp::endpoint endpoint, boost::asio::yield_context yield, handler handl){
    boost::system::error_code ec;

    tcp::acceptor acceptor(ioc);
    acceptor.open(endpoint.protocol(), ec);
    acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor.bind(endpoint, ec);
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

    for(;;)
    {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if(ec)
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

void ws(std::string hostname, unsigned short port, boost::asio::io_context& ioc, handler handl) {

    auto const address = boost::asio::ip::make_address(hostname);

    boost::asio::spawn(ioc,
        std::bind(
            &do_listen,
            std::ref(ioc),
            tcp::endpoint{address, port},
            std::placeholders::_1, handl));
}



}

