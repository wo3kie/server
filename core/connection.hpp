#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
#include <boost/asio/ssl.hpp>
#endif

#include <boost/enable_shared_from_this.hpp>

#include "./iconnection.hpp"
#include "./iserver.hpp"
#include "./itask.hpp"

class Connection
    : public IConnection
    , public boost::enable_shared_from_this< Connection >
{
public:

    Connection( 
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    );

    void start(
        Action const action
    );

    void restart(
        Action const action
    );

    void stop();

    void parseError();

    void process();

    void doNothing(
        sys::error_code const & errorCode
    );

    void disconnect();

public: // api

    void read() override;

    void response(
        char const * const message,
        std::size_t const size
    ) override;

    IConnection::Action getStartAction() const
    {
        return m_task->getStartAction();
    }

public: // ssl

    #ifdef SERVER_SSL
        ssl::stream< asio::ip::tcp::socket > & socket();
    #else
        ip::tcp::socket & socket();
    #endif

protected:

    void parse(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void restartAgain(
        sys::error_code const & errorCode,
        IConnection::Action const action
    );

protected:

    IServer * m_server;
    ITaskPtr m_task;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];

protected: // ssl

    #ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > m_socket;
    #else
        ip::tcp::socket m_socket;
    #endif

};

Connection::Connection( 
    asio::io_service & ioService,
    IServer * server,
    ITaskPtr task
)

    #ifdef SERVER_SSL
        : m_socket( ioService, server.getSSLContext() )
    #else
        : m_socket( ioService )
    #endif

    , m_server( server )
    , m_task( task )
{
    m_task->setConnection( this );
}

#ifdef SERVER_SSL
    ssl::stream< asio::ip::tcp::socket > &
#else
    ip::tcp::socket &
#endif
    Connection::socket()
    {
        return m_socket;
    }

void Connection::start(
    IConnection::Action const action
)
{

    #ifdef SERVER_SSL
        auto const restart = boost::bind(
            & Connection::restartAgain,
            Connection::shared_from_this(),
            placeholders::error,
            action
        );

        m_socket.async_handshake(
            ssl::stream_base::server,
            restart
        );
    #else
        restart( action );
    #endif

}

void Connection::restart(
    IConnection::Action const action
)
{
    switch( action )
    {
        case Action::Process:
        {
            process();
            
            break;
        }

        case Action::ReadError:
        {
            parseError();

            break;
        }

        case Action::Read:
        {
            auto const parse = boost::bind(
                & Connection::parse,
                Connection::shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            m_socket.async_read_some(
                asio::buffer( m_buffer, m_maxSize ),
                parse
            );

            break;
        }
    }
}

void Connection::read()
{
    restart( Action::Read );
}

void Connection::parseError()
{
    m_task->parseError();
}

void Connection::process()
{
    m_task->process();
}

void Connection::parse(
    sys::error_code const & errorCode,
    std::size_t const bytesTransferred
)
{
    if( errorCode )
    {
        std::cerr << "Parse Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        restart( m_task->parse( m_buffer, bytesTransferred ) );
    }
}

void Connection::restartAgain(
    sys::error_code const & errorCode,
    IConnection::Action const action
)
{
    if( errorCode )
    {
        std::cerr << "Restart Again Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        restart( action );
    }
}

void Connection::response(
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        & Connection::doNothing,
        Connection::shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

void Connection::doNothing(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Do Nothing Error: " << errorCode.message() << std::endl;

        disconnect();
    }
}

void Connection::stop()
{
    char const * const message = "Goodbye.";
    response( message, strlen( message ) );
}

void Connection::disconnect()
{
    m_server->disconnect( Connection::shared_from_this() );
}

#endif

