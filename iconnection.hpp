#ifndef _ICONNECTION_HPP_
#define _ICONNECTION_HPP_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;

#ifdef SERVER_SSL
namespace ssl = asio::ssl;
#endif

namespace sys = boost::system;

class IServer;

struct IConnection
{
    enum class Action
    {
        Read,
        ReadError,
        Process
    };

    virtual
#ifdef SERVER_SSL
    ssl::stream< asio::ip::tcp::socket > &
#else
    ip::tcp::socket &
#endif
    socket() = 0;

    virtual void read() = 0;

    virtual void broadcast(
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void log(
        char const * const message,
        std::size_t const size 
    ) = 0;

    virtual void disconnect() = 0;

    virtual void start(
        Action const action
    ) = 0;

    virtual void response(
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual Action getStartAction() const = 0;
};

#endif

