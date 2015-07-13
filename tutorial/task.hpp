#ifndef _MY_TASK_HPP_
#define _MY_TASK_HPP_

// This include is required
#include "../core/itask.hpp"

// This include is required
#include "./connection.hpp"

// Create your Task class here
class MyTask
    : public ITask
{
public:

    // Most of the time default constructor is fine
    MyTask()
    {
    }

    // This function is called after a connection is established. It says to a Server
    // what a first action should be. You can pick between reading or writing.
    IConnection::Action getStartAction() const
    {
        // Return 'Read' if you want to read some data initially. 'parse' function will
        // be called as next.
        return IConnection::Action::Read;

        // It is not forbidden however it makes no sense here. If you return
        // 'ReadError' then 'parseError' function will be called as next.
        return IConnection::Action::ReadError;

        // Return 'Process' if you want to skip reading and write some data instead.
        // As a result 'process' function will be called as next.
        return IConnection::Action::Process;
    }

    // This function is called every time there are new incoming data
    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    )
    {
        // It may happen that a request is very big and will not be transported in one
        // chunk. If you still want to read some more data return 'Read'. As a result
        // 'parse' function will be called again with a new chunk of data.
        return IConnection::Action::Read;

        // If you find your data incorrect return 'ReadError'. As a result 'parseError'
        // function will be called next.
        return IConnection::Action::ReadError;

        // If you have read all your required data and everything is fine return
        // 'Process'. As a result 'process' function will be called next.
        return IConnection::Action::Process;
    }

    // This function is called everytime you have returned
    // 'IConnection::Action::ReadError'
    void parseError()
    {
    }

    // This function is called everytime you have returned 'IConnection::Action::Process'
    void process()
    {
        // This is good place to put your logic here
    
        // You can cast IConnection to your connection class and use its extended
        // interface
        auto const myConnection
            = dynamic_cast< MyConnection * >( m_connection );

        // At the end you can send some data back
        m_connection->response(
            "response",
            strlen( "response" )
        );

        // Unless you do not call 'read' again your connection will be closed
        m_connection->read();
    }
};

#endif

