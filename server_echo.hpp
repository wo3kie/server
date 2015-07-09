#ifndef _TASK_ECHO_HPP_
#define _TASK_ECHO_HPP_

#include <iostream>
#include <sstream>
#include <string>

#include "./myconnection.hpp"

class TaskEcho
{
public:
    TaskEcho( MyConnection * connection )
        : m_connection( connection )
    {
    }

    static MyConnection::Action start()
    {
        return MyConnection::Action::Read;
    }

    MyConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void parseError();

    void process();

private:
    MyConnection * m_connection;
};

typename MyConnection::Action TaskEcho::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    m_connection->response( buffer, bytesTransferred );
    m_connection->read();

    return MyConnection::Action::Process;
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

