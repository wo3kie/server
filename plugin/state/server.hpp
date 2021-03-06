#ifndef _STATE_SERVER_HPP_
#define _STATE_SERVER_HPP_

#include "../../core/server.hpp"

#include "./connection.hxx"

template<
    typename TState
>
class StateServer
    : virtual public Server
{
public:
    StateServer( std::string const & port )
        : Server( port )
    {
    }

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new StateConnection< TState >( m_ioService, this, createTask() ) );
    }

    TState & getState(){ return m_state; }

protected:

    TState m_state;
};

#endif

