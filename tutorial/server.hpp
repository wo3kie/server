#ifndef _MY_SERVER_HPP_
#define _MY_SERVER_HPP_

// Extensions can extend your server functionality. You can find them in 'server/ext'
// directory.
//
// Because your task can not call server's methods directly every time you extend
// a server you have to extend connection as well.

#include "../ext/.../server.hpp"

// These includes are required
#include "./task.hpp"
#include "./connection.hpp"

// Create your Server class here
struct MyServer
    : public LogServer
{
    // Create constructor here
    MyServer( std::string const & port )
        : Server( port )
        , LogServer( port )
    {
    }

    // You have to say to Server how to create your task
    ITaskPtr createTask() override
    {
        return ITaskPtr( new MyTask() );
    }

    // You have to say to Server how to create your connection
    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new MyConnection( m_ioService, this, createTask() ) );
    }
};

#endif

