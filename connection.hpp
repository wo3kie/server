#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <string>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "./iconnection.hpp"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

template< typename TTask >
class Server;

template< typename TTask >
class Connection
    : public IConnection
    , public boost::enable_shared_from_this< Connection< TTask > >
{
public:

    typedef boost::shared_ptr< Connection< TTask > > ConnectionPtr;

public:

    Connection( 
        asio::io_service & ioService,
        Server< TTask > & server
    );

    ip::tcp::socket & socket();

    void disconnect();

    void stop();

    void start(
        Action const action
    );

public: // api

    void setId(
        std::string const & id
    );

    std::string const & getId() const ;

    void read();

    void parseError();

    void process();

    void response(
        char const * const message,
        std::size_t const size
    );

    void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void broadcast(
        char const * const message,
        std::size_t const size
    );

    void log(
        char const * const message,
        std::size_t const size 
    );

    void doNothing(
        sys::error_code const & errorCode
    );

private:

    void parse(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void startAgain(
        sys::error_code const & errorCode
    );

private:

    ip::tcp::socket m_socket;
    asio::io_service::strand m_strand;
    Server< TTask > & m_server;

    TTask m_task;

    std::string m_id;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];
};

#endif

