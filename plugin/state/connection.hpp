#ifndef _STATE_CONNECTION_HPP_
#define _STATE_CONNECTION_HPP_

#include "./connection.hpp"
#include "./server.hpp"

template<
    typename TState
>
TState & StateConnection< TState >::getState()
{
    return dynamic_cast< StateServer< TState > * >( m_server )->getState();
}

#endif

