#ifndef _ICONNECTION_HPP_
#define _ICONNECTION_HPP_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;
#ifdef SERVER_SSL
namespace ssl = asio::ssl;
#endif

class IServer;

struct IConnection
{
    enum class Action
    {
        Read,
        ReadError,
        Process
    };

    virtual ~IConnection(){}

#ifdef SERVER_SSL
    virtual ssl::stream< asio::ip::tcp::socket > & socket() = 0;
#else
    virtual ip::tcp::socket & socket() = 0;
#endif

    virtual Action getStartAction() const = 0;

    virtual void start(
        Action const action
    ) = 0;

    virtual void disconnect() = 0;

    virtual void read() = 0;

    virtual void response(
        char const * const message,
        std::size_t const size
    ) = 0;
};

#endif

