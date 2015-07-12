#ifndef _ITASK_HPP_
#define _ITASK_HPP_

class IConnection;

struct ITask
{
public:

    virtual IConnection::Action getStartAction() const = 0;

    virtual IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    ) = 0;

    virtual void parseError() = 0;

    virtual void process() = 0;

public:

    virtual ~ITask();

    virtual void setConnection(
        IConnection * connection
    );

    virtual IConnection * getConnection();

protected:

    IConnection * m_connection;
};

ITask::~ITask()
{
}

void ITask::setConnection(
    IConnection * connection
)
{
    m_connection = connection;
}

IConnection * ITask::getConnection()
{
    return m_connection;
}

#endif

