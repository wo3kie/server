#ifndef _MY_CONNECTION_HPP_
#define _MY_CONNECTION_HPP_

// Extensions can extend your server functionality. You can find them  in 'server/ext'
// directory.
//
// Because your task can not call server's methods directly every time you extend a
// server you have to extend connection as well.

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

#include "../ext/.../connection.hpp"

// Create your Connection class here
struct MyConnection
    : public ConnectionExt

    // Create constructor here. No more method are required
    MyConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        // Because between Connection and ConnectionExt there is a virtual inheritance
        // you have to call Connection class constructor explicitly in addition to
        // ConnectionExt
        : Connection( ioService, server, task )
        , ConnectionExt( ioService, server, task )
    {
    }
};

#endif

