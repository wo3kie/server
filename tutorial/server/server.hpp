#ifndef _MY_SERVER_HPP_
#define _MY_SERVER_HPP_

// Extensions can improve your server functionality. You can find them in 'server/ext'
// directory.
//
// Because your task can not call server's methods directly every time you extend
// a server you have to extend a connection as well.

//
//        ------{uses}------           ------{uses}------
//       v                  \         v                  \
// IServer <>--{contains}-- IConnection <>--{contains}--ITask
//    ^                          ^                        ^
//    |                          |                        |
// Server                   Connection                    |
//    ^                          ^                        |
//    |                          |                        |
// ServerExt                ConnectionExt                 |
// doIt()                   doIt()                        |
//    ^                          ^                        |
//    |                          |                        |
// MyServer                 MyConnection               MyTask
//

//
// MyTask                   MyConnection               MyServer
//  |                        |                          |
//  ---{doIt}--------------->|                          |
//  |                        |---{doIt}---------------->|
//  |                        |                          |
//

#include "../ext/.../server.hpp"

// These includes are required.
#include "./task.hpp"
#include "./connection.hpp"

// Create your Server class here.
struct MyServer
    : public LogServer
{
    // Create constructor here.
    MyServer( std::string const & port )
        : Server( port )
        , LogServer( port )
    {
    }

    // You have to say to Server how to create your task.
    ITaskPtr createTask() override
    {
        return ITaskPtr( new MyTask() );
    }

    // You have to say to Server how to create your connection.
    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new MyConnection( m_ioService, this, createTask() ) );
    }
};

#endif

