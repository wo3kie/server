#include "./connection.hpp"
#include "./server.hpp"

Connection::Connection( 
    asio::io_service & ioService,
    ConnectionManager & connectionManager,
    Server & server
)
    : m_socket( ioService )
    , m_strand( ioService )
    , m_server( server )
    , m_task( this )
{
}

ip::tcp::socket & Connection::socket()
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
    Connection::Action const action
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
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            m_socket.async_read_some(
                asio::buffer( m_buffer, m_maxSize ),
                m_strand.wrap( parse )
            );

            break;
        }
    }
}
void Connection::read()
{
    start( Action::Read );
}

void Connection::parseError()
{
    m_task.parseError();
}

void Connection::process()
{
    m_task.process();
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
        start( m_task.parse( m_buffer, bytesTransferred ) );
    }
}

void Connection::startAgain(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Start Again Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        start();
    }
}

void Connection::response(
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        & Connection::doNothing,
        shared_from_this(),
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
    m_server.broadcast( shared_from_this(), message, size );
}

void Connection::unicast(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    m_server.unicast( shared_from_this(), receiverId, message, size );
}

void Connection::disconnect()
{
    m_server.disconnect( shared_from_this() );
}

