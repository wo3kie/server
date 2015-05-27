#ifndef _ICONNECTION_HPP
#define _ICONNECTION_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

class IConnection
{
public:

    enum class Action
    {
        Read,
        ReadError,
        Process
    };

    virtual ip::tcp::socket & socket() = 0;

    virtual void disconnect() = 0;

    virtual void stop() = 0;

    virtual void start(
        Action const action
    ) = 0;

public: // api

    virtual void setId(
        std::string const & id
    ) = 0;

    virtual std::string const & getId() const = 0;

    virtual void read() = 0;

    virtual void parseError() = 0;

    virtual void process() = 0;

    virtual void response(
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void broadcast(
        char const * const message,
        std::size_t const size
    ) = 0;

    virtual void log(
        char const * const message,
        std::size_t const size 
    ) = 0;

    virtual void doNothing(
        sys::error_code const & errorCode
    ) = 0;
};

#endif

