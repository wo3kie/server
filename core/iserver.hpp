#ifndef _CORE_ISERVER_HPP_
#define _CORE_ISERVER_HPP_

#include <boost/shared_ptr.hpp>

class IConnection;
class ITask;

typedef boost::shared_ptr< IConnection > IConnectionPtr;
typedef boost::shared_ptr< ITask > ITaskPtr;

struct IServer
{
    virtual ~IServer(){}

    virtual IConnectionPtr createConnection() = 0;

    virtual ITaskPtr createTask() = 0;

    virtual void run() = 0;

    virtual void disconnect(
        IConnectionPtr const & sender
    ) = 0;

    #ifdef SERVER_SSL
        virtual ssl::context & getSSLContext() = 0;
    #endif
};

#endif

