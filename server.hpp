#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include "./connection.hpp"
#include "./connection_manager.hpp"
#include "./iserver.hpp"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

class Server
    : public IServer
{
public:

    Server(
        std::string const & port
    );

    Connection * createConnection() override
    {
        return new Connection( m_ioService, this, createTask() );
    }

    void run();

    void run(
        int argc,
        char* argv[]
    );

    void broadcast(
        ConnectionPtr const & sender,
        char const * const message,
        std::size_t const size
    );

    void disconnect(
        ConnectionPtr const & sender
    );

#ifdef SERVER_SSL
    ssl::context & getSSLContext() override
    {
        return m_sslContext;
    }
#endif

private:

    void init( int argc, char* argv[] );
    void destroy();

    void startAccept();

    void onAccepted(
        sys::error_code const & errorCode
    );

    void stop();

protected:
    std::string m_port;

    asio::io_service m_ioService;
    asio::io_service::strand m_strand;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

#ifdef SERVER_SSL
    ssl::context m_sslContext;
#endif

    ConnectionPtr m_newConnection;
    ConnectionManager m_connectionManager;
};

Server::Server(
    std::string const & port
)
    : m_port( port )
    , m_strand( m_ioService )
    , m_acceptor( m_ioService )
    , m_signals( m_ioService )

#ifdef SERVER_SSL
    , m_sslContext( ssl::context::sslv23 )
#endif

{
}

void Server::init( int argc, char* argv[] )
{
}

void Server::destroy()
{
}

void Server::run()
{
    run( 0, {} );
}

void Server::run(
    int argc,
    char* argv[]
)
{
    m_signals.add( SIGINT );
    m_signals.add( SIGTERM );
    m_signals.add( SIGQUIT );

    m_signals.async_wait(
        boost::bind( & Server::stop, this )
    );

    ip::tcp::resolver resolver( m_ioService );
    ip::tcp::resolver::query query( "127.0.0.1", m_port );
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

void Server::broadcast(
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

void Server::disconnect(
    ConnectionPtr const & sender
)
{
    m_connectionManager.remove( sender );
}

void Server::startAccept()
{
    m_newConnection.reset( createConnection() );

    auto const onAccepted = boost::bind(
        & Server::onAccepted,
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

void Server::onAccepted(
    sys::error_code const & errorCode
)
{
    if( ! errorCode )
    {
        m_connectionManager.add( m_newConnection );

        m_newConnection->start( m_newConnection->getStartAction() );
    }

    startAccept();
}

void Server::stop()
{
    m_ioService.stop();
}


#endif

