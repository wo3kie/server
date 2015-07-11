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

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new Connection( m_ioService, this, createTask() ) );
    }

    virtual ITaskPtr createTask() = 0;

    void run() override;

    void disconnect(
        IConnectionPtr const & sender
    ) override;

#ifdef SERVER_SSL
    ssl::context & getSSLContext() override
    {
        return m_sslContext;
    }
#endif

private:

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

    IConnectionPtr m_newConnection;
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

void Server::run()
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

    m_sslContext.use_certificate_chain_file( "../pem/server.pem" );
    m_sslContext.use_private_key_file( "../pem/server-key.pem", ssl::context::pem );
#endif

    startAccept();

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
}

void Server::disconnect(
    IConnectionPtr const & sender
)
{
    m_connectionManager.remove( sender );
}

void Server::startAccept()
{
    m_newConnection = createConnection();

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

