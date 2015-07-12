#ifndef _LOG_CONNECTION_HPP_
#define _LOG_CONNECTION_HPP_

#include "../../core/connection.hpp"

class LogConnection
    : virtual public Connection
{
public:

    LogConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task ) 
    {
    }

    void log(
        char const * const message,
        std::size_t const size
    );
};

#endif

