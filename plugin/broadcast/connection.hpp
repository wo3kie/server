#ifndef _BROADCAST_CONNECTION_HPP_
#define _BROADCAST_CONNECTION_HPP_

#include "./connection.hxx"
#include "./server.hpp"

inline
void BroadcastConnection::broadcast(
    char const * const message,
    std::size_t const size
)
{
    return dynamic_cast< BroadcastServer * >( m_server )->broadcast(
        shared_from_this(),
        message,
        size
    );
}

#endif

