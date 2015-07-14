#ifndef _MY_CONNECTION_HXX_
#define _MY_CONNECTION_HXX_

// This include is required.
#include "../../core/connection.hpp"

// Define your connection class here.
class MyConnection
    : virtual public Connection
{
public:

    // Such a simple constructor in most cases is fine.
    MyConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task ) 
    {
    }

    // Define all your functions here.
    void myFunction();
};

#endif

