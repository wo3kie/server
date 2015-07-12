#include "./connection.hpp"
#include "./server.hpp"

void LogConnection::log(
    char const * const message,
    std::size_t const size
)
{
    auto const logServer = dynamic_cast< LogServer * >( m_server );
    logServer->log( message, size );
}

