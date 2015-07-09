#ifndef _ECHO_TASK_HPP_
#define _ECHO_TASK_HPP_

#include <iostream>
#include <sstream>
#include <string>

#include "./myconnection.hpp"

class EchoTask
{
public:
    EchoTask( MyConnection * connection )
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

typename MyConnection::Action EchoTask::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    m_connection->response( buffer, bytesTransferred );
    m_connection->read();

    return MyConnection::Action::Process;
}

void EchoTask::parseError()
{
    std::cerr
        << "Parse error"
        << std::endl;
}

void EchoTask::process()
{
}

#endif

