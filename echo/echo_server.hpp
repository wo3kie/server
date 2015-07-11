#ifndef _ECHO_SERVER_HPP_
#define _ECHO_SERVER_HPP_

#include "./echo_task.hpp"
#include "./echo_connection.hpp"

struct EchoServer
    : public Server
{
    EchoServer( std::string const & port )
        : Server( port )
    {
    }

    ITaskPtr createTask() override
    {
        return ITaskPtr( new EchoTask() );
    }

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new EchoConnection( m_ioService, this, createTask() ) );
    }
};

#endif

