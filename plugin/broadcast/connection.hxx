#ifndef _BROADCAST_CONNECTION_HXX_
#define _BROADCAST_CONNECTION_HXX_

#include "../../core/connection.hpp"

class BroadcastConnection
    : virtual public Connection
{
public:

    BroadcastConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task ) 
    {
    }

    void broadcast(
        char const * const message,
        std::size_t const size
    );
};

#endif

