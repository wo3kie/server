#ifndef _CORE_SERVER_HPP_
#define _CORE_SERVER_HPP_

#include <string>

#include <boost/asio.hpp>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include "./connection.hpp"
#include "./connection_manager.hpp"
#include "./iserver.hpp"

class Server
    : public IServer
{
public:

    Server(
        std::string const & port
    );

    void run() override;

    void disconnect(
        IConnectionPtr const & sender
    ) override;

    #ifdef SERVER_SSL
        ssl::context & getSSLContext() override;
    #endif

protected:

    virtual void setupSignals();

    virtual std::string getLocalHostIp();

    virtual std::string getPort();

    virtual ip::tcp::endpoint resolve();

    virtual bool reuseAddress();

    virtual void runThreadPool();

    virtual unsigned getThreadNumber();

    virtual void startAccept();

    virtual void onAccepted(
        sys::error_code const & errorCode
    );

    IConnectionPtr createConnection() override;

    virtual ITaskPtr createTask() = 0;

    virtual void stop();

    #ifdef SERVER_SSL

        virtual void sslInit();

        virtual int getSslOptions();
        
        virtual std::string passwordCallback(
            std::size_t size,
            std::size_t purppose
        );
        
        virtual std::string getCertificateChainFileName();
        
        virtual std::string getPrivateKeyFileName();

    #endif

protected:

    std::string m_port;
    asio::io_service m_ioService;
    asio::io_service::strand m_strand;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;
    IConnectionPtr m_newConnection;
    ConnectionManager m_connectionManager;

    #ifdef SERVER_SSL
        ssl::context m_sslContext;
    #endif

};

inline
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

inline
void Server::setupSignals()
{
    m_signals.add( SIGINT );
    m_signals.add( SIGTERM );
    m_signals.add( SIGQUIT );

    auto const onSignal = [ this ](
        const sys::error_code& error,
        int signal_number
    )
    {
        this->stop();
    };

    m_signals.async_wait( onSignal );
}

inline
std::string Server::getLocalHostIp()
{
    return "127.0.0.1";
}

inline
std::string Server::getPort()
{
    return m_port;
}

inline
bool Server::reuseAddress()
{
    return true;
}

inline
IConnectionPtr Server::createConnection()
{
    return IConnectionPtr( new Connection( m_ioService, this, createTask() ) );
}

inline
ip::tcp::endpoint Server::resolve() 
{
    ip::tcp::resolver resolver( m_ioService );
    ip::tcp::resolver::query query( getLocalHostIp(), getPort() );
    return * resolver.resolve( query );
}

inline
unsigned Server::getThreadNumber() 
{
    return boost::thread::hardware_concurrency();
}

inline
void Server::runThreadPool()
{
    boost::thread_group threadGroup; 

    auto const threadBody = [ this ]()
    {
        this->m_ioService.run();
    };

    for(
        unsigned i = 0, threadNumber = getThreadNumber();
        i < threadNumber;
        ++ i
    )
    {
        threadGroup.create_thread( threadBody );
    }

    threadGroup.join_all();
}

inline
void Server::run()
{
    setupSignals();

    auto const endpoint = resolve();

    m_acceptor.open( endpoint.protocol() );
    m_acceptor.set_option( ip::tcp::acceptor::reuse_address( reuseAddress() ) );
    m_acceptor.bind( endpoint );
    m_acceptor.listen();

    #ifdef SERVER_SSL
        sslInit();
    #endif

    startAccept();

    runThreadPool();
}

inline
void Server::disconnect(
    IConnectionPtr const & connection
)
{
    m_connectionManager.remove( connection );
}

inline
void Server::startAccept()
{
    m_newConnection = createConnection();

    auto const onAccepted = [ this ](
        sys::error_code const & errorCode
    )
    {
        this->onAccepted( errorCode );
    };
   
    m_acceptor.async_accept(
        
        #ifdef SERVER_SSL
            m_newConnection->socket().lowest_layer(),
        #else
            m_newConnection->socket(),
        #endif
        
        onAccepted
    );
}

inline
void Server::onAccepted(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "On Accepted Error: " << errorCode.message() << std::endl;
    }
    else
    {
        m_connectionManager.add( m_newConnection );
        m_newConnection->start( m_newConnection->getStartAction() );
    }

    startAccept();
}

inline
void Server::stop()
{
    m_ioService.stop();
}

#ifdef SERVER_SSL

    inline
    ssl::context & Server::getSSLContext()
    {
        return m_sslContext;
    }

    inline
    int Server::getSslOptions()
    {
        return ssl::context::default_workarounds
            | ssl::context::no_sslv2
            | ssl::context::single_dh_use;
    }

    inline
    std::string Server::passwordCallback(
        std::size_t size,
        std::size_t purppose
    )
    {
        return "test";
    }

    inline
    std::string Server::getCertificateChainFileName()
    {
        return "../pem/server.pem";
    }
        
    inline
    std::string Server::getPrivateKeyFileName()
    {
        return "../pem/server-key.pem";
    }

    inline
    void Server::sslInit()
    {
        m_sslContext.set_options( getSslOptions() );

        m_sslContext.set_password_callback(
            [ & ]( std::size_t size, std::size_t purpose )
            {
                return this->passwordCallback( size, purpose );
            }
        );

        m_sslContext.use_certificate_chain_file( getCertificateChainFileName() );
        m_sslContext.use_private_key_file( getPrivateKeyFileName(), ssl::context::pem );
    }

#endif // SERVER_SSL

#endif // _SERVER_HPP_

