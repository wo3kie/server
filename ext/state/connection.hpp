#ifndef _STATE_CONNECTION_HPP_
#define _STATE_CONNECTION_HPP_

#include "../../core/connection.hpp"

template<
    typename TState
>
class StateConnection
    : virtual public Connection
{
public:

    StateConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task ) 
    {
    }

    TState & getState();
};

#endif

