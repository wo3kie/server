#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "./connection.hpp"
#include "./connection_manager.hpp"

class Server
{
public:

    Server(
        std::string const & port
    );

    void run();

    void broadcast(
        ConnectionPtr const & sender,
        char const * const message,
        std::size_t const size
    );

    void unicast(
        ConnectionPtr const & sender,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void disconnect(
        ConnectionPtr const & sender
    );

private:

    void startAccept();

    void onAccepted(
        sys::error_code const & errorCode
    );

    void stop();

private:

    asio::io_service m_ioService;
    asio::io_service::strand m_strand;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

    ConnectionPtr m_newConnection;
    ConnectionManager m_connectionManager;
};

#endif

