#ifndef _MY_CONNECTION_HPP_
#define _MY_CONNECTION_HPP_

// Plugins can extend your server functionality. You can find them in
// 'server/plugin' directory.
//
// Because your task can not call server's methods directly every time
// you extend a server you have to extend a connection as well.

//
//        ------{uses}------           ------{uses}------
//       v                  \         v                  \
// IServer <>--{contains}-- IConnection <>--{contains}--ITask
//    ^                          ^                        ^
//    |                          |                        |
// Server                   Connection                    |
//    ^                          ^                        |
//    |                          |                        |
// ServerPlugin             ConnectionPlugin              |
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

#include "../plugin/.../connection.hpp"

// Create your Connection class here.
struct MyConnection
    : public ConnectionPlugin

    // Create constructor here. No more method are required.
    MyConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        // Because between Connection and ConnectionPlugin there is a virtual
        // inheritance you have to call Connection class constructor explicitly
        // in addition to ConnectionPlugin.
        : Connection( ioService, server, task )
        , ConnectionPlugin( ioService, server, task )
    {
    }
};

#endif

