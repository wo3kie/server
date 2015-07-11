#ifndef _CHAT_CONNECTION_HPP_
#define _CHAT_CONNECTION_HPP_

#include "../ext/unicast_connection.hpp"
#include "../ext/broadcast_connection.hpp"
#include "../ext/log_connection.hpp"

struct ChatConnection
    : public UnicastConnection
    , public LogConnection
    , public BroadcastConnection
{
    ChatConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task )
        , UnicastConnection( ioService, server, task ) 
        , LogConnection( ioService, server, task )
        , BroadcastConnection( ioService, server, task )
    {
    }
};
    

#endif

