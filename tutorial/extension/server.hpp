#ifndef _MY_SERVER_HPP_
#define _MY_SERVER_HPP_

// This include is required.
#include "../../core/server.hpp"

// This include is required.
#include "./connection.hxx"

class MyServer
    : virtual public Server
{
public:

    // Such a simple constructor is enough.
    MyServer( std::string const & port )
        : Server( port )
    {
    }

    // Let your server know about your connection.
    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new MyConnection( m_ioService, this, createTask() ) );
    }

    // Define all your required functions here.
    void myFunction()
    {
    }

};

#endif

