#ifndef _CHAT_SERVER_HPP_
#define _CHAT_SERVER_HPP_

#include "../ext/unicast/server.hpp"
#include "../ext/log/server.hpp"
#include "../ext/broadcast/server.hpp"

#include "./task.hpp"
#include "./connection.hpp"

struct ChatServer
    : public UnicastServer
    , public LogServer
    , public BroadcastServer
{
    ChatServer( std::string const & port )
        : Server( port )
        , UnicastServer( port )
        , LogServer( port )
        , BroadcastServer( port )
    {
    }

    ITaskPtr createTask() override
    {
        return ITaskPtr( new ChatTask() );
    }

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new ChatConnection( m_ioService, this, createTask() ) );
    }
};

#endif

