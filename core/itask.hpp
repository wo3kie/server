#ifndef _ITASK_HPP_
#define _ITASK_HPP_

class IConnection;

struct ITask
{
    virtual ~ITask(){}

    virtual IConnection::Action getStartAction() const = 0;

    virtual IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    ) = 0;

    virtual void parseError() = 0;

    virtual void process() = 0;

    virtual void setConnection(
        IConnection * connection
    ) = 0;
};

#endif

