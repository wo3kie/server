#ifndef _ECHO_CONNECTION_HPP_
#define _ECHO_CONNECTION_HPP_

struct EchoConnection
    : public Connection
{
    EchoConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task )
    {
    }
};
    

#endif

