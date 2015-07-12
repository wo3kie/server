#ifndef _UNICAST_CONNECTION_HPP_
#define _UNICAST_CONNECTION_HPP_

#include "./connection.hxx"
#include "./server.hpp"

void UnicastConnection::unicast(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const unicastServer = dynamic_cast< UnicastServer * >( m_server );
    unicastServer->unicast( shared_from_this(), receiverId, message, size );
}

#endif

