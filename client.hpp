#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
#include <boost/asio/ssl.hpp>
#endif

#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

#ifdef SERVER_SSL
namespace ssl = asio::ssl;
#endif

struct Task
{
    Task(
        asio::io_service & ioService,

#ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > & socket
#else
        ip::tcp::socket & socket
#endif
    );

    void run();

protected:

    virtual void runImpl() = 0;

protected:

    asio::io_service & m_ioService;

#ifdef SERVER_SSL
    ssl::stream< ip::tcp::socket > & m_socket;
#else
    ip::tcp::socket & m_socket;
#endif
};

template< typename TReader, typename TWriter >
struct Client
{
    Client(
        asio::io_service & ioService,
        std::string const & host,
        std::string const & port,
        int argc = 0,
        char* argv[] = {}
    );

private:

#ifdef SERVER_SSL
    bool verifyCertificate(
        bool const preverified,
        ssl::verify_context & ctx 
    );
#endif

    void onResolved(
        sys::error_code const & errorCode,
        ip::tcp::resolver::iterator endpointIterator 
    );

    void onConnected(
        sys::error_code const & errorCode 
    );

    void onHandShake(
        sys::error_code const & errorCode
    );

    asio::io_service & m_ioService;

#ifdef SERVER_SSL
    ssl::context m_ctx;
    ssl::stream< ip::tcp::socket > m_socket;
#else
    ip::tcp::socket m_socket;
#endif

    TReader m_reader;
    TWriter m_writer;
};

#include "./client.tpp"

#endif

