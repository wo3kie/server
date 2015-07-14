#ifndef _LOG_CONNECTION_HPP_
#define _LOG_CONNECTION_HPP_

#include "./connection.hpp"
#include "./server.hpp"

inline
void LogConnection::log(
    char const * const message,
    std::size_t const size
)
{
    dynamic_cast< LogServer * >( m_server )->log( message, size );
}

#endif

