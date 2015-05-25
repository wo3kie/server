#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <string>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "./task.hpp"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

class ConnectionManager;
class Server;

class Connection
    : public boost::enable_shared_from_this< Connection >
{
public:

    typedef boost::shared_ptr< Connection > ConnectionPtr;

public:

    enum class Action
    {
        Read,
        ReadError,
        Process
    };

    Connection( 
        asio::io_service & ioService,
        ConnectionManager & connectionManager,
        Server & server
    );

    ip::tcp::socket & socket();

    void disconnect();

    void stop();

    void start(
        Action const action = Task< Connection >::start()
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
    )
    {
        if( errorCode )
        {
            std::cerr << "Do Nothing Error: " << errorCode.message() << std::endl;

            disconnect();
        }
    }

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
    Server & m_server;

    Task< Connection > m_task;

    std::string m_id;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];
};

typedef Connection::ConnectionPtr ConnectionPtr;

#endif

