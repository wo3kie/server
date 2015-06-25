#ifndef _TASK_DAY_TIME_HPP_
#define _TASK_DAY_TIME_HPP_

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include "./iconnection.hpp"

class TaskDayTime
{
public:
    TaskDayTime( IConnection * connection )
        : m_connection( connection )
    {
    }

    static IConnection::Action start()
    {
        return IConnection::Action::Process;
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

typename IConnection::Action TaskDayTime::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    return IConnection::Action::Process;
}

void TaskDayTime::parseError()
{
}

void TaskDayTime::process()
{
    std::time_t now = std::time( 0 );
    std::string const dayTime = ctime( & now );

    m_connection->response( dayTime.c_str(), dayTime.size() );
    m_connection->disconnect();
}

#endif

