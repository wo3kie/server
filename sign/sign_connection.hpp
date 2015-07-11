#ifndef _SIGN_CONNECTION_HPP_
#define _SIGN_CONNECTION_HPP_

#include "../ext/state_connection.hpp"

template<
    typename TState
>
struct SignConnection
    : public StateConnection< TState >
{
    SignConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task )
        , StateConnection< TState >( ioService, server, task ) 
    {
    }
};
    
#endif

