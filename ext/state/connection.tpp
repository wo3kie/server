#include "./connection.hpp"
#include "./server.hpp"

template<
    typename TState
>
TState & StateConnection< TState >::getState()
{
    auto const stateServer = dynamic_cast< StateServer< TState > * >( m_server );
    return stateServer->getState();
}

