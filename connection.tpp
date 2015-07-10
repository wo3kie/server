#include "./connection.hpp"
#include "./server.hpp"

Connection::Connection( 
    asio::io_service & ioService,
    IServer * server,
    ITask * task
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

void Connection::setId(
    std::string const & id
)
{
    m_id = id;
}

std::string const & Connection::getId() const 
{
    return m_id;
}

void Connection::start(
    MyConnection::Action const action
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
    MyConnection::Action const action
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
    MyConnection::Action const action
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

void Connection::log(
    char const * const message,
    std::size_t const size
)
{
    std::cout << ( getId().empty() ? "-" : getId() ) << ": ";
    std::cout.write( message, size );
    std::cout << std::endl;
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

void Connection::broadcast(
    char const * const message,
    std::size_t const size
)
{
    m_server->broadcast( Connection::shared_from_this(), message, size );
}

void Connection::unicast(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    m_server->unicast( Connection::shared_from_this(), receiverId, message, size );
}

void Connection::disconnect()
{
    m_server->disconnect( Connection::shared_from_this() );
}

