#ifndef _UNICAST_CONNECTION_HXX_
#define _UNICAST_CONNECTION_HXX_

#include "../../core/connection.hpp"

class UnicastConnection
    : virtual public Connection
{
public:

    UnicastConnection(
        asio::io_service & ioService,
        IServer * server,
        ITaskPtr task
    )
        : Connection( ioService, server, task ) 
    {
    }

    void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void setId(
        std::string const & id
    )
    {
        m_id = id;
    }

    std::string const & getId() const
    {
        return m_id;
    }

    std::string m_id;
};

#endif

