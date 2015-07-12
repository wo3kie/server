#ifndef _ECHO_SERVER_HPP_
#define _ECHO_SERVER_HPP_

#include "./task.hpp"
#include "../core/connection.hpp"

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
};

#endif

