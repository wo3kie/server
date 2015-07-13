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

    // This function is called at the beginnig of processing. It says to a Server what
    // first action should be. Do you want to read some data at the beginning, or you
    // want to write something immidiately after a connection is established?
    IConnection::Action getStartAction() const
    {
        // If you want read some data at the beginning return 'Read'.
        // The next function call will be 'parse'
        return IConnection::Action::Read;

        // It is not forbidden however it makes no sense here.
        // If returned the next function call will be 'parseError'
        return IConnection::Action::ReadError;

        // If you do not want to read any data return 'Process'.
        // The next function call will be 'process'
        return IConnection::Action::Process;
    }

    // This function is called every time there are new incoming data to a server
    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    )
    {
        // If you want read more data return 'Read'. It may happen that request
        // is very huge and will not be transported in one chunk. If returned
        // the next function call will be 'parse' again with a new chunk of data
        return IConnection::Action::Read;

        // If you find data incorrect return 'ReadError'. If returned the next
        // function call will be 'parseError'
        return IConnection::Action::ReadError;

        // If you read all required data and everything is fine return 'Process'.
        // If returned the next function call will be 'process'
        return IConnection::Action::Process;
    }

    // This function is called everytime you return 'IConnection::Action::ReadError'
    void parseError()
    {
    }

    // This function is called everytime you return 'IConnection::Action::Process'
    void process()
    {
        // This is good place to put your logic here
    
        // You can cast IConnection to your connection class and use its extended
        // interface IConnection* -> MyConnection*
        auto const myConnection
            = dynamic_cast< MyConnection * >( m_connection );

        // At the end you can send some data back
        m_connection->response(
            "response",
            strlen( "response" )
        );

        // Unless you do not call read again your connection will be closed
        m_connection->read();
    }
};

#endif

