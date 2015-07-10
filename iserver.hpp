#ifndef _ISERVER_HPP_
#define _ISERVER_HPP_

class IConnection;
class ITask;

typedef boost::shared_ptr< IConnection > ConnectionPtr;

struct IServer
{
    virtual void broadcast(
        ConnectionPtr const & sender,
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void disconnect(
        ConnectionPtr const & sender
    ) = 0;

#ifdef SERVER_SSL
    virtual ssl::context & getSSLContext() = 0;
#endif

    virtual IConnection * createConnection() = 0;

    virtual ITask * createTask() = 0;
};

#endif

