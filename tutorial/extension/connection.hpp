#ifndef _MY_CONNECTION_HPP_
#define _MY_CONNECTION_HPP_

// These includes are required.
#include "./connection.hxx"
#include "./server.hpp"

// Implement all your functions here.
void MyServer::myFunction()
{
    // You can cast 'IServer*' to 'MyServer*' and use its interface.
    auto const myServer = dynamic_cast< MyServer * >( m_server );

    // Most of the time you have to just forward function call from
    // Connection to Server.
    //  
    // MyTask                   MyConnection               MyServer 
    //  |                        |                          | 
    //  ---{myFunction}--------->|                          | 
    //  |                        |---{myFunction}---------->| 
    //  |                        |                          | 

    broadcastServer->myFunction( shared_from_this() );
}

#endif

