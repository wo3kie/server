#ifndef _LOG_SERVER_HPP_
#define _LOG_SERVER_HPP_

#include "../../core/server.hpp"

#include "./connection.hxx"

class LogServer
    : virtual public Server
{
public:
    LogServer( std::string const & port )
        : Server( port )
    {
    }

    void log(
        char const * const message,
        std::size_t const size
    );

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new LogConnection( m_ioService, this, createTask() ) );
    }
};

void LogServer::log(
    char const * const message,
    std::size_t const size
)
{
    std::cout.write( message, size );
    std::flush( std::cout );
}

#endif

