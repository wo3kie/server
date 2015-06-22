#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "./connection.hpp"
#include "./connection_manager.hpp"

template< typename TTask >
class Server
{
public:

    typedef boost::shared_ptr< Connection< TTask > > ConnectionPtr;

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

#ifdef SERVER_SSL
    ssl::context & getSSLContext()
    {
        return m_sslContext;
    }
#endif

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

#ifdef SERVER_SSL
    ssl::context m_sslContext;
#endif

    ConnectionPtr m_newConnection;
    ConnectionManager< TTask > m_connectionManager;
};

#include "./server.tpp"
#include "./connection.tpp"

#endif

