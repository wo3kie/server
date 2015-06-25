#ifndef _TASK_ECHO_HPP_
#define _TASK_ECHO_HPP_

#include <iostream>
#include <sstream>
#include <string>

#include "./iconnection.hpp"

class TaskEcho
{
public:
    TaskEcho( IConnection * connection )
        : m_connection( connection )
    {
    }

    static IConnection::Action start()
    {
        return IConnection::Action::Read;
    }

    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void parseError();

    void process();

private:
    IConnection * m_connection;
};

typename IConnection::Action TaskEcho::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    m_connection->response( buffer, bytesTransferred );
    m_connection->read();

    return IConnection::Action::Process;
}

void TaskEcho::parseError()
{
    std::cerr
        << "Parse error"
        << std::endl;
}

void TaskEcho::process()
{
}

#endif

