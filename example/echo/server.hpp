#ifndef _ECHO_SERVER_HPP_
#define _ECHO_SERVER_HPP_

#include "core/server.hpp"

#include "./task.hpp"

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

