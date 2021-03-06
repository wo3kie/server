#ifndef _CORE_ICONNECTION_HPP_
#define _CORE_ICONNECTION_HPP_

#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
    #include <boost/asio/ssl.hpp>
#endif

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
        Process
    };

    virtual ~IConnection(){}

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

    #ifdef SERVER_SSL
        virtual ssl::stream< asio::ip::tcp::socket > & socket() = 0;
    #else
        virtual ip::tcp::socket & socket() = 0;
    #endif
};

#endif

