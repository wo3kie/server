#ifndef _CORE_ITASK_HPP_
#define _CORE_ITASK_HPP_

class IConnection;

struct ITask
{
    virtual IConnection::Action getStartAction() const = 0;

    virtual IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    ) = 0;

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

inline
ITask::~ITask()
{
}

inline
void ITask::setConnection(
    IConnection * connection
)
{
    m_connection = connection;
}

inline
IConnection * ITask::getConnection()
{
    return m_connection;
}

#endif

