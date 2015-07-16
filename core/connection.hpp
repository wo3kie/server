#ifndef _CORE_CONNECTION_HPP_
#define _CORE_CONNECTION_HPP_

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

    IConnection::Action getStartAction() const;

    void start(
        Action const action
    );

    void process();

    void read() override;

    void response(
        char const * const message,
        std::size_t const size
    ) override;

    void stop();

    void disconnect();

    #ifdef SERVER_SSL
        ssl::stream< asio::ip::tcp::socket > & socket();
    #else
        ip::tcp::socket & socket();
    #endif

protected:

    void dispatch(
        sys::error_code const & errorCode,
        IConnection::Action const action
    );

    void disconnectOnError(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void parse(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

protected:

    IServer * m_server;
    ITaskPtr m_task;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];

    #ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > m_socket;
    #else
        ip::tcp::socket m_socket;
    #endif
};

inline
Connection::Connection( 
    asio::io_service & ioService,
    IServer * server,
    ITaskPtr task
)
    #ifdef SERVER_SSL
        : m_socket( ioService, server->getSSLContext() )
    #else
        : m_socket( ioService )
    #endif

    , m_server( server )
    , m_task( task )
{
    m_task->setConnection( this );
}

#ifdef SERVER_SSL
    inline
    ssl::stream< asio::ip::tcp::socket > & Connection::socket()
#else
    inline
    ip::tcp::socket & Connection::socket()
#endif
    {
        return m_socket;
    }

inline
void Connection::start(
    IConnection::Action const action
)
{
    #ifdef SERVER_SSL

        auto const self = shared_from_this();

        auto const dispatch = [ self, action ](
            sys::error_code const & errorCode
        )
        {
            self->dispatch( errorCode, action );
        };

        m_socket.async_handshake(
            ssl::stream_base::server,
            dispatch
        );

    #else
        dispatch( sys::error_code(), action );
    #endif
}

inline
IConnection::Action Connection::getStartAction() const
{
    return m_task->getStartAction();
}

inline
void Connection::read()
{
    dispatch( sys::error_code(), Action::Read );
}

inline
void Connection::process()
{
    m_task->process();
}

inline
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
        dispatch( sys::error_code(), m_task->parse( m_buffer, bytesTransferred ) );
    }
}

inline
void Connection::dispatch(
    sys::error_code const & errorCode,
    IConnection::Action const action
)
{
    if( errorCode )
    {
        std::cerr << "dispatch Again Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        switch( action )
        {
            case Action::Process:
            {
                process();
                
                break;
            }

            case Action::Read:
            {
                auto self = shared_from_this();

                auto const parse = [ self ](
                    sys::error_code const & errorCode,
                    std::size_t const bytesTransferred
                )
                {
                    self->parse( errorCode, bytesTransferred );
                };

                m_socket.async_read_some(
                    asio::buffer( m_buffer, m_maxSize ),
                    parse
                );

                break;
            }
        }
    }
}

inline
void Connection::response(
    char const * const message,
    std::size_t const size
)
{
    auto self = shared_from_this();

    auto const continuation = [ self ](
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    )
    {
        self->disconnectOnError( errorCode, bytesTransferred );
    };

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

inline
void Connection::disconnectOnError(
    sys::error_code const & errorCode,
    std::size_t const bytesTransferred
)
{
    if( errorCode )
    {
        std::cerr << "Disconnect On Error Error: " << errorCode.message() << std::endl;

        disconnect();
    }
}

inline
void Connection::stop()
{
    char const * const message = "Goodbye.";
    response( message, strlen( message ) );
}

inline
void Connection::disconnect()
{
    m_server->disconnect( shared_from_this() );
}

#endif

