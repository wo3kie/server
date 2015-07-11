#ifndef _BROADCAST_SERVER_HPP_
#define _BROADCAST_SERVER_HPP_

#include "../core/server.hpp"

#include "./broadcast_connection.hpp"

class BroadcastServer
    : virtual public Server
{
public:
    BroadcastServer( std::string const & port )
        : Server( port )
    {
    }

    void broadcast(
        IConnectionPtr const & sender,
        char const * const message,
        std::size_t const size
    );

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new BroadcastConnection( m_ioService, this, createTask() ) );
    }
};

void BroadcastServer::broadcast(
    IConnectionPtr const & sender,
    char const * const message,
    std::size_t const size
)
{
    auto const skipSender = [ & sender ]( IConnectionPtr const & connectionPtr )
    {
        return sender->socket().native_handle() != connectionPtr->socket().native_handle();
    };

    auto sendMessage = [ this, & sender, & size, & message ]( IConnectionPtr const & connectionPtr )
    {
        connectionPtr->response( message, size );
    };

    m_connectionManager.forEachIf( skipSender, sendMessage );
}

#endif

