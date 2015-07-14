#ifndef _ECHO_TASK_HPP_
#define _ECHO_TASK_HPP_

#include <iostream>
#include <sstream>
#include <string>

#include "core/iconnection.hpp"
#include "core/itask.hpp"

struct EchoTask
    : public ITask
{
    EchoTask()
    {
    }

    IConnection::Action getStartAction() const;

    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void parseError();

    void process();
};

inline
IConnection::Action EchoTask::getStartAction() const
{
    return IConnection::Action::Read;
}

inline
typename IConnection::Action EchoTask::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    m_connection->response( buffer, bytesTransferred );
    m_connection->read();

    return IConnection::Action::Process;
}

inline
void EchoTask::parseError()
{
    std::cerr << "Parse error" << std::endl;
}

inline
void EchoTask::process()
{
}

#endif

