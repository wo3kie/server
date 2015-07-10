#ifndef _UNICAST_SERVER_HPP_
#define _UNICAST_SERVER_HPP_

#include "./server.hpp"
#include "./unicast_connection.hpp"

class UnicastServer
    : public Server
{
public:
    UnicastServer( std::string const & port )
        : Server( port )
    {
    }

    void unicast(
        ConnectionPtr const & sender,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    UnicastConnection * createConnection() override
    {
        return new UnicastConnection( m_ioService, this, createTask() );
    }
};

void UnicastServer::unicast(
    ConnectionPtr const & sender,
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( ConnectionPtr const & connectionPtr )
    {
        return receiverId == dynamic_cast< UnicastConnection * >( connectionPtr.get() )->getId();
    };

    auto sendMessage = [ this, & sender, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        connectionPtr->response( message, size );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}


#endif

