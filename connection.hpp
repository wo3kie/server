#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
#include <boost/asio/ssl.hpp>
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "./iconnection.hpp"
#include "./iserver.hpp"
#include "./itask.hpp"

class Connection
    : public IConnection
    , public boost::enable_shared_from_this< Connection >
{
public:

    Connection( 
        asio::io_service & ioService,
        IServer * server,
        ITask * task
    );

#ifdef SERVER_SSL
    ssl::stream< asio::ip::tcp::socket > &
#else
    ip::tcp::socket &
#endif
    socket();

    void start(
        Action const action
    );

    void restart(
        Action const action
    );

    void stop();

    void parseError();

    void process();

    void doNothing(
        sys::error_code const & errorCode
    );

    void disconnect();

public: // api

    void setId(
        std::string const & id
    ) override;

    std::string const & getId() const override;

    void read() override;

    void response(
        char const * const message,
        std::size_t const size
    ) override;

    void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    ) override;

    void broadcast(
        char const * const message,
        std::size_t const size
    ) override;

    void log(
        char const * const message,
        std::size_t const size 
    ) override;

    void const * getState() const override
    {
        return m_server->getState();
    }

    MyConnection::Action getStartAction() const
    {
        return m_task->getStartAction();
    }

private:

    void parse(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void restartAgain(
        sys::error_code const & errorCode,
        MyConnection::Action const action
    );

private:

#ifdef SERVER_SSL
    ssl::stream< ip::tcp::socket > m_socket;
#else
    ip::tcp::socket m_socket;
#endif

    IServer * m_server;
    ITask * m_task;

    std::string m_id;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];
};

#endif

