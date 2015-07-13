#ifndef _CHAT_TASK_HPP_
#define _CHAT_TASK_HPP_

#include <iostream>
#include <sstream>
#include <string>

#include "core/itask.hpp"

#include "./connection.hpp"

class ChatTask
    : public ITask
{
public:

    ChatTask();

    IConnection::Action getStartAction() const;

    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void parseError();

    void process();
};

ChatTask::ChatTask()
{
}

IConnection::Action ChatTask::getStartAction() const
{
    return IConnection::Action::Read;
}

typename IConnection::Action ChatTask::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    std::istringstream iss( std::string( buffer, bytesTransferred ) );

    char command, space, cr, lf;
    iss >> command;

    if( command == 'h' )
    {
        std::string id;
        iss >> id;
        dynamic_cast< ChatConnection * >( m_connection )->setId( id );

        const std::string message = "Hello '" + id + "'.";

        m_connection->response( message.c_str(), message.size() );
        m_connection->read();
    }
    else if( command == 'b' )
    {
        std::string message;
        iss >> message;

        dynamic_cast< ChatConnection * >( m_connection )->broadcast( message.c_str(), message.size() );
        m_connection->read();
    }
    else if( command == 'u' )
    {
        std::string id;
        std::string message;
        iss >> id >> message;

        dynamic_cast< ChatConnection * >( m_connection )->unicast( id, message.c_str(), message.size() );
        m_connection->read();
    }
    else if( command == 'd' )
    {
        m_connection->disconnect();
    }
    else if( command == 'l' )
    {
        std::string id;
        std::string message;
        iss >> message;

        auto const logConnection  = dynamic_cast< LogConnection * >( m_connection );
        logConnection->log( message.c_str(), message.size() );

        m_connection->read();
    }
    else if( command == 'e' )
    {
        std::string message;
        iss >> message;

        m_connection->response( message.c_str(), message.size() );
        m_connection->read();
    }
    else
    {
        std::string const message = "Unknown command '" + std::string( 1, command ) + "'.";

        m_connection->response( message.c_str(), message.size() );
        m_connection->read();
    }

    return IConnection::Action::Process;
}

void ChatTask::parseError()
{
    std::cerr
        << "Parse error"
        << std::endl;
}

void ChatTask::process()
{
}

#endif

