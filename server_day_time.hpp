#ifndef _TASK_DAY_TIME_HPP_
#define _TASK_DAY_TIME_HPP_

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include "./myconnection.hpp"

class TaskDayTime
{
public:
    TaskDayTime( MyConnection * connection )
        : m_connection( connection )
    {
    }

    static MyConnection::Action start()
    {
        return MyConnection::Action::Process;
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

typename MyConnection::Action TaskDayTime::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    return MyConnection::Action::Process;
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

