#include "./connection.hpp"
#include "./server.hpp"

void BroadcastConnection::broadcast(
    char const * const message,
    std::size_t const size
)
{
    auto const broadcastServer = dynamic_cast< BroadcastServer * >( m_server );
    broadcastServer->broadcast( shared_from_this(), message, size );
}

