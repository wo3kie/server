#include "./connection.hpp"
#include "./server.hpp"

template<
	typename TTask,
	typename TState
>
Connection< TTask, TState >::Connection( 
    asio::io_service & ioService,
    Server< TTask, TState > & server
)

#ifdef SERVER_SSL
    : m_socket( ioService, server.getSSLContext() )
#else
    : m_socket( ioService )
#endif

    , m_server( server )
    , m_task( this )

{
}

template<
	typename TTask,
	typename TState
>
#ifdef SERVER_SSL
ssl::stream< asio::ip::tcp::socket > &
#else
ip::tcp::socket &
#endif
Connection< TTask, TState >::socket()
{
    return m_socket;
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::setId(
    std::string const & id
)
{
    m_id = id;
}

template<
	typename TTask,
	typename TState
>
std::string const & Connection< TTask, TState >::getId() const 
{
    return m_id;
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::start(
    IConnection::Action const action
)
{

#ifdef SERVER_SSL
    auto const restart = boost::bind(
        & Connection< TTask, TState >::restartAgain,
        Connection< TTask, TState >::shared_from_this(),
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

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::restart(
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
                & Connection< TTask, TState >::parse,
                Connection< TTask, TState >::shared_from_this(),
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

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::read()
{
    restart( Action::Read );
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::parseError()
{
    m_task.parseError();
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::process()
{
    m_task.process();
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::parse(
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
        restart( m_task.parse( m_buffer, bytesTransferred ) );
    }
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::restartAgain(
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

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::response(
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        & Connection< TTask, TState >::doNothing,
        Connection< TTask, TState >::shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::log(
    char const * const message,
    std::size_t const size
)
{
    std::cout << ( getId().empty() ? "-" : getId() ) << ": ";
    std::cout.write( message, size );
    std::cout << std::endl;
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::doNothing(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Do Nothing Error: " << errorCode.message() << std::endl;

        disconnect();
    }
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::stop()
{
    char const * const message = "Goodbye.";
    response( message, strlen( message ) );
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::broadcast(
    char const * const message,
    std::size_t const size
)
{
    m_server.broadcast( Connection< TTask, TState >::shared_from_this(), message, size );
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::unicast(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    m_server.unicast( Connection< TTask, TState >::shared_from_this(), receiverId, message, size );
}

template<
	typename TTask,
	typename TState
>
void Connection< TTask, TState >::disconnect()
{
    m_server.disconnect( Connection< TTask, TState >::shared_from_this() );
}

