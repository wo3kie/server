#ifndef _ICONNECTION_HPP_
#define _ICONNECTION_HPP_

#include "./myconnection.hpp"

struct IConnection
    : public MyConnection
{
    virtual
#ifdef SERVER_SSL
    ssl::stream< asio::ip::tcp::socket > &
#else
    ip::tcp::socket &
#endif
    socket() = 0;

    virtual void start(
        Action const action
    ) = 0;

    virtual void setId(
        std::string const & id
    ) = 0;

    virtual std::string const & getId() const = 0;

    virtual void response(
        char const * const message,
        std::size_t const size
    ) = 0;
};

#endif

