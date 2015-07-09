#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "./connection.hpp"
#include "./connection_manager.hpp"
#include "./iserver.hpp"
#include "./iconnection.hpp"

template< typename TState >
struct State
{
    void stateInit( int argc, char* argv[] ){ m_state.init( argc, argv ); }
    void stateDestroy(){ m_state.destroy(); }

    void * getState() { return & m_state; }
    void const * getState() const { return & m_state; }

private:
    TState m_state;
};

template<>
struct State< void >
{
    void stateInit( int argc, char* argv[] ){}
    void stateDestroy(){}

    void * getState() { return nullptr; }
    void const * getState() const { return nullptr; }
};

template<
    typename TTask,
    typename TState = void
>
class Server
    : public IServer
    , private State< TState >
{
public:
    
    using State< TState >::stateInit;
    using State< TState >::stateDestroy;
    using State< TState >::getState;

public:

    Server(
        std::string const & port
    );

    IConnection * createConnection() override
    {
        return new Connection< TTask >( m_ioService, *this );
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

    void unicast(
        ConnectionPtr const & sender,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void disconnect(
        ConnectionPtr const & sender
    );

    void * getState() override
    {
        return State< TState >::getState();
    }

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

private:

    asio::io_service m_ioService;
    asio::io_service::strand m_strand;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

#ifdef SERVER_SSL
    ssl::context m_sslContext;
#endif

    ConnectionPtr m_newConnection;
    ConnectionManager< TTask > m_connectionManager;
};

#include "./server.tpp"
#include "./connection.tpp"

#endif

