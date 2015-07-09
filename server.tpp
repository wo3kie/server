#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

#include "./server_chat.hpp"
#include "./connection.hpp"
#include "./server.hpp"

template<
	typename TTask,
	typename TState
>
Server< TTask, TState >::Server(
    std::string const & port
)
    : m_strand( m_ioService )
    , m_acceptor( m_ioService )
    , m_signals( m_ioService )

#ifdef SERVER_SSL
    , m_sslContext( ssl::context::sslv23 )
#endif

{
    m_signals.add( SIGINT );
    m_signals.add( SIGTERM );
    m_signals.add( SIGQUIT );

    m_signals.async_wait(
        boost::bind( & Server< TTask, TState >::stop, this )
    );

    ip::tcp::resolver resolver( m_ioService );
    ip::tcp::resolver::query query( "127.0.0.1", port );
    ip::tcp::endpoint endpoint = * resolver.resolve( query );

    m_acceptor.open( endpoint.protocol() );
    m_acceptor.set_option( ip::tcp::acceptor::reuse_address( true ) );
    m_acceptor.bind( endpoint );
    m_acceptor.listen();

#ifdef SERVER_SSL
    m_sslContext.set_options(
        ssl::context::default_workarounds
        | ssl::context::no_sslv2
        | ssl::context::single_dh_use
    );

    auto const passwordCallback = [ & ](
        std::size_t size,
        std::size_t purpose
    ) -> std::string
    {
        return "test";
    };

    m_sslContext.set_password_callback( passwordCallback );

    m_sslContext.use_certificate_chain_file( "server.pem" );
    m_sslContext.use_private_key_file( "server-key.pem", ssl::context::pem );
#endif

    startAccept();
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::init( int argc, char* argv[] )
{
    stateInit( argc, argv );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::destroy()
{
    stateDestroy();
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::run()
{
    run( 0, {} );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::run(
    int argc,
    char* argv[]
)
{
    init( argc, argv );

    boost::thread_group threadGroup; 

    auto const threadBody = boost::bind(
        & asio::io_service::run,
        & m_ioService
    );

    for( unsigned i = 0 ; i < 4 ; ++ i )
    {
        threadGroup.create_thread( threadBody );
    }

    threadGroup.join_all();

    destroy();
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::broadcast(
    ConnectionPtr const & sender,
    char const * const message,
    std::size_t const size
)
{
    auto const skipSender = [ & sender ]( ConnectionPtr const & connectionPtr )
    {
        return sender->socket().native_handle() != connectionPtr->socket().native_handle();
    };

    auto sendMessage = [ this, & sender, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        connectionPtr->response( message, size );
    };

    m_connectionManager.forEachIf( skipSender, sendMessage );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::unicast(
    ConnectionPtr const & sender,
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( ConnectionPtr const & connectionPtr )
    {
        return receiverId == connectionPtr->getId();
    };

    auto sendMessage = [ this, & sender, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        connectionPtr->response( message, size );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::disconnect(
    ConnectionPtr const & sender
)
{
    m_connectionManager.remove( sender );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::startAccept()
{
    m_newConnection.reset( createConnection() );

    auto const onAccepted = boost::bind(
        & Server< TTask, TState >::onAccepted,
        this,
        placeholders::error
    );
   
    m_acceptor.async_accept(
#ifdef SERVER_SSL
        m_newConnection->socket().lowest_layer(),
#else
        m_newConnection->socket(),
#endif
        onAccepted
    );
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::onAccepted(
    sys::error_code const & errorCode
)
{
    if( ! errorCode )
    {
        m_connectionManager.add( m_newConnection );

        m_newConnection->start( TTask::start() );
    }

    startAccept();
}

template<
	typename TTask,
	typename TState
>
void Server< TTask, TState >::stop()
{
    m_ioService.stop();
}

