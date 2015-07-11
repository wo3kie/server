#ifndef _UNICAST_SERVER_HPP_
#define _UNICAST_SERVER_HPP_

#include "../core/server.hpp"

#include "./unicast_connection.hpp"

class UnicastServer
    : virtual public Server
{
public:
    UnicastServer( std::string const & port )
        : Server( port )
    {
    }

    void unicast(
        IConnectionPtr const & sender,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new UnicastConnection( m_ioService, this, createTask() ) );
    }
};

void UnicastServer::unicast(
    IConnectionPtr const & sender,
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( IConnectionPtr const & connectionPtr )
    {
        return receiverId == dynamic_cast< UnicastConnection * >( connectionPtr.get() )->getId();
    };

    auto sendMessage = [ this, & sender, & size, & message ]( IConnectionPtr const & connectionPtr )
    {
        connectionPtr->response( message, size );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}

#endif

