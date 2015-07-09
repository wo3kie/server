#ifndef _ISERVER_HPP_
#define _ISERVER_HPP_

class IConnection;

typedef boost::shared_ptr< IConnection > ConnectionPtr;

struct IServer
{
    virtual void broadcast(
        ConnectionPtr const & sender,
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void unicast(
        ConnectionPtr const & sender,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void disconnect(
        ConnectionPtr const & sender
    ) = 0;

#ifdef SERVER_SSL
    virtual ssl::context & getSSLContext() = 0;
#endif

    virtual void * getState() = 0;

    virtual IConnection * createConnection() = 0;
};

#endif

