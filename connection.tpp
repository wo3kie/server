#include "./connection.hpp"
#include "./server.hpp"

template< typename TTask >
Connection< TTask >::Connection( 
    asio::io_service & ioService,
    Server< TTask > & server
)
    : m_socket( ioService )
    , m_server( server )
    , m_task( this )
{
}

template< typename TTask >
ip::tcp::socket & Connection< TTask >::socket()
{
    return m_socket;
}

template< typename TTask >
void Connection< TTask >::setId(
    std::string const & id
)
{
    m_id = id;
}

template< typename TTask >
std::string const & Connection< TTask >::getId() const 
{
    return m_id;
}

template< typename TTask >
void Connection< TTask >::start(
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
                & Connection< TTask >::parse,
                Connection< TTask >::shared_from_this(),
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

template< typename TTask >
void Connection< TTask >::read()
{
    start( Action::Read );
}

template< typename TTask >
void Connection< TTask >::parseError()
{
    m_task.parseError();
}

template< typename TTask >
void Connection< TTask >::process()
{
    m_task.process();
}

template< typename TTask >
void Connection< TTask >::parse(
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

template< typename TTask >
void Connection< TTask >::startAgain(
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

template< typename TTask >
void Connection< TTask >::response(
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        & Connection< TTask >::doNothing,
        Connection< TTask >::shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

template< typename TTask >
void Connection< TTask >::log(
    char const * const message,
    std::size_t const size
)
{
    std::cout << ( getId().empty() ? "-" : getId() ) << ": ";
    std::cout.write( message, size );
    std::cout << std::endl;
}

template< typename TTask >
void Connection< TTask >::doNothing(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Do Nothing Error: " << errorCode.message() << std::endl;

        disconnect();
    }
}

template< typename TTask >
void Connection< TTask >::stop()
{
    char const * const message = "Goodbye.";
    response( message, strlen( message ) );
}

template< typename TTask >
void Connection< TTask >::broadcast(
    char const * const message,
    std::size_t const size
)
{
    m_server.broadcast( Connection< TTask >::shared_from_this(), message, size );
}

template< typename TTask >
void Connection< TTask >::unicast(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    m_server.unicast( Connection< TTask >::shared_from_this(), receiverId, message, size );
}

template< typename TTask >
void Connection< TTask >::disconnect()
{
    m_server.disconnect( Connection< TTask >::shared_from_this() );
}

